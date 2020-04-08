//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "accel_rgroup.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/sdk/asic/pd/pd_accel_rgroup.hpp"

using namespace sdk::asic::pd::accel;

namespace hal {

hal_ret_t
accel_rgroup_init(int tid, uint32_t accel_total_rings)
{
   sdk_ret_t sdk_ret;

   sdk_ret = asicpd_accel_rgroup_init(tid, accel_total_rings);
   return hal_sdk_ret_to_hal_ret(sdk_ret);
}


hal_ret_t
accel_rgroup_fini(int tid)
{
   sdk_ret_t sdk_ret;
   sdk_ret = asicpd_accel_rgroup_fini(tid);
   return hal_sdk_ret_to_hal_ret(sdk_ret);
}


hal_ret_t
accel_rgroup_add(const AccelRGroupAddRequest& request,
                 AccelRGroupAddResponse *response)
{
    sdk_ret_t sdk_ret;
    hal_ret_t ret;

    sdk_ret = asicpd_accel_rgroup_add(
                            request.rgroup_name().c_str(),
                            request.metrics_mem_addr(),
                            request.metrics_mem_size()
                            );
   ret = hal_sdk_ret_to_hal_ret(sdk_ret);

   response->set_api_status(ret == HAL_RET_OK ? types::API_STATUS_OK :
                                                types::API_STATUS_ERR);
   return ret;
}

hal_ret_t
accel_rgroup_del(const AccelRGroupDelRequest& request,
                 AccelRGroupDelResponse *response)
{
    sdk_ret_t   sdk_ret;
    hal_ret_t   ret;

    sdk_ret = asicpd_accel_rgroup_del(request.rgroup_name().c_str());
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);

    response->set_api_status(ret == HAL_RET_OK ? types::API_STATUS_OK :
                                                 types::API_STATUS_ERR);
    return ret;
}

hal_ret_t
accel_rgroup_ring_add(const AccelRGroupRingAddRequest& request,
                      AccelRGroupRingAddResponse *response)
{
    sdk_ret_t   sdk_ret;
    hal_ret_t   ret;

    sdk_ret = asicpd_accel_rgroup_ring_add(
                             request.rgroup_name().c_str(),
                             request.ring_name().c_str(),
                             request.ring_handle()
                            );
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);

    response->set_ring_handle(request.ring_handle());
    response->set_api_status(ret == HAL_RET_OK ? types::API_STATUS_OK :
                                                 types::API_STATUS_ERR);
    return ret;
}

hal_ret_t
accel_rgroup_ring_del(const AccelRGroupRingDelRequest& request,
                      AccelRGroupRingDelResponse *response)
{
    sdk_ret_t   sdk_ret;
    hal_ret_t   ret;

    sdk_ret = asicpd_accel_rgroup_ring_del(
                             request.rgroup_name().c_str(),
                             request.ring_name().c_str()
                             );
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);

    response->set_api_status(ret == HAL_RET_OK ? types::API_STATUS_OK :
                                                 types::API_STATUS_ERR);
    return ret;
}

hal_ret_t
accel_rgroup_reset_set(const AccelRGroupResetSetRequest& request,
                       AccelRGroupResetSetResponse *response)
{
    sdk_ret_t   sdk_ret;
    hal_ret_t   ret;
    uint32_t    last_ring_handle = 0;
    uint32_t    last_sub_ring = 0;

    sdk_ret = asicpd_accel_rgroup_reset_set(
                                            request.rgroup_name().c_str(),
                                            request.sub_ring(),
                                            &last_ring_handle,
                                            &last_sub_ring,
                                            request.reset_sense()
                                           );
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);

    response->set_api_status(ret == HAL_RET_OK ? types::API_STATUS_OK :
                                                 types::API_STATUS_ERR);

    response->set_last_ring_handle(last_ring_handle);
    response->set_last_sub_ring(last_sub_ring);

    return ret;
}

hal_ret_t
accel_rgroup_enable_set(const AccelRGroupEnableSetRequest& request,
                        AccelRGroupEnableSetResponse *response)
{
    sdk_ret_t   sdk_ret;
    hal_ret_t   ret;
    uint32_t    last_ring_handle = 0;
    uint32_t    last_sub_ring = 0;

    sdk_ret = asicpd_accel_rgroup_enable_set(
                       request.rgroup_name().c_str(),
                       request.sub_ring(),
                       &last_ring_handle,
                       &last_sub_ring,
                       request.enable_sense());
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);

    response->set_api_status(ret == HAL_RET_OK ? types::API_STATUS_OK :
                                                 types::API_STATUS_ERR);

    response->set_last_ring_handle(last_ring_handle);
    response->set_last_sub_ring(last_sub_ring);
    return ret;
}

