//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/hal/src/internal/accel_rgroup.hpp"
#include "nic/include/pd_api.hpp"

namespace hal {

hal_ret_t
AccelRGroupAdd(const AccelRGroupAddRequest& request,
               AccelRGroupAddResponse *response)
{
    pd::pd_capri_accel_rgroup_add_args_t    args = {0};
    pd::pd_func_args_t                      pd_func_args = {0};
    hal_ret_t                               ret;

    args.rgroup_name = request.rgroup_name().c_str();
    pd_func_args.pd_capri_accel_rgroup_add = &args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_ACCEL_RGROUP_ADD, &pd_func_args);
    response->set_api_status(ret == HAL_RET_OK ? types::API_STATUS_OK :
                                                 types::API_STATUS_ERR);
    return ret;
}

hal_ret_t
AccelRGroupDel(const AccelRGroupDelRequest& request,
               AccelRGroupDelResponse *response)
{
    pd::pd_capri_accel_rgroup_del_args_t    args = {0};
    pd::pd_func_args_t                      pd_func_args = {0};
    hal_ret_t                               ret;

    args.rgroup_name = request.rgroup_name().c_str();
    pd_func_args.pd_capri_accel_rgroup_del = &args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_ACCEL_RGROUP_DEL, &pd_func_args);
    response->set_api_status(ret == HAL_RET_OK ? types::API_STATUS_OK :
                                                 types::API_STATUS_ERR);
    return ret;
}

hal_ret_t
AccelRGroupRingAdd(const AccelRGroupRingAddRequest& request,
                   AccelRGroupRingAddResponse *response)
{
    pd::pd_capri_accel_rgroup_ring_add_args_t   args = {0};
    pd::pd_func_args_t                          pd_func_args = {0};
    hal_ret_t                                   ret;

    args.rgroup_name = request.rgroup_name().c_str();
    args.ring_name = request.ring_name().c_str();
    args.ring_handle = request.ring_handle();
    pd_func_args.pd_capri_accel_rgroup_ring_add = &args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_ACCEL_RGROUP_RING_ADD, &pd_func_args);

    response->set_ring_handle(args.ring_handle);
    response->set_api_status(ret == HAL_RET_OK ? types::API_STATUS_OK :
                                                 types::API_STATUS_ERR);
    return ret;
}

hal_ret_t
AccelRGroupRingDel(const AccelRGroupRingDelRequest& request,
                   AccelRGroupRingDelResponse *response)
{
    pd::pd_capri_accel_rgroup_ring_del_args_t   args = {0};
    pd::pd_func_args_t                          pd_func_args = {0};
    hal_ret_t                                   ret;

    args.rgroup_name = request.rgroup_name().c_str();
    args.ring_name = request.ring_name().c_str();
    pd_func_args.pd_capri_accel_rgroup_ring_del = &args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_ACCEL_RGROUP_RING_DEL, &pd_func_args);

    response->set_api_status(ret == HAL_RET_OK ? types::API_STATUS_OK :
                                                 types::API_STATUS_ERR);
    return ret;
}

hal_ret_t
AccelRGroupResetSet(const AccelRGroupResetSetRequest& request,
                    AccelRGroupResetSetResponse *response)
{
    pd::pd_capri_accel_rgroup_reset_set_args_t  args = {0};
    pd::pd_func_args_t                          pd_func_args = {0};
    hal_ret_t                                   ret;

    args.rgroup_name = request.rgroup_name().c_str();
    args.sub_ring = request.sub_ring();
    args.reset_sense = request.reset_sense();
    pd_func_args.pd_capri_accel_rgroup_reset_set = &args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_ACCEL_RGROUP_RESET_SET, &pd_func_args);
    response->set_api_status(ret == HAL_RET_OK ? types::API_STATUS_OK :
                                                 types::API_STATUS_ERR);
    response->set_last_ring_handle(args.last_ring_handle);
    response->set_last_sub_ring(args.last_sub_ring);
    return ret;
}

hal_ret_t
AccelRGroupEnableSet(const AccelRGroupEnableSetRequest& request,
                     AccelRGroupEnableSetResponse *response)
{
    pd::pd_capri_accel_rgroup_enable_set_args_t args = {0};
    pd::pd_func_args_t                          pd_func_args = {0};
    hal_ret_t                                   ret;

    args.rgroup_name = request.rgroup_name().c_str();
    args.sub_ring = request.sub_ring();
    args.enable_sense = request.enable_sense();
    pd_func_args.pd_capri_accel_rgroup_enable_set = &args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_ACCEL_RGROUP_ENABLE_SET, &pd_func_args);
    response->set_api_status(ret == HAL_RET_OK ? types::API_STATUS_OK :
                                                 types::API_STATUS_ERR);
    response->set_last_ring_handle(args.last_ring_handle);
    response->set_last_sub_ring(args.last_sub_ring);
    return ret;
}

