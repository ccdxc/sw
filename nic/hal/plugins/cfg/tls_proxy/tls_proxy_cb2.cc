//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/include/base.hpp"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_lock.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/hal/plugins/cfg/tls_proxy/tls_proxy_cb2.hpp"
#include "nic/hal/plugins/cfg/nw/vrf.hpp"
#include "nic/include/pd_api.hpp"

namespace hal {
void *
tls_proxy_cb_get_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((tls_proxy_cb_t *)entry)->cb_id);
}

uint32_t
tls_proxy_cb_compute_hash_func (void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(tls_proxy_cb_id_t)) % ht_size;
}

bool
tls_proxy_cb_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(tls_proxy_cb_id_t *)key1 == *(tls_proxy_cb_id_t *)key2) {
        return true;
    }
    return false;
}

void *
tls_proxy_cb_get_handle_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((tls_proxy_cb_t *)entry)->hal_handle);
}

uint32_t
tls_proxy_cb_compute_handle_hash_func (void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(hal_handle_t)) % ht_size;
}

bool
tls_proxy_cb_compare_handle_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(hal_handle_t *)key1 == *(hal_handle_t *)key2) {
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
// validate an incoming TLSCB create request
// TODO:
// 1. check if TLSCB exists already
//------------------------------------------------------------------------------
static hal_ret_t
validate_tls_proxy_cb_create (TlsProxyCbSpec& spec, TlsProxyCbResponse *rsp)
{
    // must have key-handle set
    if (!spec.has_key_or_handle()) {
        rsp->set_api_status(types::API_STATUS_TLS_CB_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }
    // must have key in the key-handle
    if (spec.key_or_handle().key_or_handle_case() !=
            TlsProxyCbKeyHandle::kTlsProxyCbId) {
        rsp->set_api_status(types::API_STATUS_TLS_CB_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// insert this TLS CB in all meta data structures
//------------------------------------------------------------------------------
static inline hal_ret_t
add_tls_proxy_cb_to_db (tls_proxy_cb_t *tls_proxy_cb)
{
    g_hal_state->tlscb_id_ht()->insert(tls_proxy_cb, &tls_proxy_cb->ht_ctxt);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a TLS CB create request
// TODO: if TLS CB exists, treat this as modify (vrf id in the meta must
// match though)
//------------------------------------------------------------------------------
hal_ret_t
tls_proxy_cb_create (TlsProxyCbSpec& spec, TlsProxyCbResponse *rsp)
{
    hal_ret_t              ret = HAL_RET_OK;
    tls_proxy_cb_t                *tls_proxy_cb;
    pd::pd_tls_proxy_cb_create_args_t    pd_tls_proxy_cb_args;
    pd::pd_func_args_t          pd_func_args = {0};

    // validate the request message
    ret = validate_tls_proxy_cb_create(spec, rsp);

    // instantiate TLS CB
    tls_proxy_cb = tls_proxy_cb_alloc_init();
    if (tls_proxy_cb == NULL) {
        rsp->set_api_status(types::API_STATUS_OUT_OF_MEM);
        return HAL_RET_OOM;
    }

    tls_proxy_cb->cb_id = spec.key_or_handle().tls_proxy_cb_id();
    tls_proxy_cb->command = spec.command();
    tls_proxy_cb->crypto_key_idx = spec.crypto_key_idx();
    tls_proxy_cb->crypto_hmac_key_idx = spec.crypto_hmac_key_idx();
    tls_proxy_cb->debug_dol = spec.debug_dol();
    tls_proxy_cb->salt = spec.salt();
    tls_proxy_cb->explicit_iv = spec.explicit_iv();
    tls_proxy_cb->serq_pi = spec.serq_pi();
    tls_proxy_cb->serq_ci = spec.serq_ci();
    tls_proxy_cb->is_decrypt_flow = spec.is_decrypt_flow();
    tls_proxy_cb->other_fid = spec.other_fid();
    tls_proxy_cb->l7_proxy_type = spec.l7_proxy_type();
    tls_proxy_cb->cpu_id = spec.cpu_id();

    tls_proxy_cb->hal_handle = hal_alloc_handle();

    // allocate all PD resources and finish programming
    pd::pd_tls_proxy_cb_create_args_init(&pd_tls_proxy_cb_args);
    pd_tls_proxy_cb_args.tls_proxy_cb = tls_proxy_cb;
    pd_func_args.pd_tls_proxy_cb_create = &pd_tls_proxy_cb_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_TLS_PROXY_CB_CREATE, &pd_func_args);

    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD TLS CB create failure, err : {}", ret);
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
        goto cleanup;
    }

    // add this L2 segment to our db
    ret = add_tls_proxy_cb_to_db(tls_proxy_cb);
    HAL_ASSERT(ret == HAL_RET_OK);

    // prepare the response
    rsp->set_api_status(types::API_STATUS_OK);
    rsp->mutable_tls_proxy_cb_status()->set_tls_proxy_cb_handle(tls_proxy_cb->hal_handle);
    return HAL_RET_OK;

cleanup:
    tls_proxy_cb_free(tls_proxy_cb);
    return ret;
}

//------------------------------------------------------------------------------
// process a TLS CB update request
//------------------------------------------------------------------------------
hal_ret_t
tls_proxy_cb_update (TlsProxyCbSpec& spec, TlsProxyCbResponse *rsp)
{
    hal_ret_t              ret = HAL_RET_OK;
    tls_proxy_cb_t*               tls_proxy_cb;
    pd::pd_tls_proxy_cb_update_args_t    pd_tls_proxy_cb_args;
    bool                   is_decrypt_flow = false;
    pd::pd_func_args_t          pd_func_args = {0};

    auto kh = spec.key_or_handle();

    tls_proxy_cb = find_tls_proxy_cb_by_id(kh.tls_proxy_cb_id());
    if (tls_proxy_cb == NULL) {
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_TLS_CB_NOT_FOUND;
    }

    tls_proxy_cb->command = spec.command();
    tls_proxy_cb->crypto_key_idx = spec.crypto_key_idx();
    tls_proxy_cb->crypto_hmac_key_idx = spec.crypto_hmac_key_idx();
    tls_proxy_cb->debug_dol = spec.debug_dol();
    tls_proxy_cb->salt = spec.salt();
    tls_proxy_cb->explicit_iv = spec.explicit_iv();
    tls_proxy_cb->serq_pi = spec.serq_pi();
    tls_proxy_cb->serq_ci = spec.serq_ci();
    tls_proxy_cb->cpu_id = spec.cpu_id();
    is_decrypt_flow = (uint8_t) spec.is_decrypt_flow();
    HAL_TRACE_DEBUG("{}: TLSCBID: 0x{:x}: is_decrypt_flow: 0x{:x}",
            __FUNCTION__, tls_proxy_cb->cb_id, is_decrypt_flow);
    if (is_decrypt_flow == true)
        tls_proxy_cb->is_decrypt_flow = 1;
    else
        tls_proxy_cb->is_decrypt_flow = 0;
    HAL_TRACE_DEBUG("{}: TLSCBID: 0x{:x}: is_decrypt_flow: 0x{:x}",
            __FUNCTION__, tls_proxy_cb->cb_id, tls_proxy_cb->is_decrypt_flow);

    tls_proxy_cb->other_fid = spec.other_fid();
    tls_proxy_cb->l7_proxy_type = spec.l7_proxy_type();

    pd::pd_tls_proxy_cb_update_args_init(&pd_tls_proxy_cb_args);
    pd_tls_proxy_cb_args.tls_proxy_cb = tls_proxy_cb;

    pd_func_args.pd_tls_proxy_cb_update = &pd_tls_proxy_cb_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_TLS_PROXY_CB_UPDATE, &pd_func_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD TLSCB: Update Failed, err: {}", ret);
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_HW_FAIL;
    }

    // fill stats of this TLS CB
    rsp->set_api_status(types::API_STATUS_OK);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a TLS CB get request
//------------------------------------------------------------------------------
hal_ret_t
tls_proxy_cb_get (TlsProxyCbGetRequest& req, TlsProxyCbGetResponseMsg *resp)
{
    hal_ret_t              ret = HAL_RET_OK;
    tls_proxy_cb_t                rtls_proxy_cb;
    tls_proxy_cb_t*               tls_proxy_cb;
    pd::pd_tls_proxy_cb_get_args_t    pd_tls_proxy_cb_args;
    pd::pd_func_args_t         pd_func_args = {0};
    TlsProxyCbGetResponse *rsp = resp->add_response();

    auto kh = req.key_or_handle();

    tls_proxy_cb = find_tls_proxy_cb_by_id(kh.tls_proxy_cb_id());
    if (tls_proxy_cb == NULL) {
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_TLS_CB_NOT_FOUND;
    }

    tls_proxy_cb_init(&rtls_proxy_cb);
    rtls_proxy_cb.cb_id = tls_proxy_cb->cb_id;
    pd::pd_tls_proxy_cb_get_args_init(&pd_tls_proxy_cb_args);
    pd_tls_proxy_cb_args.tls_proxy_cb = &rtls_proxy_cb;

    pd_func_args.pd_tls_proxy_cb_get = &pd_tls_proxy_cb_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_TLS_PROXY_CB_GET, &pd_func_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD TLSCB: Failed to get, err: {}", ret);
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_HW_FAIL;
    }

    // fill config spec of this TLS CB
    rsp->mutable_spec()->mutable_key_or_handle()->set_tls_proxy_cb_id(rtls_proxy_cb.cb_id);

    rsp->mutable_spec()->set_command(rtls_proxy_cb.command);
    rsp->mutable_spec()->set_serq_pi(rtls_proxy_cb.serq_pi);
    rsp->mutable_spec()->set_serq_ci(rtls_proxy_cb.serq_ci);
    rsp->mutable_spec()->set_bsq_pi(rtls_proxy_cb.bsq_pi);
    rsp->mutable_spec()->set_bsq_ci(rtls_proxy_cb.bsq_ci);

    HAL_TRACE_DEBUG("tnmdr_alloc : {:#x}, tnmpr_alloc : {:#x}, enc_requests : {:#x}",
                    rtls_proxy_cb.tnmdr_alloc, rtls_proxy_cb.tnmpr_alloc, rtls_proxy_cb.enc_requests);
    rsp->mutable_spec()->set_tnmdr_alloc(rtls_proxy_cb.tnmdr_alloc);
    rsp->mutable_spec()->set_tnmpr_alloc(rtls_proxy_cb.tnmpr_alloc);
    rsp->mutable_spec()->set_rnmdr_free(rtls_proxy_cb.rnmdr_free);
    rsp->mutable_spec()->set_rnmpr_free(rtls_proxy_cb.rnmpr_free);
    rsp->mutable_spec()->set_enc_requests(rtls_proxy_cb.enc_requests);
    rsp->mutable_spec()->set_enc_completions(rtls_proxy_cb.enc_completions);
    rsp->mutable_spec()->set_enc_failures(rtls_proxy_cb.enc_failures);
    rsp->mutable_spec()->set_dec_requests(rtls_proxy_cb.dec_requests);
    rsp->mutable_spec()->set_dec_completions(rtls_proxy_cb.dec_completions);
    rsp->mutable_spec()->set_dec_failures(rtls_proxy_cb.dec_failures);
    rsp->mutable_spec()->set_pre_debug_stage0_7_thread(rtls_proxy_cb.pre_debug_stage0_7_thread);
    rsp->mutable_spec()->set_post_debug_stage0_7_thread(rtls_proxy_cb.post_debug_stage0_7_thread);
    rsp->mutable_spec()->set_mac_requests(rtls_proxy_cb.mac_requests);
    rsp->mutable_spec()->set_mac_completions(rtls_proxy_cb.mac_completions);
    rsp->mutable_spec()->set_mac_failures(rtls_proxy_cb.mac_failures);

    rsp->mutable_spec()->set_command(rtls_proxy_cb.command);
    rsp->mutable_spec()->set_crypto_key_idx(rtls_proxy_cb.crypto_key_idx);
    rsp->mutable_spec()->set_crypto_hmac_key_idx(rtls_proxy_cb.crypto_hmac_key_idx);
    rsp->mutable_spec()->set_debug_dol(rtls_proxy_cb.debug_dol);
    rsp->mutable_spec()->set_salt(rtls_proxy_cb.salt);
    rsp->mutable_spec()->set_explicit_iv(rtls_proxy_cb.explicit_iv);
    rsp->mutable_spec()->set_other_fid(rtls_proxy_cb.other_fid);
    rsp->mutable_spec()->set_l7_proxy_type(rtls_proxy_cb.l7_proxy_type);
    rsp->mutable_spec()->set_cpu_id(rtls_proxy_cb.cpu_id);

    // fill operational state of this TLS CB
    rsp->mutable_status()->set_tls_proxy_cb_handle(tls_proxy_cb->hal_handle);

    // fill stats of this TLS CB
    rsp->set_api_status(types::API_STATUS_OK);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a TLS CB delete request
//------------------------------------------------------------------------------
hal_ret_t
tls_proxy_cb_delete (tls_proxy::TlsProxyCbDeleteRequest& req, tls_proxy::TlsProxyCbDeleteResponseMsg *rsp)
{
    hal_ret_t              ret = HAL_RET_OK;
    tls_proxy_cb_t*               tls_proxy_cb;
    pd::pd_tls_proxy_cb_delete_args_t    pd_tls_proxy_cb_args;
    pd::pd_func_args_t          pd_func_args = {0};

    auto kh = req.key_or_handle();
    tls_proxy_cb = find_tls_proxy_cb_by_id(kh.tls_proxy_cb_id());
    if (tls_proxy_cb == NULL) {
        rsp->add_api_status(types::API_STATUS_OK);
        return HAL_RET_OK;
    }

    pd::pd_tls_proxy_cb_delete_args_init(&pd_tls_proxy_cb_args);
    pd_tls_proxy_cb_args.tls_proxy_cb = tls_proxy_cb;

    pd_func_args.pd_tls_proxy_cb_delete = &pd_tls_proxy_cb_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_TLS_PROXY_CB_DELETE, &pd_func_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD TLSCB: delete Failed, err: {}", ret);
        rsp->add_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_HW_FAIL;
    }

    // fill stats of this TLS CB
    rsp->add_api_status(types::API_STATUS_OK);
    return HAL_RET_OK;
}

}    // namespace hal
