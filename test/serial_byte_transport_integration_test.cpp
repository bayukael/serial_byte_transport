#include <atomic>
#include <byte_transport/ByteTransportFactory.h>
#include <byte_transport/IByteTransport.h>
#include <chrono>
#include <dlfcn.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <thread>
#include <vector>

const std::string g_path_pty_1("/tmp/pty1");
const std::string g_path_pty_2("/tmp/pty2");

using ByteTransportFactory = pendarlab::lib::comm::ByteTransportFactory;

namespace
{
  struct ReaderConfig {
    std::shared_ptr<pendarlab::lib::comm::IByteTransport> bt;
    unsigned int buf_size;
    unsigned int read_delay_us;
    size_t expected_size;
    unsigned int timeout_in_ms;
  };
  class SerialByteTransportReader
  {
  public:
    SerialByteTransportReader(const ReaderConfig& c);
    static void routine(SerialByteTransportReader&);
    std::vector<uint8_t> data();
    void stop_reading();

  private:
    ReaderConfig cfg_;
    std::vector<uint8_t> data_read_;
    std::atomic<bool> keep_reading_;
  };

  SerialByteTransportReader::SerialByteTransportReader(const ReaderConfig& c) : cfg_(c), keep_reading_(true)
  {
  }

  void SerialByteTransportReader::routine(SerialByteTransportReader& reader)
  {
    auto tp_now = std::chrono::steady_clock::now();
    auto timeout = tp_now + std::chrono::milliseconds(reader.cfg_.timeout_in_ms);
    while (reader.keep_reading_ || tp_now > timeout) {

      uint8_t buf[reader.cfg_.buf_size];
      int bytes_read = reader.cfg_.bt->read(buf, reader.cfg_.buf_size);
      for (size_t i = 0; i < bytes_read; i++) {
        reader.data_read_.push_back(buf[i]);
      }
      if(reader.data_read_.size() >= reader.cfg_.expected_size){
        reader.keep_reading_ = false;
      }
      std::this_thread::sleep_for(std::chrono::microseconds(reader.cfg_.read_delay_us));
      tp_now = std::chrono::steady_clock::now();
    }
  }

  std::vector<uint8_t> SerialByteTransportReader::data()
  {
    return data_read_;
  }

  void SerialByteTransportReader::stop_reading()
  {
    keep_reading_ = false;
  }

  struct WriterConfig {
    std::shared_ptr<pendarlab::lib::comm::IByteTransport> bt;
    std::vector<uint8_t> data_to_send;
    unsigned int num_of_bytes_on_each_write;
    int write_delay_us;
  };
  class SerialByteTransportWriter
  {
  public:
    SerialByteTransportWriter(const WriterConfig&);
    static void routine(SerialByteTransportWriter&);

  private:
    WriterConfig cfg_;
  };

  SerialByteTransportWriter::SerialByteTransportWriter(const WriterConfig& c) : cfg_(c)
  {
  }

  void SerialByteTransportWriter::routine(SerialByteTransportWriter& writer)
  {
    std::vector<uint8_t> bytes;
    for (size_t i = 0; i < writer.cfg_.data_to_send.size(); i++) {
      bytes.push_back(writer.cfg_.data_to_send[i]);
      if (bytes.size() == writer.cfg_.num_of_bytes_on_each_write || i == writer.cfg_.data_to_send.size()-1 ) {
        writer.cfg_.bt->write(&bytes[0], bytes.size());
        bytes.clear();
        std::this_thread::sleep_for(std::chrono::microseconds(writer.cfg_.write_delay_us));
      }
    }
  }

  class SerialByteTransportIntegrationTest : public testing::Test
  {
  protected:
    SerialByteTransportIntegrationTest() {}
    void SetUp() override
    {
      // SERIAL_BYTE_TRANSPORT_LIB_PATH is defined in CMakeLists.txt
      plugin_handle_ = dlopen(SERIAL_BYTE_TRANSPORT_LIB_PATH, RTLD_NOW | RTLD_GLOBAL);
      ASSERT_NE(plugin_handle_, nullptr);

      umap_config["baud_rate"] = "115200";
      umap_config["num_of_bits_per_byte"] = "8";
      umap_config["parity"] = "none";
      umap_config["stop_bits"] = "1";
      umap_config["use_hardware_flow_control"] = "false";
      umap_config["use_software_flow_control"] = "false";

      umap_config["device_path"] = g_path_pty_1;
      byte_transport_1 = ByteTransportFactory::instance().create("SerialByteTransport", umap_config);

      umap_config["device_path"] = g_path_pty_2;
      byte_transport_2 = ByteTransportFactory::instance().create("SerialByteTransport", umap_config);
    }
    void TearDown() override {}

