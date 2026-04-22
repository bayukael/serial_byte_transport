#include <gtest/gtest.h>
#include <serial_byte_transport/SerialByteTransportConfig.h>
#include <string>
#include <unordered_map>

namespace
{
  class GoodConfigTest : public testing::Test
  {
  protected:
    GoodConfigTest() {
      umap_config["device_path"] = "/a/valid/device/path";
      umap_config["baud_rate"] = "115200";
      umap_config["num_of_bits_per_byte"] = "8";
      umap_config["parity"] = "none";
      umap_config["stop_bits"] = "1";
      umap_config["use_hardware_flow_control"] = "false";
      umap_config["use_software_flow_control"] = "false";
    }
    std::unordered_map<std::string, std::string> umap_config;
  };

  TEST_F(GoodConfigTest, ValidTest){
    using Config = pendarlab::lib::comm::SerialByteTransportConfig;
    auto parse_result = Config::parse(umap_config);
    EXPECT_EQ(parse_result.ok, true);
    EXPECT_EQ(parse_result.cfg.has_value(), true);
    EXPECT_EQ(parse_result.cfg.value().device_path, "/a/valid/device/path");
  }
} // namespace

int main(int argc, char* argv[])
{
  testing::InitGoogleTest(&argc, argv);

  int test_result = RUN_ALL_TESTS();
  return test_result; // <-- Google Test return code
}