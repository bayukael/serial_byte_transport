#include <gtest/gtest.h>
#include <serial_byte_transport/SerialByteTransportConfig.h>
#include <string>
#include <unordered_map>

namespace
{
  class TestSetUp
  {
  protected:
    using Config = pendarlab::lib::comm::SerialByteTransportConfig;

    TestSetUp()
    {
      umap_config["device_path"] = "/a/valid/device/path";
      umap_config["baud_rate"] = "115200";
      umap_config["num_of_bits_per_byte"] = "7";
      umap_config["parity"] = "odd";
      umap_config["stop_bits"] = "2";
      umap_config["use_hardware_flow_control"] = "true";
      umap_config["use_software_flow_control"] = "true";
    }
    std::unordered_map<std::string, std::string> umap_config;
    Config::ParseResult parse_result;
  };
  class OverallConfigTest : public TestSetUp, public testing::Test
  {
  };

  TEST_F(OverallConfigTest, GoodConfigShouldParseCorrectly)
  {
    using SerialDevice = pendarlab::lib::comm::SerialDevice;

    parse_result = Config::parse(umap_config);
    EXPECT_EQ(parse_result.ok, true);
    ASSERT_TRUE(parse_result.cfg.has_value());
    EXPECT_EQ(parse_result.cfg.value().device_path, "/a/valid/device/path");
    EXPECT_EQ(parse_result.cfg.value().baud_rate, SerialDevice::BaudRate::B_115200);
    EXPECT_EQ(parse_result.cfg.value().num_of_bits_per_byte, SerialDevice::NumOfBitsPerByte::SEVEN);
    EXPECT_EQ(parse_result.cfg.value().parity, SerialDevice::Parity::ODD);
    EXPECT_EQ(parse_result.cfg.value().stop_bits, SerialDevice::StopBits::TWO);
    EXPECT_EQ(parse_result.cfg.value().use_hardware_flow_control, true);
    EXPECT_EQ(parse_result.cfg.value().use_software_flow_control, true);
  }

  TEST_F(OverallConfigTest, MissingOptionalFieldShouldGiveDefaultValue)
  {
    using SerialDevice = pendarlab::lib::comm::SerialDevice;

    umap_config.erase("num_of_bits_per_byte");
    parse_result = Config::parse(umap_config);
    EXPECT_EQ(parse_result.ok, true);
    ASSERT_TRUE(parse_result.cfg.has_value());
    EXPECT_EQ(parse_result.cfg.value().num_of_bits_per_byte, SerialDevice::NumOfBitsPerByte::EIGHT);

    umap_config.erase("parity");
    parse_result = Config::parse(umap_config);
    EXPECT_EQ(parse_result.ok, true);
    ASSERT_TRUE(parse_result.cfg.has_value());
    EXPECT_EQ(parse_result.cfg.value().parity, SerialDevice::Parity::NONE);

    umap_config.erase("stop_bits");
    parse_result = Config::parse(umap_config);
    EXPECT_EQ(parse_result.ok, true);
    ASSERT_TRUE(parse_result.cfg.has_value());
    EXPECT_EQ(parse_result.cfg.value().stop_bits, SerialDevice::StopBits::ONE);

    umap_config.erase("use_hardware_flow_control");
    parse_result = Config::parse(umap_config);
    EXPECT_EQ(parse_result.ok, true);
    ASSERT_TRUE(parse_result.cfg.has_value());
    EXPECT_EQ(parse_result.cfg.value().use_hardware_flow_control, false);

    umap_config.erase("use_software_flow_control");
    parse_result = Config::parse(umap_config);
    EXPECT_EQ(parse_result.ok, true);
    ASSERT_TRUE(parse_result.cfg.has_value());
    EXPECT_EQ(parse_result.cfg.value().use_software_flow_control, false);
  }

  TEST_F(OverallConfigTest, MissingRequiredFieldShouldBeFailed)
  {
    using SerialDevice = pendarlab::lib::comm::SerialDevice;

    auto missing_device_path_config = umap_config;
    missing_device_path_config.erase("device_path");
    parse_result = Config::parse(missing_device_path_config);
    EXPECT_EQ(parse_result.ok, false);
    ASSERT_FALSE(parse_result.cfg.has_value());

    auto missing_baud_rate_config = umap_config;
    missing_baud_rate_config.erase("baud_rate");
    parse_result = Config::parse(missing_baud_rate_config);
    EXPECT_EQ(parse_result.ok, false);
    ASSERT_FALSE(parse_result.cfg.has_value());
  }

