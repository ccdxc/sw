// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_lock.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/hal/src/nwsec.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/src/if_utils.hpp"


namespace hal {

// Security Group Policy Related
void *
nwsec_policy_cfg_get_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((nwsec_policy_cfg_t *)entry)->plcy_key);
}

uint32_t
nwsec_policy_cfg_compute_hash_func (void *key, uint32_t ht_size)
{
    return utils::hash_algo::fnv_hash(key,
                                      sizeof(nwsec_policy_key_t)) % ht_size;
}

bool
nwsec_policy_cfg_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (!memcmp(key1, key2, sizeof(nwsec_policy_key_t))) {
        return true;
    }
    return false;
}

//------------------------------------------------------------------------
// validate an incoming security group create request
// ------------------------------------------------------------------------
static hal_ret_t
validate_nwsec_policy_cfg_create(nwsec::SecurityGroupPolicySpec& spec,
                                 nwsec::SecurityGroupPolicyResponse *rsp)
{
    // key-handle field must be set
    if (!spec.has_key_or_handle()) {
        HAL_TRACE_ERR("{}: security group id or handle not set in request",
                      __FUNCTION__);
        rsp->set_api_status(types::API_STATUS_INVALID_ARG);
        return HAL_RET_INVALID_ARG;
    }
    
    auto kh = spec.key_or_handle();
    if (kh.policy_key_or_handle_case() != 
            SecurityGroupPolicyKeyHandle::kSecurityGroupPolicyId) {
        // key-handle field set, but securityGroup id not provided
        HAL_TRACE_ERR("{}: security group id not set in"
                      "request", __FUNCTION__);
        rsp->set_api_status(types::API_STATUS_SECURITY_POLICY_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }
    
    // Need to check if the sg id is in the valid range
    return HAL_RET_OK;

}

hal_ret_t
nwsec_policy_cfg_create_add_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                               ret = HAL_RET_OK;
    dllist_ctxt_t                           *lnode = NULL;
    dhl_entry_t                             *dhl_entry = NULL;
    nwsec_policy_cfg_t                      *nwsec_plcy_cfg = NULL;
    //nwsec_policy_cfg_create_app_ctxt_t    *app_ctxt = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("{}: invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }
    
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    //app_ctxt = (nwsec_policy_cfg_create_app_ctxt_t *)cfg_ctxt->app_ctxt;
    
    nwsec_plcy_cfg = (nwsec_policy_cfg_t *) dhl_entry->obj;
    
    HAL_TRACE_DEBUG("{}: policy_id {} {}",
                   __FUNCTION__, 
                   nwsec_plcy_cfg->plcy_key.sg_id, 
                   nwsec_plcy_cfg->plcy_key.peer_sg_id);
    // No PD calls for security group objects

end:
    return ret;
}

//------------------------------------------------------------------------
// 1. Update the PI DBs as 
//     a. Add to sg_id hash table  cfg database
//     b. Populate the oper db
///------------------------------------------------------------------------
hal_ret_t 
nwsec_policy_cfg_create_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t             ret = HAL_RET_OK;
    dllist_ctxt_t          *lnode = NULL;
    dhl_entry_t           *dhl_entry  = NULL;
    nwsec_policy_cfg_t    *nwsec_plcy_cfg;
    hal_handle_t          hal_handle = 0;
    
    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("{}: invalid cfg_ctxt",
                      __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    nwsec_plcy_cfg = (nwsec_policy_cfg_t *) dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("{}:sg_id {} handle {}",
                    __FUNCTION__, 
                    nwsec_plcy_cfg->plcy_key.sg_id, 
                    nwsec_plcy_cfg->plcy_key.peer_sg_id, 
                    hal_handle);
    ret = add_nwsec_policy_cfg_to_db(nwsec_plcy_cfg);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}: failed to add sg_id {} to db,"
                      "err : {}", __FUNCTION__, nwsec_plcy_cfg->plcy_key.sg_id,
                      ret);
        goto end;
    }
end:
    return ret;
}


