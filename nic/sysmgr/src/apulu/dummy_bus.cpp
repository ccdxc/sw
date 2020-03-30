#include <memory>
#include <string>

#include "../bus_api.hpp"

class DummyBus : public SysmgrBus {
public:
    virtual void Connect(void) override;
    virtual void SystemFault(std::string reason) override;
    virtual void ProcessDied(std::string name, pid_t pid,
                             std::string reason) override;

};
typedef std::shared_ptr<DummyBus> DummyBusPtr;

SysmgrBusPtr
init_bus (bus_api_t *api)
{
    return std::make_shared<DummyBus>();
}

void
DummyBus::Connect(void) {
}

void
DummyBus::SystemFault(std::string readon) {
}

void
DummyBus::ProcessDied(std::string name, pid_t pid, std::string reason) {
}
