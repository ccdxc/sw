#include <base.h>
#include <hal.hpp>
#include <hal_lock.hpp>
#include <hal_state.hpp>
#include <tlscb.hpp>
#include <tlscb_svc.hpp>
#include <tenant.hpp>
#include <pd_api.hpp>

namespace hal {

void *
tlscb_get_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((tlscb_t *)entry)->cb_id);
}

uint32_t
tlscb_compute_hash_func (void *key, uint32_t ht_size)
{
    return utils::hash_algo::fnv_hash(key, sizeof(tlscb_id_t)) % ht_size;
}

bool
tlscb_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(tlscb_id_t *)key1 == *(tlscb_id_t *)key2) {
        return true;
    }
    return false;
}

void *
tlscb_get_handle_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((tlscb_t *)entry)->hal_handle);
}

uint32_t
tlscb_compute_handle_hash_func (void *key, uint32_t ht_size)
{
    return utils::hash_algo::fnv_hash(key, sizeof(hal_handle_t)) % ht_size;
}

bool
tlscb_compare_handle_key_func (void *key1, void *key2)
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
validate_tlscb_create (TlsCbSpec& spec, TlsCbResponse *rsp)
{
    // must have key-handle set
    if (!spec.has_key_or_handle()) {
        rsp->set_api_status(types::API_STATUS_TLS_CB_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    // must have key in the key-handle
    if (spec.key_or_handle().key_or_handle_case() !=
            TlsCbKeyHandle::kTlscbId) {
        rsp->set_api_status(types::API_STATUS_TLS_CB_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// insert this TLS CB in all meta data structures
//------------------------------------------------------------------------------
static inline hal_ret_t
add_tlscb_to_db (tlscb_t *tlscb)
{
    g_hal_state->tlscb_hal_handle_ht()->insert(tlscb,
                                               &tlscb->hal_handle_ht_ctxt);
    g_hal_state->tlscb_id_ht()->insert(tlscb, &tlscb->ht_ctxt);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a TLS CB create request
// TODO: if TLS CB exists, treat this as modify (tenant id in the meta must
// match though)
//------------------------------------------------------------------------------
hal_ret_t
tlscb_create (TlsCbSpec& spec, TlsCbResponse *rsp)
{
    hal_ret_t              ret = HAL_RET_OK;
    tlscb_t                *tlscb;
    pd::pd_tlscb_args_t    pd_tlscb_args;

    // validate the request message
    ret = validate_tlscb_create(spec, rsp);
    if (ret != HAL_RET_OK) {
        // api_status already set, just return
        return ret;
    }

    // instantiate TLS CB
    tlscb = tlscb_alloc_init();
    if (tlscb == NULL) {
        rsp->set_api_status(types::API_STATUS_OUT_OF_MEM);
        return HAL_RET_OOM;
    }

    tlscb->cb_id = spec.key_or_handle().tlscb_id();
    tlscb->cipher_type = spec.cipher_type();
    tlscb->hal_handle = hal_alloc_handle();

    // allocate all PD resources and finish programming
    pd::pd_tlscb_args_init(&pd_tlscb_args);
    pd_tlscb_args.tlscb = tlscb;
    ret = pd::pd_tlscb_create(&pd_tlscb_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD TLS CB create failure, err : {}", ret);
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
        goto cleanup;
    }

    // add this L2 segment to our db
    ret = add_tlscb_to_db(tlscb);
    HAL_ASSERT(ret == HAL_RET_OK);

    // prepare the response
    rsp->set_api_status(types::API_STATUS_OK);
    rsp->mutable_tlscb_status()->set_tlscb_handle(tlscb->hal_handle);
    return HAL_RET_OK;

cleanup:

    tlscb_free(tlscb);
    return ret;
}

//------------------------------------------------------------------------------
// process a TLS CB update request
//------------------------------------------------------------------------------
hal_ret_t
tlscb_update (TlsCbSpec& spec, TlsCbResponse *rsp)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a TLS CB get request
//------------------------------------------------------------------------------
hal_ret_t
tlscb_get (TlsCbGetRequest& req, TlsCbGetResponse *rsp)
{
    tlscb_t    *tlscb;

    if (!req.has_meta()) {
        rsp->set_api_status(types::API_STATUS_TENANT_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    if (!req.has_key_or_handle()) {
        rsp->set_api_status(types::API_STATUS_TLS_CB_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }
    auto kh = req.key_or_handle();

    if (kh.key_or_handle_case() == tlscb::TlsCbKeyHandle::kTlscbId) {
        tlscb = find_tlscb_by_id(kh.tlscb_id());
    } else if (kh.key_or_handle_case() == tlscb::TlsCbKeyHandle::kTlscbHandle) {
        tlscb = find_tlscb_by_handle(kh.tlscb_handle());
    } else {
        rsp->set_api_status(types::API_STATUS_INVALID_ARG);
        return HAL_RET_INVALID_ARG;
    }

    if (tlscb == NULL) {
        rsp->set_api_status(types::API_STATUS_TLS_CB_NOT_FOUND);
        return HAL_RET_TLS_CB_NOT_FOUND;
    }

    // fill config spec of this TLS CB 
    rsp->mutable_spec()->mutable_key_or_handle()->set_tlscb_id(tlscb->cb_id);

    // fill operational state of this TLS CB
    rsp->mutable_status()->set_tlscb_handle(tlscb->hal_handle);

    // fill stats of this TLS CB
    rsp->set_api_status(types::API_STATUS_OK);
    return HAL_RET_OK;
}

}    // namespace hal