    std::unordered_map<std::string, std::string> umap_config;
    std::shared_ptr<pendarlab::lib::comm::IByteTransport> byte_transport_1;
    std::shared_ptr<pendarlab::lib::comm::IByteTransport> byte_transport_2;

    void* plugin_handle_ = nullptr;
  };

  TEST_F(SerialByteTransportIntegrationTest, CreatingSerialByteTransportShouldSucceed)
  {
    EXPECT_NE(byte_transport_1, nullptr);
    EXPECT_NE(byte_transport_2, nullptr);
  }

  TEST_F(SerialByteTransportIntegrationTest, NonExistentDevicePathShouldReturnNullptr)
  {
    umap_config["device_path"] = "/non/existent/device";
    auto byte_transport = ByteTransportFactory::instance().create("SerialByteTransport", umap_config);
    EXPECT_EQ(byte_transport, nullptr);
  }

  // According to https://man7.org/linux/man-pages/man3/termios.3.html, Linux capped the maximum bytes to read to 4095 bytes.
  // Therefore, bulk transfer tests are intentionally kept under this limit. Callers of IByteTransport are responsible for
  // handling larger payloads if needed.
  TEST_F(SerialByteTransportIntegrationTest, BulkReadShouldReadAllBulkWrite)
  {
    ASSERT_NE(byte_transport_1, nullptr);
    ASSERT_NE(byte_transport_2, nullptr);

    const unsigned int array_size = 4000;
    uint8_t data_to_send[array_size] = { 0 };
    for (size_t i = 0; i < array_size; i++) { // init data_to_send with 4000 bytes of 0, 1, 2, ..., 255, 0, 1, 2, ..., 255, 0, 1, 2, ...
      data_to_send[i] = (uint8_t)i;
    }

    int bytes_written = byte_transport_1->write(data_to_send, array_size);
    EXPECT_EQ(bytes_written, array_size);

    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    uint8_t data_buf[array_size] = { 0 };
    int bytes_read = byte_transport_2->read(data_buf, array_size);
    EXPECT_EQ(bytes_read, bytes_written);

    for (size_t i = 0; i < array_size; i++) {
      EXPECT_EQ(data_to_send[i], data_buf[i]);
    }
  }

  TEST_F(SerialByteTransportIntegrationTest, ReadOnDifferentThreadShouldReadAllData)
  {
    std::vector<uint8_t> data;
    for (size_t i = 0; i < 10000; i++) {
      data.push_back((uint8_t)i);
    }
    ReaderConfig reader_cfg = { .bt = byte_transport_2, .buf_size = 50, .read_delay_us = 100, .expected_size=data.size(), .timeout_in_ms=500 };
    WriterConfig writer_cfg = { .bt = byte_transport_1, .data_to_send = data, .num_of_bytes_on_each_write = 200, .write_delay_us = 20 };

    SerialByteTransportReader reader(reader_cfg);
    SerialByteTransportWriter writer(writer_cfg);

    std::thread read_thread = std::thread(&SerialByteTransportReader::routine, std::ref(reader));
    std::thread write_thread = std::thread(&SerialByteTransportWriter::routine, std::ref(writer));

    write_thread.join();
    read_thread.join();
    EXPECT_EQ(reader.data(), data);
  }

} // namespace

int main(int argc, char* argv[])
{
  testing::InitGoogleTest(&argc, argv);
  pid_t pid = fork();
  if (pid < 0) {
    perror("fork failed");
    return 1;
  } else if (pid == 0) {
    std::string writer_arg = "pty,raw,echo=0,link=" + g_path_pty_1;
    std::string reader_arg = "pty,raw,echo=0,link=" + g_path_pty_2;
    prctl(PR_SET_PDEATHSIG, SIGTERM);
    execlp("socat", "socat", writer_arg.c_str(), reader_arg.c_str(), (char*)nullptr);
    perror("execlp failed");
    return 1;
  } else {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    int test_result = RUN_ALL_TESTS();

    kill(pid, SIGTERM);
    waitpid(pid, nullptr, 0);

    return test_result; // <-- Google Test return code
  }
}