  // #########################################################################################################
  // DevicePathTest
  // #########################################################################################################
  class DevicePathTest : public TestSetUp, public testing::Test
  {
  };

  TEST_F(DevicePathTest, NonEmptyStringShouldParseCorrectly)
  {
    using SerialDevice = pendarlab::lib::comm::SerialDevice;

    umap_config["device_path"] = "";
    parse_result = Config::parse(umap_config);
    EXPECT_EQ(parse_result.ok, false);
    ASSERT_FALSE(parse_result.cfg.has_value());

    umap_config["device_path"] = " /a/valid/device/path";
    parse_result = Config::parse(umap_config);
    EXPECT_EQ(parse_result.ok, true);
    ASSERT_TRUE(parse_result.cfg.has_value());
    EXPECT_EQ(parse_result.cfg.value().device_path, " /a/valid/device/path");

    umap_config["device_path"] = " /a/valid/device/path ";
    parse_result = Config::parse(umap_config);
    EXPECT_EQ(parse_result.ok, true);
    ASSERT_TRUE(parse_result.cfg.has_value());
    EXPECT_EQ(parse_result.cfg.value().device_path, " /a/valid/device/path ");

    umap_config["device_path"] = "/a/valid/device/path ";
    parse_result = Config::parse(umap_config);
    EXPECT_EQ(parse_result.ok, true);
    ASSERT_TRUE(parse_result.cfg.has_value());
    EXPECT_EQ(parse_result.cfg.value().device_path, "/a/valid/device/path ");
  }

  TEST_F(DevicePathTest, EmptyStringShouldBeFailed)
  {
    using SerialDevice = pendarlab::lib::comm::SerialDevice;

    umap_config["device_path"] = "";
    parse_result = Config::parse(umap_config);
    EXPECT_EQ(parse_result.ok, false);
    ASSERT_FALSE(parse_result.cfg.has_value());
  }

  // #########################################################################################################
  // BaudRateTest
  // #########################################################################################################

  struct BaudRateTestCase {
    using SerialDevice = pendarlab::lib::comm::SerialDevice;
    std::string input;
    SerialDevice::BaudRate expected;
  };

  class BaudRateTest : public TestSetUp, public testing::TestWithParam<BaudRateTestCase>
  {
  };

  TEST_P(BaudRateTest, ValidValueShouldParseCorrectly)
  {
    using SerialDevice = pendarlab::lib::comm::SerialDevice;

    auto [input, expected] = GetParam();

    umap_config["baud_rate"] = input;
    parse_result = Config::parse(umap_config);
    EXPECT_EQ(parse_result.ok, true);
    ASSERT_TRUE(parse_result.cfg.has_value());
    EXPECT_EQ(parse_result.cfg.value().baud_rate, expected);
  }

