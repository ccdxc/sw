//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include "platform/src/lib/nicmgr/include/logger.hpp"
#include "devapi_iris.hpp"
#include "devapi_mem.hpp"
#include "accel.hpp"
#include "hal_grpc.hpp"
#include "utils.hpp"
#include "print.hpp"

namespace iris {

devapi_accel *devapi_accel::accel_ = NULL;

devapi_accel *
devapi_accel::factory()
{
    sdk_ret_t ret = SDK_RET_OK;
    void *mem = NULL;
    devapi_accel *accel = NULL;

    api_trace("accel create");

    mem = (devapi_accel *)DEVAPI_CALLOC(DEVAPI_MEM_ALLOC_ACCEL,
                                  sizeof(devapi_accel));
    if (mem) {
        accel = new (mem) devapi_accel();
        ret = accel->init_();
        if (ret != SDK_RET_OK) {
            accel->~devapi_accel();
            DEVAPI_FREE(DEVAPI_MEM_ALLOC_ACCEL, mem);
            accel = NULL;
            goto end;
        }
    }

end:
    return accel;
}

sdk_ret_t
devapi_accel::init_()
{
    accel_ = this;

    return SDK_RET_OK;
}

void
devapi_accel::destroy(devapi_accel *accel)
{
    if (!accel) {
        return;
    }
    api_trace("accel delete");

    accel->~devapi_accel();
    DEVAPI_FREE(DEVAPI_MEM_ALLOC_ACCEL, accel);
    devapi_accel::accel_ = NULL;
}

devapi_accel *
devapi_accel::find_or_create()
{
    devapi_accel *accel = NULL;

    accel = devapi_accel::get_accel();
    if (!accel) {
        NIC_LOG_DEBUG("Failed to find devapi_accel. Creating ...");
        accel = devapi_accel::factory();
        if (!accel) {
            NIC_LOG_ERR("Failed to create devapi_accel");
            goto end;
        }
    }
end:
    return accel;
}

sdk_ret_t
devapi_accel::accel_rgroup_add(string name,
                               uint64_t metrics_mem_addr,
                               uint32_t metrics_mem_size)
{
    sdk_ret_t                   ret = SDK_RET_OK;
    AccelRGroupAddRequestMsg    req_msg;
    AccelRGroupAddResponseMsg   rsp_msg;
    grpc::Status                status;

    auto req = req_msg.add_request();
    req->set_rgroup_name(name);
    req->set_metrics_mem_addr(metrics_mem_addr);
    req->set_metrics_mem_size(metrics_mem_size);
    VERIFY_HAL_RETURN();
    status = hal->accel_rgroup_add(req_msg, rsp_msg);
    if (!status.ok()) {
        NIC_FUNC_ERR("GRPC status {} {}", status.error_code(),
                     status.error_message());
        ret = SDK_RET_ERR;
        return ret;
    }
    auto rsp = rsp_msg.response(0);
    if (rsp.api_status() != types::API_STATUS_OK) {
        NIC_FUNC_ERR("API status {} rgroup_name {}", rsp.api_status(),
                     name);
        ret = SDK_RET_ERR;
        return ret;
    }

    return ret;
}

sdk_ret_t
devapi_accel::accel_rgroup_del(string name)
{
    sdk_ret_t                   ret = SDK_RET_OK;
    AccelRGroupDelRequestMsg    req_msg;
    AccelRGroupDelResponseMsg   rsp_msg;
    grpc::Status                status;

    auto req = req_msg.add_request();
    req->set_rgroup_name(name);
    VERIFY_HAL_RETURN();
    status = hal->accel_rgroup_del(req_msg, rsp_msg);
    if (!status.ok()) {
        NIC_FUNC_ERR("GRPC status {} {}", status.error_code(),
                     status.error_message());
        ret = SDK_RET_ERR;
        return ret;
    }
    auto rsp = rsp_msg.response(0);
    if (rsp.api_status() != types::API_STATUS_OK) {
        NIC_FUNC_ERR("API status {} rgroup_name {}", rsp.api_status(),
                     name);
        ret = SDK_RET_ERR;
        return ret;
    }

    return ret;
}

sdk_ret_t
devapi_accel::accel_rgroup_ring_add(string name,
                                    std::vector<std::pair<const std::string,uint32_t>>& ring_vec)
{
    sdk_ret_t                       ret = SDK_RET_OK;
    AccelRGroupRingAddRequestMsg    req_msg;
    AccelRGroupRingAddResponseMsg   rsp_msg;
    grpc::Status                    status;
    int                             i;

    for (i = 0; i < (int)ring_vec.size(); i++) {
        auto req = req_msg.add_request();
        req->set_rgroup_name(name);
        req->set_ring_name(ring_vec[i].first);
        req->set_ring_handle(ring_vec[i].second);
    }
    VERIFY_HAL_RETURN();
    status = hal->accel_rgroup_ring_add(req_msg, rsp_msg);
    if (!status.ok()) {
        NIC_FUNC_ERR("GRPC status {} {}", status.error_code(),
                     status.error_message());
        ret = SDK_RET_ERR;
        return ret;
    }

    assert(rsp_msg.response_size() == (int)ring_vec.size());
    for (i = 0; i < rsp_msg.response_size(); i++) {
        auto rsp = rsp_msg.response(i);
        if (rsp.api_status() != types::API_STATUS_OK) {
            NIC_FUNC_ERR("API status {} rgroup_name {} ring_name {}",
                         rsp.api_status(), name, ring_vec[i].first);
            ret = SDK_RET_ERR;
            return ret;
        }
    }

    return ret;
}

sdk_ret_t
devapi_accel::accel_rgroup_ring_del(string name,
                                    std::vector<std::pair<const std::string,uint32_t>>& ring_vec)
{
    sdk_ret_t                       ret = SDK_RET_OK;
    AccelRGroupRingDelRequestMsg    req_msg;
    AccelRGroupRingDelResponseMsg   rsp_msg;
    grpc::Status                    status;
    int                             i;

    for (i = 0; i < (int)ring_vec.size(); i++) {
        auto req = req_msg.add_request();
        req->set_rgroup_name(name);
        req->set_ring_name(ring_vec[i].first);
    }
    VERIFY_HAL_RETURN();
    status = hal->accel_rgroup_ring_del(req_msg, rsp_msg);
    if (!status.ok()) {
        NIC_FUNC_ERR("GRPC status {} {}", status.error_code(),
                     status.error_message());
        ret = SDK_RET_ERR;
        return ret;
    }

    assert(rsp_msg.response_size() == (int)ring_vec.size());
    for (i = 0; i < rsp_msg.response_size(); i++) {
        auto rsp = rsp_msg.response(i);
        if (rsp.api_status() != types::API_STATUS_OK) {
            NIC_FUNC_ERR("API status {} rgroup_name {} ring_name {}",
                         rsp.api_status(), name, ring_vec[i].first);
            ret = SDK_RET_ERR;
            return ret;
        }
    }

    return ret;
}

sdk_ret_t
devapi_accel::accel_rgroup_reset_set(string name, uint32_t sub_ring,
                                     bool reset_sense)
{
    sdk_ret_t                       ret = SDK_RET_OK;
    AccelRGroupResetSetRequestMsg   req_msg;
    AccelRGroupResetSetResponseMsg  rsp_msg;
    grpc::Status                    status;

    auto req = req_msg.add_request();
    req->set_rgroup_name(name);
    req->set_sub_ring(sub_ring);
    req->set_reset_sense(reset_sense);
    VERIFY_HAL_RETURN();
    status = hal->accel_rgroup_reset_set(req_msg, rsp_msg);
    if (!status.ok()) {
        NIC_FUNC_ERR("GRPC status {} {}", status.error_code(),
                     status.error_message());
        ret = SDK_RET_ERR;
        return ret;
    }
    auto rsp = rsp_msg.response(0);
    if (rsp.api_status() != types::API_STATUS_OK) {
        NIC_FUNC_ERR("API status {} rgroup_name {} last_ring_handle {} last_sub_ring {}",
                     rsp.api_status(), name, rsp.last_ring_handle(),
                     rsp.last_sub_ring());
        ret = SDK_RET_ERR;
        return ret;
    }
    return ret;
}

sdk_ret_t
devapi_accel::accel_rgroup_enable_set(string name, uint32_t sub_ring,
                                      bool enable_sense)
{
    sdk_ret_t                       ret = SDK_RET_OK;
    AccelRGroupEnableSetRequestMsg  req_msg;
    AccelRGroupEnableSetResponseMsg rsp_msg;
    grpc::Status                    status;

    auto req = req_msg.add_request();
    req->set_rgroup_name(name);
    req->set_sub_ring(sub_ring);
    req->set_enable_sense(enable_sense);
    VERIFY_HAL_RETURN();
    status = hal->accel_rgroup_enable_set(req_msg, rsp_msg);
    if (!status.ok()) {
        NIC_FUNC_ERR("GRPC status {} {}", status.error_code(),
                     status.error_message());
        ret = SDK_RET_ERR;
        return ret;
    }
    auto rsp = rsp_msg.response(0);
    if (rsp.api_status() != types::API_STATUS_OK) {
        NIC_FUNC_ERR("API status {} rgroup_name {} last_ring_handle {} last_sub_ring {}",
                     rsp.api_status(), name, rsp.last_ring_handle(),
                     rsp.last_sub_ring());
        ret = SDK_RET_ERR;
        return ret;
    }
    return ret;
}

sdk_ret_t
devapi_accel::accel_rgroup_pndx_set(string name, uint32_t sub_ring,
                                    uint32_t val, bool conditional)
{
    sdk_ret_t                       ret = SDK_RET_OK;
    AccelRGroupPndxSetRequestMsg    req_msg;
    AccelRGroupPndxSetResponseMsg   rsp_msg;
    grpc::Status                    status;

    auto req = req_msg.add_request();
    req->set_rgroup_name(name);
    req->set_sub_ring(sub_ring);
    req->set_val(val);
    req->set_conditional(conditional);
    VERIFY_HAL_RETURN();
    status = hal->accel_rgroup_pndx_set(req_msg, rsp_msg);
    if (!status.ok()) {
        NIC_FUNC_ERR("GRPC status {} {}", status.error_code(),
                     status.error_message());
        ret = SDK_RET_ERR;
        return ret;
    }
    auto rsp = rsp_msg.response(0);
    if (rsp.api_status() != types::API_STATUS_OK) {
        NIC_FUNC_ERR("API status {} rgroup_name {} last_ring_handle {} last_sub_ring {}",
                     rsp.api_status(), name, rsp.last_ring_handle(),
                     rsp.last_sub_ring());
        ret = SDK_RET_ERR;
        return ret;
    }
    return ret;
}

sdk_ret_t
devapi_accel::accel_rgroup_info_get(string name, uint32_t sub_ring,
                                    accel_rgroup_rinfo_rsp_cb_t rsp_cb_func,
                                    void *user_ctx, uint32_t *ret_num_entries)
{
    sdk_ret_t                       ret = SDK_RET_OK;
    AccelRGroupInfoGetRequestMsg    req_msg;
    AccelRGroupInfoGetResponseMsg   rsp_msg;
    grpc::Status                    status;
    int                             i;

    *ret_num_entries = 0;
    auto req = req_msg.add_request();
    req->set_rgroup_name(name);
    req->set_sub_ring(sub_ring);
    VERIFY_HAL_RETURN();
    status = hal->accel_rgroup_info_get(req_msg, rsp_msg);
    if (!status.ok()) {
        NIC_FUNC_ERR("GRPC status {} {}", status.error_code(),
                     status.error_message());
        ret = SDK_RET_ERR;
        return ret;
    }
    auto rsp = rsp_msg.response(0);
    if (rsp.api_status() != types::API_STATUS_OK) {
        NIC_FUNC_ERR("API status {} rgroup_name {}",
                     rsp.api_status(), name);
        ret = SDK_RET_ERR;
        return ret;
    }

    *ret_num_entries = (uint32_t)rsp.ring_info_spec_size();
    for (i = 0; i < (int)*ret_num_entries; i++) {
        accel_rgroup_rinfo_rsp_t rinfo = {0};
        auto spec = rsp.ring_info_spec(i);

        rinfo.ring_handle = spec.ring_handle();
        rinfo.sub_ring = spec.sub_ring();
        rinfo.base_pa = spec.base_pa();
        rinfo.pndx_pa = spec.pndx_pa();
        rinfo.shadow_pndx_pa = spec.shadow_pndx_pa();
        rinfo.opaque_tag_pa = spec.opaque_tag_pa();
        rinfo.opaque_tag_size = spec.opaque_tag_size();
        rinfo.ring_size = spec.ring_size();
        rinfo.desc_size = spec.desc_size();
        rinfo.pndx_size = spec.pndx_size();
        rinfo.sw_reset_capable = spec.sw_reset_capable();
        rinfo.sw_enable_capable = spec.sw_enable_capable();
        (*rsp_cb_func)(user_ctx, rinfo);
    }
    return ret;
}

sdk_ret_t
devapi_accel::accel_rgroup_indices_get(string name, uint32_t sub_ring,
                                       accel_rgroup_rindices_rsp_cb_t rsp_cb_func,
                                       void *user_ctx, uint32_t *ret_num_entries)
{
    sdk_ret_t                           ret = SDK_RET_OK;
    AccelRGroupIndicesGetRequestMsg     req_msg;
    AccelRGroupIndicesGetResponseMsg    rsp_msg;
    grpc::Status                        status;
    int                                 i;

    auto req = req_msg.add_request();
    req->set_rgroup_name(name);
    req->set_sub_ring(sub_ring);
    VERIFY_HAL_RETURN();
    status = hal->accel_rgroup_indices_get(req_msg, rsp_msg);
    if (!status.ok()) {
        NIC_FUNC_ERR("GRPC status {} {}", status.error_code(),
                     status.error_message());
        ret = SDK_RET_ERR;
        return ret;
    }
    auto rsp = rsp_msg.response(0);
    if (rsp.api_status() != types::API_STATUS_OK) {
        NIC_FUNC_ERR("API status {} rgroup_name {}",
                     rsp.api_status(), name);
        ret = SDK_RET_ERR;
        return ret;
    }

    *ret_num_entries = (uint32_t)rsp.ring_indices_spec_size();
    for (i = 0; i < (int)*ret_num_entries; i++) {
        accel_rgroup_rindices_rsp_t rindices = {0};
        auto spec = rsp.ring_indices_spec(i);

        rindices.ring_handle = spec.ring_handle();
        rindices.sub_ring = spec.sub_ring();
        rindices.pndx = spec.pndx();
        rindices.cndx = spec.cndx();
        rindices.endx = spec.endx();
        (*rsp_cb_func)(user_ctx, rindices);
    }
    return ret;
}

sdk_ret_t
devapi_accel::accel_rgroup_metrics_get(string name, uint32_t sub_ring,
                                       accel_rgroup_rmetrics_rsp_cb_t rsp_cb_func,
                                       void *user_ctx, uint32_t *ret_num_entries)
{
    sdk_ret_t                           ret = SDK_RET_OK;
    AccelRGroupMetricsGetRequestMsg     req_msg;
    AccelRGroupMetricsGetResponseMsg    rsp_msg;
    grpc::Status                        status;
    int                                 i;

    auto req = req_msg.add_request();
    req->set_rgroup_name(name);
    req->set_sub_ring(sub_ring);
    VERIFY_HAL_RETURN();
    status = hal->accel_rgroup_metrics_get(req_msg, rsp_msg);
    if (!status.ok()) {
        NIC_FUNC_ERR("GRPC status {} {}", status.error_code(),
                     status.error_message());
        ret = SDK_RET_ERR;
        return ret;
    }
    auto rsp = rsp_msg.response(0);
    if (rsp.api_status() != types::API_STATUS_OK) {
        NIC_FUNC_ERR("API status {} rgroup_name {}",
                     rsp.api_status(), name);
        ret = SDK_RET_ERR;
        return ret;
    }

    *ret_num_entries = (uint32_t)rsp.ring_metrics_spec_size();
    for (i = 0; i < (int)*ret_num_entries; i++) {
        accel_rgroup_rmetrics_rsp_t rmetrics = {0};
        auto spec = rsp.ring_metrics_spec(i);

        rmetrics.ring_handle = spec.ring_handle();
        rmetrics.sub_ring = spec.sub_ring();
        rmetrics.input_bytes = spec.input_bytes();
        rmetrics.output_bytes = spec.output_bytes();
        rmetrics.soft_resets = spec.soft_resets();
        (*rsp_cb_func)(user_ctx, rmetrics);
    }
    return ret;
}

sdk_ret_t
devapi_accel::accel_rgroup_misc_get(string name, uint32_t sub_ring,
                                    accel_rgroup_rmisc_rsp_cb_t rsp_cb_func,
                                    void *user_ctx, uint32_t *ret_num_entries)
{
    sdk_ret_t                           ret = SDK_RET_OK;
    AccelRGroupMiscGetRequestMsg        req_msg;
    AccelRGroupMiscGetResponseMsg       rsp_msg;
    grpc::Status                        status;

    auto req = req_msg.add_request();
    req->set_rgroup_name(name);
    req->set_sub_ring(sub_ring);
    VERIFY_HAL_RETURN();
    status = hal->accel_rgroup_misc_get(req_msg, rsp_msg);
    if (!status.ok()) {
        NIC_FUNC_ERR("GRPC status {} {}", status.error_code(),
                     status.error_message());
        ret = SDK_RET_ERR;
        return ret;
    }
    auto rsp = rsp_msg.response(0);
    if (rsp.api_status() != types::API_STATUS_OK) {
        NIC_FUNC_ERR("API status {} rgroup_name {}",
                     rsp.api_status(), name);
        ret = SDK_RET_ERR;
        return ret;
    }

    *ret_num_entries = (uint32_t)rsp.ring_misc_spec_size();
    for (int i = 0; i < (int)*ret_num_entries; i++) {
        accel_rgroup_rmisc_rsp_t rmisc = {0};
        auto spec = rsp.ring_misc_spec(i);

        rmisc.ring_handle = spec.ring_handle();
        rmisc.sub_ring = spec.sub_ring();
        rmisc.num_reg_vals = std::min(spec.num_reg_vals(),
                                      (uint32_t)ACCEL_RING_NUM_REGS_MAX);
        for (uint32_t j = 0; j < rmisc.num_reg_vals; j++) {
             auto reg_val = spec.reg_val(j);
             strncpy(rmisc.reg_val[j].name, reg_val.name().c_str(),
                     sizeof(rmisc.reg_val[j].name));
             rmisc.reg_val[j].name[sizeof(rmisc.reg_val[j].name)-1] = '\0';
             rmisc.reg_val[j].val = reg_val.val();
        }
        (*rsp_cb_func)(user_ctx, rmisc);
    }
    return ret;
}

sdk_ret_t
devapi_accel::crypto_key_index_upd(uint32_t key_index,
                                   crypto_key_type_t key_type,
                                   void *key, uint32_t key_size)
{
    sdk_ret_t                           ret = SDK_RET_OK;
    CryptoKeyCreateWithIdRequestMsg     create_req_msg;
    CryptoKeyCreateWithIdResponseMsg    create_rsp_msg;
    CryptoKeyUpdateRequestMsg           update_req_msg;
    CryptoKeyUpdateResponseMsg          update_rsp_msg;
    grpc::Status                        status;

    auto create_req = create_req_msg.add_request();
    create_req->set_keyindex(key_index);
    create_req->set_allow_dup_alloc(true);
    VERIFY_HAL_RETURN();
    status = hal->crypto_create(create_req_msg, create_rsp_msg);
    if (!status.ok()) {
        NIC_FUNC_ERR("GRPC create status {} {}", status.error_code(),
                     status.error_message());
        ret = SDK_RET_ERR;
        return ret;
    }

    auto create_rsp = create_rsp_msg.response(0);
    if (create_rsp.keyindex() != key_index) {
        NIC_FUNC_ERR("GRPC create expected key_index {} actual {}",
                     key_index, create_rsp.keyindex());
        ret = SDK_RET_ERR;
        return ret;
    }

    auto update_req = update_req_msg.add_request();
    auto update_spec = update_req->mutable_key();
    update_spec->set_key(key, key_size);
    update_spec->set_keyindex(key_index);
    update_spec->set_key_size(key_size);
    update_spec->set_key_type((types::CryptoKeyType)key_type);

    VERIFY_HAL_RETURN();
    status = hal->crypto_update(update_req_msg, update_rsp_msg);
    if (!status.ok()) {
        NIC_FUNC_ERR("GRPC update status {} {}", status.error_code(),
                     status.error_message());
        ret = SDK_RET_ERR;
        return ret;
    }
    auto update_rsp = update_rsp_msg.response(0);
    if (update_rsp.api_status() != types::API_STATUS_OK) {
        NIC_FUNC_ERR("API update status {} key_index {}",
                     update_rsp.api_status(), key_index);
        ret = SDK_RET_ERR;
        return ret;
    }
    return ret;
}





}    // namespace iris
