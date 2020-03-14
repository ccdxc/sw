//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __PORT_HPP__
#define __PORT_HPP__

#include "gen/proto/port.grpc.pb.h"
#include "devapi_object.hpp"
#include "devapi_types.hpp"

namespace iris {
class devapi_port : public devapi_object {
private:
    uint32_t port_num_;
    port_config_t config_;
    port_status_t stauts_;

    // port_num -> port
    static std::map<uint32_t, devapi_port*> port_db_;

private:
    sdk_ret_t init_(uint32_t port_num);
    devapi_port() {}
    ~devapi_port() {}
    sdk_ret_t port_get_(PortGetResponse *rsp);
    static sdk_ret_t port_speed_mbps_to_enum(uint32_t speed,
                                             port::PortSpeed *port_speed);
public:
    static devapi_port *factory(uint32_t port_num);
    static void destroy(devapi_port *port);
    static devapi_port *find_or_create(uint32_t port_num);

    static uint32_t port_speed_enum_to_mbps(port::PortSpeed speed_enum);

    static devapi_port *get_port(uint32_t port_num);
    uint32_t get_port_num(void);

    sdk_ret_t port_hal_get_config(port_config_t *config);
    sdk_ret_t port_hal_get_status(port_status_t *status);
    sdk_ret_t port_hal_update_config(port_config_t *config);
};

}    // namespace iris

using iris::devapi_port;

#endif /* __PORT_HPP__ */
