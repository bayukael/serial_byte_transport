#include "serial_byte_transport/SerialByteTransport.h"

#include <byte_transport/ByteTransportFactory.h>
#include <byte_transport/IByteTransport.h>
#include <serial_byte_transport/SerialByteTransportConfig.h>

namespace pendarlab::lib::comm
{
  struct SerialByteTransport::SerialByteTransportImpl{
    
  };

  std::shared_ptr<IByteTransport> SerialByteTransport::create(const std::unordered_map<std::string, std::string>& config)
  {
    SerialByteTransportConfig::ValidationResult result = SerialByteTransportConfig::validateConfig(config);
    if (!result.ok) {
      return nullptr;
    }
    return std::make_shared<SerialByteTransport>();
  }

  ByteTransportFactory::ValidationResult SerialByteTransport::validateConfig(const std::unordered_map<std::string, std::string>& config)
  {
    SerialByteTransportConfig::ValidationResult result_serial = SerialByteTransportConfig::validateConfig(config);

    // Translate validation result from SerialByteTransportConfig format to ByteTransportFactory format;
    ByteTransportFactory::ValidationResult result_factory;
    result_factory.ok = result_serial.ok;
    result_factory.msg = result_serial.msg;

    return result_factory;
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

REGISTER_BYTE_TRANSPORT("SerialByteTransport", &pendarlab::lib::comm::SerialByteTransport::create,
                        &pendarlab::lib::comm::SerialByteTransport::validateConfig)