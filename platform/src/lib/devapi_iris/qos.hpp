//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __QOS_HPP__
#define __QOS_HPP__

#include "devapi_object.hpp"
#include "devapi_types.hpp"

namespace iris {

class devapi_qos : public devapi_object {
public:
    static int32_t get_txtc_cos(const std::string &group,
                                uint32_t uplink_port);

    static sdk_ret_t qos_class_get(uint8_t group, qos_class_info_t *info);
    static sdk_ret_t qos_class_exist(uint8_t group);
    static sdk_ret_t qos_class_create(qos_class_info_t *info);
    static sdk_ret_t qos_class_delete(uint8_t group, bool clear_stats);
    static sdk_ret_t qos_clear_port_stats(uint32_t port_num);
    static sdk_ret_t qos_class_set_global_pause_type(uint8_t pause_type);
};

}    // namespace iris

#endif
