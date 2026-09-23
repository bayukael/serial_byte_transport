#pragma once

#include <byte_transport/Transport.h>
#include <memory>
#include <serial_device/SerialDevice.h>

namespace pendarlab::lib::comm
{
  using SerialDevice = transport::SerialDevice;

  class SerialByteTransport : public pendarlab::lib::comm::byte_transport::Transport
  {
  public:
    SerialByteTransport(std::unique_ptr<SerialDevice> device);
    ~SerialByteTransport();

    virtual int read(unsigned char* buf, unsigned int buf_size) override;
    virtual int write(const unsigned char* buf, unsigned int length) override;

  private:
    struct SerialByteTransportImpl;
    std::unique_ptr<SerialByteTransportImpl> p_impl_;
  };
} // namespace pendarlab::lib::comm
