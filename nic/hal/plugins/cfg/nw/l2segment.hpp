//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __L2SEGMENT_HPP__
#define __L2SEGMENT_HPP__

#include "nic/include/base.hpp"
#include "nic/sdk/include/sdk/encap.hpp"
#include "lib/list/list.hpp"
#include "nic/hal/plugins/cfg/nw/vrf.hpp"
#include "lib/ht/ht.hpp"
#include "nic/include/pd.hpp"
#include "nic/hal/src/utils/utils.hpp"

#include "gen/proto/l2segment.pb.h"

// Max. number of uplinks
#define HAL_MAX_UPLINKS 8

using l2segment::L2SegmentDeleteRequest;
using l2segment::MulticastFwdPolicy;
using l2segment::BroadcastFwdPolicy;
using types::L2SegmentType;

using sdk::lib::ht_ctxt_t;
using sdk::lib::dllist_ctxt_t;

using kh::L2SegmentKeyHandle;
using l2segment::L2SegmentSpec;
using l2segment::L2SegmentStatus;
using l2segment::L2SegmentStats;
using l2segment::L2SegmentResponse;
using l2segment::L2SegmentRequestMsg;
using l2segment::L2SegmentResponseMsg;
using l2segment::L2SegmentDeleteRequestMsg;
using l2segment::L2SegmentDeleteResponseMsg;
using l2segment::L2SegmentDeleteResponse;
using l2segment::L2SegmentGetRequest;
using l2segment::L2SegmentGetRequestMsg;
using l2segment::L2SegmentGetResponse;
using l2segment::L2SegmentGetResponseMsg;
using eplearn::EpLearnType;