  // clang-format off
  INSTANTIATE_TEST_SUITE_P(BaudRateTestValues, BaudRateTest, testing::Values(
    BaudRateTestCase{"50",      pendarlab::lib::comm::SerialDevice::BaudRate::B_50},
    BaudRateTestCase{"75",      pendarlab::lib::comm::SerialDevice::BaudRate::B_75},
    BaudRateTestCase{"110",     pendarlab::lib::comm::SerialDevice::BaudRate::B_110},
    BaudRateTestCase{"134",     pendarlab::lib::comm::SerialDevice::BaudRate::B_134},
    BaudRateTestCase{"150",     pendarlab::lib::comm::SerialDevice::BaudRate::B_150},
    BaudRateTestCase{"200",     pendarlab::lib::comm::SerialDevice::BaudRate::B_200},
    BaudRateTestCase{"600",     pendarlab::lib::comm::SerialDevice::BaudRate::B_600},
    BaudRateTestCase{"1200",    pendarlab::lib::comm::SerialDevice::BaudRate::B_1200},
    BaudRateTestCase{"1800",    pendarlab::lib::comm::SerialDevice::BaudRate::B_1800},
    BaudRateTestCase{"2400",    pendarlab::lib::comm::SerialDevice::BaudRate::B_2400},
    BaudRateTestCase{"4800",    pendarlab::lib::comm::SerialDevice::BaudRate::B_4800},
    BaudRateTestCase{"9600",    pendarlab::lib::comm::SerialDevice::BaudRate::B_9600},
    BaudRateTestCase{"19200",   pendarlab::lib::comm::SerialDevice::BaudRate::B_19200},
    BaudRateTestCase{"38400",   pendarlab::lib::comm::SerialDevice::BaudRate::B_38400},
    BaudRateTestCase{"57600",   pendarlab::lib::comm::SerialDevice::BaudRate::B_57600},
    BaudRateTestCase{"115200",  pendarlab::lib::comm::SerialDevice::BaudRate::B_115200},
    BaudRateTestCase{"230400",  pendarlab::lib::comm::SerialDevice::BaudRate::B_230400},
    BaudRateTestCase{"460800",  pendarlab::lib::comm::SerialDevice::BaudRate::B_460800},
    BaudRateTestCase{"500000",  pendarlab::lib::comm::SerialDevice::BaudRate::B_500000},
    BaudRateTestCase{"576000",  pendarlab::lib::comm::SerialDevice::BaudRate::B_576000},
    BaudRateTestCase{"921600",  pendarlab::lib::comm::SerialDevice::BaudRate::B_921600},
    BaudRateTestCase{"1000000", pendarlab::lib::comm::SerialDevice::BaudRate::B_1000000},
    BaudRateTestCase{"1152000", pendarlab::lib::comm::SerialDevice::BaudRate::B_1152000},
    BaudRateTestCase{"1500000", pendarlab::lib::comm::SerialDevice::BaudRate::B_1500000},
    BaudRateTestCase{"2000000", pendarlab::lib::comm::SerialDevice::BaudRate::B_2000000},
    BaudRateTestCase{"2500000", pendarlab::lib::comm::SerialDevice::BaudRate::B_2500000},
    BaudRateTestCase{"3000000", pendarlab::lib::comm::SerialDevice::BaudRate::B_3000000},
    BaudRateTestCase{"3500000", pendarlab::lib::comm::SerialDevice::BaudRate::B_3500000},
    BaudRateTestCase{"4000000", pendarlab::lib::comm::SerialDevice::BaudRate::B_4000000}
  )
  ); // INSTANTIATE_TEST_SUITE_P
  // clang-format on

  TEST_F(BaudRateTest, InvalidValueShouldBeFailed)
  {
    using SerialDevice = pendarlab::lib::comm::SerialDevice;

    umap_config["baud_rate"] = "some_invalid_value";
    parse_result = Config::parse(umap_config);
    EXPECT_EQ(parse_result.ok, false);
    ASSERT_FALSE(parse_result.cfg.has_value());
  }

  // #########################################################################################################
  // NumOfBitsPerByteTest
  // #########################################################################################################

  struct NumOfBitsPerByteTestCase {
    using SerialDevice = pendarlab::lib::comm::SerialDevice;
    std::string input;
    SerialDevice::NumOfBitsPerByte expected;
  };

  class NumOfBitsPerByteTest : public TestSetUp, public testing::TestWithParam<NumOfBitsPerByteTestCase>
  {
  };

  TEST_P(NumOfBitsPerByteTest, ValidValueShouldParseCorrectly)
  {
    using SerialDevice = pendarlab::lib::comm::SerialDevice;

    auto [input, expected] = GetParam();

    umap_config["num_of_bits_per_byte"] = input;
    parse_result = Config::parse(umap_config);
    EXPECT_EQ(parse_result.ok, true);
    ASSERT_TRUE(parse_result.cfg.has_value());
    EXPECT_EQ(parse_result.cfg.value().num_of_bits_per_byte, expected);
  }

