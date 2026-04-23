#include "serial_byte_transport/SerialByteTransport.h"

#include <byte_transport/ByteTransportFactory.h>
#include <byte_transport/IByteTransport.h>
#include <serial_byte_transport/SerialByteTransportConfig.h>
#include <serial_device/SerialDevice.h>
#include <memory>

namespace
{
  using namespace pendarlab::lib::comm;
  std::shared_ptr<IByteTransport> create(const std::unordered_map<std::string, std::string>& config)
  {
    SerialByteTransportConfig::ParseResult result_serial = SerialByteTransportConfig::parse(config);
    if (!result_serial.ok) {
      return nullptr;
    }
    return SerialByteTransport::create(result_serial.cfg.value());
  }

  ByteTransportFactory::ValidationResult validateConfig(const std::unordered_map<std::string, std::string>& config)
  {
    SerialByteTransportConfig::ParseResult result_serial = SerialByteTransportConfig::parse(config);

    // Translate validation result from SerialByteTransportConfig format to ByteTransportFactory format;
    ByteTransportFactory::ValidationResult result_factory;
    result_factory.ok = result_serial.ok;
    result_factory.msg = result_serial.msg;

    return result_factory;
  }
} // namespace

namespace pendarlab::lib::comm
{
  struct SerialByteTransport::SerialByteTransportImpl {
    SerialByteTransportImpl(std::unique_ptr<SerialDevice> device);
    std::unique_ptr<SerialDevice> serial_dev_;
  };

  SerialByteTransport::SerialByteTransportImpl::SerialByteTransportImpl(std::unique_ptr<SerialDevice> device) : serial_dev_(std::move(device))
  {
  }

  SerialByteTransport::SerialByteTransport(std::unique_ptr<SerialDevice> device) : p_impl_(std::make_unique<SerialByteTransport::SerialByteTransportImpl>(std::move(device)))
  {
  }

  std::shared_ptr<IByteTransport> SerialByteTransport::create(const SerialByteTransportConfig& cfg)
  {
    auto device = std::make_unique<SerialDevice>();
    // SerialDevice device;
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
    return 0;
  }

  int SerialByteTransport::write(const unsigned char* buf, unsigned int length)
  {
    return 0;
  }
} // namespace pendarlab::lib::comm

REGISTER_BYTE_TRANSPORT("SerialByteTransport", create, validateConfig)