hal_ret_t
accel_rgroup_pndx_set(const AccelRGroupPndxSetRequest& request,
                      AccelRGroupPndxSetResponse *response)
{
    sdk_ret_t   sdk_ret;
    hal_ret_t   ret;
    uint32_t    last_ring_handle = 0;
    uint32_t    last_sub_ring = 0;

    sdk_ret  = asicpd_accel_rgroup_pndx_set(
                                request.rgroup_name().c_str(),
                                request.sub_ring(),
                                &last_ring_handle,
                                &last_sub_ring,
                                request.val(),
                                request.conditional());
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);


    response->set_api_status(ret == HAL_RET_OK ? types::API_STATUS_OK :
                                                 types::API_STATUS_ERR);
    response->set_last_ring_handle(last_ring_handle);
    response->set_last_sub_ring(last_sub_ring);
    return ret;
}

typedef struct {
    AccelRGroupInfoGetResponse  *response;
    uint32_t                    rsp_count;
} rgroup_info_get_ctx_t;

static void
rgroup_info_get_cb(void *user_ctx,
                   const accel_rgroup_ring_info_t& info)
{
    rgroup_info_get_ctx_t       *ctx = (rgroup_info_get_ctx_t *)user_ctx;
    AccelRGroupInfoGetResponse  *response = ctx->response;

    response->add_ring_info_spec();
    response->mutable_ring_info_spec(ctx->rsp_count)->set_ring_handle(info.ring_handle);
    response->mutable_ring_info_spec(ctx->rsp_count)->set_sub_ring(info.sub_ring);
    response->mutable_ring_info_spec(ctx->rsp_count)->set_base_pa(info.base_pa);
    response->mutable_ring_info_spec(ctx->rsp_count)->set_pndx_pa(info.pndx_pa);
    response->mutable_ring_info_spec(ctx->rsp_count)->set_shadow_pndx_pa(info.shadow_pndx_pa);
    response->mutable_ring_info_spec(ctx->rsp_count)->set_opaque_tag_pa(info.opaque_tag_pa);
    response->mutable_ring_info_spec(ctx->rsp_count)->set_opaque_tag_size(info.opaque_tag_size);
    response->mutable_ring_info_spec(ctx->rsp_count)->set_ring_size(info.ring_size);
    response->mutable_ring_info_spec(ctx->rsp_count)->set_desc_size(info.desc_size);
    response->mutable_ring_info_spec(ctx->rsp_count)->set_pndx_size(info.pndx_size);
    response->mutable_ring_info_spec(ctx->rsp_count)->set_sw_reset_capable(info.sw_reset_capable);
    response->mutable_ring_info_spec(ctx->rsp_count)->set_sw_enable_capable(info.sw_enable_capable);
    ctx->rsp_count++;
}

hal_ret_t
accel_rgroup_info_get(const AccelRGroupInfoGetRequest& request,
                      AccelRGroupInfoGetResponse *response)
{
    rgroup_info_get_ctx_t               ctx = {0};
    sdk_ret_t                           sdk_ret;
    hal_ret_t                           ret;

    ctx.response = response;
    sdk_ret = asicpd_accel_rgroup_info_get(
                                request.rgroup_name().c_str(),
                                request.sub_ring(),
                                &rgroup_info_get_cb,
                                &ctx
                               );
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);

    if (ret == HAL_RET_OK) {
        response->set_api_status(types::API_STATUS_OK);
        return ret;
    }

    HAL_TRACE_ERR("{} rgroup_name {} error {}", __FUNCTION__,
                  request.rgroup_name().c_str(), ret);
    response->set_api_status(types::API_STATUS_ERR);
    return ret;
}

typedef struct {
    AccelRGroupIndicesGetResponse   *response;
    uint32_t                        rsp_count;
} rgroup_indices_get_ctx_t;

static void
rgroup_indices_get_cb(void *user_ctx,
                      const accel_rgroup_ring_indices_t& indices)
{
    rgroup_indices_get_ctx_t        *ctx = (rgroup_indices_get_ctx_t *)user_ctx;
    AccelRGroupIndicesGetResponse   *response = ctx->response;

    response->add_ring_indices_spec();
    response->mutable_ring_indices_spec(ctx->rsp_count)->set_ring_handle(indices.ring_handle);
    response->mutable_ring_indices_spec(ctx->rsp_count)->set_sub_ring(indices.sub_ring);
    response->mutable_ring_indices_spec(ctx->rsp_count)->set_pndx(indices.pndx);
    response->mutable_ring_indices_spec(ctx->rsp_count)->set_cndx(indices.cndx);
    response->mutable_ring_indices_spec(ctx->rsp_count)->set_endx(indices.endx);
    ctx->rsp_count++;
}

hal_ret_t
accel_rgroup_indices_get(const AccelRGroupIndicesGetRequest& request,
                         AccelRGroupIndicesGetResponse *response)
{
    rgroup_indices_get_ctx_t                ctx = {0};
    sdk_ret_t                               sdk_ret;
    hal_ret_t                               ret;

    ctx.response = response;
    sdk_ret = asicpd_accel_rgroup_indices_get(request.rgroup_name().c_str(),
                                   request.sub_ring(),
                                   (accel_rgroup_ring_indices_cb_t) &rgroup_indices_get_cb,
                                   &ctx);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);

    if (ret == HAL_RET_OK) {
        response->set_api_status(types::API_STATUS_OK);
        return ret;
    }

    HAL_TRACE_ERR("{} rgroup_name {} error {}", __FUNCTION__,
                  request.rgroup_name().c_str(), ret);
    response->set_api_status(types::API_STATUS_ERR);
    return ret;
}

