#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/src/barco_rings.hpp"
// #include "nic/hal/pd/capri/capri_barco_rings.hpp"

namespace hal {

hal_ret_t   GetOpaqueTagAddr(const GetOpaqueTagAddrRequest& request,
    GetOpaqueTagAddrResponse *response)
{
    hal_ret_t           ret = HAL_RET_OK;
    uint64_t addr;

    pd::pd_get_opaque_tag_addr_args_t args;
    args.ring_type = request.ring_type();
    args.addr = &addr;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_OPAQUE_TAG_ADDR, (void *)&args);
    // ret = pd::get_opaque_tag_addr(request.ring_type(), &addr);
    if ((ret != HAL_RET_OK)) {
        response->set_api_status(types::API_STATUS_ERR);
    }
    else {
        response->set_opaque_tag_addr(addr);
        response->set_api_status(types::API_STATUS_OK);
    }
    return ret;
}

hal_ret_t BarcoGetReqDescrEntry(const BarcoGetReqDescrEntryRequest& request,
				BarcoGetReqDescrEntryResponse *response)
{
    hal_ret_t          ret = HAL_RET_OK;
    barco_symm_descr_t symm_req_descr;
    barco_asym_descr_t asym_req_descr;
    pd::pd_capri_barco_asym_req_descr_get_args_t args;
    pd::pd_capri_barco_symm_req_descr_get_args_t sym_args;

    switch (request.ring_type()) {
    case types::BARCO_RING_ASYM:
        memset(&asym_req_descr, 0, sizeof(asym_req_descr));
        args.slot_index = request.slot_index();
        args.asym_req_descr = &asym_req_descr;
        ret = pd::hal_pd_call(pd::PD_FUNC_ID_BARCO_ASYM_REQ_DSC_GET, (void *)&args);
        // ret = pd::capri_barco_asym_req_descr_get(request.slot_index(), &asym_req_descr);
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
        sym_args.ring_type = request.ring_type();
        sym_args.slot_index = request.slot_index();
        sym_args.symm_req_descr = &symm_req_descr;
        ret = pd::hal_pd_call(pd::PD_FUNC_ID_BARCO_SYM_REQ_DSC_GET, (void *)&sym_args);
#if 0
        ret = pd::capri_barco_symm_req_descr_get(request.ring_type(), request.slot_index(),
						 &symm_req_descr);
#endif
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

hal_ret_t BarcoGetRingMeta(const BarcoGetRingMetaRequest& request,
			   BarcoGetRingMetaResponse *response)
{
    hal_ret_t          ret = HAL_RET_OK;
    uint32_t           pi = 0, ci = 0;
    pd::pd_capri_barco_ring_meta_get_args_t args;

    args.ring_type = request.ring_type();
    args.pi = &pi;
    args.ci = &ci;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_BARCO_RING_META_GET, (void *)&args);
    // ret = pd::capri_barco_ring_meta_get(request.ring_type(), &pi, &ci);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("BarcoGetRingMeta({}): Failed to get PI/CI, "
		      "err: {}", types::BarcoRings_Name(request.ring_type()), ret);
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


}
