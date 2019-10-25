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
class devapi_enic;
class devapi_l2seg;

// (vrf, vlan)
typedef std::tuple<uint32_t, vlan_t> l2seg_key_t;
// uplink_id -> Uplink
typedef std::map<uplink_id_t, devapi_uplink*> uplink_map_t;
typedef std::map<uint32_t, devapi_enic*> enic_map_t;

class devapi_l2seg : public devapi_object {
private:
    uint32_t id_;
    vlan_t vlan_;
    devapi_vrf *vrf_;
    uplink_map_t uplink_refs_;
    enic_map_t enic_refs_;
    bool single_wire_mgmt_;
    devapi_uplink *undesignated_up_;

    static sdk::lib::indexer *allocator_;          // id indexer
    static constexpr uint64_t max_l2segs = 4096;
    static std::map<l2seg_key_t, devapi_l2seg*> l2seg_db_;

private:
    sdk_ret_t init_(devapi_vrf *vrf, vlan_t vlan);
    sdk_ret_t trigger_halupdate(void);
    devapi_l2seg() {}
    ~devapi_l2seg() {}

public:
    static devapi_l2seg *factory(devapi_vrf *vrf, vlan_t vlan);
    static void destroy(devapi_l2seg *l2seg);

    sdk_ret_t l2seg_halcreate(void);
    sdk_ret_t l2seg_haldelete(void);
    void deallocate_id(void);

    static devapi_l2seg *lookup(devapi_vrf *vrf, vlan_t vlan);

    sdk_ret_t add_uplink(devapi_uplink *uplink);
    sdk_ret_t del_uplink(devapi_uplink *uplink, bool trigger_hal_update = true);
    sdk_ret_t add_enic(devapi_enic *enic);
    sdk_ret_t del_enic(devapi_enic *enic);

    uint64_t get_id();
    devapi_vrf *get_vrf();
    vlan_t get_vlan() { return vlan_; }
    bool is_single_wire_mgmt() { return single_wire_mgmt_; }
    void set_single_wire_mgmt(bool swm) { single_wire_mgmt_ = swm; }
    devapi_uplink *undesignated_up() { return undesignated_up_; }
    void set_undesignated_up(devapi_uplink *up) { undesignated_up_ = up; }
    uint32_t num_enics() { return enic_refs_.size(); }

    static void probe();
};
}    // namespace iris

using iris::devapi_l2seg;

#endif /* __L2SEG_HPP__ */
