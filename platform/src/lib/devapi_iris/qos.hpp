//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __QOS_HPP__
#define __QOS_HPP__

#include "devapi_object.hpp"

class devapi_qos : public devapi_object {
public:
    static int32_t get_txtc_cos(const std::string &group,
                                uint32_t uplink_port);
};

#endif