//------------------------------------------------------------------------
// Network policyt config create abort:
//------------------------------------------------------------------------
hal_ret_t
nwsec_policy_cfg_create_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t               ret = HAL_RET_OK;

    return ret;
    
}

//------------------------------------------------------------------------
// Dummy create cleanup callback
//------------------------------------------------------------------------
hal_ret_t
nwsec_policy_cfg_create_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t ret = HAL_RET_OK;
    return ret;
}


//------------------------------------------------------------------------------
// Converts hal_ret_t to API status
//------------------------------------------------------------------------------
hal_ret_t
nwsec_policy_cfg_prepare_rsp(SecurityGroupPolicyResponse *rsp, hal_ret_t ret,
                             hal_handle_t hal_handle)
{
    if (ret == HAL_RET_OK) {
        rsp->mutable_status()->set_policy_handle(hal_handle);
    }
    rsp->set_api_status(hal_prepare_rsp(ret));
    return HAL_RET_OK;
}

hal_ret_t
extract_rules_from_sg_spec(SecurityGroupPolicySpec& spec, 
                           nwsec_policy_cfg_t *nwsec_plcy_cfg)
{   
    hal_ret_t                   ret = HAL_RET_OK;
    nwsec_policy_rules_t        *nwsec_plcy_rules = NULL;
    nwsec_policy_svc_t          *nwsec_plcy_svc = NULL;
    uint32_t                    fw_sz = 0, svcs_sz = 0;

    if (spec.has_policy_rules()) {
        fw_sz = spec.policy_rules().in_fw_rules_size();
        HAL_TRACE_DEBUG("Policy_rules:: Firewall Size {}", fw_sz);
        for (uint32_t i = 0; i < fw_sz; i++) {
            nwsec_plcy_rules = nwsec_policy_rules_alloc_and_init();
            if (nwsec_plcy_rules == NULL) {
                HAL_TRACE_ERR("{}: unable to"
                              "allocate handle/memory"
                              "ret: {}", __FUNCTION__, ret);
                //ToDo:Cleanup the nwsec_plcy_rules allocated till now
                return HAL_RET_OOM;
           }
           svcs_sz = spec.policy_rules().in_fw_rules(i).svc_size();
           HAL_TRACE_DEBUG("Policy_rules::Services Size {}", svcs_sz);
           for (uint32_t svcs_cnt = 0; svcs_cnt < svcs_sz; svcs_cnt++) {
                nwsec_plcy_svc = nwsec_policy_svc_alloc_and_init();
                if (nwsec_plcy_svc == NULL) {
                    HAL_TRACE_ERR("{}: unable to"
                                  "allocate handle/memory"
                                  "ret: {}", __FUNCTION__, ret);
                    //ToDo:Cleanup the nwsec_plcy_rules allocated till now
                    return HAL_RET_OOM;
                }
                nwsec_plcy_svc->ipproto =
                                spec.policy_rules().in_fw_rules(i).svc(svcs_cnt).ip_protocol();
                if (spec.policy_rules().in_fw_rules(i).svc(svcs_cnt).l4_info_case()
                                                     == Service::kDstPort){
                    nwsec_plcy_svc->dst_port =
                            spec.policy_rules().in_fw_rules(i).svc(svcs_cnt).dst_port();

                } else {
                    nwsec_plcy_svc->icmp_msg_type =
                            spec.policy_rules().in_fw_rules(i).svc(svcs_cnt).icmp_msg_type();
                }
                HAL_TRACE_DEBUG("Policy_rules::Ipproto {}, port {}", nwsec_plcy_svc->ipproto, nwsec_plcy_svc->dst_port);
                //To Do: Check to Get lock on nwsec_plcy_rules ??
                dllist_add(&nwsec_plcy_rules->fw_svc_list_head,
                           &nwsec_plcy_svc->lentry);
            }
            dllist_add(&nwsec_plcy_cfg->rules_head,
                       &nwsec_plcy_rules->lentry);
        }
    }
    return ret;
}

