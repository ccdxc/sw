//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __ENIC_HPP__
#define __ENIC_HPP__

#include "lib/indexer/indexer.hpp"
#include "devapi_types.hpp"
#include "devapi_object.hpp"

namespace iris {

class devapi_l2seg;
class devapi_lif;
typedef struct l2seg_info_s {
    devapi_l2seg *l2seg;
    uint32_t filter_ref_cnt;
} l2seg_info_t;

class devapi_enic : public devapi_object {
private:
    uint32_t id_;
    mac_t mac_;
    vlan_t vlan_;
    devapi_lif *lif_;
    bool skip_native_vlan_;
    std::map<vlan_t, l2seg_info_t *> l2seg_refs_;
    static indexer *allocator_;
    static constexpr uint32_t max_enics = 4096;

private:
  sdk_ret_t init_(devapi_lif *lif);
  devapi_enic() {}
  ~devapi_enic() {}
  sdk_ret_t trigger_halupdate(void);
  void deallocate_id(void);
public:
    static devapi_enic *factory(devapi_lif *lif);
    static void destroy(devapi_enic *enic);

    sdk_ret_t enic_halcreate(void);
    sdk_ret_t enic_haldelete(void);

    // Classic enic APIs only
    sdk_ret_t add_vlan(vlan_t vlan);
    void del_vlan(vlan_t vlan, bool skip_vlan = false);

    uint32_t get_id(void);
    devapi_l2seg *get_l2seg(vlan_t vlan);

    sdk_ret_t set_skip_native_vlan(bool skip);
    bool get_skip_native_vlan(void) { return skip_native_vlan_; }
};

}     // namespace iris

using iris::devapi_enic;

#endif /* __ENIC_HPP__ */
