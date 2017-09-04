#ifndef __NWSEC_HPP__
#define __NWSEC_HPP__

#include <base.h>
#include <hal_state.hpp>
#include <ht.hpp>
#include <nwsec.pb.h>
#include <pd.hpp>

using hal::utils::ht_ctxt_t;

namespace hal {

typedef uint32_t nwsec_profile_id_t;

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
    uint32_t              tcp_syncookie_en:1;
    uint32_t              tcp_split_handshake_detect_en:1;
    uint32_t              tcp_split_handshake_drop:1;
    uint32_t              tcp_rsvd_flags_action:2;
    uint32_t              tcp_unexpected_mss_action:2;
    uint32_t              tcp_unexpected_win_scale_action:2;
    uint32_t              tcp_urg_ptr_not_set_action:2;
    uint32_t              tcp_urg_flag_not_set_action:2;
    uint32_t              tcp_urg_payload_missing_action:2;
    uint32_t              tcp_rst_with_data_action:2;
    uint32_t              tcp_data_len_gt_mss_action:2;
    uint32_t              tcp_data_len_gt_win_size_action:2;
    uint32_t              tcp_unexpected_ts_option_action:2;
    uint32_t              tcp_unexpected_echo_ts_action:2;
    uint32_t              tcp_ts_not_present_drop:1;
    uint32_t              tcp_invalid_flags_drop:1;
    uint32_t              tcp_nonsyn_noack_drop:1;

    // operational state of L2 segment
    hal_handle_t          hal_handle;             // HAL allocated handle

    // PD state
    void                  *pd;                    // all PD specific state

    // meta data maintained for tenant
    ht_ctxt_t             ht_ctxt;                // profile id based hash table ctxt
    ht_ctxt_t             hal_handle_ht_ctxt;     // hal handle based hash table ctxt
} __PACK__ nwsec_profile_t;

// max. number of security profiles supported  (TODO: we can take this from cfg file)
#define HAL_MAX_NWSEC_PROFILES                       256

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
    HAL_SPINLOCK_INIT(&sec_prof->slock, PTHREAD_PROCESS_PRIVATE);

    // initialize the operational state

    // initialize meta information
    sec_prof->ht_ctxt.reset();
    sec_prof->hal_handle_ht_ctxt.reset();

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

// insert a security profile in all meta data structures
static inline hal_ret_t
add_nwsec_profile_to_db (nwsec_profile_t *sec_prof)
{
    g_hal_state->nwsec_profile_id_ht()->insert(sec_prof, &sec_prof->ht_ctxt);
    g_hal_state->nwsec_profile_hal_handle_ht()->insert(sec_prof,
                                                       &sec_prof->hal_handle_ht_ctxt);
    return HAL_RET_OK;
}

// find a security profile instance by its id
static inline nwsec_profile_t *
find_nwsec_profile_by_id (nwsec_profile_id_t profile_id)
{
    return (nwsec_profile_t *)g_hal_state->nwsec_profile_id_ht()->lookup(&profile_id);
}

// find a security profile instance by its handle
static inline nwsec_profile_t *
find_nwsec_profile_by_handle (hal_handle_t handle)
{
    return (nwsec_profile_t *)g_hal_state->nwsec_profile_hal_handle_ht()->lookup(&handle);
}

extern void *nwsec_profile_get_key_func(void *entry);
extern uint32_t nwsec_profile_compute_hash_func(void *key, uint32_t ht_size);
extern bool nwsec_profile_compare_key_func(void *key1, void *key2);

extern void *nwsec_profile_get_handle_key_func(void *entry);
extern uint32_t nwsec_profile_compute_handle_hash_func(void *key, uint32_t ht_size);
extern bool nwsec_profile_compare_handle_key_func(void *key1, void *key2);

hal_ret_t security_profile_create(nwsec::SecurityProfileSpec& spec,
                                  nwsec::SecurityProfileResponse *rsp);

hal_ret_t security_profile_update(nwsec::SecurityProfileSpec& spec,
                                  nwsec::SecurityProfileResponse *rsp);

hal_ret_t security_profile_get(nwsec::SecurityProfileGetRequest& req,
                               nwsec::SecurityProfileGetResponse *rsp);

}    // namespace hal

#endif    // __NWSEC_HPP__

