//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __VRF_HPP__
#define __VRF_HPP__

#include "nic/include/base.hpp"
#include "lib/list/list.hpp"
#include "lib/ht/ht.hpp"
#include "nic/utils/block_list/block_list.hpp"
#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/include/pd.hpp"
#include "nic/include/hal.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/hal/plugins/sfw/cfg/nwsec.hpp"

#include "gen/proto/vrf.pb.h"
#include "gen/proto/kh.pb.h"

using sdk::lib::ht_ctxt_t;
using sdk::lib::dllist_ctxt_t;
using hal::utils::block_list;
using kh::VrfKeyHandle;

using vrf::VrfSpec;
using vrf::VrfStatus;
using vrf::VrfStats;
using vrf::VrfResponse;
using vrf::VrfRequestMsg;
using vrf::VrfResponseMsg;
using vrf::VrfDeleteRequest;
using vrf::VrfDeleteResponse;
using vrf::VrfDeleteRequestMsg;
using vrf::VrfDeleteResponseMsg;
using vrf::VrfGetRequest;
using vrf::VrfGetRequestMsg;
using vrf::VrfGetResponse;
using vrf::VrfGetResponseMsg;

namespace hal {

typedef struct vrf_s {
    sdk_spinlock_t     slock;                // lock to protect this structure
    types::VrfType     vrf_type;             // type of the vrf
    vrf_id_t           vrf_id;               // app provided vrf id
    hal_handle_t       nwsec_profile_handle; // security profile handle
    ip_prefix_t        gipo_prefix;          // the prefix to terminate gipo
    ip_addr_t          mytep_ip;             // mytep address for this VRF
    hal_handle_t       designated_uplink;    // classic: isolated to this upl.

    // operational state of vrf
    hal_handle_t       hal_handle;           // HAL allocated handle

    // vrf stats
    uint32_t           num_l2seg;            // no. of L2 segments
    uint32_t           num_sg;               // no. of security groups
    uint32_t           num_l4lb_svc;         // no. of L4 LB services
    uint32_t           num_ep;               // no. of endpoints

    // back references
    block_list         *l2seg_list;          // L2 segment list
    block_list         *acl_list;            // acl list
    block_list         *route_list;          // route list

    // PD state
    void               *pd;                  // all PD specific state
} __PACK__ vrf_t;

typedef struct vrf_create_app_ctxt_s {
    nwsec_profile_t    *sec_prof;
} __PACK__ vrf_create_app_ctxt_t;

typedef struct vrf_update_app_ctxt_s {
    bool                nwsec_prof_change;
    bool                gipo_prefix_change;

    // valid for nwsec_prof_change
    hal_handle_t        nwsec_profile_handle;   // new profile handle
    nwsec_profile_t     *nwsec_prof;            // new nwsec profile
    // valid for gipo change
    ip_prefix_t         new_gipo_prefix;        // new gipo prefix
} __PACK__ vrf_update_app_ctxt_t;

// max. number of VRFs supported  (TODO: we can take this from cfg file)
#define HAL_MAX_VRFS                                 256

static inline void
vrf_lock (vrf_t *vrf, const char *fname, int lineno, const char *fxname)
{
    SDK_SPINLOCK_LOCK(&vrf->slock);
}

static inline void
vrf_unlock (vrf_t *vrf, const char *fname, int lineno, const char *fxname)
{
    SDK_SPINLOCK_UNLOCK(&vrf->slock);
}

//------------------------------------------------------------------------------
// find a vrf instance by its handle
//------------------------------------------------------------------------------
static inline vrf_t *
vrf_lookup_by_handle (hal_handle_t handle)
{
    if (handle == HAL_HANDLE_INVALID) {
        return NULL;
    }

    auto hal_handle = hal_handle_get_from_handle_id(handle);
    if (!hal_handle) {
        HAL_TRACE_DEBUG("Failed to find object with handle {}", handle);
        return NULL;
    }
    if (hal_handle->obj_id() != HAL_OBJ_ID_VRF) {
        HAL_TRACE_DEBUG("Failed to find vrf with handle {}", handle);
        return NULL;
    }
    return (vrf_t *)hal_handle->obj();
}

void *vrf_id_get_key_func(void *entry);
uint32_t vrf_id_key_size(void);
vrf_t *vrf_lookup_by_id(vrf_id_t tid);
vrf_t *vrf_lookup_key_or_handle(const VrfKeyHandle& kh);
const char *vrf_spec_keyhandle_to_str(const VrfKeyHandle& key_handle);
const char *vrf_keyhandle_to_str(vrf_t *vrf);
bool vrf_is_mgmt(vrf_t *vrf);


hal_ret_t vrf_create(VrfSpec& spec, VrfResponse *rsp);
hal_ret_t vrf_update(VrfSpec& spec, VrfResponse *rsp);
hal_ret_t vrf_delete(VrfDeleteRequest& req, VrfDeleteResponse *rsp);
hal_ret_t vrf_get(VrfGetRequest& req, VrfGetResponseMsg *rsp);

hal_ret_t vrf_store_cb(void *obj, uint8_t *mem, uint32_t len, uint32_t *mlen);
uint32_t vrf_restore_cb(void *obj, uint32_t len);

}    // namespace hal

#endif    // __VRF_HPP__

