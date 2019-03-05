//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __L2SEG_HPP__
#define __L2SEG_HPP__

#include "lib/indexer/indexer.hpp"
#include "devapi_types.hpp"
#include "devapi_object.hpp"
#include "devapi_iris_types.hpp"

namespace iris {

class devapi_vrf;
class devapi_uplink;
class devapi_l2seg;

// (vrf, vlan)
typedef std::tuple<uint32_t, vlan_t> l2seg_key_t;
// uplink_id -> Uplink
typedef std::map<uplink_id_t, devapi_uplink*> uplink_map_t;

class devapi_l2seg : public devapi_object {
private:
    uint32_t id_;
    vlan_t vlan_;
    devapi_vrf *vrf_;
    uplink_map_t uplink_refs_;

    static sdk::lib::indexer *allocator_;          // id indexer
    static constexpr uint64_t max_l2segs = 4096;
    static std::map<l2seg_key_t, devapi_l2seg*> l2seg_db_;

private:
    sdk_ret_t init_(devapi_vrf *vrf, vlan_t vlan);
    sdk_ret_t trigger_halupdate();
    devapi_l2seg() {}
    ~devapi_l2seg() {}

public:
    static devapi_l2seg *factory(devapi_vrf *vrf, vlan_t vlan);
    static void destroy(devapi_l2seg *l2seg);

    sdk_ret_t l2seg_create();
    sdk_ret_t l2seg_delete();
    void deallocate_id();

    static devapi_l2seg *lookup(devapi_vrf *vrf, vlan_t vlan);

    sdk_ret_t add_uplink(devapi_uplink *uplink);
    sdk_ret_t del_uplink(devapi_uplink *uplink);

    uint64_t get_id();
    devapi_vrf *get_vrf();
    vlan_t get_vlan() { return vlan_; }

    static void probe();
};
} // namespace iris

using iris::devapi_l2seg;

#endif /* __L2SEG_HPP__ */
