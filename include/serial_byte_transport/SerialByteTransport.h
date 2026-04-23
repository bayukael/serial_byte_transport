#pragma once

#include <byte_transport/IByteTransport.h>
#include <memory>
#include <serial_byte_transport/SerialByteTransportConfig.h>
#include <serial_device/SerialDevice.h>
#include <string>
#include <unordered_map>

namespace pendarlab::lib::comm
{
  class SerialByteTransport : public IByteTransport
  {
  public:
    static std::shared_ptr<IByteTransport> create(const SerialByteTransportConfig& cfg);

    int read(unsigned char* buf, unsigned int buf_size) override;
    int write(const unsigned char* buf, unsigned int length) override;

  private:
    SerialByteTransport(std::unique_ptr<SerialDevice> device);
    struct SerialByteTransportImpl;
    std::unique_ptr<SerialByteTransportImpl> p_impl_;
  };
} // namespace pendarlab::lib::comm