  // clang-format off
  INSTANTIATE_TEST_SUITE_P(NumOfBitsPerByteTestValues, NumOfBitsPerByteTest, testing::Values(
    NumOfBitsPerByteTestCase{"5", pendarlab::lib::comm::SerialDevice::NumOfBitsPerByte::FIVE},
    NumOfBitsPerByteTestCase{"6", pendarlab::lib::comm::SerialDevice::NumOfBitsPerByte::SIX},
    NumOfBitsPerByteTestCase{"7", pendarlab::lib::comm::SerialDevice::NumOfBitsPerByte::SEVEN},
    NumOfBitsPerByteTestCase{"8", pendarlab::lib::comm::SerialDevice::NumOfBitsPerByte::EIGHT}
  )
  ); // INSTANTIATE_TEST_SUITE_P
  // clang-format on

  TEST_F(NumOfBitsPerByteTest, InvalidValueShouldReturnDefaultValue)
  {
    using SerialDevice = pendarlab::lib::comm::SerialDevice;

    umap_config["num_of_bits_per_byte"] = "some_invalid_value";
    parse_result = Config::parse(umap_config);
    EXPECT_EQ(parse_result.ok, true);
    ASSERT_TRUE(parse_result.cfg.has_value());
    EXPECT_EQ(parse_result.cfg.value().num_of_bits_per_byte, SerialDevice::NumOfBitsPerByte::EIGHT);
  }

  // #########################################################################################################
  // ParityTest
  // #########################################################################################################

  struct ParityTestCase {
    using SerialDevice = pendarlab::lib::comm::SerialDevice;
    std::string input;
    SerialDevice::Parity expected;
  };

  class ParityTest : public TestSetUp, public testing::TestWithParam<ParityTestCase>
  {
  };

  TEST_P(ParityTest, ValidValueShouldParseCorrectly)
  {
    using SerialDevice = pendarlab::lib::comm::SerialDevice;

    auto [input, expected] = GetParam();

    umap_config["parity"] = input;
    parse_result = Config::parse(umap_config);
    EXPECT_EQ(parse_result.ok, true);
    ASSERT_TRUE(parse_result.cfg.has_value());
    EXPECT_EQ(parse_result.cfg.value().parity, expected);
  }

  // clang-format off
  INSTANTIATE_TEST_SUITE_P(ParityTestValues, ParityTest, testing::Values(
    ParityTestCase{"none", pendarlab::lib::comm::SerialDevice::Parity::NONE},
    ParityTestCase{"even", pendarlab::lib::comm::SerialDevice::Parity::EVEN},
    ParityTestCase{"odd", pendarlab::lib::comm::SerialDevice::Parity::ODD}
  )
  ); // INSTANTIATE_TEST_SUITE_P
  // clang-format on

  TEST_F(ParityTest, InvalidValueShouldReturnDefaultValue)
  {
    using SerialDevice = pendarlab::lib::comm::SerialDevice;

    umap_config["parity"] = "some_invalid_value";
    parse_result = Config::parse(umap_config);
    EXPECT_EQ(parse_result.ok, true);
    ASSERT_TRUE(parse_result.cfg.has_value());
    EXPECT_EQ(parse_result.cfg.value().parity, SerialDevice::Parity::NONE);
  }

  // #########################################################################################################
  // StopBitsTest
  // #########################################################################################################

  struct StopBitsTestCase {
    using SerialDevice = pendarlab::lib::comm::SerialDevice;
    std::string input;
    SerialDevice::StopBits expected;
  };

  class StopBitsTest : public TestSetUp, public testing::TestWithParam<StopBitsTestCase>
  {
  };

  TEST_P(StopBitsTest, ValidValueShouldParseCorrectly)
  {
    using SerialDevice = pendarlab::lib::comm::SerialDevice;

    auto [input, expected] = GetParam();

    umap_config["stop_bits"] = input;
    parse_result = Config::parse(umap_config);
    EXPECT_EQ(parse_result.ok, true);
    ASSERT_TRUE(parse_result.cfg.has_value());
    EXPECT_EQ(parse_result.cfg.value().stop_bits, expected);
  }

  // clang-format off
  INSTANTIATE_TEST_SUITE_P(StopBitsTestValues, StopBitsTest, testing::Values(
    StopBitsTestCase{"1", pendarlab::lib::comm::SerialDevice::StopBits::ONE},
    StopBitsTestCase{"2", pendarlab::lib::comm::SerialDevice::StopBits::TWO}
  )
  ); // INSTANTIATE_TEST_SUITE_P
  // clang-format on

