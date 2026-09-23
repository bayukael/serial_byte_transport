#include "serial_byte_transport/SerialByteTransportDefinition.h"

#include "serial_byte_transport/SerialByteTransport.h"
#include "serial_byte_transport/SerialByteTransportConfig.h"

#include <memory>
#include <optional>
#include <string>

namespace pendarlab::lib::comm
{
  using Config = pendarlab::lib::comm::byte_transport::Config;
  using ConfigParseResult = pendarlab::lib::comm::byte_transport::ConfigParseResult;
  using Transport = pendarlab::lib::comm::byte_transport::Transport;

  namespace
  {
    using BaudRate = SerialDevice::BaudRate;

    std::optional<BaudRate> parseBaudRate(const std::string& val_str)
    {
      int val_int;
      try {
        val_int = std::stoi(val_str);
      } catch (const std::exception& e) {
        return std::nullopt;
      }
      switch (val_int) {
        case 50: return BaudRate::B_50;
        case 75: return BaudRate::B_75;
        case 110: return BaudRate::B_110;
        case 134: return BaudRate::B_134;
        case 150: return BaudRate::B_150;
        case 200: return BaudRate::B_200;
        case 300: return BaudRate::B_300;
        case 600: return BaudRate::B_600;
        case 1200: return BaudRate::B_1200;
        case 1800: return BaudRate::B_1800;
        case 2400: return BaudRate::B_2400;
        case 4800: return BaudRate::B_4800;
        case 9600: return BaudRate::B_9600;
        case 19200: return BaudRate::B_19200;
        case 38400: return BaudRate::B_38400;
        case 57600: return BaudRate::B_57600;
        case 115200: return BaudRate::B_115200;
        case 230400: return BaudRate::B_230400;
        case 460800: return BaudRate::B_460800;
        case 500000: return BaudRate::B_500000;
        case 576000: return BaudRate::B_576000;
        case 921600: return BaudRate::B_921600;
        case 1000000: return BaudRate::B_1000000;
        case 1152000: return BaudRate::B_1152000;
        case 1500000: return BaudRate::B_1500000;
        case 2000000: return BaudRate::B_2000000;
        case 2500000: return BaudRate::B_2500000;
        case 3000000: return BaudRate::B_3000000;
        case 3500000: return BaudRate::B_3500000;
        case 4000000: return BaudRate::B_4000000;
        default: return std::nullopt;
      }
    }
  } // namespace

  ConfigParseResult SerialByteTransportDefinition::parseConfig(const std::unordered_map<std::string, std::string>& cfg) const
  {
    ConfigParseResult result;
    auto serial_config = std::make_unique<SerialByteTransportConfig>();

    auto it_device_path = cfg.find("device_path");
    if (it_device_path == cfg.end()) {
      result.messages.push_back("[SerialByteTransport] : a required entry is missing: device_path");
    } else {
      serial_config->device_path = it_device_path->second;
    }

    auto it_baud_rate = cfg.find("baud_rate");
    if (it_baud_rate == cfg.end()) {
      result.messages.push_back("[SerialByteTransport] : a required entry is missing: baud_rate");
    } else {
      auto baud_rate = parseBaudRate(it_baud_rate->second);
      if (!baud_rate.has_value()) {
        result.messages.push_back("[SerialByteTransport] : bad value for key 'baud_rate': " + it_baud_rate->second);
      } else {
        serial_config->baud_rate = baud_rate.value();
      }
    }

    if (result.messages.empty()) {
      result.config = std::move(serial_config);
    }
    return result;
  }

  std::unique_ptr<Transport> SerialByteTransportDefinition::create(const Config& cfg) const
  {
    const auto& serial_cfg = dynamic_cast<const SerialByteTransportConfig&>(cfg);

    auto device = std::make_unique<SerialDevice>();
    device->setBaudRate(serial_cfg.baud_rate);
    device->setNumOfBitsPerByte(serial_cfg.num_of_bits_per_byte);
    device->setParity(serial_cfg.parity);
    device->setStopBits(serial_cfg.stop_bits);
    device->setHardwareFlowControl(serial_cfg.use_hardware_flow_control);
    device->setSoftwareFlowControl(serial_cfg.use_software_flow_control);
    if (device->connect(serial_cfg.device_path, SerialDevice::RWMode::BOTH)) {
      return std::make_unique<SerialByteTransport>(std::move(device));
    }
    return nullptr;
  }

  extern "C" const SerialByteTransportDefinition* getSerialByteTransportDefinition()
  {
    static SerialByteTransportDefinition serial_byte_transport_def;
    return &serial_byte_transport_def;
  }
} // namespace pendarlab::lib::comm
