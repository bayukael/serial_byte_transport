#pragma once

#include <byte_transport/Config.h>
#include <serial_device/SerialDevice.h>
#include <string>

namespace pendarlab::lib::comm
{
  using SerialDevice = transport::SerialDevice;

  class SerialByteTransportConfig : public pendarlab::lib::comm::byte_transport::Config
  {
  public:
    SerialByteTransportConfig() = default;

    std::string device_path;
    SerialDevice::BaudRate baud_rate;
    SerialDevice::NumOfBitsPerByte num_of_bits_per_byte = SerialDevice::NumOfBitsPerByte::EIGHT;
    SerialDevice::Parity parity = SerialDevice::Parity::NONE;
    SerialDevice::StopBits stop_bits = SerialDevice::StopBits::ONE;
    bool use_hardware_flow_control = false;
    bool use_software_flow_control = false;
  };
} // namespace pendarlab::lib::comm