typedef struct {
    AccelRGroupMetricsGetResponse   *response;
    uint32_t                        rsp_count;
} rgroup_metrics_get_ctx_t;

static void
rgroup_metrics_get_cb(void *user_ctx,
                      const accel_rgroup_ring_metrics_t& metrics)
{
    rgroup_metrics_get_ctx_t        *ctx = (rgroup_metrics_get_ctx_t *)user_ctx;
    AccelRGroupMetricsGetResponse   *response = ctx->response;

    response->add_ring_metrics_spec();
    response->mutable_ring_metrics_spec(ctx->rsp_count)->set_ring_handle(metrics.ring_handle);
    response->mutable_ring_metrics_spec(ctx->rsp_count)->set_sub_ring(metrics.sub_ring);
    response->mutable_ring_metrics_spec(ctx->rsp_count)->set_input_bytes(metrics.input_bytes);
    response->mutable_ring_metrics_spec(ctx->rsp_count)->set_output_bytes(metrics.output_bytes);
    response->mutable_ring_metrics_spec(ctx->rsp_count)->set_soft_resets(metrics.soft_resets);
    ctx->rsp_count++;
}

hal_ret_t
accel_rgroup_metrics_get(const AccelRGroupMetricsGetRequest& request,
                         AccelRGroupMetricsGetResponse *response)
{
    rgroup_metrics_get_ctx_t                ctx = {0};
    sdk_ret_t                               sdk_ret;
    hal_ret_t                               ret;

    ctx.response = response;
    sdk_ret = asicpd_accel_rgroup_metrics_get(
                                   request.rgroup_name().c_str(),
                                   request.sub_ring(),
                                   (accel_rgroup_ring_metrics_cb_t) &rgroup_metrics_get_cb,
                                   &ctx);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);

    if (ret == HAL_RET_OK) {
        response->set_api_status(types::API_STATUS_OK);
        return ret;
    }

    HAL_TRACE_ERR("{} rgroup_name {} error {}", __FUNCTION__,
                  request.rgroup_name().c_str(), ret);
    response->set_api_status(types::API_STATUS_ERR);
    return ret;
}

typedef struct {
    AccelRGroupMiscGetResponse      *response;
    uint32_t                        rsp_count;
} rgroup_misc_get_ctx_t;

static void
rgroup_misc_get_cb(void *user_ctx,
                   const accel_rgroup_ring_misc_t& misc)
{
    rgroup_misc_get_ctx_t        *ctx = (rgroup_misc_get_ctx_t *)user_ctx;
    AccelRGroupMiscGetResponse   *response = ctx->response;
    uint32_t                     num_reg_vals;

    response->add_ring_misc_spec();
    response->mutable_ring_misc_spec(ctx->rsp_count)->set_ring_handle(misc.ring_handle);
    response->mutable_ring_misc_spec(ctx->rsp_count)->set_sub_ring(misc.sub_ring);
    num_reg_vals = std::min(misc.num_reg_vals, (uint32_t)ACCEL_RING_NUM_REGS_MAX);
    response->mutable_ring_misc_spec(ctx->rsp_count)->set_num_reg_vals(num_reg_vals);
    for (uint32_t i = 0; i < num_reg_vals; i++) {
        response->mutable_ring_misc_spec(ctx->rsp_count)->add_reg_val();
        response->mutable_ring_misc_spec(ctx->rsp_count)->mutable_reg_val(i)->set_name(misc.reg_val[i].name);
        response->mutable_ring_misc_spec(ctx->rsp_count)->mutable_reg_val(i)->set_val(misc.reg_val[i].val);
    }
    ctx->rsp_count++;
}

hal_ret_t
accel_rgroup_misc_get(const AccelRGroupMiscGetRequest& request,
                      AccelRGroupMiscGetResponse *response)
{
    rgroup_misc_get_ctx_t                ctx = {0};
    sdk_ret_t                            sdk_ret;
    hal_ret_t                            ret;

    ctx.response = response;
    sdk_ret = asicpd_accel_rgroup_misc_get(
                                   request.rgroup_name().c_str(),
                                   request.sub_ring(),
                                   (accel_rgroup_ring_misc_cb_t) &rgroup_misc_get_cb,
                                   &ctx);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);

    if (ret == HAL_RET_OK) {
        response->set_api_status(types::API_STATUS_OK);
        return ret;
    }

    HAL_TRACE_ERR("{} rgroup_name {} error {}", __FUNCTION__,
                  request.rgroup_name().c_str(), ret);
    response->set_api_status(types::API_STATUS_ERR);
    return ret;
}

}    // namespace hal
