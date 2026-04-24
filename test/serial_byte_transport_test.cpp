#include <byte_transport/ByteTransportFactory.h>
#include <byte_transport/IByteTransport.h>
#include <dlfcn.h>
#include <gtest/gtest.h>
#include <unordered_map>

namespace
{
  class SerialByteTransportTest : public testing::Test
  {
  protected:
    using IByteTransport = pendarlab::lib::comm::IByteTransport;
    SerialByteTransportTest() {}

    void SetUp() override
    {
      umap_config["device_path"] = "/a/path/to/serial/device";
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

    void TearDown() override
    {
      if (plugin_handle_) {
        dlclose(plugin_handle_);
        plugin_handle_ = nullptr;
      }
    }

    std::unordered_map<std::string, std::string> umap_config;

    void* plugin_handle_ = nullptr;
  };

  TEST_F(SerialByteTransportTest, SerialByteTransportShouldBeRegistered)
  {
    using ByteTransportFactory = pendarlab::lib::comm::ByteTransportFactory;
    auto registration_result = ByteTransportFactory::instance().isRegistered("SerialByteTransport");
    EXPECT_EQ(registration_result, true);
  }

  TEST_F(SerialByteTransportTest, ValidateConfigShouldSucceed)
  {
    using ByteTransportFactory = pendarlab::lib::comm::ByteTransportFactory;
    auto validation_result = ByteTransportFactory::instance().validateConfig("SerialByteTransport", umap_config);
    EXPECT_EQ(validation_result.ok, true);
  }

  TEST_F(SerialByteTransportTest, CreatingSerialByteTransportShouldFailBecauseItRequiresASerialPort)
  {
    using ByteTransportFactory = pendarlab::lib::comm::ByteTransportFactory;
    auto byte_transport = ByteTransportFactory::instance().create("SerialByteTransport", umap_config);
    EXPECT_EQ(byte_transport, nullptr);
  }

} // namespace

int main(int argc, char* argv[])
{
  testing::InitGoogleTest(&argc, argv);
  int test_result = RUN_ALL_TESTS();
  return test_result; // <-- Google Test return code
}