  TEST_F(StopBitsTest, InvalidValueShouldReturnDefaultValue)
  {
    using SerialDevice = pendarlab::lib::comm::SerialDevice;

    umap_config["stop_bits"] = "some_invalid_value";
    parse_result = Config::parse(umap_config);
    EXPECT_EQ(parse_result.ok, true);
    ASSERT_TRUE(parse_result.cfg.has_value());
    EXPECT_EQ(parse_result.cfg.value().stop_bits, SerialDevice::StopBits::ONE);
  }

  // #########################################################################################################
  // HardwareFlowControlTest
  // #########################################################################################################

  struct HardwareFlowControlTestCase {
    using SerialDevice = pendarlab::lib::comm::SerialDevice;
    std::string input;
    bool expected;
  };

  class HardwareFlowControlTest : public TestSetUp, public testing::TestWithParam<HardwareFlowControlTestCase>
  {
  };

  TEST_P(HardwareFlowControlTest, ValidValueShouldParseCorrectly)
  {
    using SerialDevice = pendarlab::lib::comm::SerialDevice;

    auto [input, expected] = GetParam();

    umap_config["use_hardware_flow_control"] = input;
    parse_result = Config::parse(umap_config);
    EXPECT_EQ(parse_result.ok, true);
    ASSERT_TRUE(parse_result.cfg.has_value());
    EXPECT_EQ(parse_result.cfg.value().use_hardware_flow_control, expected);
  }

  // clang-format off
  INSTANTIATE_TEST_SUITE_P(HardwareFlowControlTestValues, HardwareFlowControlTest, testing::Values(
    HardwareFlowControlTestCase{"true", true},
    HardwareFlowControlTestCase{"false", false}
  )
  ); // INSTANTIATE_TEST_SUITE_P
  // clang-format on

  TEST_F(HardwareFlowControlTest, InvalidValueShouldReturnDefaultValue)
  {
    using SerialDevice = pendarlab::lib::comm::SerialDevice;

    umap_config["use_hardware_flow_control"] = "some_invalid_value";
    parse_result = Config::parse(umap_config);
    EXPECT_EQ(parse_result.ok, true);
    ASSERT_TRUE(parse_result.cfg.has_value());
    EXPECT_EQ(parse_result.cfg.value().use_hardware_flow_control, false);
  }

  // #########################################################################################################
  // SoftwareFlowControlTest
  // #########################################################################################################
  struct SoftwareFlowControlTestCase {
    using SerialDevice = pendarlab::lib::comm::SerialDevice;
    std::string input;
    bool expected;
  };
  class SoftwareFlowControlTest : public TestSetUp, public testing::TestWithParam<SoftwareFlowControlTestCase>
  {
  };
  TEST_P(SoftwareFlowControlTest, ValidValueShouldParseCorrectly)
  {
    using SerialDevice = pendarlab::lib::comm::SerialDevice;
    auto [input, expected] = GetParam();
    umap_config["use_software_flow_control"] = input;
    parse_result = Config::parse(umap_config);
    EXPECT_EQ(parse_result.ok, true);
    ASSERT_TRUE(parse_result.cfg.has_value());
    EXPECT_EQ(parse_result.cfg.value().use_software_flow_control, expected);
  }
  // clang-format off
  INSTANTIATE_TEST_SUITE_P(SoftwareFlowControlTestValues, SoftwareFlowControlTest, testing::Values(
    SoftwareFlowControlTestCase{"true", true},
    SoftwareFlowControlTestCase{"false", false}
  )
  ); // INSTANTIATE_TEST_SUITE_P
  // clang-format on
  TEST_F(SoftwareFlowControlTest, InvalidValueShouldReturnDefaultValue)
  {
    using SerialDevice = pendarlab::lib::comm::SerialDevice;
    umap_config["use_software_flow_control"] = "some_invalid_value";
    parse_result = Config::parse(umap_config);
    EXPECT_EQ(parse_result.ok, true);
    ASSERT_TRUE(parse_result.cfg.has_value());
    EXPECT_EQ(parse_result.cfg.value().use_software_flow_control, false);
  }

} // namespace

int main(int argc, char* argv[])
{
  testing::InitGoogleTest(&argc, argv);

  int test_result = RUN_ALL_TESTS();
  return test_result; // <-- Google Test return code
}