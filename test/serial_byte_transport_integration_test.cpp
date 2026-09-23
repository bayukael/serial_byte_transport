#include <byte_transport/ConfigParseResult.h>
#include <byte_transport/Registry.h>
#include <byte_transport/RegistryUserAccess.h>
#include <byte_transport/Transport.h>
#include <chrono>
#include <gtest/gtest.h>
#include <memory>
#include <serial_byte_transport/SerialByteTransportDefinition.h>
#include <string>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <thread>
#include <unordered_map>
#include <vector>

using Registry = pendarlab::lib::comm::byte_transport::Registry;
using RegistryUserAccess = pendarlab::lib::comm::byte_transport::RegistryUserAccess;
using Transport = pendarlab::lib::comm::byte_transport::Transport;
using SerialByteTransportDefinition = pendarlab::lib::comm::SerialByteTransportDefinition;

const std::string g_path_pty_1("/tmp/sbt_pty1");
const std::string g_path_pty_2("/tmp/sbt_pty2");

namespace
{
  // Reads from the given transport until data_read_ has accumulated expected_size bytes,
  // or until timeout_in_ms has elapsed.
  std::vector<uint8_t> readUntil(Transport& transport, size_t expected_size, unsigned int timeout_in_ms)
  {
    std::vector<uint8_t> data_read;
    const auto start = std::chrono::steady_clock::now();
    while (data_read.size() < expected_size) {
      const auto now = std::chrono::steady_clock::now();
      if (std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count() > timeout_in_ms) {
        break;
      }
      uint8_t buf[1024];
      int bytes_read = transport.read(buf, sizeof(buf));
      if (bytes_read > 0) {
        for (int i = 0; i < bytes_read; i++) {
          data_read.push_back(buf[i]);
        }
      } else {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
    }
    return data_read;
  }

  class SerialByteTransportIntegrationTest : public testing::Test
  {
  protected:
    static constexpr const char* kType = "SerialByteTransport";

    SerialByteTransportIntegrationTest() : registry_user_(test_registry_.createUser())
    {
      test_registry_.addTransportDefinition(kType, transport_def_);
    }

    void SetUp() override
    {
      umap_config_["device_path"] = g_path_pty_1;
      umap_config_["baud_rate"] = "115200";
      transport_1_ = createTransport(umap_config_);

      umap_config_["device_path"] = g_path_pty_2;
      transport_2_ = createTransport(umap_config_);
    }

    void TearDown() override {}

    std::unique_ptr<Transport> createTransport(const std::unordered_map<std::string, std::string>& cfg) const
    {
      auto parse_result = (*registry_user_)[kType]->parseConfig(cfg);
      if (!parse_result.ok()) {
        return nullptr;
      }
      return (*registry_user_)[kType]->create(*parse_result.config);
    }

    Registry test_registry_;
    std::unique_ptr<RegistryUserAccess> registry_user_;
    SerialByteTransportDefinition transport_def_;
    std::unordered_map<std::string, std::string> umap_config_;
    std::unique_ptr<Transport> transport_1_;
    std::unique_ptr<Transport> transport_2_;
  };

  TEST_F(SerialByteTransportIntegrationTest, CreatingOnBothPtyEndsShouldSucceed)
  {
    EXPECT_NE(transport_1_, nullptr);
    EXPECT_NE(transport_2_, nullptr);
  }

  // According to https://man7.org/linux/man-pages/man3/termios.3.html, Linux caps the maximum bytes to read to 4095.
  // Bulk transfer tests are intentionally kept under this limit.
  TEST_F(SerialByteTransportIntegrationTest, BulkReadShouldReadAllBulkWrite)
  {
    ASSERT_NE(transport_1_, nullptr);
    ASSERT_NE(transport_2_, nullptr);

    const size_t array_size = 4000;
    std::vector<uint8_t> data_to_send(array_size);
    for (size_t i = 0; i < array_size; i++) {
      data_to_send[i] = (uint8_t)i;
    }

    int bytes_written = transport_1_->write(data_to_send.data(), array_size);
    EXPECT_EQ(bytes_written, static_cast<int>(array_size));

    auto data_read = readUntil(*transport_2_, array_size, 2000);
    EXPECT_EQ(data_read.size(), array_size);
    ASSERT_EQ(data_read.size(), data_to_send.size());
    for (size_t i = 0; i < array_size; i++) {
      EXPECT_EQ(data_to_send[i], data_read[i]);
    }
  }

  TEST_F(SerialByteTransportIntegrationTest, SmallPayloadShouldRoundTrip)
  {
    ASSERT_NE(transport_1_, nullptr);
    ASSERT_NE(transport_2_, nullptr);

    const std::vector<uint8_t> payload = { 'h', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd' };
    int bytes_written = transport_1_->write(payload.data(), payload.size());
    EXPECT_EQ(bytes_written, static_cast<int>(payload.size()));

    auto data_read = readUntil(*transport_2_, payload.size(), 2000);
    EXPECT_EQ(data_read.size(), payload.size());
    ASSERT_EQ(data_read.size(), payload.size());
    for (size_t i = 0; i < payload.size(); i++) {
      EXPECT_EQ(payload[i], data_read[i]);
    }
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

    return test_result;
  }
}
