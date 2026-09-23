#include <byte_transport/ConfigParseResult.h>
#include <byte_transport/Registry.h>
#include <byte_transport/RegistryUserAccess.h>
#include <gtest/gtest.h>
#include <serial_byte_transport/SerialByteTransportConfig.h>
#include <serial_byte_transport/SerialByteTransportDefinition.h>
#include <string>
#include <unordered_map>

using Registry = pendarlab::lib::comm::byte_transport::Registry;
using RegistryUserAccess = pendarlab::lib::comm::byte_transport::RegistryUserAccess;
using ConfigParseResult = pendarlab::lib::comm::byte_transport::ConfigParseResult;
using SerialByteTransportDefinition = pendarlab::lib::comm::SerialByteTransportDefinition;

namespace
{
  class SerialByteTransportConfigTest : public testing::Test
  {
  protected:
    static constexpr const char* kType = "SerialByteTransport";

    SerialByteTransportConfigTest() :
        registry_user_(test_registry_.createUser())
    {
      test_registry_.addTransportDefinition(kType, transport_def_);
    }

    void SetUp() override {}

    ConfigParseResult parseConfig(const std::unordered_map<std::string, std::string>& cfg) const
    {
      return (*registry_user_)[kType]->parseConfig(cfg);
    }

    Registry test_registry_;
    std::unique_ptr<RegistryUserAccess> registry_user_;
    SerialByteTransportDefinition transport_def_;

    std::unordered_map<std::string, std::string> valid_config_ = {
      { "device_path", "/a/valid/device/path" },
      { "baud_rate", "115200" }
    };
  };

  TEST_F(SerialByteTransportConfigTest, GoodConfigShouldParseCorrectly)
  {
    auto result = parseConfig(valid_config_);
    EXPECT_EQ(result.ok(), true);
    ASSERT_NE(result.config, nullptr);

    const auto& serial_cfg = dynamic_cast<const pendarlab::lib::comm::SerialByteTransportConfig&>(*result.config);
    EXPECT_EQ(serial_cfg.device_path, "/a/valid/device/path");
    EXPECT_EQ(serial_cfg.baud_rate, pendarlab::lib::comm::SerialDevice::BaudRate::B_115200);
  }

  TEST_F(SerialByteTransportConfigTest, MissingDevicePathShouldFail)
  {
    std::unordered_map<std::string, std::string> cfg = valid_config_;
    cfg.erase("device_path");

    auto result = parseConfig(cfg);
    EXPECT_EQ(result.ok(), false);
    bool found_device_path = false;
    for (const auto& msg : result.messages) {
      if (msg.find("device_path") != std::string::npos) {
        found_device_path = true;
        break;
      }
    }
    EXPECT_EQ(found_device_path, true);
  }

  TEST_F(SerialByteTransportConfigTest, MissingBaudRateShouldFail)
  {
    std::unordered_map<std::string, std::string> cfg = valid_config_;
    cfg.erase("baud_rate");

    auto result = parseConfig(cfg);
    EXPECT_EQ(result.ok(), false);
    bool found_baud_rate = false;
    for (const auto& msg : result.messages) {
      if (msg.find("baud_rate") != std::string::npos) {
        found_baud_rate = true;
        break;
      }
    }
    EXPECT_EQ(found_baud_rate, true);
  }

  TEST_F(SerialByteTransportConfigTest, InvalidBaudRateShouldFail)
  {
    std::unordered_map<std::string, std::string> cfg = valid_config_;
    cfg["baud_rate"] = "not_a_number";

    auto result = parseConfig(cfg);
    EXPECT_EQ(result.ok(), false);
    bool found_bad_value = false;
    for (const auto& msg : result.messages) {
      if (msg.find("baud_rate") != std::string::npos) {
        found_bad_value = true;
        break;
      }
    }
    EXPECT_EQ(found_bad_value, true);
  }

} // namespace

int main(int argc, char* argv[])
{
  testing::InitGoogleTest(&argc, argv);
  int test_result = RUN_ALL_TESTS();
  return test_result;
}
