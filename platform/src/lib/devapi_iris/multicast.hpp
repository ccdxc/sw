//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __MULTICAST_HPP__
#define __MULTICAST_HPP__

#include "devapi_types.hpp"
#include "devapi_object.hpp"

namespace iris {

class devapi_l2seg;
class devapi_enic;
// (l2seg, mac)
typedef std::tuple<devapi_l2seg *, mac_t> mcast_key_t;
class devapi_mcast : public devapi_object {
private:
    devapi_l2seg *l2seg_;
    mac_t mac_;
    std::map<uint64_t, devapi_enic*> enic_refs_;
    static std::map<mcast_key_t, devapi_mcast*> mcast_db_;

private:
  sdk_ret_t init_(devapi_l2seg *l2seg, mac_t mac);
  devapi_mcast() {}
  ~devapi_mcast() {}

public:
    static devapi_mcast *factory(devapi_l2seg *l2seg, mac_t mac);
    static void destroy(devapi_mcast *mcast);

    sdk_ret_t mcast_halcreate();
    sdk_ret_t mcast_haldelete();
    static devapi_mcast *find_or_create(devapi_l2seg *l2seg, mac_t mac,
                                        bool create);

    sdk_ret_t add_enic(devapi_enic *enic);
    sdk_ret_t del_enic(devapi_enic *enic);
    sdk_ret_t trigger_hal();
    devapi_l2seg *get_l2seg();
    mac_t get_mac();
    uint32_t get_numenics();
};
} // namespace iris

using iris::devapi_mcast;
#endif  /* __MULTICAST_HPP__ */
