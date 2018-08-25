
#include <string>
#include <memory>
#include <iostream>
#include <grpc++/grpc++.h>

#include "hal.hpp"
#include "hal_client.hpp"

using namespace std;

HalClient *HalClient::instance = NULL;
#if 0
shared_ptr<HalClient> HalClient::instance = 0;
shared_ptr<HalClient>
#endif

HalClient *
HalClient::GetInstance()
{
    if (!instance) {
        HAL_TRACE_ERR("HalGRPCClient instance is not created!");
    }

    return instance;
}

HalClient::HalClient()
{

    // Initializing traces
    iris_c::utils::logger_init(0x3, true);
    HAL_TRACE_DEBUG("Logger Init....");

}
HalClient::HalClient(enum ForwardingMode mode)
{
    this->mode = mode;

    // Initializing traces
    iris_c::utils::logger_init(0x3, true);
    HAL_TRACE_DEBUG("Logger Init....");
#if 0
    iris_c::utils::trace_init("iris-c", 0x3, true,
                              "iris-c.log",
                              hal::utils::trace_debug);
#endif

}

enum ForwardingMode
HalClient::GetMode()
{
    return mode;
}
