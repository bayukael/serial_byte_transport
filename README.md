# serial_byte_transport

A C++17 library that implements a serial transport as a `byte_transport` plugin, built with CMake and installed as a consumable CMake package.

## What it does

`serial_byte_transport` provides a concrete `Transport` backed by a serial device. It is implemented as a plugin for the `byte_transport` registry: it subclasses `byte_transport::TransportDefinition` so that it can be registered under a type name, looked up by a `RegistryUserAccess`, and instantiated from a set of key/value config strings.

The library is split into three pieces:

- `SerialByteTransport` — a `byte_transport::Transport` that wraps a `serial_device::SerialDevice` and forwards `read()`/`write()` to it. Uses the PIMPL idiom.
- `SerialByteTransportConfig` — a `byte_transport::Config` subclass carrying the fields needed to set up a `SerialDevice` (device path, baud rate, bits-per-byte, parity, stop bits, flow control).
- `SerialByteTransportDefinition` — a `byte_transport::TransportDefinition` that parses config and creates the transport. It exports an `extern "C"` `getSerialByteTransportDefinition()` entry point so it can be discovered by a dynamic lib loader.

## Architecture

Public headers live in `include/serial_byte_transport/` and use the namespace `pendarlab::lib::comm`.

- `SerialByteTransport` — abstract read/write interface for a transport.
- `SerialByteTransportConfig` — holds serial settings; derives from `byte_transport::Config`.
- `SerialByteTransportDefinition` — plugin interface: `parseConfig()` + `create()`.

`SerialByteTransport` uses the PIMPL idiom: the implementation (`SerialByteTransportImpl`) owns a `std::unique_ptr<serial_device::SerialDevice>`, so serial internals are hidden from the public header.

### Config parsing

`SerialByteTransportDefinition::parseConfig()` reads a `std::unordered_map<std::string, std::string>` and:

- requires the `device_path` key
- requires the `baud_rate` key (a numeric string mapped to `SerialDevice::BaudRate`)

If a required key is missing or the baud rate is invalid, it adds a message to the result and `ok()` returns `false`. On success it returns a `SerialByteTransportConfig` populated with the parsed values (optional fields use defaults).

### Creating a transport

`SerialByteTransportDefinition::create()` takes the parsed `Config` (downcast to `SerialByteTransportConfig`), constructs and configures a `SerialDevice`, and attempts to connect to the device path in read/write mode. It returns a `SerialByteTransport` on success, or `nullptr` if the connection fails.

## Quick start (build)

Requires CMake >= 3.13, the installed `PendarlabByteTransport` and `PendarlabSerialDevice` packages, and GoogleTest (`find_package(GTest CONFIG REQUIRED)`). Point `CMAKE_PREFIX_PATH` at the directory containing the installed pendarlab packages.

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=/path/to/installed/packages
cmake --build build
./build/serial_byte_transport_config_test       # config parsing
./build/serial_byte_transport_test              # create() behavior
./build/serial_byte_transport_integration_test  # real read/write over a socat PTY pair
```

Run a single test:

```bash
./build/serial_byte_transport_config_test --gtest_filter='*GoodConfigShouldParseCorrectly*'
```

There are no lint/format targets; building and running the test executables is the only verification.

## Using the library from another project

The library installs as the CMake package `PendarlabSerialByteTransport` (exported target `pendarlab::SerialByteTransport`):

```cmake
find_package(PendarlabSerialByteTransport REQUIRED)
target_link_libraries(my_app PRIVATE pendarlab::SerialByteTransport)
```

Register the transport with a `byte_transport::Registry` under a type name, then create it from config:

```cpp
#include <byte_transport/Registry.h>
#include <memory>
#include <serial_byte_transport/SerialByteTransportDefinition.h>

using namespace pendarlab::lib::comm;
using byte_transport::Registry;

Registry registry;
SerialByteTransportDefinition def;
registry.addTransportDefinition("SerialByteTransport", def);

auto user = registry.createUser();
const auto* transport_def = (*user)["SerialByteTransport"]; // nullptr if absent

std::unordered_map<std::string, std::string> cfg = {
  { "device_path", "/dev/ttyUSB0" },
  { "baud_rate", "115200" },
};
auto parsed = transport_def->parseConfig(cfg);   // requires device_path + baud_rate
if (parsed.ok()) {
  auto transport = transport_def->create(*parsed.config); // nullptr if connect fails
  // transport->read(buf, size); transport->write(buf, size);
}
```

## Adding a transport plugin

1. Subclass `byte_transport::Transport` for the concrete transport.
2. Subclass `byte_transport::Config` to carry transport-specific settings.
3. Subclass `byte_transport::TransportDefinition`; implement `parseConfig()` and `create()`.
4. Export an `extern "C"` getter returning the definition (see `getSerialByteTransportDefinition()`).

## Tests

- `test/serial_byte_transport_config_test.cpp` — `parseConfig()` accepts a valid config and rejects a missing `device_path`, missing `baud_rate`, or an invalid `baud_rate`.
- `test/serial_byte_transport_test.cpp` — `create()` returns `nullptr` for a non-existent device path.
- `test/serial_byte_transport_integration_test.cpp` — spawns a `socat` PTY pair and verifies real `read()`/`write()` round-trips (bulk and small payloads).

The integration test requires `socat` at runtime.