hal_ret_t
security_group_policy_create(nwsec::SecurityGroupPolicySpec& spec,
                             nwsec::SecurityGroupPolicyResponse *rsp)
{
    hal_ret_t                           ret;
    nwsec_policy_cfg_t                  *nwsec_plcy_cfg;
    nwsec_policy_cfg_create_app_ctxt_t  *app_ctxt  = NULL;
   
    dhl_entry_t             dhl_entry = { 0 };
    cfg_op_ctxt_t           cfg_ctxt =  { 0 };
 
    
    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("{}: Creating nwsec policy, policy_id {}", __FUNCTION__,
                    spec.key_or_handle().security_group_policy_id().security_group_id());
    
    ret = validate_nwsec_policy_cfg_create(spec, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}: validation failed, sg_id {} ret: {}",
                      __FUNCTION__,  spec.key_or_handle().security_group_policy_id().security_group_id(),
                      ret);
        goto end;
    }
    // Check of the sgid is already present
    
    // instanstiate the security_policy
    nwsec_plcy_cfg  = nwsec_policy_cfg_alloc_init();
    if (nwsec_plcy_cfg == NULL) {
        HAL_TRACE_ERR("{}: unable to allocate handle/memory"
                      "ret: {}", __FUNCTION__, ret);
        ret = HAL_RET_OOM;
        goto end;
    }
    nwsec_plcy_cfg->plcy_key.sg_id = spec.key_or_handle().security_group_policy_id().security_group_id();
    nwsec_plcy_cfg->plcy_key.peer_sg_id = spec.key_or_handle().security_group_policy_id().peer_security_group_id();

    ret = extract_rules_from_sg_spec(spec, nwsec_plcy_cfg);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}: unable to extract rules from"
                      "spec err:{}", __FUNCTION__, ret);
        nwsec_policy_cfg_free(nwsec_plcy_cfg);
        goto end;
    }

    // allocate hal handle id
    nwsec_plcy_cfg->hal_handle = hal_handle_alloc(HAL_OBJ_ID_SECURITY_POLICY);
    if (nwsec_plcy_cfg->hal_handle == HAL_HANDLE_INVALID) {
        HAL_TRACE_ERR("{}: failed to alloc handle for policy_id: {} {}",
                     __FUNCTION__, nwsec_plcy_cfg->plcy_key.sg_id, nwsec_plcy_cfg->plcy_key.peer_sg_id);
        nwsec_policy_cfg_free(nwsec_plcy_cfg);
        //ToDo: lseshan: Call free for rules and svc list
        ret = HAL_RET_HANDLE_INVALID;
        goto end;
    }

    // form cntxt and call infra add. No app ctxt as of now.
    //app_ctxt.   =  ;
    dhl_entry.handle = nwsec_plcy_cfg->hal_handle;
    dhl_entry.obj  = nwsec_plcy_cfg;
    cfg_ctxt.app_ctxt = &app_ctxt;
    
    utils::dllist_reset(&cfg_ctxt.dhl);
    utils::dllist_reset(&dhl_entry.dllist_ctxt);
    utils::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_add_obj(nwsec_plcy_cfg->hal_handle, &cfg_ctxt,
                             nwsec_policy_cfg_create_add_cb,
                             nwsec_policy_cfg_create_commit_cb,
                             nwsec_policy_cfg_create_abort_cb,
                             nwsec_policy_cfg_create_cleanup_cb);
end:
    nwsec_policy_cfg_prepare_rsp(rsp, ret, nwsec_plcy_cfg->hal_handle);
     
    HAL_TRACE_DEBUG("----------------------- API End ------------------------");
    return ret;
}


hal_ret_t
security_group_policy_update(nwsec::SecurityGroupPolicySpec& spec,
                      nwsec::SecurityGroupPolicyResponse *res)
{
    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("{}: Updating nwsec group, sg_id {} handle {}", __FUNCTION__,
                    spec.key_or_handle().security_group_policy_id().security_group_id(),
                    spec.key_or_handle().security_group_policy_handle());
    return HAL_RET_OK;

}  

