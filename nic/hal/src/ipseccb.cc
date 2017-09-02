#include <base.h>
#include <hal.hpp>
#include <hal_lock.hpp>
#include <hal_state.hpp>
#include <ipseccb.hpp>
#include <ipseccb_svc.hpp>
#include <tenant.hpp>
#include <pd_api.hpp>

namespace hal {
void *
ipseccb_get_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((ipseccb_t *)entry)->cb_id);
}

uint32_t
ipseccb_compute_hash_func (void *key, uint32_t ht_size)
{
    return utils::hash_algo::fnv_hash(key, sizeof(ipseccb_id_t)) % ht_size;
}

bool
ipseccb_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(ipseccb_id_t *)key1 == *(ipseccb_id_t *)key2) {
        return true;
    }
    return false;
}

void *
ipseccb_get_handle_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((ipseccb_t *)entry)->hal_handle);
}

uint32_t
ipseccb_compute_handle_hash_func (void *key, uint32_t ht_size)
{
    return utils::hash_algo::fnv_hash(key, sizeof(hal_handle_t)) % ht_size;
}

bool
ipseccb_compare_handle_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(hal_handle_t *)key1 == *(hal_handle_t *)key2) {
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
// validate an incoming IPSECCB create request
// TODO:
// 1. check if IPSECCB exists already
//------------------------------------------------------------------------------
static hal_ret_t
validate_ipseccb_create (IpsecCbSpec& spec, IpsecCbResponse *rsp)
{
    // must have key-handle set
    if (!spec.has_key_or_handle()) {
        rsp->set_api_status(types::API_STATUS_IPSEC_CB_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }
    // must have key in the key-handle
    if (spec.key_or_handle().key_or_handle_case() !=
            IpsecCbKeyHandle::kIpseccbId) {
        rsp->set_api_status(types::API_STATUS_IPSEC_CB_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// insert this IPSEC CB in all meta data structures
//------------------------------------------------------------------------------
static inline hal_ret_t
add_ipseccb_to_db (ipseccb_t *ipseccb)
{
    g_hal_state->ipseccb_hal_handle_ht()->insert(ipseccb,
                                               &ipseccb->hal_handle_ht_ctxt);
    g_hal_state->ipseccb_id_ht()->insert(ipseccb, &ipseccb->ht_ctxt);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a IPSEC CB create request
// TODO: if IPSEC CB exists, treat this as modify (tenant id in the meta must
// match though)
//------------------------------------------------------------------------------
hal_ret_t
ipseccb_create (IpsecCbSpec& spec, IpsecCbResponse *rsp)
{
    hal_ret_t              ret = HAL_RET_OK;
    ipseccb_t                *ipseccb;
    pd::pd_ipseccb_args_t    pd_ipseccb_args;

    // validate the request message
    ret = validate_ipseccb_create(spec, rsp);
    
    ipseccb = ipseccb_alloc_init();
    if (ipseccb == NULL) {
        rsp->set_api_status(types::API_STATUS_OUT_OF_MEM);
        return HAL_RET_OOM;
    }

    ipseccb->cb_id = spec.key_or_handle().ipseccb_id();
    
    ipseccb->hal_handle = hal_alloc_handle();

    // allocate all PD resources and finish programming
    pd::pd_ipseccb_args_init(&pd_ipseccb_args);
    pd_ipseccb_args.ipseccb = ipseccb;
    ret = pd::pd_ipseccb_create(&pd_ipseccb_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD IPSEC CB create failure, err : {}", ret);
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
        goto cleanup;
    }

    // add this L2 segment to our db
    ret = add_ipseccb_to_db(ipseccb);
    HAL_ASSERT(ret == HAL_RET_OK);

    // prepare the response
    rsp->set_api_status(types::API_STATUS_OK);
    rsp->mutable_ipseccb_status()->set_ipseccb_handle(ipseccb->hal_handle);
    return HAL_RET_OK;

cleanup:

    ipseccb_free(ipseccb);
    return ret;
}

//------------------------------------------------------------------------------
// process a IPSEC CB update request
//------------------------------------------------------------------------------
hal_ret_t
ipseccb_update (IpsecCbSpec& spec, IpsecCbResponse *rsp)
{
    hal_ret_t              ret = HAL_RET_OK; 
    ipseccb_t*               ipseccb;
    pd::pd_ipseccb_args_t    pd_ipseccb_args;

    auto kh = spec.key_or_handle();

    ipseccb = find_ipseccb_by_id(kh.ipseccb_id());
    if (ipseccb == NULL) {
        rsp->set_api_status(types::API_STATUS_IPSEC_CB_NOT_FOUND);
        return HAL_RET_IPSEC_CB_NOT_FOUND;
    }
 
    pd::pd_ipseccb_args_init(&pd_ipseccb_args);
    pd_ipseccb_args.ipseccb = ipseccb;
    
    ret = pd::pd_ipseccb_update(&pd_ipseccb_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD IPSECCB: Update Failed, err: ", ret);
        rsp->set_api_status(types::API_STATUS_IPSEC_CB_NOT_FOUND);
        return HAL_RET_HW_FAIL;
    }
    
    rsp->set_api_status(types::API_STATUS_OK);
 
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a IPSEC CB get request
//------------------------------------------------------------------------------
hal_ret_t
ipseccb_get (IpsecCbGetRequest& req, IpsecCbGetResponse *rsp)
{
    hal_ret_t              ret = HAL_RET_OK; 
    ipseccb_t                ripseccb;
    ipseccb_t*               ipseccb;
    pd::pd_ipseccb_args_t    pd_ipseccb_args;

    auto kh = req.key_or_handle();

    ipseccb = find_ipseccb_by_id(kh.ipseccb_id());
    if (ipseccb == NULL) {
        rsp->set_api_status(types::API_STATUS_IPSEC_CB_NOT_FOUND);
        return HAL_RET_IPSEC_CB_NOT_FOUND;
    }
    
    ipseccb_init(&ripseccb);
    ripseccb.cb_id = ipseccb->cb_id;
    pd::pd_ipseccb_args_init(&pd_ipseccb_args);
    pd_ipseccb_args.ipseccb = &ripseccb;
    
    ret = pd::pd_ipseccb_get(&pd_ipseccb_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD IPSECCB: Failed to get, err: {}", ret);
        rsp->set_api_status(types::API_STATUS_IPSEC_CB_NOT_FOUND);
        return HAL_RET_HW_FAIL;
    }

    // fill config spec of this IPSEC CB 
    rsp->mutable_spec()->mutable_key_or_handle()->set_ipseccb_id(ripseccb.cb_id);

    rsp->mutable_spec()->set_pi(ripseccb.pi);
    rsp->mutable_spec()->set_ci(ripseccb.ci);

    // fill operational state of this IPSEC CB
    rsp->mutable_status()->set_ipseccb_handle(ipseccb->hal_handle);

    // fill stats of this IPSEC CB
    rsp->set_api_status(types::API_STATUS_OK);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a IPSEC CB delete request
//------------------------------------------------------------------------------
hal_ret_t
ipseccb_delete (ipseccb::IpsecCbDeleteRequest& req, ipseccb::IpsecCbDeleteResponseMsg *rsp)
{
    hal_ret_t              ret = HAL_RET_OK; 
    ipseccb_t*               ipseccb;
    pd::pd_ipseccb_args_t    pd_ipseccb_args;

    auto kh = req.key_or_handle();
    ipseccb = find_ipseccb_by_id(kh.ipseccb_id());
    if (ipseccb == NULL) {
        rsp->add_api_status(types::API_STATUS_OK);
        return HAL_RET_OK;
    }
 
    pd::pd_ipseccb_args_init(&pd_ipseccb_args);
    pd_ipseccb_args.ipseccb = ipseccb;
    
    ret = pd::pd_ipseccb_delete(&pd_ipseccb_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD IPSECCB: delete Failed, err: {}", ret);
        rsp->add_api_status(types::API_STATUS_IPSEC_CB_NOT_FOUND);
        return HAL_RET_HW_FAIL;
    }
    
    // fill stats of this IPSEC CB
    rsp->add_api_status(types::API_STATUS_OK);
 
    return HAL_RET_OK;
}

}    // namespace hal
