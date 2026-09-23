#include "serial_byte_transport/SerialByteTransport.h"

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

  SerialByteTransport::~SerialByteTransport() = default;

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