hal_ret_t
security_group_policy_delete(nwsec::SecurityGroupPolicySpec& spec,
                      nwsec::SecurityGroupPolicyResponse *res)
{
    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("{}: Deleting nwsec group, sg_id {}", __FUNCTION__,
                    spec.key_or_handle().security_group_policy_id().security_group_id());
    return HAL_RET_OK;
}    

hal_ret_t
security_group_policy_get(nwsec::SecurityGroupPolicyGetRequest& spec,
                   nwsec::SecurityGroupPolicyGetResponse *res)
{
    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("{}: Deleting nwsec group, sg_id {}", __FUNCTION__,
                    spec.key_or_handle().security_group_policy_id().security_group_id());
    return HAL_RET_OK;
}

// Security Group create
void *
nwsec_group_get_key_func (void *entry)
{
    hal_handle_id_ht_entry_t   *ht_entry;
    nwsec_group_t              *nwsec_grp = NULL;
    HAL_ASSERT(entry != NULL);
    ht_entry = (hal_handle_id_ht_entry_t *) entry;
    if (ht_entry == NULL) {
        return NULL;
    }
    nwsec_grp = (nwsec_group_t *)hal_handle_get_obj(ht_entry->handle_id);
    return (void *)&(nwsec_grp->sg_id);
}

uint32_t
nwsec_group_compute_hash_func (void *key, uint32_t ht_size)
{
    return utils::hash_algo::fnv_hash(key,
                                      sizeof(nwsec_group_id_t)) % ht_size;
}

bool
nwsec_group_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(nwsec_group_id_t *) key1 == *(nwsec_group_id_t *)key2) {
        return true;
    } 
    return false;
}

static hal_ret_t
validate_nwsec_group_create(nwsec::SecurityGroupSpec& spec,
                            nwsec::SecurityGroupResponse *rsp)
{

    // key-handle field must be set
    if (!spec.has_key_or_handle()) {
        HAL_TRACE_ERR("{}: security group id or handle not set in request",
                      __FUNCTION__);
        rsp->set_api_status(types::API_STATUS_INVALID_ARG);
        return HAL_RET_INVALID_ARG;
    }

    auto kh = spec.key_or_handle();
    if (kh.key_or_handle_case() !=
            SecurityGroupKeyHandle::kSecurityGroupId) {
        // key-handle field set, but securityGroup id not provided
        HAL_TRACE_ERR("{}: security group id not set in"
                      "request", __FUNCTION__);
        rsp->set_api_status(types::API_STATUS_SECURITY_GROUP_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    return HAL_RET_OK;
}

hal_ret_t
nwsec_group_create_add_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                               ret = HAL_RET_OK;
    dllist_ctxt_t                           *lnode = NULL;
    dhl_entry_t                             *dhl_entry = NULL;
    nwsec_group_t                           *nwsec_grp = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("{}: invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    nwsec_grp = (nwsec_group_t *) dhl_entry->obj;

    HAL_TRACE_DEBUG("{}: policy_id {}",
                   __FUNCTION__,
                   nwsec_grp->sg_id);
    // No PD calls for security group objects

end:
    return ret;
}

hal_ret_t
nwsec_group_create_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t             ret = HAL_RET_OK;
    dllist_ctxt_t         *lnode = NULL;
    dhl_entry_t           *dhl_entry  = NULL;
    nwsec_group_t         *nwsec_grp;
    hal_handle_t          hal_handle = 0;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("{}: invalid cfg_ctxt",
                      __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    nwsec_grp = (nwsec_group_t *) dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("{}:sg_id {} handle {}",
                    __FUNCTION__,
                    nwsec_grp->sg_id,
                    hal_handle);
    ret = add_nwsec_group_to_db(nwsec_grp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}: failed to add sg_id {} to db,"
                      "err : {}", __FUNCTION__, nwsec_grp->sg_id,
                      ret);
        goto end;
    }
end:
    return ret;
}

//------------------------------------------------------------------------
// Network group create abort:
//------------------------------------------------------------------------
hal_ret_t
nwsec_group_create_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t               ret = HAL_RET_OK;

    return ret;

}

