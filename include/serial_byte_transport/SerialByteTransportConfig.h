#pragma once

#include <byte_transport/ByteTransportFactory.h>
#include <unordered_map>
#include <string>

namespace pendarlab::lib::comm
{
  struct SerialByteTransportConfig{
    struct ValidationResult {
      bool ok;
      std::string msg;
    };

    static ValidationResult validateConfig(const std::unordered_map<std::string, std::string>& config);
    static SerialByteTransportConfig create(const std::unordered_map<std::string, std::string>& config);

    std::string device_path_;
    
  };
} // namespace pendarlab::lib::comm