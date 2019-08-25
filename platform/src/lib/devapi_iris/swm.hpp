//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __SWM_HPP__
#define __SWM_HPP__

#include "devapi_types.hpp"
#include "devapi_object.hpp"

namespace iris {

class devapi_swm : public devapi_object {
private:
    uint32_t port_num_;
    uint32_t vlan_;
    mac_t mac_;

    // Single ton class
    static devapi_swm *swm_;

private:
    sdk_ret_t init_(uint32_t port_num, uint32_t vlan, mac_t mac);
    devapi_swm() {}
    ~devapi_swm() {}

    static bool swm_changed_(uint32_t port_num, uint32_t vlan, mac_t mac);

public:
    static devapi_swm *factory(uint32_t port_num, uint32_t vlan, mac_t mac);
    static void destroy(devapi_swm *swm);

    static sdk_ret_t swm_configure(uint32_t port_num, uint32_t vlan, mac_t mac);
    static sdk_ret_t swm_unconfigure(void);

    uint32_t port_num() { return port_num_; }
    uint32_t vlan() { return vlan_; }
    mac_t mac() { return mac_; }
};

}     // namespace iris

#endif
