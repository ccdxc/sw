#include "nic/include/hal_lock.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/iris/tenant_pd.hpp"
#include "nic/p4/nw/include/defines.h"
#include "if_pd_utils.hpp"

namespace hal {
namespace pd {

//-----------------------------------------------------------------------------
// PD Tenant Create
//-----------------------------------------------------------------------------
hal_ret_t
pd_tenant_create (pd_tenant_args_t *args)
{
    hal_ret_t               ret;
    pd_tenant_t             *tenant_pd;

    HAL_ASSERT_RETURN((args != NULL), HAL_RET_INVALID_ARG);
    HAL_TRACE_DEBUG("pd-tenant:{}:creating pd state for tenant {}",
                    __FUNCTION__, args->tenant->tenant_id);

    // allocate PD tenant state
    tenant_pd = tenant_pd_alloc_init();
    if (tenant_pd == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    // Link PI & PD
    link_pi_pd(tenant_pd, args->tenant);

    // allocate resources
    ret = tenant_pd_alloc_res(tenant_pd);

end:
    if (ret != HAL_RET_OK) {
        tenant_pd_cleanup(tenant_pd);
    }

    return ret;
}

//-----------------------------------------------------------------------------
// PD Tenant Update
//-----------------------------------------------------------------------------
hal_ret_t
pd_tenant_update (pd_tenant_args_t *args)
{
    // Nothing to do for now
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// PD Tenant Delete
//-----------------------------------------------------------------------------
hal_ret_t
pd_tenant_delete (pd_tenant_args_t *args)
{
    hal_ret_t      ret = HAL_RET_OK;
    pd_tenant_t    *tenant_pd;

    HAL_ASSERT_RETURN((args != NULL), HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN((args->tenant != NULL), HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN((args->tenant->pd != NULL), HAL_RET_INVALID_ARG);
    HAL_TRACE_DEBUG("pd-tenant:{}:Deleting pd state for tenant {}",
                    __FUNCTION__, args->tenant->tenant_id);
    tenant_pd = (pd_tenant_t *)args->tenant->pd;

    ret = tenant_pd_cleanup(tenant_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-tenant:{}:failed pd tenant delete",
                      __FUNCTION__);
    }

    return ret;
}

hal_ret_t
pd_tenant_program_input_mapping_table(ip_prefix_t *ip_prefix,
                                      uint8_t tunnel_type,
                                      bool inner_v4_vld,
                                      bool inner_v6_vld,
                                      uint8_t actionid,
                                      p4pd_table_id tbl_id, uint32_t *idx)
{
    hal_ret_t                           ret = HAL_RET_OK;
    input_mapping_native_swkey_t        key = {0};
    input_mapping_native_swkey_mask_t   mask = {0};
    input_mapping_native_actiondata     data = {0};
    Tcam                                *tcam;
    uint32_t                            ret_idx;

    tcam = g_hal_state_pd->tcam_table(tbl_id);
    HAL_ASSERT(tcam != NULL);
    /* Input mapping native and tunneled tables have the same key, mask and data
     * So, we can populate the structs and typecast accordingly */
    memset(&key, 0, sizeof(input_mapping_native_swkey_t));
    memset(&mask, 0, sizeof(input_mapping_native_swkey_mask_t));
    key.inner_ipv4_valid = (uint8_t)inner_v4_vld;
    key.inner_ipv6_valid = (uint8_t)inner_v6_vld;
    key.tunnel_metadata_tunnel_type = tunnel_type;
    mask.inner_ipv4_valid_mask = 0xFF;
    mask.inner_ipv6_valid_mask = 0xFF;
    mask.tunnel_metadata_tunnel_type_mask = 0xFF;

    if (ip_prefix->addr.af == IP_AF_IPV4) {
        key.ipv4_valid = 1;
        key.input_mapping_native_u1.ipv4_dstAddr = ip_prefix->addr.addr.v4_addr;
        mask.ipv4_valid_mask = 0xFF;
        mask.input_mapping_native_mask_u1.ipv4_dstAddr_mask =
                ipv4_prefix_len_to_mask(ip_prefix->len);
    } else {
        key.ipv6_valid = 1;
        memcpy(key.input_mapping_native_u1.ipv6_dstAddr,
               ip_prefix->addr.addr.v6_addr.addr8, IP6_ADDR8_LEN);
        memrev(key.input_mapping_native_u1.ipv6_dstAddr, IP6_ADDR8_LEN);
        mask.ipv6_valid_mask = 0xFF;
        ipv6_prefix_len_to_mask(
                (ipv6_addr_t*)(mask.input_mapping_native_mask_u1.ipv6_dstAddr_mask),
                ip_prefix->len);
        memrev(mask.input_mapping_native_mask_u1.ipv6_dstAddr_mask, IP6_ADDR8_LEN);
    }
    data.actionid = actionid;
    ret = tcam->insert(&key, &mask, &data, &ret_idx);
    if (ret == HAL_RET_DUP_INS_FAIL) {
        /* Entry already exists. Can be skipped */
        *idx = INVALID_INDEXER_INDEX;
    } else {
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Input mapping table tcam write failure, idx : {}, err : {}",
                          ret_idx, ret);
            return ret;
        }
    }
    HAL_TRACE_DEBUG("Input mapping table tcam write, idx : {}, ret: {}",
                    ret_idx, ret);
    *idx = (int) ret_idx;
    return ret;
}

// ----------------------------------------------------------------------------
// Program input mapping table to terminate GIPo tunnels
// ----------------------------------------------------------------------------
hal_ret_t
pd_tenant_del_gipo_termination_prefix(pd_tenant_t *tenant_pd,
                                      p4pd_table_id tbl_id)
{
    Tcam         *tcam;
    uint32_t     *arr;
    hal_ret_t    ret = HAL_RET_OK;

    tcam = g_hal_state_pd->tcam_table(tbl_id);
    HAL_ASSERT(tcam != NULL);

    if (tbl_id == P4TBL_ID_INPUT_MAPPING_NATIVE) {
        arr = tenant_pd->gipo_imn_idx;
    } else {
        arr = tenant_pd->gipo_imt_idx;
    }

    for (int i = 0; i < 3; i++) {
        if (arr[i] != INVALID_INDEXER_INDEX) {
            ret = tcam->remove((uint32_t)arr[i]);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Input mapping native tcam remove failure, "
                              "idx : {}, err : {}", arr[i], ret);
            }
            arr[i] = INVALID_INDEXER_INDEX;
        }
    }
    return ret;
}

// ----------------------------------------------------------------------------
// De-Program HW
// ----------------------------------------------------------------------------
hal_ret_t
pd_tenant_deprogram_gipo_termination_prefix(pd_tenant_t *tenant_pd)
{
    hal_ret_t ret;
    /* Deprogram input mapping native */
    ret = pd_tenant_del_gipo_termination_prefix(tenant_pd,
                                                P4TBL_ID_INPUT_MAPPING_NATIVE);
    /* Deprogram input mapping tunneled */
    ret = pd_tenant_del_gipo_termination_prefix(tenant_pd,
                                                P4TBL_ID_INPUT_MAPPING_TUNNELED);
    return ret;
}

// ----------------------------------------------------------------------------
// Program input mapping table to terminate GIPo tunnels
// ----------------------------------------------------------------------------
hal_ret_t
pd_tenant_program_gipo_termination_prefix(pd_tenant_t *tenant_pd)
{
    uint32_t     idx;
    hal_ret_t    ret = HAL_RET_OK;
    ip_prefix_t  *gipo_prefix = &((tenant_t*)(tenant_pd->tenant))->gipo_prefix;

    if (gipo_prefix->len == 0) {
        return ret; // GIPo not specified. Nothing to program.
    }

    /* We program 3 entries in the INPUT_MAPPING_NATIVE Table for the GIPo Entry */
    /* Entry 1 */
    ret = pd_tenant_program_input_mapping_table(gipo_prefix,
                                                INGRESS_TUNNEL_TYPE_VXLAN,
                                                true, false,
                                                INPUT_MAPPING_NATIVE_NOP_ID,
                                                P4TBL_ID_INPUT_MAPPING_NATIVE,
                                                &idx);
    if ((ret != HAL_RET_OK) && (ret != HAL_RET_DUP_INS_FAIL))
        goto fail_flag;
    tenant_pd->gipo_imn_idx[0] = idx;
    /* Entry 2 */
    ret = pd_tenant_program_input_mapping_table(gipo_prefix,
                                                INGRESS_TUNNEL_TYPE_VXLAN,
                                                false, true,
                                                INPUT_MAPPING_NATIVE_NOP_ID,
                                                P4TBL_ID_INPUT_MAPPING_NATIVE,
                                                &idx);
    if ((ret != HAL_RET_OK) && (ret != HAL_RET_DUP_INS_FAIL))
        goto fail_flag;
    tenant_pd->gipo_imn_idx[1] = idx;
    /* Entry 3 */
    ret = pd_tenant_program_input_mapping_table(gipo_prefix,
                                                INGRESS_TUNNEL_TYPE_VXLAN,
                                                false, false,
                                                INPUT_MAPPING_NATIVE_NOP_ID,
                                                P4TBL_ID_INPUT_MAPPING_NATIVE,
                                                &idx);
    if ((ret != HAL_RET_OK) && (ret != HAL_RET_DUP_INS_FAIL))
        goto fail_flag;
    tenant_pd->gipo_imn_idx[2] = idx;

    /* We program 3 entries in the INPUT_MAPPING_TUNNELED Table for the GIPo Entry */
    /* Entry 1 */
    ret = pd_tenant_program_input_mapping_table(gipo_prefix,
                                    INGRESS_TUNNEL_TYPE_VXLAN,
                                    true, false,
                                    INPUT_MAPPING_TUNNELED_TUNNELED_IPV4_PACKET_ID,
                                    P4TBL_ID_INPUT_MAPPING_TUNNELED,
                                    &idx);
    if ((ret != HAL_RET_OK) && (ret != HAL_RET_DUP_INS_FAIL))
        goto fail_flag;
    tenant_pd->gipo_imt_idx[0] = idx;
    /* Entry 2 */
    ret = pd_tenant_program_input_mapping_table(gipo_prefix,
                                    INGRESS_TUNNEL_TYPE_VXLAN,
                                    false, true,
                                    INPUT_MAPPING_TUNNELED_TUNNELED_IPV6_PACKET_ID,
                                    P4TBL_ID_INPUT_MAPPING_TUNNELED,
                                    &idx);
    if ((ret != HAL_RET_OK) && (ret != HAL_RET_DUP_INS_FAIL))
        goto fail_flag;
    tenant_pd->gipo_imt_idx[1] = idx;
    /* Entry 3 */
    ret = pd_tenant_program_input_mapping_table(gipo_prefix,
                                    INGRESS_TUNNEL_TYPE_VXLAN,
                                    false, false,
                                    INPUT_MAPPING_TUNNELED_TUNNELED_NON_IP_PACKET_ID,
                                    P4TBL_ID_INPUT_MAPPING_TUNNELED,
                                    &idx);
    if ((ret != HAL_RET_OK) && (ret != HAL_RET_DUP_INS_FAIL))
        goto fail_flag;
    tenant_pd->gipo_imt_idx[2] = idx;

    return HAL_RET_OK;

fail_flag:
    pd_tenant_deprogram_gipo_termination_prefix(tenant_pd);
    return ret;
}

//-----------------------------------------------------------------------------
// Allocate resources. 
//-----------------------------------------------------------------------------
hal_ret_t
tenant_pd_alloc_res(pd_tenant_t *tenant_pd)
{
    hal_ret_t           ret = HAL_RET_OK;
    indexer::status     rs;

    // allocate h/w id for this tenant
    rs = g_hal_state_pd->tenant_hwid_idxr()->
                         alloc((uint32_t *)&tenant_pd->ten_hw_id);
    if (rs != indexer::SUCCESS) {
        HAL_TRACE_ERR("pd-tenant:{}:failed to alloc ten_hw_id err: {}", 
                      __FUNCTION__, rs);
        tenant_pd->ten_hw_id = INVALID_INDEXER_INDEX;
        ret = HAL_RET_NO_RESOURCE;
        goto end;
    }

    HAL_TRACE_DEBUG("pd-tenant:{}:allocated ten_hw_id: {}", 
                    __FUNCTION__, tenant_pd->ten_hw_id);

    if (((tenant_t *)(tenant_pd->tenant))->tenant_type == types::TENANT_TYPE_INFRA) {
        ret = pd_tenant_program_gipo_termination_prefix(tenant_pd);
    }

end:
    return ret;
}

//-----------------------------------------------------------------------------
// De-Allocate resources. 
//-----------------------------------------------------------------------------
hal_ret_t
tenant_pd_dealloc_res(pd_tenant_t *tenant_pd)
{
    hal_ret_t           ret = HAL_RET_OK;
    indexer::status     rs;

    if (tenant_pd->ten_hw_id != INVALID_INDEXER_INDEX) {
        rs = g_hal_state_pd->tenant_hwid_idxr()->free(tenant_pd->ten_hw_id);
        if (rs != indexer::SUCCESS) {
            HAL_TRACE_ERR("pd-tenant:{}:failed to free ten_hw_id err: {}", 
                          __FUNCTION__, tenant_pd->ten_hw_id);
            ret = HAL_RET_INVALID_OP;
            goto end;
        }

        HAL_TRACE_DEBUG("pd-tenant:{}:freed ten_hw_id: {}", 
                        __FUNCTION__, tenant_pd->ten_hw_id);
    }

    if (((tenant_t *)(tenant_pd->tenant))->tenant_type == types::TENANT_TYPE_INFRA) {
        ret = pd_tenant_deprogram_gipo_termination_prefix(tenant_pd);
    }

end:
    return ret;
}

//-----------------------------------------------------------------------------
// PD Tenant Cleanup
//  - Release all resources
//  - Delink PI <-> PD
//  - Free PD Tenant
//  Note:
//      - Just free up whatever PD has. 
//      - Dont use this inplace of delete. Delete may result in giving callbacks
//        to others.
//-----------------------------------------------------------------------------
hal_ret_t
tenant_pd_cleanup(pd_tenant_t *tenant_pd)
{
    hal_ret_t       ret = HAL_RET_OK;

    if (!tenant_pd) {
        // Nothing to do
        goto end;
    }

    // Check if l2segs have been removed before tenant cleanup
    // index 0 is reserved.
    if (tenant_pd->l2seg_hw_id_idxr_->usage() > 1) {
        HAL_TRACE_ERR("pd-tenant:{}:l2seg idxr still in use. usage:{}", 
                      __FUNCTION__, tenant_pd->l2seg_hw_id_idxr_->usage());
        ret = HAL_RET_INVALID_OP;
        goto end;
    }

    // Releasing resources
    ret = tenant_pd_dealloc_res(tenant_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-tenant:{}: unable to dealloc res for tenant: {}", 
                      __FUNCTION__, 
                      ((tenant_t *)(tenant_pd->tenant))->tenant_id);
        goto end;
    }

