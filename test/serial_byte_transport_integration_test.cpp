#include <chrono>
#include <dlfcn.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <thread>

const std::string g_path_pty_1("/tmp/pty1");
const std::string g_path_pty_2("/tmp/pty2");

namespace
{
  class SerialByteTransportIntegrationTest : public testing::Test
  {
  protected:
    SerialByteTransportIntegrationTest() {}
    void SetUp() override
    {
      umap_config["device_path"] = g_path_pty_1;
      umap_config["baud_rate"] = "115200";
      umap_config["num_of_bits_per_byte"] = "7";
      umap_config["parity"] = "odd";
      umap_config["stop_bits"] = "2";
      umap_config["use_hardware_flow_control"] = "true";
      umap_config["use_software_flow_control"] = "true";
      // SERIAL_BYTE_TRANSPORT_LIB_PATH is defined in CMakeLists.txt
      plugin_handle_ = dlopen(SERIAL_BYTE_TRANSPORT_LIB_PATH, RTLD_NOW | RTLD_GLOBAL);
      ASSERT_NE(plugin_handle_, nullptr);
    }
    void TearDown() override {}

    std::unordered_map<std::string, std::string> umap_config;

    void* plugin_handle_ = nullptr;
  };

  TEST_F(SerialByteTransportIntegrationTest, CreatingSerialByteTransportShouldSucceed)
  {
  }

  TEST_F(SerialByteTransportIntegrationTest, BulkWriteThenBulkReadTest)
  {
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