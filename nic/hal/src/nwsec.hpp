// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __NWSEC_HPP__
#define __NWSEC_HPP__

#include "nic/include/base.h"
#include "nic/include/hal_state.hpp"
#include "sdk/ht.hpp"
#include "nic/gen/proto/hal/nwsec.pb.h"
#include "nic/gen/proto/hal/kh.pb.h"
#include "nic/include/pd.hpp"

using sdk::lib::ht_ctxt_t;

using kh::SecurityProfileKeyHandle;
using nwsec::SecurityProfileSpec;
using nwsec::SecurityProfileRequestMsg;
using nwsec::SecurityProfileStatus;
using nwsec::SecurityProfileResponse;
using nwsec::SecurityProfileResponseMsg;
using nwsec::SecurityProfileDeleteRequest;
using nwsec::SecurityProfileDeleteResponse;
using nwsec::SecurityProfileDeleteRequestMsg;
using nwsec::SecurityProfileDeleteResponseMsg;
using nwsec::SecurityProfileGetRequest;
using nwsec::SecurityProfileGetRequestMsg;
using nwsec::SecurityProfileStats;
using nwsec::SecurityProfileGetResponse;
using nwsec::SecurityProfileGetResponseMsg;

namespace hal {

typedef struct nwsec_profile_s {
    hal_spinlock_t        slock;                  // lock to protect this structure
    nwsec_profile_id_t    profile_id;             // profile id
    uint32_t              cnxn_tracking_en:1;
    uint32_t              ipsg_en:1;
    uint32_t              tcp_rtt_estimate_en:1;
    uint32_t              session_idle_timeout;
    uint32_t              tcp_cnxn_setup_timeout;
    uint32_t              tcp_close_timeout;
    uint32_t              tcp_close_wait_timeout;

    uint32_t              ip_normalization_en:1;
    uint32_t              tcp_normalization_en:1;
    uint32_t              icmp_normalization_en:1;

    uint32_t              ip_ttl_change_detect_en:1;
    uint32_t              ip_rsvd_flags_action:2;
    uint32_t              ip_df_action:2;
    uint32_t              ip_options_action:2;
    uint32_t              ip_invalid_len_action:2;
    uint32_t              ip_normalize_ttl:8;

    uint32_t              icmp_redirect_msg_drop:1;
    uint32_t              icmp_deprecated_msgs_drop:1;
    uint32_t              icmp_invalid_code_action:2;

    uint32_t              tcp_non_syn_first_pkt_drop:1;
    uint32_t              tcp_split_handshake_drop:1;
    uint32_t              tcp_rsvd_flags_action:2;
    uint32_t              tcp_unexpected_mss_action:2;
    uint32_t              tcp_unexpected_win_scale_action:2;
    uint32_t              tcp_unexpected_sack_perm_action:2;
    uint32_t              tcp_urg_ptr_not_set_action:2;
    uint32_t              tcp_urg_flag_not_set_action:2;
    uint32_t              tcp_urg_payload_missing_action:2;
    uint32_t              tcp_rst_with_data_action:2;
    uint32_t              tcp_data_len_gt_mss_action:2;
    uint32_t              tcp_data_len_gt_win_size_action:2;
    uint32_t              tcp_unexpected_ts_option_action:2;
    uint32_t              tcp_unexpected_sack_option_action:2;
    uint32_t              tcp_unexpected_echo_ts_action:2;
    uint32_t              tcp_ts_not_present_drop:1;
    uint32_t              tcp_invalid_flags_drop:1;
    uint32_t              tcp_nonsyn_noack_drop:1;

    hal_handle_t          hal_handle;             // HAL allocated handle

    // operational state of nwsec profile
    dllist_ctxt_t         vrf_list_head;

    // PD state
    void                  *pd;                    // all PD specific state
} __PACK__ nwsec_profile_t;

typedef struct nwsec_create_app_ctxt_s {
} __PACK__ nwsec_create_app_ctxt_t;

typedef struct nwsec_update_app_ctxt_s {
    bool        ipsg_changed;                       // ipsg changed
    bool        nwsec_changed;                      // Any field changed

    // valid for ipsg_changed
    // uint32_t    ipsg_en:1;                          // new ipsg_en value

    // valid for any change
    SecurityProfileSpec *spec;
} __PACK__ nwsec_update_app_ctxt_t;

// max. number of security profiles supported  (TODO: we can take this from cfg file)
#define HAL_MAX_NWSEC_PROFILES                       256

static inline void 
nwsec_profile_lock(nwsec_profile_t *nwsec_profile, const char *fname, 
                   int lineno, const char *fxname)
{
    HAL_TRACE_DEBUG("{}:operlock:locking nwsec_profile:{} from {}:{}:{}", 
                    __FUNCTION__, nwsec_profile->profile_id,
                    fname, lineno, fxname);
    HAL_SPINLOCK_LOCK(&nwsec_profile->slock);
}

static inline void 
nwsec_profile_unlock(nwsec_profile_t *nwsec_profile, const char *fname, 
                     int lineno, const char *fxname)
{
    HAL_TRACE_DEBUG("{}:operlock:unlocking nwsec_profile:{} from {}:{}:{}", 
                    __FUNCTION__, nwsec_profile->profile_id,
                    fname, lineno, fxname);
    HAL_SPINLOCK_UNLOCK(&nwsec_profile->slock);
}

// allocate a security profile instance
static inline nwsec_profile_t *
nwsec_profile_alloc (void)
{
    nwsec_profile_t    *sec_prof;

    sec_prof = (nwsec_profile_t *)g_hal_state->nwsec_profile_slab()->alloc();
    if (sec_prof == NULL) {
        return NULL;
    }
    return sec_prof;
}

// initialize a security profile instance
static inline nwsec_profile_t *
nwsec_profile_init (nwsec_profile_t *sec_prof)
{
    if (!sec_prof) {
        return NULL;
    }
    memset(sec_prof, 0, sizeof(nwsec_profile_t));


    HAL_SPINLOCK_INIT(&sec_prof->slock, PTHREAD_PROCESS_PRIVATE);

    // initialize the operational state
    utils::dllist_reset(&sec_prof->vrf_list_head);

    return sec_prof;
}

// allocate and initialize a security profile instance
static inline nwsec_profile_t *
nwsec_profile_alloc_init (void)
{
    return nwsec_profile_init(nwsec_profile_alloc());
}

// free security profile instance
static inline hal_ret_t
nwsec_profile_free (nwsec_profile_t *sec_prof)
{
    HAL_SPINLOCK_DESTROY(&sec_prof->slock);
    g_hal_state->nwsec_profile_slab()->free(sec_prof);
    return HAL_RET_OK;
}

// find a security profile instance by its id
static inline nwsec_profile_t *
find_nwsec_profile_by_id (nwsec_profile_id_t profile_id)
{
    // return (nwsec_profile_t *)g_hal_state->nwsec_profile_id_ht()->lookup(&profile_id);
    hal_handle_id_ht_entry_t    *entry;
    nwsec_profile_t             *sec_prof;

    entry = (hal_handle_id_ht_entry_t *)g_hal_state->
        nwsec_profile_id_ht()->lookup(&profile_id);
    if (entry) {
        // check for object type
        HAL_ASSERT(hal_handle_get_from_handle_id(entry->handle_id)->obj_id() == 
                   HAL_OBJ_ID_SECURITY_PROFILE);

        sec_prof = (nwsec_profile_t *)hal_handle_get_obj(entry->handle_id);
        return sec_prof;
    }
    return NULL;
}

// find a security profile instance by its handle
static inline nwsec_profile_t *
find_nwsec_profile_by_handle (hal_handle_t handle)
{
    // return (nwsec_profile_t *)g_hal_state->nwsec_profile_hal_handle_ht()->lookup(&handle);
    // check for object type
    auto hal_handle = hal_handle_get_from_handle_id(handle);
    if (!hal_handle) {
        HAL_TRACE_DEBUG("{}:failed to find object with handle:{}",
                        __FUNCTION__, handle);
        return NULL;
    }
    if (hal_handle->obj_id() != HAL_OBJ_ID_SECURITY_PROFILE) {
        HAL_TRACE_DEBUG("{}:failed to find nwsec profile with handle:{}",
                        __FUNCTION__, handle);
        return NULL;
    }
    // HAL_ASSERT(hal_handle->obj_id() == 
    //           HAL_OBJ_ID_SECURITY_PROFILE);
    return (nwsec_profile_t *)hal_handle_get_obj(handle); 
}

extern void *nwsec_profile_id_get_key_func(void *entry);
extern uint32_t nwsec_profile_id_compute_hash_func(void *key, uint32_t ht_size);
extern bool nwsec_profile_id_compare_key_func(void *key1, void *key2);
nwsec_profile_t *nwsec_lookup_key_or_handle (const SecurityProfileKeyHandle& kh);

// Forward declarations
hal_ret_t security_profile_create(nwsec::SecurityProfileSpec& spec,
                                  nwsec::SecurityProfileResponse *rsp);

hal_ret_t security_profile_update(nwsec::SecurityProfileSpec& spec,
                                  nwsec::SecurityProfileResponse *rsp);

hal_ret_t security_profile_delete(nwsec::SecurityProfileDeleteRequest& req,
                                  nwsec::SecurityProfileDeleteResponse *rsp);

hal_ret_t security_profile_get(nwsec::SecurityProfileGetRequest& req,
                               nwsec::SecurityProfileGetResponse *rsp);

}    // namespace hal

#endif    // __NWSEC_HPP__

