#include "serial_byte_transport/SerialByteTransportConfig.h"

#include <optional>
#include <stdexcept>
#include <string>
#include <variant>

namespace pendarlab::lib::comm
{
  static bool parseDevicePath(SerialByteTransportConfig& cfg, const std::string& val_str)
  {
    if (val_str.empty()) {
      return false;
    }
    cfg.device_path = val_str;
    return true;
  }

  static bool parseBaudRate(SerialByteTransportConfig& cfg, const std::string& val_str)
  {
    int val_int;
    try {
      val_int = std::stoi(val_str);
    } catch (const std::exception& e) {
      return false;
    }
    using BaudRate = transport::SerialDevice::BaudRate;
    switch (val_int) {
      case 50: cfg.baud_rate = BaudRate::B_50;
      case 75: cfg.baud_rate = BaudRate::B_75;
      case 110: cfg.baud_rate = BaudRate::B_110;
      case 134: cfg.baud_rate = BaudRate::B_134;
      case 150: cfg.baud_rate = BaudRate::B_150;
      case 200: cfg.baud_rate = BaudRate::B_200;
      case 600: cfg.baud_rate = BaudRate::B_600;
      case 1200: cfg.baud_rate = BaudRate::B_1200;
      case 1800: cfg.baud_rate = BaudRate::B_1800;
      case 2400: cfg.baud_rate = BaudRate::B_2400;
      case 4800: cfg.baud_rate = BaudRate::B_4800;
      case 9600: cfg.baud_rate = BaudRate::B_9600;
      case 19200: cfg.baud_rate = BaudRate::B_19200;
      case 38400: cfg.baud_rate = BaudRate::B_38400;
      case 57600: cfg.baud_rate = BaudRate::B_57600;
      case 115200: cfg.baud_rate = BaudRate::B_115200;
      case 230400: cfg.baud_rate = BaudRate::B_230400;
      case 460800: cfg.baud_rate = BaudRate::B_460800;
      case 500000: cfg.baud_rate = BaudRate::B_500000;
      case 576000: cfg.baud_rate = BaudRate::B_576000;
      case 921600: cfg.baud_rate = BaudRate::B_921600;
      case 1000000: cfg.baud_rate = BaudRate::B_1000000;
      case 1152000: cfg.baud_rate = BaudRate::B_1152000;
      case 1500000: cfg.baud_rate = BaudRate::B_1500000;
      case 2000000: cfg.baud_rate = BaudRate::B_2000000;
      case 2500000: cfg.baud_rate = BaudRate::B_2500000;
      case 3000000: cfg.baud_rate = BaudRate::B_3000000;
      case 3500000: cfg.baud_rate = BaudRate::B_3500000;
      case 4000000: cfg.baud_rate = BaudRate::B_4000000;
      default: return false;
    }
    return true;
  }

  static bool parseNumOfBitsPerByte(SerialByteTransportConfig& cfg, const std::string& val_str)
  {
    int val_int;
    try {
      val_int = std::stoi(val_str);
    } catch (const std::exception& e) {
      return false;
    }
    using NumOfBitsPerByte = transport::SerialDevice::NumOfBitsPerByte;
    switch (val_int) {
      case 5: cfg.num_of_bits_per_byte = NumOfBitsPerByte::FIVE;
      case 6: cfg.num_of_bits_per_byte = NumOfBitsPerByte::SIX;
      case 7: cfg.num_of_bits_per_byte = NumOfBitsPerByte::SEVEN;
      case 8: cfg.num_of_bits_per_byte = NumOfBitsPerByte::EIGHT;
      default: return false;
    }
    return true;
  }

  static bool parseParity(SerialByteTransportConfig& cfg, const std::string& val_str)
  {
    using Parity = transport::SerialDevice::Parity;
    if (val_str == "NONE") {
      cfg.parity = Parity::NONE;
    } else if (val_str == "ODD") {
      cfg.parity = Parity::ODD;
    } else if (val_str == "EVEN") {
      cfg.parity = Parity::EVEN;
    } else {
      return false;
    }
    return true;
  }

