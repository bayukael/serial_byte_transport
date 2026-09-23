#include <byte_transport/ConfigParseResult.h>
#include <byte_transport/Registry.h>
#include <byte_transport/RegistryUserAccess.h>
#include <byte_transport/Transport.h>
#include <gtest/gtest.h>
#include <memory>
#include <serial_byte_transport/SerialByteTransportDefinition.h>
#include <string>
#include <unordered_map>

using Registry = pendarlab::lib::comm::byte_transport::Registry;
using RegistryUserAccess = pendarlab::lib::comm::byte_transport::RegistryUserAccess;
using Transport = pendarlab::lib::comm::byte_transport::Transport;
using SerialByteTransportDefinition = pendarlab::lib::comm::SerialByteTransportDefinition;

namespace
{
  class SerialByteTransportTest : public testing::Test
  {
  protected:
    static constexpr const char* kType = "SerialByteTransport";

    SerialByteTransportTest() : registry_user_(test_registry_.createUser())
    {
      test_registry_.addTransportDefinition(kType, transport_def_);
    }

    void SetUp() override {}

    std::unique_ptr<Transport> createWith(const std::unordered_map<std::string, std::string>& cfg) const
    {
      auto parse_result = (*registry_user_)[kType]->parseConfig(cfg);
      if (!parse_result.ok()) {
        return nullptr;
      }
      return (*registry_user_)[kType]->create(*parse_result.config);
    }

    Registry test_registry_;
    std::unique_ptr<RegistryUserAccess> registry_user_;
    SerialByteTransportDefinition transport_def_;
  };

  TEST_F(SerialByteTransportTest, CreatingWithNonExistentDevicePathShouldReturnNullptr)
  {
    std::unordered_map<std::string, std::string> cfg = {
      { "device_path", "/non/existent/device" },
      { "baud_rate", "115200" }
    };
    auto transport = createWith(cfg);
    EXPECT_EQ(transport, nullptr);
  }

} // namespace

int main(int argc, char* argv[])
{
  testing::InitGoogleTest(&argc, argv);
  int test_result = RUN_ALL_TESTS();
  return test_result;
}
