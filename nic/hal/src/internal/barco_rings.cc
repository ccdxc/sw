//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/include/base.hpp"
#include "nic/hal/hal.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/plugins/cfg/aclqos/barco_rings.hpp"
#include "nic/sdk/asic/pd/pd.hpp"

using namespace sdk::asic::pd;

namespace hal {

hal_ret_t   get_opaque_tag_addr(GetOpaqueTagAddrRequest& request,
    GetOpaqueTagAddrResponseMsg *response_msg)
{
    hal_ret_t           ret = HAL_RET_OK;
    uint64_t addr;
    pd::pd_func_args_t          pd_func_args = {0};
    GetOpaqueTagAddrResponse *response = response_msg->add_response();

    pd::pd_get_opaque_tag_addr_args_t args;
    args.ring_type = request.ring_type();
    args.addr = &addr;
    pd_func_args.pd_get_opaque_tag_addr = &args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_OPAQUE_TAG_ADDR, &pd_func_args);
    if ((ret != HAL_RET_OK)) {
        response->set_api_status(types::API_STATUS_ERR);
    }
    else {
        response->set_opaque_tag_addr(addr);
        response->set_api_status(types::API_STATUS_OK);
    }
    return ret;
}

hal_ret_t barco_get_opaque_tag_addr(types::BarcoRings ring_type, uint64_t* addr)
{
    hal_ret_t           ret = HAL_RET_OK;
    pd::pd_func_args_t  pd_func_args = {0};

    pd::pd_get_opaque_tag_addr_args_t args;
    args.ring_type = ring_type;
    args.addr = addr;
    pd_func_args.pd_get_opaque_tag_addr = &args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_OPAQUE_TAG_ADDR, &pd_func_args);
    return ret;
}

hal_ret_t barco_get_req_descr_entry(BarcoGetReqDescrEntryRequest& request,
				BarcoGetReqDescrEntryResponseMsg *response_msg)
{
    sdk_ret_t          sdk_ret;
    hal_ret_t          ret = HAL_RET_OK;
    barco_symm_descr_t symm_req_descr;
    barco_asym_descr_t asym_req_descr;

    BarcoGetReqDescrEntryResponse *response = response_msg->add_response();

    switch (request.ring_type()) {
    case types::BARCO_RING_ASYM:
        memset(&asym_req_descr, 0, sizeof(asym_req_descr));
        sdk_ret = asicpd_barco_asym_req_descr_get(request.slot_index(), &asym_req_descr);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
	if (ret != HAL_RET_OK) {
	    HAL_TRACE_ERR("BarcoGetReqDescr({}-{:d}): Failed to get Asym req "
			  "descriptor, err: {}", types::BarcoRings_Name(request.ring_type()),
			  request.slot_index(), ret);
	    response->set_api_status(types::API_STATUS_ERR);
	    return HAL_RET_HW_FAIL;
	}
	HAL_TRACE_DEBUG("BarcoGetReqDescr({}-{:d}): Get Asym req descriptor, success!",
			types::BarcoRings_Name(request.ring_type()), request.slot_index());
	response->mutable_asym_req_descr()->set_ilist_addr(asym_req_descr.ilist_addr);
	response->mutable_asym_req_descr()->set_olist_addr(asym_req_descr.olist_addr);
	response->mutable_asym_req_descr()->set_key_desc_index(asym_req_descr.key_desc_index);
	response->mutable_asym_req_descr()->set_status_addr(asym_req_descr.status_addr);
	response->mutable_asym_req_descr()->set_opaque_tag_value(asym_req_descr.opaque_tag_value);
	response->mutable_asym_req_descr()->set_opaque_tag_wr_en(asym_req_descr.opaque_tag_wr_en);
	response->mutable_asym_req_descr()->set_flag_a(asym_req_descr.flag_a);
	response->mutable_asym_req_descr()->set_flag_b(asym_req_descr.flag_b);
	break;
    default:
        memset(&symm_req_descr, 0, sizeof(symm_req_descr));
        sdk_ret = asicpd_barco_symm_req_descr_get((barco_rings_t) request.ring_type(), 
                                              request.slot_index(), &symm_req_descr);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
	if (ret != HAL_RET_OK) {
	    HAL_TRACE_ERR("BarcoGetReqDescr({}-{:d}): Failed to get Symm req "
			  "descriptor, err: {}", types::BarcoRings_Name(request.ring_type()),
			  request.slot_index(), ret);
	    response->set_api_status(types::API_STATUS_ERR);
	    return HAL_RET_HW_FAIL;
	}
	HAL_TRACE_DEBUG("BarcoGetReqDescr({}-{:d}): Get Symm req descriptor, success!",
			types::BarcoRings_Name(request.ring_type()), request.slot_index());
	response->mutable_symm_req_descr()->set_ilist_addr(symm_req_descr.ilist_addr);
	response->mutable_symm_req_descr()->set_olist_addr(symm_req_descr.olist_addr);
	response->mutable_symm_req_descr()->set_command(symm_req_descr.command);
	response->mutable_symm_req_descr()->set_key_desc_index(symm_req_descr.key_desc_index);
	response->mutable_symm_req_descr()->set_iv_addr(symm_req_descr.iv_addr);
	response->mutable_symm_req_descr()->set_status_addr(symm_req_descr.status_addr);
	response->mutable_symm_req_descr()->set_doorbell_addr(symm_req_descr.doorbell_addr);
	response->mutable_symm_req_descr()->set_doorbell_data(symm_req_descr.doorbell_data);
	response->mutable_symm_req_descr()->set_salt(symm_req_descr.salt);
	response->mutable_symm_req_descr()->set_explicit_iv(symm_req_descr.explicit_iv);
	response->mutable_symm_req_descr()->set_header_size(symm_req_descr.header_size);
	response->mutable_symm_req_descr()->set_barco_status(symm_req_descr.barco_status);
	response->mutable_symm_req_descr()->set_second_key_desc_index(symm_req_descr.second_key_desc_index);
	break;
    }

    // fill in the common fields in the response
    response->set_ring_type(request.ring_type());
    response->set_slot_index(request.slot_index());

    response->set_api_status(types::API_STATUS_OK);
    return HAL_RET_OK;
}