//------------------------------------------------------------------------
// Dummy create cleanup callback
//------------------------------------------------------------------------
hal_ret_t
nwsec_group_create_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t ret = HAL_RET_OK;
    return ret;
}

//------------------------------------------------------------------------------
// Converts hal_ret_t to API status
//------------------------------------------------------------------------------
hal_ret_t
nwsec_group_prepare_rsp(SecurityGroupResponse  *rsp,
                        hal_ret_t              ret,
                        hal_handle_t           hal_handle)
{
    if (ret == HAL_RET_OK) {
        rsp->mutable_status()->set_sg_handle(hal_handle);
    }
    rsp->set_api_status(hal_prepare_rsp(ret));
    return HAL_RET_OK;
}

hal_ret_t
security_group_create(nwsec::SecurityGroupSpec&     spec,
                      nwsec::SecurityGroupResponse *res)
{
    hal_ret_t                     ret;
    nwsec_group_t                 *nwsec_grp;
    nwsec_group_create_app_ctxt_t *app_ctxt  = NULL;
    
    dhl_entry_t                   dhl_entry = { 0 };
    cfg_op_ctxt_t                 cfg_ctxt =  { 0 };
    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("{}: Creating nwsec group, sg_id {}", __FUNCTION__,
                    spec.key_or_handle().security_group_id());

    ret = validate_nwsec_group_create(spec, res);

    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}: validation failed, sg_id {} ret: {}",
                      __FUNCTION__,  spec.key_or_handle().security_group_id(),
                      ret);
        goto end;
    }
    // Check of the sgid is already present

    // instanstiate the security_policy
    nwsec_grp  = nwsec_group_alloc_init();
    if (nwsec_grp == NULL) {
        HAL_TRACE_ERR("{}: unable to allocate handle/memory"
                      "ret: {}", __FUNCTION__, ret);
        ret = HAL_RET_OOM;
        goto end;
    }

    nwsec_grp->sg_id =  spec.key_or_handle().security_group_id();
    nwsec_grp->hal_handle = hal_handle_alloc(HAL_OBJ_ID_SECURITY_GROUP);
    if (nwsec_grp->hal_handle == HAL_HANDLE_INVALID) {
        HAL_TRACE_ERR("{}: failed to alloc handle for policy_id: {}",
                     __FUNCTION__, nwsec_grp->sg_id);
        nwsec_group_free(nwsec_grp);
        //ToDo: lseshan: Call free for rules and svc list
        ret = HAL_RET_HANDLE_INVALID;
        goto end;
    }

    // form cntxt and call infra add. No app ctxt as of now.
    //app_ctxt.   =  ;
    dhl_entry.handle = nwsec_grp->hal_handle;
    dhl_entry.obj  = nwsec_grp;
    cfg_ctxt.app_ctxt = &app_ctxt;

    utils::dllist_reset(&cfg_ctxt.dhl);
    utils::dllist_reset(&dhl_entry.dllist_ctxt);
    utils::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_add_obj(nwsec_grp->hal_handle, &cfg_ctxt,
                             nwsec_group_create_add_cb,
                             nwsec_group_create_commit_cb,
                             nwsec_group_create_abort_cb,
                             nwsec_group_create_cleanup_cb);
end:
    nwsec_group_prepare_rsp(res, ret, nwsec_grp->hal_handle);

    HAL_TRACE_DEBUG("----------------------- API End ------------------------");
    return HAL_RET_OK;
}

hal_ret_t
security_group_update(nwsec::SecurityGroupSpec& spec,
                      nwsec::SecurityGroupResponse *res)
{
    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("{}: Updating nwsec group, sg_id {} handle {}", __FUNCTION__,
                    spec.key_or_handle().security_group_id(),
                    spec.key_or_handle().security_group_handle());
    return HAL_RET_OK;

}

hal_ret_t
security_group_delete(nwsec::SecurityGroupSpec& spec,
                      nwsec::SecurityGroupResponse *res)
{
    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("{}: Deleting nwsec group, sg_id {}", __FUNCTION__,
                    spec.key_or_handle().security_group_id());
    return HAL_RET_OK;
}