    // Delinking PI<->PD
    delink_pi_pd(tenant_pd, (tenant_t *)tenant_pd->tenant);

    // Freeing PD
    tenant_pd_free(tenant_pd);
end:
    return ret;
}

//-----------------------------------------------------------------------------
// Allocate l2seg hwid per tenant
//-----------------------------------------------------------------------------
hal_ret_t
tenant_pd_alloc_l2seg_hw_id(pd_tenant_t *tenant_pd, uint32_t *l2seg_hw_id)
{
    hal_ret_t           ret = HAL_RET_OK;
    indexer::status     rs = indexer::SUCCESS;

    if (!tenant_pd || !l2seg_hw_id) {
        ret =  HAL_RET_INVALID_ARG;
        goto end;
    }

    rs = tenant_pd->l2seg_hw_id_idxr_->alloc(l2seg_hw_id);
    if (rs != indexer::SUCCESS) {
        HAL_TRACE_ERR("pd-tenant:{}:failed to alloc l2seg_hw_id err: {}", 
                      __FUNCTION__, rs);
        *l2seg_hw_id = INVALID_INDEXER_INDEX;
        ret = HAL_RET_NO_RESOURCE;
        goto end;
    }

    HAL_TRACE_DEBUG("pd-tenant:{}:allocated l2seg_hw_id: {} for tenant: {}", 
                    __FUNCTION__, *l2seg_hw_id, 
                    ((tenant_t *)(tenant_pd->tenant))->tenant_id);

end:
    return ret;
}

