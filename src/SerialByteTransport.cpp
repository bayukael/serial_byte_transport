#include "serial_byte_transport/SerialByteTransport.h"

#include <byte_transport/ByteTransportFactory.h>
#include <byte_transport/IByteTransport.h>
#include <iostream>
#include <memory>
#include <serial_byte_transport/SerialByteTransportConfig.h>
#include <serial_device/SerialDevice.h>

namespace
{
  using namespace pendarlab::lib::comm;
  std::shared_ptr<IByteTransport> create(const std::unordered_map<std::string, std::string>& config)
  {
    SerialByteTransportConfig::ParseResult parse_result = SerialByteTransportConfig::parse(config);
    if (!parse_result.ok) {
      return nullptr;
    }
    return SerialByteTransport::create(parse_result.cfg.value());
  }

  ByteTransportFactory::ValidationResult validateConfig(const std::unordered_map<std::string, std::string>& config)
  {
    SerialByteTransportConfig::ParseResult parse_result = SerialByteTransportConfig::parse(config);

    // Translate validation result from SerialByteTransportConfig format to ByteTransportFactory format;
    ByteTransportFactory::ValidationResult parse_result_factory;
    parse_result_factory.ok = parse_result.ok;
    parse_result_factory.msg = parse_result.msg;

    return parse_result_factory;
  }
} // namespace

namespace pendarlab::lib::comm
{
  struct SerialByteTransport::SerialByteTransportImpl {
    SerialByteTransportImpl(std::unique_ptr<SerialDevice> device);
    std::unique_ptr<SerialDevice> serial_dev_;
  };

  SerialByteTransport::SerialByteTransportImpl::SerialByteTransportImpl(std::unique_ptr<SerialDevice> device) :
      serial_dev_(std::move(device))
  {
  }

  SerialByteTransport::SerialByteTransport(std::unique_ptr<SerialDevice> device) :
      p_impl_(std::make_unique<SerialByteTransport::SerialByteTransportImpl>(std::move(device)))
  {
  }

  std::shared_ptr<IByteTransport> SerialByteTransport::create(const SerialByteTransportConfig& cfg)
  {
    auto device = std::make_unique<SerialDevice>();
    device->setBaudRate(cfg.baud_rate);
    device->setNumOfBitsPerByte(cfg.num_of_bits_per_byte);
    device->setParity(cfg.parity);
    device->setStopBits(cfg.stop_bits);
    device->setHardwareFlowControl(cfg.use_hardware_flow_control);
    device->setSoftwareFlowControl(cfg.use_software_flow_control);
    if (device->connect(cfg.device_path, SerialDevice::RWMode::BOTH)) {
      return std::make_shared<SerialByteTransport>(SerialByteTransport(std::move(device)));
    }
    return nullptr;
  }

  int SerialByteTransport::read(unsigned char* buf, unsigned int buf_size)
  {
    if (!p_impl_->serial_dev_) { // If there is no serial device
      return -1;
    }
    int ret_val = p_impl_->serial_dev_->readData(buf, buf_size);
    if (ret_val < 0) {
      return -1;
    }
    return ret_val;
  }

  int SerialByteTransport::write(const unsigned char* buf, unsigned int length)
  {
    if (!p_impl_->serial_dev_) { // If there is no serial device
      return -1;
    }
    int ret_val = p_impl_->serial_dev_->writeData(buf, length);
    if (ret_val < 0) {
      return -1;
    }
    return ret_val;
  }
} // namespace pendarlab::lib::comm

REGISTER_BYTE_TRANSPORT("SerialByteTransport", create, validateConfig)