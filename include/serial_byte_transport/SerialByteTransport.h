#pragma once

#include <byte_transport/IByteTransport.h>
#include <unordered_map>
#include <memory>
#include <string>

namespace pendarlab::lib::comm
{
  class SerialByteTransport : public IByteTransport
  {
  public:
    SerialByteTransport(const SerialByteTransportConfig& cfg);
    static std::shared_ptr<IByteTransport> create(const std::unordered_map<std::string, std::string>& config);
    static ByteTransportFactory::ValidationResult validateConfig(const std::unordered_map<std::string, std::string>& config);
    
    int read(unsigned char* buf, unsigned int buf_size) override;
    int write(const unsigned char* buf, unsigned int length) override;

  private:
    struct SerialByteTransportImpl;
    std::unique_ptr<SerialByteTransportImpl> p_impl_;
  };
} // namespace pendarlab::lib::comm