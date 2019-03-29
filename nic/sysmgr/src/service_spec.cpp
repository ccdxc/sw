#include "service_spec.hpp"

#include <memory>

ServiceSpecDepPtr ServiceSpecDep::create()
{
    return std::make_shared<ServiceSpecDep>();
}

ServiceSpecPtr ServiceSpec::create()
{
    return std::make_shared<ServiceSpec>();
}