hal_ret_t
security_group_get(nwsec::SecurityGroupGetRequest& spec,
                   nwsec::SecurityGroupGetResponse *res)
{
    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("{}: Deleting nwsec group, sg_id {}", __FUNCTION__,
                    spec.key_or_handle().security_group_id());
    return HAL_RET_OK;
}

// Security Policy nw handle get
dllist_ctxt_t *
get_nw_list_for_security_group(uint32_t sg_id)
{
    nwsec_group_t    *nwsec_grp = NULL;

    nwsec_grp = nwsec_group_lookup_by_key(sg_id);
    if (nwsec_grp == NULL) {
        HAL_TRACE_DEBUG("segment id {} not found", sg_id);
        return NULL;
    }
    return &nwsec_grp->nw_list_head;
}

// Security Policy nw_handle add
hal_ret_t
add_nw_to_security_group(uint32_t sg_id, hal_handle_t nw_handle_id)
{
    nwsec_group_t               *nwsec_grp = NULL;
    hal_handle_id_list_entry_t  *nwsec_policy_nw_info = NULL;

    nwsec_grp = nwsec_group_lookup_by_key(sg_id);
    if (nwsec_grp == NULL) {
        HAL_TRACE_DEBUG("segment id {} not found");
        return HAL_RET_SG_NOT_FOUND;
    }

    nwsec_policy_nw_info = (hal_handle_id_list_entry_t *)
                            g_hal_state->hal_handle_id_list_entry_slab()->alloc();
    if (nwsec_policy_nw_info == NULL) {
        HAL_TRACE_DEBUG("Unable to alloc and store nw_info for nw_handle:{}",
                        nw_handle_id);
        return HAL_RET_OOM;
    }
    nwsec_policy_nw_info->handle_id = nw_handle_id;
    dllist_reset(&nwsec_policy_nw_info->dllist_ctxt);

    HAL_SPINLOCK_LOCK(&nwsec_grp->slock);
    dllist_add(&nwsec_grp->nw_list_head, &nwsec_policy_nw_info->dllist_ctxt);
    HAL_SPINLOCK_UNLOCK(&nwsec_grp->slock);

    return HAL_RET_OK;
}

// Security Policy ep_info get
dllist_ctxt_t *
get_ep_list_for_security_group(uint32_t sg_id)
{
    nwsec_group_t     *nwsec_group = NULL;

    nwsec_group = nwsec_group_lookup_by_key(sg_id);
    if (nwsec_group == NULL) {
        HAL_TRACE_DEBUG("segment id {} not found", sg_id);
        return NULL;
    }
    return &nwsec_group->ep_list_head;
}

//Security Policy ep_info add
hal_ret_t
add_ep_to_security_group(uint32_t sg_id, hal_handle_t ep_handle_id)
{
    nwsec_group_t               *nwsec_grp = NULL;
    hal_handle_id_list_entry_t  *nwsec_policy_ep_info = NULL;

    nwsec_grp  = nwsec_group_lookup_by_key(sg_id);
    if (nwsec_grp == NULL) {
        HAL_TRACE_DEBUG("segment id {} not found");
        return HAL_RET_SG_NOT_FOUND;
    }

    nwsec_policy_ep_info = (hal_handle_id_list_entry_t *)
                            g_hal_state->hal_handle_id_list_entry_slab()->alloc();
    if (nwsec_policy_ep_info == NULL) {
        HAL_TRACE_DEBUG("Unable to alloc and store ep_info for ep_handle:{}",
                        ep_handle_id);
        return HAL_RET_OOM;
    }
    nwsec_policy_ep_info->handle_id = ep_handle_id;
    dllist_reset(&nwsec_policy_ep_info->dllist_ctxt);

    HAL_SPINLOCK_LOCK(&nwsec_grp->slock);
    dllist_add(&nwsec_grp->ep_list_head, &nwsec_policy_ep_info->dllist_ctxt);
    HAL_SPINLOCK_UNLOCK(&nwsec_grp->slock);

    return HAL_RET_OK;
}
}    // namespace hal