namespace hal {

typedef struct eplearn_arp_cfg_s {
    bool     enabled;
    uint32_t entry_timeout;
    bool     probe_enabled;
} __PACK__ eplearn_arp_cfg_t;

typedef struct eplearn_dhcp_cfg_s {
    bool        enabled;
    block_list *trusted_servers_list;
} __PACK__  eplearn_dhcp_cfg_t;

typedef struct eplearn_dpkt_cfg_s {
    bool     enabled;
} __PACK__ eplearn_dpkt_cfg_t;

typedef struct eplearn_cfg_s {
    EpLearnType          learn_type;
    eplearn_dhcp_cfg_t   dhcp_cfg;
    eplearn_arp_cfg_t    arp_cfg;
    eplearn_dpkt_cfg_t   dpkt_cfg;
} __PACK__  eplearn_cfg_t;

typedef struct l2seg_s {
    sdk_spinlock_t        slock;                        // lock to protect this structure
    hal_handle_t          vrf_handle;                   // vrf's handle
    l2seg_id_t            seg_id;                       // L2 segment id
    L2SegmentType         segment_type;                 // type of L2 segment
    encap_t               wire_encap;                   // wire encap
    encap_t               tunnel_encap;                 // tunnel encap
    eplearn_cfg_t         eplearn_cfg;                  // eplearn cfg
    MulticastFwdPolicy    mcast_fwd_policy;             // multicast policy
    BroadcastFwdPolicy    bcast_fwd_policy;             // broadcast policy
    ip_addr_t             gipo;                         // gipo for vxlan
    // bool                  is_shared_inband_mgmt;        // vrf is cust or inband
    bool                  single_wire_mgmt;             // vrf is inband
    // bool                  single_wire_mgmt_cust;        // vrf is cust 
    // bool                  have_shared_oifls;            // have shared oifls
    hal_handle_t          other_shared_mgmt_l2seg_hdl[HAL_MAX_UPLINKS];  

    oif_list_id_t         base_oifl_id;                 // Base replication list id
    oif_list_id_t         base_cust_oifl_id[HAL_MAX_UPLINKS]; // Only for customer l2seg
    oif_list_id_t         shared_cust_oifl_id[HAL_MAX_UPLINKS]; // only for attaching from classic l2seg
    hal_handle_t          pinned_uplink;                // pinned uplink

    // operational state of L2 segment
    hal_handle_t          hal_handle;                   // HAL allocated handle
    uint32_t              num_ep;                       // no. of endpoints
    // forward references
    block_list            *nw_list;                     // network list
    block_list            *mbrif_list;                  // interface list
    // back references
    block_list            *acl_list;                    // acl list
    block_list            *if_list;                     // back_iflist

    // Looks like sessions need only if, ep, network
    // dllist_ctxt_t         ep_list_head;            // endpoint list
    // dllist_ctxt_t         session_list_head;       // vrf's L2 segment list link

    // PD state
    void                  *pd;                          // all PD specific state
    bool                   proxy_arp_enabled;

    ht_ctxt_t              uplink_oif_list_ht_ctxt;     // hash table for uplink OIF lists

} __PACK__ l2seg_t;

#define HAL_OIFLIST_BLOCK                   9
#define HAL_BC_MGMT_OIFL_OFFSET             0
#define HAL_MC_MGMT_OIFL_OFFSET             1
#define HAL_PR_MGMT_OIFL_OFFSET             2
#define HAL_BC_MSEG_BM_OIFL_OFFSET          3
#define HAL_MC_MSEG_BM_OIFL_OFFSET          4
#define HAL_PR_MSEG_BM_OIFL_OFFSET          5
#define HAL_BC_MGMT_MSEG_BM_OIFL_OFFSET     6
#define HAL_MC_MGMT_MSEG_BM_OIFL_OFFSET     7
#define HAL_PR_MGMT_MSEG_BM_OIFL_OFFSET     8

#define HAL_SHARED_OIFLIST_BLOCK                   3
#define HAL_SHARED_BC_MSEG_BM_OIFL_OFFSET          0
#define HAL_SHARED_MC_MSEG_BM_OIFL_OFFSET          1
#define HAL_SHARED_PR_MSEG_BM_OIFL_OFFSET          2

// cb data structures
typedef struct l2seg_create_app_ctxt_s {
    vrf_t    *vrf;
} __PACK__ l2seg_create_app_ctxt_t;

typedef struct l2seg_update_app_ctxt_s {
    bool                l2seg_change;               // global change
    bool                mcast_fwd_policy_change;
    bool                bcast_fwd_policy_change;
    bool                nwlist_change;
    bool                iflist_change;
    bool                swm_change;
    bool                shared_mgmt_change;

    MulticastFwdPolicy  new_mcast_fwd_policy;
    BroadcastFwdPolicy  new_bcast_fwd_policy;
    // nw list change
    block_list          *add_nwlist;
    block_list          *del_nwlist;
    block_list          *aggr_nwlist;
    // mbr ifs change
    block_list          *add_iflist;
    block_list          *del_iflist;
    block_list          *agg_iflist;
    // swm change
    bool                new_single_wire_mgmt;
    bool                new_shared_mgmt;

} __PACK__ l2seg_update_app_ctxt_t;

// max. number of L2 segments supported  (TODO: we can take this from cfg file)
#define HAL_MAX_L2SEGMENTS                           2048

static inline void
l2seg_lock (l2seg_t *l2seg, const char *fname, int lineno, const char *fxname)
{
    HAL_TRACE_DEBUG("{}:operlock:locking l2seg:{} from {}:{}:{}",
                    __FUNCTION__, l2seg->seg_id,
                    fname, lineno, fxname);
    SDK_SPINLOCK_LOCK(&l2seg->slock);
}

static inline void
l2seg_unlock (l2seg_t *l2seg, const char *fname, int lineno, const char *fxname)
{
    HAL_TRACE_DEBUG("{}:operlock:unlocking l2seg:{} from {}:{}:{}",
                    __FUNCTION__, l2seg->seg_id,
                    fname, lineno, fxname);
    SDK_SPINLOCK_UNLOCK(&l2seg->slock);
}

static inline bool
is_l2seg_wire_encap_vxlan (l2seg_t *l2seg)
{
    return (l2seg->wire_encap.type == types::ENCAP_TYPE_VXLAN);
}

extern void *l2seg_id_get_key_func(void *entry);
extern uint32_t l2seg_id_key_size(void);
l2seg_t *find_l2seg_by_id(l2seg_id_t l2seg_id);
l2seg_t *find_l2seg_by_wire_encap(encap_t encap, types::VrfType vrf_type,
                                  hal_handle_t designated_uplink_hdl);
l2seg_t *l2seg_lookup_by_handle(hal_handle_t handle);
l2seg_t *l2seg_lookup_key_or_handle(const L2SegmentKeyHandle& kh);
const char *l2seg_spec_keyhandle_to_str(const L2SegmentKeyHandle& key_handle);
const char *l2seg_keyhandle_to_str(l2seg_t *l2seg);

// SVC CRUD APIs
hal_ret_t l2segment_create(L2SegmentSpec& spec,
                           L2SegmentResponse *rsp);
hal_ret_t l2segment_update(L2SegmentSpec& spec,
                           L2SegmentResponse *rsp);
hal_ret_t l2segment_delete(L2SegmentDeleteRequest& req,
                           L2SegmentDeleteResponse *rsp);
hal_ret_t l2segment_get(l2segment::L2SegmentGetRequest& req,
                        l2segment::L2SegmentGetResponseMsg *rsp);

hal_ret_t l2seg_store_cb(void *obj, uint8_t *mem,
                          uint32_t len, uint32_t *mlen);
uint32_t l2seg_restore_cb(void *obj, uint32_t len);

typedef struct l2_seg_uplink_oif_list_key_s {
    hal_handle_t l2seg_handle;
    hal_handle_t uplink_handle;
} __PACK__ l2_seg_uplink_oif_list_key_t;

typedef struct l2_seg_uplink_oif_list_s {
    l2_seg_uplink_oif_list_key_t key;
    oif_list_id_t                oif_list_id;
    ht_ctxt_t                    ht_ctxt;
} __PACK__ l2_seg_uplink_oif_list_t;

void *l2seg_uplink_oif_get_key_func(void *entry);
uint32_t l2seg_uplink_oif_key_size(void);
bool l2seg_is_mbr_if(l2seg_t *l2seg, if_id_t if_id);
bool l2seg_is_oob_mgmt(l2seg_t *l2seg);
bool l2seg_is_inband_mgmt(l2seg_t *l2seg);
bool l2seg_is_mgmt(l2seg_t *l2seg);
bool l2seg_is_cust(l2seg_t *l2seg);
bool l2seg_is_telemetry(l2seg_t *l2seg);
hal_ret_t l2seg_select_pinned_uplink(l2seg_t *l2seg);
hal_ret_t l2seg_handle_repin(l2seg_t *l2seg);
hal_ret_t l2seg_attach_mgmt(l2seg_t *l2seg);
hal_ret_t l2seg_detach_mgmt_oifls(l2seg_t *l2seg);
void l2seg_print_attached_l2segs(l2seg_t *l2seg);
hal_ret_t l2seg_oifl_set_hi(l2seg_t *l2seg, oif_list_id_t base_oifl_id);
hal_ret_t l2seg_oifl_clear_hi(l2seg_t *l2seg, oif_list_id_t base_oifl_id);

oif_list_id_t l2seg_bc_mgmt_oifl(oif_list_id_t id);
oif_list_id_t l2seg_mc_mgmt_oifl(oif_list_id_t id);
oif_list_id_t l2seg_pr_mgmt_oifl(oif_list_id_t id);
oif_list_id_t l2seg_bc_mseg_bm_oifl(oif_list_id_t id);
oif_list_id_t l2seg_mc_mseg_bm_oifl(oif_list_id_t id);
oif_list_id_t l2seg_pr_mseg_bm_oifl(oif_list_id_t id);
oif_list_id_t l2seg_bc_mgmt_mseg_bm_oifl(oif_list_id_t id);
oif_list_id_t l2seg_mc_mgmt_mseg_bm_oifl(oif_list_id_t id);
oif_list_id_t l2seg_pr_mgmt_mseg_bm_oifl(oif_list_id_t id);
oif_list_id_t l2seg_shared_bc_mseg_bm_oifl(oif_list_id_t id);
oif_list_id_t l2seg_shared_mc_mseg_bm_oifl(oif_list_id_t id);
oif_list_id_t l2seg_shared_pr_mseg_bm_oifl(oif_list_id_t id);

}    // namespace hal

#endif    // __L2SEGMENT_HPP__