  static bool parseStopBits(SerialByteTransportConfig& cfg, const std::string& val_str)
  {
    int val_int;
    try {
      val_int = std::stoi(val_str);
    } catch (const std::exception& e) {
      return false;
    }
    using StopBits = transport::SerialDevice::StopBits;
    switch (val_int) {
      case 1: cfg.stop_bits = StopBits::ONE;
      case 2: cfg.stop_bits = StopBits::TWO;
      default: return false;
    }
    return true;
  }

  static bool parseHardwareFlowControl(SerialByteTransportConfig& cfg, const std::string& val_str)
  {
    if (val_str == "true") {
      cfg.use_hardware_flow_control = true;
    } else {
      return false;
    }
    return true;
  }

  static bool parseSoftwareFlowControl(SerialByteTransportConfig& cfg, const std::string& val_str)
  {
    if (val_str == "true") {
      cfg.use_software_flow_control = true;
    } else {
      return false;
    }
    return true;
  }

  SerialByteTransportConfig::ParseResult SerialByteTransportConfig::parse(const std::unordered_map<std::string, std::string>& config)
  {
    SerialByteTransportConfig::ParseResult result;
    result.ok = true;
    result.msg = "";

    using namespace transport;
    SerialByteTransportConfig parsed_config;

    for (const auto& [key, spec] : SPEC) {
      auto it = config.find(key);
      if (it == config.end()) {
        if (spec.presence == FieldSpec::Presence::REQUIRED) {
          result.ok = false;
          result.msg += "Missing required key: [" + key + "]\n";
        }
        continue;
      }
      const std::string& config_val = it->second;

      if (spec.parse_fn(parsed_config, config_val)) {
        if (spec.presence == FieldSpec::Presence::REQUIRED) {
          result.ok = false;
        }
        result.msg += "Bad value for key '" + key + "': " + config_val + "\n";
        continue;
      }
    }

    if (result.ok) {
      result.cfg = parsed_config;
    }
    return result;
  }

  // clang-format off
  using FieldSpec = SerialByteTransportConfig::FieldSpec;
  inline const std::unordered_map<std::string, FieldSpec> SerialByteTransportConfig::SPEC =
  {
    {
      "device_path",
      {
        FieldSpec::Presence::REQUIRED,
        "Path to the serial device",
        parseDevicePath
      }
    },
    {
      "baud_rate", 
      {
        FieldSpec::Presence::REQUIRED,
        "Baud rate to be used",
        parseBaudRate
      }
    },
    {
      "num_of_bits_per_byte", 
      {
        FieldSpec::Presence::OPTIONAL,
        "Number of bits per byte used in the serial communication. Choose between 5, 6, 7, or 8. Default to 8.",
        parseNumOfBitsPerByte
      }
    },
    {
      "parity", 
      {
        FieldSpec::Presence::OPTIONAL,
        "Specify the usage of parity bit. Choose between NONE, ODD, or EVEN. Default to NONE.",
        parseParity
      }
    },
    {
      "stop_bits", 
      {
        FieldSpec::Presence::OPTIONAL,
        "Number of stop bits to use. Choose between 1 or 2. Default to 1.",
        parseStopBits
      }
    },
    {
      "use_hardware_flow_control", 
      {
        FieldSpec::Presence::OPTIONAL,
        "Use hardware flow control. Choose between true or false. Default to false.",
        parseHardwareFlowControl
      }
    },
    {
      "use_software_flow_control", 
      {
        FieldSpec::Presence::OPTIONAL,
        "Use software flow control. Choose between true or false. Default to false.",
        parseSoftwareFlowControl
      }
    }
  };
  // clang-format on
} // namespace pendarlab::lib::comm
