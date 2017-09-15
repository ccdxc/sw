#ifndef __L2SEGMENT_HPP__
#define __L2SEGMENT_HPP__

#include <base.h>
#include <encap.hpp>
#include <list.hpp>
#include <ht.hpp>
#include <tenant.hpp>
#include <l2segment.pb.h>
#include <pd.hpp>

using l2segment::MulticastFwdPolicy;
using l2segment::BroadcastFwdPolicy;
using types::L2SegmentType;

using hal::utils::ht_ctxt_t;
using hal::utils::dllist_ctxt_t;

namespace hal {

typedef uint32_t l2seg_id_t;

typedef struct l2seg_s {
    hal_spinlock_t        slock;                   // lock to protect this structure
    tenant_id_t           tenant_id;               // tenant this L2 segment belongs to
    l2seg_id_t            seg_id;                  // L2 segment id
    L2SegmentType         segment_type;            // type of L2 segment
    // hal_handle_t          nw_handle;               // network this belongs to
    encap_t               access_encap;            // encap within the host
    encap_t               fabric_encap;            // encap on the wire
    MulticastFwdPolicy    mcast_fwd_policy;        // multicast policy
    BroadcastFwdPolicy    bcast_fwd_policy;        // broadcast policy

    oif_list_id_t         bcast_oif_list;          // outgoing interface list for broadcast/flood

    // operational state of L2 segment
    hal_handle_t          hal_handle;              // HAL allocated handle
    uint32_t              num_ep;                  // no. of endpoints

    // PD state
    void                  *pd;                     // all PD specific state

    // meta data maintained for tenant
    ht_ctxt_t             ht_ctxt;                 // segment id based hash table ctxt
    ht_ctxt_t             hal_handle_ht_ctxt;      // hal handle based hash table ctxt
    dllist_ctxt_t         nw_list_head;            // network list
    dllist_ctxt_t         ep_list_head;            // endpoint list
    dllist_ctxt_t         tenant_l2seg_lentry;     // tenant's L2 segment list link
    dllist_ctxt_t         session_list_head;       // tenant's L2 segment list link
} __PACK__ l2seg_t;

// max. number of L2 segments supported  (TODO: we can take this from cfg file)
#define HAL_MAX_L2SEGMENTS                           2048

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

    // initialize the operational state
    l2seg->num_ep = 0;
    l2seg->pd = NULL;

    // initialize meta information
    l2seg->ht_ctxt.reset();
    l2seg->hal_handle_ht_ctxt.reset();
    utils::dllist_reset(&l2seg->nw_list_head);
    utils::dllist_reset(&l2seg->ep_list_head);
    utils::dllist_reset(&l2seg->tenant_l2seg_lentry);
    utils::dllist_reset(&l2seg->session_list_head);

    return l2seg;
}

// allocate and initialize a l2segment instance
static inline l2seg_t *
l2seg_alloc_init (void)
{
    return l2seg_init(l2seg_alloc());
}

static inline hal_ret_t
l2seg_free (l2seg_t *l2seg)
{
    HAL_SPINLOCK_DESTROY(&l2seg->slock);
    g_hal_state->l2seg_slab()->free(l2seg);
    return HAL_RET_OK;
}

static inline l2seg_t *
find_l2seg_by_id (l2seg_id_t l2seg_id)
{
    return (l2seg_t *)g_hal_state->l2seg_id_ht()->lookup(&l2seg_id);
}

static inline l2seg_t *
find_l2seg_by_handle (hal_handle_t handle)
{
    return (l2seg_t *)g_hal_state->l2seg_hal_handle_ht()->lookup(&handle);
}

static inline bool
is_l2seg_fabric_encap_vxlan (l2seg_t *l2seg)
{
    return (l2seg->fabric_encap.type == types::ENCAP_TYPE_VXLAN);
}

extern void *l2seg_get_key_func(void *entry);
extern uint32_t l2seg_compute_hash_func(void *key, uint32_t ht_size);
extern bool l2seg_compare_key_func(void *key1, void *key2);

extern void *l2seg_get_handle_key_func(void *entry);
extern uint32_t l2seg_compute_handle_hash_func(void *key, uint32_t ht_size);
extern bool l2seg_compare_handle_key_func(void *key1, void *key2);

hal_ret_t l2segment_create(l2segment::L2SegmentSpec& spec,
                           l2segment::L2SegmentResponse *rsp);

hal_ret_t l2segment_update(l2segment::L2SegmentSpec& spec,
                           l2segment::L2SegmentResponse *rsp);

hal_ret_t l2segment_get(l2segment::L2SegmentGetRequest& req,
                        l2segment::L2SegmentGetResponse *rsp);
}    // namespace hal

#endif    // __L2SEGMENT_HPP__