//-----------------------------------------------------------------------------
// Free l2seg hwid per tenant
//-----------------------------------------------------------------------------
hal_ret_t
tenant_pd_free_l2seg_hw_id(pd_tenant_t *tenant_pd, uint32_t l2seg_hw_id)
{
    hal_ret_t           ret = HAL_RET_OK;
    indexer::status     rs = indexer::SUCCESS;

    if (!tenant_pd) {
        ret =  HAL_RET_INVALID_ARG;
        goto end;
    }

    if (l2seg_hw_id != INVALID_INDEXER_INDEX) { 
        rs = tenant_pd->l2seg_hw_id_idxr_->free(l2seg_hw_id);
        if (rs != indexer::SUCCESS) {
            HAL_TRACE_ERR("pd-tenant:{}:Failed to free l2seg_hw_id:{} "
                    "err: {}", __FUNCTION__,
                    l2seg_hw_id, rs);
            ret = HAL_RET_NO_RESOURCE;
            goto end;
        }
        HAL_TRACE_DEBUG("pd-tenant:{}:freed l2seg_hw_id: {} for tenant: {}", 
                        __FUNCTION__, l2seg_hw_id, 
                        ((tenant_t *)(tenant_pd->tenant))->tenant_id);
    }

end:
    return ret;
}

