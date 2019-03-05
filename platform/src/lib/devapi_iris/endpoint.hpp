//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __ENDPOINT_HPP__
#define __ENDPOINT_HPP__

#include "devapi_types.hpp"
#include "devapi_object.hpp"

namespace iris {

class devapi_l2seg;
class devapi_enic;
typedef std::tuple<devapi_l2seg *, mac_t> ep_key_t;
class devapi_ep : public devapi_object {
private:
  mac_t mac_;
  devapi_l2seg *l2seg_;
  devapi_enic *enic_;
  static std::map<ep_key_t, devapi_ep*> ep_db_;

private:
  sdk_ret_t init_(devapi_l2seg *l2seg, mac_t mac, devapi_enic *enic);
  devapi_ep() {}
  ~devapi_ep() {}

public:
  static devapi_ep *factory(devapi_l2seg *l2seg, mac_t mac, devapi_enic *enic);
  static void  destroy(devapi_ep *ep);

  static devapi_ep *lookup(devapi_l2seg *l2seg, mac_t mac);
  sdk_ret_t ep_halcreate();
  sdk_ret_t ep_haldelete();
  devapi_l2seg *get_l2seg();
  mac_t get_mac();
};

} // namespace iris

using iris::devapi_ep;

#endif /* __ENDPOINT_HPP__ */
