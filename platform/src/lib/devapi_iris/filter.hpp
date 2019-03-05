//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __FILTER_HPP__
#define __FILTER_HPP__

#include "devapi_types.hpp"
#include "devapi_object.hpp"

namespace iris {

typedef kh::FilterType filter_type_t;

class devapi_lif;
class devapi_ep;
class devapi_mcast;
class devapi_filter : public devapi_object {
private:
    filter_type_t type_;
    mac_t mac_;
    vlan_t vlan_;
    devapi_lif *lif_;
    devapi_ep *ep_;
    devapi_mcast *mcast_;

private:
  sdk_ret_t init_(devapi_lif *lif,
                  mac_t mac, vlan_t vlan,
                  filter_type_t type = kh::FILTER_LIF_MAC_VLAN);
  devapi_filter() {}
  ~devapi_filter() {}

public:
  static devapi_filter *factory(devapi_lif *lif,
                                mac_t mac, vlan_t vlan,
                                filter_type_t type = kh::FILTER_LIF_MAC_VLAN);
  static void destroy(devapi_filter *filter);
  sdk_ret_t filter_create();
  sdk_ret_t filter_delete();
};

} // namespace iris

using iris::devapi_filter;

#endif  /* __FILTER_HPP__ */
