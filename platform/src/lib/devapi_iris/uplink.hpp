//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __UPLINK_HPP__
#define __UPLINK_HPP__

#include "devapi_object.hpp"

namespace iris {
class devapi_vrf;
class devapi_l2seg;
class devapi_uplink : public devapi_object {
private:
    uplink_id_t id_;              // uplink PI Id
    uint32_t port_num_;           // uplinks port number
    uint32_t num_lifs_;           // # of lifs pinned to this uplink
    bool is_oob_;                 // Out of Band mgmt. uplink
    InterfaceSpec if_spec_;       // Interface spec for HAL
    devapi_vrf *vrf_;             // Vrf on the uplink. Appl. in classic.
    devapi_l2seg *native_l2seg_;  // Native L2seg

    // port_num -> uplink
    static std::map<uint64_t, devapi_uplink*> uplink_db_;

private:
    sdk_ret_t init_(uplink_id_t id, uint32_t port_num, bool is_oob = false);
    devapi_uplink() {}
    ~devapi_uplink() {}

public:
    static devapi_uplink *factory(uplink_id_t id, uint32_t port_num,
                                  bool is_oob = false);
    static void destroy(devapi_uplink *up);

    static sdk_ret_t create_vrfs();
    static devapi_uplink *get_uplink(uint32_t port_num);

    sdk_ret_t update_hal_native_l2seg(uint32_t native_l2seg_id);
    sdk_ret_t create_vrf();
    sdk_ret_t delete_vrf();

    // Get APIs
    uint32_t get_id();
    uint32_t get_port_num();
    uint32_t get_num_lifs();
    devapi_vrf *get_vrf();
    devapi_l2seg *get_native_l2seg();

    // Set APIs
    void set_port_num(uint32_t port_num);
    void set_native_l2seg(devapi_l2seg *l2seg);

    void inc_num_lifs();
    void dec_num_lifs();

    bool is_oob();
};

} // namespace iris

using iris::devapi_uplink;

#endif /* __UPLINK_HPP__ */