hal_ret_t
AccelRGroupPndxSet(const AccelRGroupPndxSetRequest& request,
                   AccelRGroupPndxSetResponse *response)
{
    pd::pd_capri_accel_rgroup_pndx_set_args_t   args = {0};
    pd::pd_func_args_t                          pd_func_args = {0};
    hal_ret_t                                   ret;

    args.rgroup_name = request.rgroup_name().c_str();
    args.sub_ring = request.sub_ring();
    args.val = request.val();
    args.conditional = request.conditional();
    pd_func_args.pd_capri_accel_rgroup_pndx_set = &args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_ACCEL_RGROUP_PNDX_SET, &pd_func_args);
    response->set_api_status(ret == HAL_RET_OK ? types::API_STATUS_OK :
                                                 types::API_STATUS_ERR);
    response->set_last_ring_handle(args.last_ring_handle);
    response->set_last_sub_ring(args.last_sub_ring);
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
AccelRGroupInfoGet(const AccelRGroupInfoGetRequest& request,
                   AccelRGroupInfoGetResponse *response)
{
    pd::pd_capri_accel_rgroup_info_get_args_t   args = {0};
    pd::pd_func_args_t                          pd_func_args = {0};
    rgroup_info_get_ctx_t                       ctx = {0};
    hal_ret_t                                   ret;

    args.rgroup_name = request.rgroup_name().c_str();
    args.sub_ring = request.sub_ring();
    ctx.response = response;
    args.cb_func = &rgroup_info_get_cb;
    args.usr_ctx = &ctx;
    pd_func_args.pd_capri_accel_rgroup_info_get = &args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_ACCEL_RGROUP_INFO_GET, &pd_func_args);
    if (ret == HAL_RET_OK) {
        response->set_api_status(types::API_STATUS_OK);
        return ret;
    }

    HAL_TRACE_ERR("{} rgroup_name {} error {}", __FUNCTION__,
                  args.rgroup_name, ret);
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
    ctx->rsp_count++;
}

hal_ret_t
AccelRGroupIndicesGet(const AccelRGroupIndicesGetRequest& request,
                      AccelRGroupIndicesGetResponse *response)
{
    pd::pd_capri_accel_rgroup_indices_get_args_t    args = {0};
    pd::pd_func_args_t                              pd_func_args = {0};
    rgroup_indices_get_ctx_t                        ctx = {0};
    hal_ret_t                                       ret;

    args.rgroup_name = request.rgroup_name().c_str();
    args.sub_ring = request.sub_ring();
    ctx.response = response;
    args.cb_func = &rgroup_indices_get_cb;
    args.usr_ctx = &ctx;
    pd_func_args.pd_capri_accel_rgroup_indices_get = &args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_ACCEL_RGROUP_INDICES_GET, &pd_func_args);
    if (ret == HAL_RET_OK) {
        response->set_api_status(types::API_STATUS_OK);
        return ret;
    }

    HAL_TRACE_ERR("{} rgroup_name {} error {}", __FUNCTION__,
                  args.rgroup_name, ret);
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
AccelRGroupMetricsGet(const AccelRGroupMetricsGetRequest& request,
                      AccelRGroupMetricsGetResponse *response)
{
    pd::pd_capri_accel_rgroup_metrics_get_args_t    args = {0};
    pd::pd_func_args_t                              pd_func_args = {0};
    rgroup_metrics_get_ctx_t                        ctx = {0};
    hal_ret_t                                       ret;

    args.rgroup_name = request.rgroup_name().c_str();
    args.sub_ring = request.sub_ring();
    ctx.response = response;
    args.cb_func = &rgroup_metrics_get_cb;
    args.usr_ctx = &ctx;
    pd_func_args.pd_capri_accel_rgroup_metrics_get = &args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_ACCEL_RGROUP_METRICS_GET, &pd_func_args);
    if (ret == HAL_RET_OK) {
        response->set_api_status(types::API_STATUS_OK);
        return ret;
    }

    HAL_TRACE_ERR("{} rgroup_name {} error {}", __FUNCTION__,
                  args.rgroup_name, ret);
    response->set_api_status(types::API_STATUS_ERR);
    return ret;
}

}    // namespace hal
