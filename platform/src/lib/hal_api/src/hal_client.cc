
#include <string>
#include <memory>
#include <iostream>
#include <grpc++/grpc++.h>

#include "hal.hpp"
#include "hal_common_client.hpp"

using namespace std;

HalCommonClient *HalCommonClient::instance = NULL;
#if 0
shared_ptr<HalCommonClient> HalCommonClient::instance = 0;
shared_ptr<HalCommonClient>
#endif

HalCommonClient *
HalCommonClient::GetInstance()
{
    if (!instance) {
        HAL_TRACE_ERR("HalGRPCClient instance is not created!");
    }

    return instance;
}

HalCommonClient::HalCommonClient()
{

    // Initializing traces
    iris_c::utils::logger_init(0x3, true);
    HAL_TRACE_DEBUG("Logger Init....");

}
HalCommonClient::HalCommonClient(enum HalForwardingMode mode)
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

enum HalForwardingMode
HalCommonClient::GetMode()
{
    return mode;
}
