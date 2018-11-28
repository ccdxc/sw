
#include <string>
#include <memory>
#include <iostream>
#include <grpc++/grpc++.h>

#include "hal.hpp"
#include "hal_common_client.hpp"

using namespace std;

HalCommonClient *HalCommonClient::instance = NULL;

HalCommonClient *
HalCommonClient::GetInstance()
{
    if (!instance) {
        NIC_LOG_WARN("HalGRPCClient instance is not created!");
    }

    return instance;
}

HalCommonClient::HalCommonClient(){}
HalCommonClient::HalCommonClient(enum HalForwardingMode mode)
{
    this->mode = mode;
}

enum HalForwardingMode
HalCommonClient::GetMode()
{
    return mode;
}
