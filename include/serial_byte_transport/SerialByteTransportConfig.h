#pragma once

#include <byte_transport/ByteTransportFactory.h>
#include <functional>
#include <optional>
#include <serial_device/SerialDevice.h>
#include <string>
#include <unordered_map>

namespace pendarlab::lib::comm
{
  using SerialDevice = transport::SerialDevice;
  struct SerialByteTransportConfig {
    struct ParseResult;
    struct FieldSpec;
    static const std::unordered_map<std::string, FieldSpec> SPEC;
    static ParseResult parse(const std::unordered_map<std::string, std::string>& config);

    std::string device_path;
    SerialDevice::BaudRate baud_rate;
    SerialDevice::NumOfBitsPerByte num_of_bits_per_byte = SerialDevice::NumOfBitsPerByte::EIGHT;
    SerialDevice::Parity parity = SerialDevice::Parity::NONE;
    SerialDevice::StopBits stop_bits = SerialDevice::StopBits::ONE;
    bool use_hardware_flow_control = false;
    bool use_software_flow_control = false;
  };

  struct SerialByteTransportConfig::ParseResult {
    bool ok;
    std::string msg;
    std::optional<SerialByteTransportConfig> cfg;
  };

  struct SerialByteTransportConfig::FieldSpec {
    enum class Presence {
      REQUIRED,
      OPTIONAL
    };

    Presence presence;
    std::string description;
    std::function<bool(SerialByteTransportConfig&, const std::string&)> parse_fn;
  };

} // namespace pendarlab::lib::comm