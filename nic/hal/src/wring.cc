#include <base.h>
#include <hal.hpp>
#include <hal_lock.hpp>
#include <hal_state.hpp>
#include <wring.hpp>
#include <wring_svc.hpp>
#include <tenant.hpp>
#include <pd_api.hpp>

namespace hal {

void *
wring_get_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((wring_t *)entry)->wring_id);
}

uint32_t
wring_compute_hash_func (void *key, uint32_t ht_size)
{
    return utils::hash_algo::fnv_hash(key, sizeof(wring_id_t)) % ht_size;
}

bool
wring_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(wring_id_t *)key1 == *(wring_id_t *)key2) {
        return true;
    }
    return false;
}

void *
wring_get_handle_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((wring_t *)entry)->hal_handle);
}

uint32_t
wring_compute_handle_hash_func (void *key, uint32_t ht_size)
{
    return utils::hash_algo::fnv_hash(key, sizeof(hal_handle_t)) % ht_size;
}

bool
wring_compare_handle_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(hal_handle_t *)key1 == *(hal_handle_t *)key2) {
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
// validate an incoming WRING create request
// TODO:
// 1. check if WRING exists already
//------------------------------------------------------------------------------
static hal_ret_t
validate_wring_create (WRingSpec& spec, WRingResponse *rsp)
{
    // must have key-handle set
    if (!spec.has_key_or_handle()) {
        rsp->set_api_status(types::API_STATUS_WRING_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    // must have key in the key-handle
    if (spec.key_or_handle().key_or_handle_case() !=
            WRingKeyHandle::kWringId) {
        rsp->set_api_status(types::API_STATUS_WRING_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// insert this WRing in all meta data structures
//------------------------------------------------------------------------------
static inline hal_ret_t
add_wring_to_db (wring_t *wring)
{
    g_hal_state->wring_hal_handle_ht()->insert(wring,
                                               &wring->hal_handle_ht_ctxt);
    g_hal_state->wring_id_ht()->insert(wring, &wring->ht_ctxt);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a Wring create request
// match though)
//------------------------------------------------------------------------------
hal_ret_t
wring_create (WRingSpec& spec, WRingResponse *rsp)
{
    hal_ret_t              ret = HAL_RET_OK;
    wring_t                *wring;
    pd::pd_wring_args_t    pd_wring_args;

    // validate the request message
    ret = validate_wring_create(spec, rsp);
    if (ret != HAL_RET_OK) {
        // api_status already set, just return
        HAL_TRACE_ERR("PD Wringvalidate failure, err : {}", ret);
        return ret;
    }

    // instantiate WRing
    wring = wring_alloc_init();
    if (wring == NULL) {
        rsp->set_api_status(types::API_STATUS_OUT_OF_MEM);
        return HAL_RET_OOM;
    }

    wring->wring_id = spec.key_or_handle().wring_id();
    wring->hal_handle = hal_alloc_handle();

    // allocate all PD resources and finish programming
    pd::pd_wring_args_init(&pd_wring_args);
    pd_wring_args.wring = wring;
    ret = pd::pd_wring_create(&pd_wring_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD Wring create failure, err : {}", ret);
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
        goto cleanup;
    }

    // add this L2 segment to our db
    ret = add_wring_to_db(wring);
    HAL_ASSERT(ret == HAL_RET_OK);

    // prepare the response
    rsp->set_api_status(types::API_STATUS_OK);
    rsp->mutable_wring_status()->set_wring_handle(wring->hal_handle);
    return HAL_RET_OK;

cleanup:

    wring_free(wring);
    return ret;
}

//------------------------------------------------------------------------------
// process a WRing update request
//------------------------------------------------------------------------------
hal_ret_t
wring_update (WRingSpec& spec, WRingResponse *rsp)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a WRing get request
//------------------------------------------------------------------------------
hal_ret_t
wring_get (WRingGetRequest& req, WRingGetResponse *rsp)
{
    hal_ret_t               ret = HAL_RET_OK;
    wring_t                 wring;
    pd::pd_wring_args_t     pd_wring_args;
    
    if(req.type() <= types::WRING_TYPE_NONE) {
        HAL_TRACE_ERR("Invalid wring type");
        rsp->set_api_status(types::API_STATUS_WRING_TYPE_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    wring_init(&wring);
    wring.wring_type = req.type();
    wring.wring_id = req.key_or_handle().wring_id();
    wring.slot_index = req.index();

    pd::pd_wring_args_init(&pd_wring_args);
    pd_wring_args.wring = &wring;

    ret = pd::pd_wring_get(&pd_wring_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD Wring: Failed to get, err: {}", ret);
        rsp->set_api_status(types::API_STATUS_TCP_CB_NOT_FOUND);
        return HAL_RET_HW_FAIL;
    }

    // fill config spec of this WRING 
    rsp->mutable_spec()->set_type(wring.wring_type);
    rsp->mutable_spec()->mutable_key_or_handle()->set_wring_id(wring.wring_id);
    rsp->set_index(wring.slot_index);
    rsp->set_value(wring.slot_value);

    // fill operational state of this WRING
    //rsp->mutable_status()->set_wring_handle(wring->hal_handle);

    // fill stats of this WRING
    rsp->set_api_status(types::API_STATUS_OK);
    return HAL_RET_OK;
}

}    // namespace hal