hal_ret_t
barco_get_ring_meta(BarcoGetRingMetaRequest& request,
                    BarcoGetRingMetaResponseMsg *response_msg)
{
    sdk_ret_t          sdk_ret;
    hal_ret_t          ret = HAL_RET_OK;
    uint32_t           pi = 0, ci = 0;
    BarcoGetRingMetaResponse *response = response_msg->add_response();

    sdk_ret = asicpd_barco_ring_meta_get((barco_rings_t) request.ring_type(),
                                         &pi, &ci);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("BarcoGetRingMeta({}): Failed to get PI/CI,  err: {}",
                      types::BarcoRings_Name(request.ring_type()), ret);
        response->set_api_status(types::API_STATUS_ERR);
        return HAL_RET_HW_FAIL;
    }
    HAL_TRACE_DEBUG("BarcoGetRingMeta({}): Got PI {:d} CI {:d} ",
                    types::BarcoRings_Name(request.ring_type()), pi, ci);

    // fill in the common fields in the response
    response->set_ring_type(request.ring_type());
    response->set_pi(pi);
    response->set_ci(ci);

    response->set_api_status(types::API_STATUS_OK);
    return HAL_RET_OK;
}

hal_ret_t
barco_get_ring_meta_config(BarcoGetRingMetaConfigRequest& request,
                           BarcoGetRingMetaConfigResponseMsg *response_msg)
{
    sdk_ret_t          sdk_ret;
    hal_ret_t          ret = HAL_RET_OK;
    barco_ring_meta_config_t meta = {0};
    BarcoGetRingMetaConfigResponse *response = response_msg->add_response();

    sdk_ret =
        asicpd_barco_get_meta_config_info((barco_rings_t) request.ring_type(),
                                          &meta);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("BarcoGetRingMetaConfig({}): Failed, " "err: {}",
                      types::BarcoRings_Name(request.ring_type()), ret);
        response->set_api_status(types::API_STATUS_ERR);
        return HAL_RET_HW_FAIL;
    }
    HAL_TRACE_DEBUG("BarcoGetRingMetaConfig({}): succeeded ",
                    types::BarcoRings_Name(request.ring_type()));

    // fill in the common fields in the response
    response->set_ring_type(request.ring_type());
    response->set_ring_base(meta.ring_base);
    response->set_ring_size(meta.ring_size);
    response->set_producer_idx_addr(meta.producer_idx_addr);
    response->set_shadow_pndx_addr(meta.shadow_pndx_addr);
    response->set_opaque_tag_addr(meta.opaque_tag_addr);
    response->set_desc_size(meta.desc_size);
    response->set_pndx_size(meta.pndx_size);
    response->set_opaque_tag_size(meta.opaque_tag_size);

    response->set_api_status(types::API_STATUS_OK);
    return HAL_RET_OK;
}

}
