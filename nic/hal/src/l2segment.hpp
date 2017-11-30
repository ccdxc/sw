#ifndef __L2SEGMENT_HPP__
#define __L2SEGMENT_HPP__

#include "nic/include/base.h"
#include "nic/include/encap.hpp"
#include "nic/include/list.hpp"
#include "nic/utils/ht/ht.hpp"
#include "nic/hal/src/vrf.hpp"
#include "nic/gen/proto/hal/l2segment.pb.h"
#include "nic/include/pd.hpp"
#include "nic/hal/src/utils.hpp"

using l2segment::L2SegmentDeleteRequest;
using l2segment::MulticastFwdPolicy;
using l2segment::BroadcastFwdPolicy;
using types::L2SegmentType;

using hal::utils::ht_ctxt_t;
using hal::utils::dllist_ctxt_t;

using kh::L2SegmentKeyHandle;
using l2segment::L2SegmentSpec;
using l2segment::L2SegmentStatus;
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

namespace hal {

typedef struct l2seg_s {
    hal_spinlock_t        slock;                   // lock to protect this structure
    hal_handle_t          vrf_handle;           // vrf's handle
    // vrf_id_t           vrf_id;               // vrf this L2 segment belongs to
    l2seg_id_t            seg_id;                  // L2 segment id
    L2SegmentType         segment_type;            // type of L2 segment
    encap_t               wire_encap;              // wire encap
    encap_t               tunnel_encap;            // tunnel encap
    MulticastFwdPolicy    mcast_fwd_policy;        // multicast policy
    BroadcastFwdPolicy    bcast_fwd_policy;        // broadcast policy
    ip_addr_t             gipo;                    // gipo for vxlan 

    oif_list_id_t         bcast_oif_list;          // outgoing interface list for broadcast/flood
    hal_handle_t          pinned_uplink;           // pinned uplink

    // operational state of L2 segment
    hal_handle_t          hal_handle;              // HAL allocated handle
    uint32_t              num_ep;                  // no. of endpoints
    // forward references
    block_list            *nw_list;                // network list 
    // dllist_ctxt_t         nw_list_head;            // network list
    // back references
    block_list            *if_list;                // interface list  

    // Looks like sessions need only if, ep, network
    // dllist_ctxt_t         ep_list_head;            // endpoint list
    // dllist_ctxt_t         session_list_head;       // vrf's L2 segment list link

    // PD state
    void                  *pd;                     // all PD specific state

} __PACK__ l2seg_t;

// CB data structures
typedef struct l2seg_create_app_ctxt_s {
    vrf_t    *vrf;
} __PACK__ l2seg_create_app_ctxt_t;

typedef struct l2seg_update_app_ctxt_s {
    bool                l2seg_change;               // global change
    bool                mcast_fwd_policy_change;
    bool                bcast_fwd_policy_change;
    bool                nwlist_change;

    MulticastFwdPolicy  new_mcast_fwd_policy;
    BroadcastFwdPolicy  new_bcast_fwd_policy;
    // nw list change
    block_list          *add_nwlist;
    block_list          *del_nwlist;
    block_list          *aggr_nwlist;
} __PACK__ l2seg_update_app_ctxt_t;

// max. number of L2 segments supported  (TODO: we can take this from cfg file)
#define HAL_MAX_L2SEGMENTS                           2048

static inline void 
l2seg_lock(l2seg_t *l2seg, const char *fname, int lineno, const char *fxname)
{
    HAL_TRACE_DEBUG("{}:operlock:locking l2seg:{} from {}:{}:{}", 
                    __FUNCTION__, l2seg->seg_id,
                    fname, lineno, fxname);
    HAL_SPINLOCK_LOCK(&l2seg->slock);
}

static inline void 
l2seg_unlock(l2seg_t *l2seg, const char *fname, int lineno, const char *fxname)
{
    HAL_TRACE_DEBUG("{}:operlock:unlocking l2seg:{} from {}:{}:{}", 
                    __FUNCTION__, l2seg->seg_id,
                    fname, lineno, fxname);
    HAL_SPINLOCK_UNLOCK(&l2seg->slock);
}

// allocate a l2segment instance
static inline l2seg_t *
l2seg_alloc (void)
{
    l2seg_t    *l2seg;

    l2seg = (l2seg_t *)g_hal_state->l2seg_slab()->alloc();
    if (l2seg == NULL) {
        return NULL;
    }
    return l2seg;
}

// initialize a l2segment instance
static inline l2seg_t *
l2seg_init (l2seg_t *l2seg)
{
    if (!l2seg) {
        return NULL;
    }
    HAL_SPINLOCK_INIT(&l2seg->slock, PTHREAD_PROCESS_PRIVATE);
    l2seg->vrf_handle       = 0;
    l2seg->seg_id           = 0;
    l2seg->segment_type     = types::L2_SEGMENT_TYPE_NONE;
    l2seg->mcast_fwd_policy = l2segment::MULTICAST_FWD_POLICY_NONE;
    l2seg->bcast_fwd_policy = l2segment::BROADCAST_FWD_POLICY_NONE;

    // initialize the operational state
    l2seg->num_ep = 0;
    l2seg->pd     = NULL;

    // initialize meta information
    l2seg->if_list = block_list::factory(sizeof(hal_handle_t));
    l2seg->nw_list = block_list::factory(sizeof(hal_handle_t));

    return l2seg;
}

// allocate and initialize a l2segment instance
static inline l2seg_t *
l2seg_alloc_init (void)
{
    return l2seg_init(l2seg_alloc());
}

// anti l2seg_alloc
static inline hal_ret_t
l2seg_free (l2seg_t *l2seg)
{
    HAL_SPINLOCK_DESTROY(&l2seg->slock);
    g_hal_state->l2seg_slab()->free(l2seg);
    return HAL_RET_OK;
}

// anti l2seg_alloc_init
static inline hal_ret_t
l2seg_cleanup (l2seg_t *l2seg)
{
    if (l2seg->nw_list) {
        block_list::destroy(l2seg->nw_list);
    }
    if (l2seg->if_list) {
        block_list::destroy(l2seg->if_list);
    }
    l2seg_free(l2seg);
    return HAL_RET_OK;
}

static inline l2seg_t *
find_l2seg_by_id (l2seg_id_t l2seg_id)
{
    hal_handle_id_ht_entry_t    *entry;
    l2seg_t                     *l2seg;

    entry = (hal_handle_id_ht_entry_t *)g_hal_state->
        l2seg_id_ht()->lookup(&l2seg_id);
    if (entry) {
        // check for object type
        HAL_ASSERT(hal_handle_get_from_handle_id(entry->handle_id)->obj_id() == 
                   HAL_OBJ_ID_L2SEG);
        l2seg = (l2seg_t *)hal_handle_get_obj(entry->handle_id);
        return l2seg;
    }
    return NULL;
}

static inline l2seg_t *
find_l2seg_by_handle (hal_handle_t handle)
{
    auto hal_handle = hal_handle_get_from_handle_id(handle);
    if (!hal_handle) {
        HAL_TRACE_DEBUG("{}:failed to find object with handle:{}",
                        __FUNCTION__, handle);
        return NULL;
    }
    if (hal_handle->obj_id() != HAL_OBJ_ID_L2SEG) {
        HAL_TRACE_DEBUG("{}:failed to find l2seg with handle:{}",
                        __FUNCTION__, handle);
        return NULL;
    }
   return (l2seg_t *)hal_handle_get_obj(handle); 
#if 0
    // check for object type
    HAL_ASSERT(hal_handle_get_from_handle_id(handle)->obj_id() == 
               HAL_OBJ_ID_L2SEG);
    return (l2seg_t *)hal_handle_get_obj(handle);
#endif
}

static inline bool
is_l2seg_wire_encap_vxlan (l2seg_t *l2seg)
{
    return (l2seg->wire_encap.type == types::ENCAP_TYPE_VXLAN);
}

extern void *l2seg_id_get_key_func(void *entry);
extern uint32_t l2seg_id_compute_hash_func(void *key, uint32_t ht_size);
extern bool l2seg_id_compare_key_func(void *key1, void *key2);
l2seg_t *l2seg_lookup_key_or_handle (const L2SegmentKeyHandle& kh);

// SVC CRUD APIs
hal_ret_t l2segment_create(L2SegmentSpec& spec,
                           L2SegmentResponse *rsp);
hal_ret_t l2segment_update(L2SegmentSpec& spec,
                           L2SegmentResponse *rsp);
hal_ret_t l2segment_delete(L2SegmentDeleteRequest& req,
                           L2SegmentDeleteResponse *rsp);
hal_ret_t l2segment_get(l2segment::L2SegmentGetRequest& req,
                        l2segment::L2SegmentGetResponse *rsp);

}    // namespace hal

#endif    // __L2SEGMENT_HPP__