// ----------------------------------------------------------------------------
// Linking PI <-> PD
// ----------------------------------------------------------------------------
void 
link_pi_pd(pd_tenant_t *pd_ten, tenant_t *pi_ten)
{
    pd_ten->tenant = pi_ten;
    pi_ten->pd = pd_ten;
}

// ----------------------------------------------------------------------------
// Un-Linking PI <-> PD
// ----------------------------------------------------------------------------
void 
delink_pi_pd(pd_tenant_t *pd_ten, tenant_t *pi_ten)
{
    if (pd_ten) {
        pd_ten->tenant = NULL;
    }
    if (pi_ten) {
        pi_ten->pd = NULL;
    }
}

// ----------------------------------------------------------------------------
// Makes a clone
// ----------------------------------------------------------------------------
hal_ret_t
pd_tenant_make_clone(tenant_t *ten, tenant_t *clone)
{
    hal_ret_t           ret = HAL_RET_OK;
    pd_tenant_t         *pd_ten_clone = NULL;

    pd_ten_clone = tenant_pd_alloc_init();
    if (pd_ten_clone == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    memcpy(pd_ten_clone, ten->pd, sizeof(pd_tenant_t));

    link_pi_pd(pd_ten_clone, clone);

end:
    return ret;
}

// ----------------------------------------------------------------------------
// Frees PD memory without indexer free.
// ----------------------------------------------------------------------------
hal_ret_t
pd_tenant_mem_free(pd_tenant_args_t *args)
{
    hal_ret_t      ret = HAL_RET_OK;
    pd_tenant_t    *tenant_pd;

    tenant_pd = (pd_tenant_t *)args->tenant->pd;
    tenant_pd_mem_free(tenant_pd);

    return ret;
}



}    // namespace pd
}    // namespace hal
