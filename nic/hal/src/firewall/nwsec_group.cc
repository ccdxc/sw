//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_lock.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/hal/src/firewall/nwsec_group.hpp"
#include "nic/gen/hal/include/hal_api_stats.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/src/utils/if_utils.hpp"
#include "nic/gen/proto/hal/nwsec.pb.h"
#include "nic/hal/plugins/app_redir/app_redir_ctx.hpp"

namespace hal {

using nwsec::SecurityGroupSpec;
using nwsec::SecurityGroupRequestMsg;
using nwsec::SecurityGroupStatus;
using nwsec::SecurityGroupResponse;
using nwsec::SecurityGroupGetResponse;
using kh::SecurityGroupKeyHandle;
using nwsec::SecurityGroupPolicySpec;
using nwsec::SecurityGroupPolicyRequestMsg;
using nwsec::SecurityGroupPolicyStatus;
using nwsec::SecurityGroupPolicyResponse;
using nwsec::SecurityGroupPolicyGetResponse;
using kh::SecurityGroupPolicyKeyHandle;
using kh::SecurityPolicyKeyHandle;
using nwsec::Service;
using types::IPProtocol;
using types::ICMPMsgType;
using nwsec::FirewallAction;
using nwsec::ALGName;

acl::acl_config_t nwsec_rule_config_glbl = { };

nwsec_group_t *
nwsec_group_lookup_key_or_handle(const kh::SecurityGroupKeyHandle& key_or_handle)
{
    if (key_or_handle.key_or_handle_case() ==
            kh::SecurityGroupKeyHandle::kSecurityGroupId) {
        return nwsec_group_lookup_by_key(key_or_handle.security_group_id());
    }
    if (key_or_handle.key_or_handle_case() ==
            kh::SecurityGroupKeyHandle::kSecurityGroupHandle) {
        return nwsec_group_lookup_by_handle(key_or_handle.security_group_handle());
    }

    return NULL;
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
    return sdk::lib::hash_algo::fnv_hash(key,
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
    nwsec_group_t         *nwsec_grp = NULL;
    hal_handle_t           hal_handle = 0;



    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("{}: invalid cfg_ctxt",__FUNCTION__);
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
    //Insert into Hash DB
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
securitygroup_create(nwsec::SecurityGroupSpec&     spec,
                     nwsec::SecurityGroupResponse *res)
{
    hal_ret_t                     ret;
    nwsec_group_t                 *nwsec_grp = NULL;
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

    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_add_obj(nwsec_grp->hal_handle, &cfg_ctxt,
                             nwsec_group_create_add_cb,
                             nwsec_group_create_commit_cb,
                             nwsec_group_create_abort_cb,
                             nwsec_group_create_cleanup_cb);
end:
    if (ret == HAL_RET_OK) {
        HAL_API_STATS_INC(HAL_API_SECURITYGROUP_CREATE_SUCCESS);
    } else {
        HAL_API_STATS_INC(HAL_API_SECURITYGROUP_CREATE_FAIL);
    }
    nwsec_group_prepare_rsp(res, ret, nwsec_grp ? nwsec_grp->hal_handle : HAL_HANDLE_INVALID );

    HAL_TRACE_DEBUG("----------------------- API End ------------------------");
    return HAL_RET_OK;
}

hal_ret_t
securitygroup_update(nwsec::SecurityGroupSpec& spec,
                     nwsec::SecurityGroupResponse *res)
{
    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("{}: Updating nwsec group, sg_id {} handle {}", __FUNCTION__,
                    spec.key_or_handle().security_group_id(),
                    spec.key_or_handle().security_group_handle());
    HAL_API_STATS_INC(HAL_API_SECURITYGROUP_UPDATE_SUCCESS);
    return HAL_RET_OK;

}

hal_ret_t
securitygroup_delete(nwsec::SecurityGroupDeleteRequest& req,
                     nwsec::SecurityGroupDeleteResponseMsg *res)
{
    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("{}: Deleting nwsec group, sg_id {}", __FUNCTION__,
                    req.key_or_handle().security_group_id());
    HAL_API_STATS_INC(HAL_API_SECURITYGROUP_DELETE_SUCCESS);
    return HAL_RET_OK;
}

hal_ret_t
securitygroup_get(nwsec::SecurityGroupGetRequest& spec,
                  nwsec::SecurityGroupGetResponseMsg *res)
{
    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("{}: Deleting nwsec group, sg_id {}", __FUNCTION__,
                    spec.key_or_handle().security_group_id());
    HAL_API_STATS_INC(HAL_API_SECURITYGROUP_GET_SUCCESS);
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
        HAL_TRACE_DEBUG("segment id {} not found", sg_id);
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

    HAL_TRACE_DEBUG("{}: add sg => nw, handles:{} => {}",
                    __FUNCTION__, nwsec_grp->hal_handle, nw_handle_id);
    return HAL_RET_OK;
}

// Security Policy nw_handle del
hal_ret_t
del_nw_from_security_group(uint32_t sg_id, hal_handle_t nw_handle_id)
{
    hal_ret_t                   ret = HAL_RET_NW_HANDLE_NOT_FOUND;
    nwsec_group_t               *nwsec_grp = NULL;
    hal_handle_id_list_entry_t  *nwsec_policy_nw_info = NULL;
    dllist_ctxt_t               *curr = NULL, *next = NULL;

    nwsec_grp = nwsec_group_lookup_by_key(sg_id);
    if (nwsec_grp == NULL) {
        HAL_TRACE_DEBUG("segment id {} not found", sg_id);
        return HAL_RET_SG_NOT_FOUND;
    }
    HAL_SPINLOCK_LOCK(&nwsec_grp->slock);
    dllist_for_each_safe(curr, next, &nwsec_grp->nw_list_head) {
        nwsec_policy_nw_info = dllist_entry(curr, hal_handle_id_list_entry_t, dllist_ctxt);
        if (nwsec_policy_nw_info->handle_id == nw_handle_id) {
            // Remove from list
            sdk::lib::dllist_del(&nwsec_policy_nw_info->dllist_ctxt);
            // Free the entry
            hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_LIST_ENTRY,
                                      nwsec_policy_nw_info);

            ret = HAL_RET_OK;
        }
    }
    HAL_SPINLOCK_UNLOCK(&nwsec_grp->slock);

    HAL_TRACE_DEBUG("{}: del sg =/=> nw, handles:{} =/=> {}",
                    __FUNCTION__, nwsec_grp->hal_handle, nw_handle_id);
    return ret;
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
        HAL_TRACE_DEBUG("segment id {} not found", sg_id);
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


// Security Policy ep_handle del
hal_ret_t
del_ep_from_security_group(uint32_t sg_id, hal_handle_t ep_handle_id)
{
    nwsec_group_t               *nwsec_grp = NULL;
    hal_handle_id_list_entry_t  *nwsec_policy_ep_info = NULL;
    dllist_ctxt_t               *curr = NULL, *next = NULL;

    nwsec_grp = nwsec_group_lookup_by_key(sg_id);
    if (nwsec_grp == NULL) {
        HAL_TRACE_DEBUG("segment id {} not found", sg_id);
        return HAL_RET_SG_NOT_FOUND;
    }


    HAL_SPINLOCK_LOCK(&nwsec_grp->slock);
    dllist_for_each_safe(curr, next,  &nwsec_grp->ep_list_head) {
        nwsec_policy_ep_info  = dllist_entry(curr, hal_handle_id_list_entry_t, dllist_ctxt);
        if (nwsec_policy_ep_info != NULL) {
            if (nwsec_policy_ep_info->handle_id == ep_handle_id) {
                sdk::lib::dllist_del(curr);
                hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_LIST_ENTRY,
                                          nwsec_policy_ep_info);
                HAL_SPINLOCK_UNLOCK(&nwsec_grp->slock);
                return HAL_RET_OK;
            }
        }
    }
    HAL_SPINLOCK_UNLOCK(&nwsec_grp->slock);
    return HAL_RET_NW_HANDLE_NOT_FOUND;
}

bool
nwsec_rule_compare_key_func (void *key1, void *key2)
{
    dllist_ctxt_t    lentry;
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    // Compare the hash value
    if (((nwsec_rule_t *) key1)->hash_value  == ((nwsec_rule_t *)key2)->hash_value) {
        return true;
    }
    return false;
}

void *
nwsec_rule_get_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return entry;
}

uint32_t
nwsec_rule_compute_hash_func (void *key, uint32_t ht_size)
{
    return ((((nwsec_rule_t *)key)->hash_value) % ht_size);
}


bool
nwsec_policy_compare_key_func (void *key1, void *key2)
{
    dllist_ctxt_t    lentry;
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    // Compare the hash value
    if (!memcmp(key1, key2, sizeof(policy_key_t))) {
        return true;
    }
    return false;
}

void *
nwsec_policy_get_key_func (void *entry)
{

    hal_handle_id_ht_entry_t   *ht_entry;
    nwsec_policy_t             *policy = NULL;
    HAL_ASSERT(entry != NULL);
    ht_entry = (hal_handle_id_ht_entry_t *) entry;
    if (ht_entry == NULL) {
        return NULL;
    }
    policy  = (nwsec_policy_t *) hal_handle_get_obj(ht_entry->handle_id);
    return (void *)&(policy->key);
}

uint32_t
nwsec_policy_compute_hash_func (void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key,
                                     sizeof(policy_key_t)) % ht_size;
}

uint32_t
calculate_hash_value(void *key, uint32_t keylen, uint32_t hv)
{
    uint8_t *ptr = (uint8_t *)key;
    uint32_t i;

    for (i = 0; i < keylen; i++) {
        hv = (hv * 16777619) ^ ptr[i];
    }
    return hv;
}

//nwsec_rule related
hal_ret_t
extract_nwsec_rule_from_spec(nwsec::SecurityRule spec, nwsec_rule_t *rule)
{
    hal_ret_t              ret = HAL_RET_OK;
    //uint32_t             hv = 2166136261; // lns:revisit

    rule->rule_id = spec.rule_id();

    // Action
    rule->fw_rule_action.sec_action = nwsec::SECURITY_RULE_ACTION_ALLOW;
    rule->fw_rule_action.log_action =  nwsec::LOG_NONE;
    if (spec.has_action()) {
        rule->fw_rule_action.sec_action =  spec.action().sec_action();
        rule->fw_rule_action.log_action = spec.action().log_action();
    }

    // Parse APPs (set alg name only for now)
    rule->fw_rule_action.alg = nwsec::APP_SVC_NONE;
    for (int i = 0; i < spec.action().app_data_size() && rule->fw_rule_action.alg == nwsec::APP_SVC_NONE; i++) {
        auto app = spec.action().app_data(i).alg();
        rule->fw_rule_action.alg = app;
    }
    ret = rule_match_spec_extract(spec.match(), &rule->fw_rule_match);
    if ( ret != HAL_RET_OK) {
        rule_match_cleanup(&rule->fw_rule_match);
        HAL_TRACE_ERR("Failed to retrieve rule_match");
        return ret;
    }

    nwsec_policy_appid_t* nwsec_plcy_appid = NULL;
    uint32_t apps_sz = spec.appid_size();
    HAL_TRACE_DEBUG("Policy_rules::AppidSize {}", apps_sz);
    for (uint32_t apps_cnt = 0; apps_cnt < apps_sz; apps_cnt++) {
        nwsec_plcy_appid = nwsec_policy_appid_alloc_and_init();
        if (nwsec_plcy_appid == NULL) {
            HAL_TRACE_ERR("{}: unable to"
                          "allocate handle/memory"
                          "ret: {}", __FUNCTION__, ret);
            //ToDo:Cleanup the nwsec_plcy_rules allocated till now
            return HAL_RET_OOM;
        }

        uint32_t appid;
        ret = hal::app_redir::app_to_appid(
                  spec.appid(apps_cnt),
                  appid);
        // TODO: Handle resource cleanup before returning
        if(HAL_RET_OK != ret) {
            HAL_TRACE_ERR("{}: unknown app {}", __FUNCTION__,
                          spec.appid(apps_cnt));
            return ret;
        }
        nwsec_plcy_appid->appid = appid;

         //To Do: Check to Get lock on nwsec_plcy_rules ??
        dllist_add_tail(&rule->appid_list_head,
                        &nwsec_plcy_appid->lentry);
    }
    rule->hash_value = spec.rule_id(); // lns: Will evaluate at a later time: Using rule id as hv (or unique identifier).
    return ret;
}

void 
nwsec_rule_free(void *rule)
{
    return;
}

//Rule Db related
hal_ret_t
security_policy_add_to_ruledb( nwsec_policy_t *policy, const acl_ctx_t **acl_ctx)
{
    hal_ret_t ret = HAL_RET_OK;
    nwsec_rule_t    *rule;
    rule_data_t     *rule_data;

    for (uint32_t rule_index = 0; rule_index < policy->rule_len; rule_index++) {
        rule = policy->dense_rules[rule_index];
        if (rule == NULL) {
            return HAL_RET_ERR;
        }
        rule_data = rule_data_alloc_init();
        rule_data->userdata = rule;
        rule_data->data_free     = nwsec_rule_free; 
        ret = rule_match_rule_add(acl_ctx, &rule->fw_rule_match, rule->priority, rule_data);
    }
    return ret;
}

nwsec_policy_t *
security_policy_lookup_key_or_handle(SecurityPolicyKeyHandle& kh)
{
    nwsec_policy_t *policy = NULL;
    if (kh.policy_key_or_handle_case() == kh::SecurityPolicyKeyHandle::kSecurityPolicyKey) {
        uint64_t        vrf_id;
        auto policy_key = kh.security_policy_key();
        if (policy_key.vrf_id_or_handle().key_or_handle_case() == kh::VrfKeyHandle::kVrfId) {
            vrf_id = policy_key.vrf_id_or_handle().vrf_id();
        } else {
            vrf_t *vrf = vrf_lookup_by_handle(policy_key.vrf_id_or_handle().vrf_handle());
            if (!vrf) {
                HAL_TRACE_ERR("invalid vrf handle {}", policy_key.vrf_id_or_handle().vrf_handle());
                return NULL;
            }
            vrf_id = vrf->vrf_id;
        }

        // find_nwsec_policy_by_key()
        policy = find_nwsec_policy_by_key(policy_key.security_policy_id(), vrf_id);
        if (policy == NULL) {
            HAL_TRACE_ERR("Policy with id: {} not found",
                            policy_key.security_policy_id());
        }
    } else if (kh.policy_key_or_handle_case() == SecurityPolicyKeyHandle::kSecurityPolicyHandle) {
        // find by handle
        policy = find_nwsec_policy_by_handle(kh.security_policy_handle());
        if (policy == NULL) {
            HAL_TRACE_ERR("policy with handle: {} not found",
                            kh.security_policy_handle());
        }
    }

    return policy;
}


static hal_ret_t
validate_nwsec_policy_create (nwsec::SecurityPolicySpec&     spec,
                              nwsec::SecurityPolicyResponse  *rsp)
{
    return HAL_RET_OK;
}

hal_ret_t
nwsec_policy_create_add_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t           ret = HAL_RET_OK;
    dllist_ctxt_t       *lnode = NULL;
    dhl_entry_t         *dhl_entry =  NULL;
    nwsec_policy_t      *policy = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("{}: Invalid ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    policy = (nwsec_policy_t *) dhl_entry->obj;

    policy = policy;

    // Print the rule id???


end:
    return ret;
}

hal_ret_t
nwsec_policy_create_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                        ret = HAL_RET_OK;
    dllist_ctxt_t                    *lnode = NULL;
    dhl_entry_t                      *dhl_entry  = NULL;
    nwsec_policy_t                   *policy;
    hal_handle_t                     hal_handle = 0;
    nwsec_policy_create_app_ctxt_t   *app_ctx = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("invalid cfg_ctxt");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    app_ctx = (nwsec_policy_create_app_ctxt_t *) cfg_ctxt->app_ctxt;

    policy = (nwsec_policy_t *) dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    ret = acl::acl_commit(app_ctx->acl_ctx);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Policy commit failed with ret: {}", ret);
        goto end;
    }
    acl_deref(app_ctx->acl_ctx);

    HAL_TRACE_DEBUG("SFW policy handle {}", hal_handle);
    ret = add_nwsec_policy_to_db(policy);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to add policy to db, err : {}", ret);
        goto end;
    }

end:
    if (ret != HAL_RET_OK) {
        //ToDo: free resources allocated in this function
    }
    return ret;
}

hal_ret_t
nwsec_policy_create_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t           ret = HAL_RET_OK;

    return ret;
}

hal_ret_t
nwsec_policy_create_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t           ret = HAL_RET_OK;

    return ret;
}

hal_ret_t
nwsec_policy_prepare_rsp(nwsec::SecurityPolicyResponse *rsp,
                         hal_ret_t                      ret,
                         hal_handle_t                   hal_handle)
{
    if (ret == HAL_RET_OK) {
        rsp->mutable_policy_status()->set_security_policy_handle(hal_handle);
    }
    rsp->set_api_status(hal_prepare_rsp(ret));
    return HAL_RET_OK;
}

hal_ret_t
extract_policy_from_spec(nwsec::SecurityPolicySpec&     spec,
                        nwsec_policy_t                 *policy)
{
    nwsec_rule_t     *nwsec_rule;
    hal_ret_t        ret = HAL_RET_OK;

    int rule_sz = spec.rule_size();
    HAL_TRACE_DEBUG("Policy_rules:: Firewall Size {}", rule_sz);
    policy->rule_len = rule_sz;
    for (int i = 0; i < rule_sz; i++) {
        nwsec_rule = nwsec_rule_alloc_init();
        if (nwsec_rule == NULL) {
            HAL_TRACE_ERR("{}: unable to"
                          "allocate handle/memory"
                          "ret: {}", __FUNCTION__, ret);
            //ToDo:Cleanup the nwsec_rules allocated till now
            return HAL_RET_OOM;
        }
        ret = extract_nwsec_rule_from_spec(spec.rule(i), nwsec_rule);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Unable to extract rule from spec, err: {}", ret);
            nwsec_rule_free(nwsec_rule);
            //TODO: free the remaining rules as well
            ret = HAL_RET_HANDLE_INVALID;
            return ret;
        }
        nwsec_rule->priority = i;
        ret = add_nwsec_rule_to_db(policy, nwsec_rule, i);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR(" unable to add to rule to policy db");
            return ret;
        }
    }
    return ret;
}

// Security policy create
hal_ret_t
securitypolicy_create(nwsec::SecurityPolicySpec&      spec,
                      nwsec::SecurityPolicyResponse   *res)
{
    hal_ret_t                         ret;
    nwsec_policy_t                    *nwsec_policy = NULL;
    nwsec_policy_create_app_ctxt_t    app_ctxt = { 0 };
    const char                        *ctx_name = NULL;
    dhl_entry_t                       dhl_entry = { 0 };
    cfg_op_ctxt_t                     cfg_ctxt = { 0 };

    HAL_TRACE_DEBUG("---------------------- API Start ---------------------");
    HAL_TRACE_DEBUG(" {}: ", __FUNCTION__);

    ret = validate_nwsec_policy_create(spec, res);

    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}: validation failed ",__FUNCTION__);
        goto end;
    }

    nwsec_policy = nwsec_policy_alloc_init();
    if (nwsec_policy == NULL) {
        HAL_TRACE_ERR("{}: unable to"
                      " allocate handle/memory"
                      " ret: {}", __FUNCTION__, ret);
        ret = HAL_RET_OOM;
        goto end;
    }

    nwsec_policy->key.policy_id = spec.policy_key_or_handle().security_policy_key().security_policy_id();
    if (spec.policy_key_or_handle().security_policy_key().vrf_id_or_handle().key_or_handle_case() == kh::VrfKeyHandle::kVrfId) {
        nwsec_policy->key.vrf_id = spec.policy_key_or_handle().security_policy_key().vrf_id_or_handle().vrf_id();
    } else {
        vrf_t *vrf = vrf_lookup_by_handle(spec.policy_key_or_handle().security_policy_key().vrf_id_or_handle().vrf_handle());
        if (!vrf) {
            HAL_TRACE_ERR("Invalid vrf handle {}", spec.policy_key_or_handle().security_policy_key().vrf_id_or_handle().vrf_handle());
            nwsec_policy_free(nwsec_policy);
            ret = HAL_RET_HANDLE_INVALID;
            goto end;
        }
        nwsec_policy->key.vrf_id = vrf->vrf_id;
    }

    nwsec_policy->hal_handle = hal_handle_alloc(HAL_OBJ_ID_SECURITY_POLICY);
    if (nwsec_policy->hal_handle == HAL_HANDLE_INVALID) {
        HAL_TRACE_ERR("{}: failed to alloc handle for policy_id: {}",
                      __FUNCTION__, nwsec_policy->key.policy_id);
        nwsec_policy_free(nwsec_policy);
        ret = HAL_RET_HANDLE_INVALID;
        goto end;
    }

    ret = extract_policy_from_spec(spec, nwsec_policy);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to extract policy from spec");
        nwsec_policy_free(nwsec_policy);
        ret = HAL_RET_HANDLE_INVALID;
        goto end;
    }

    // Create lib acl ctx
    ctx_name = nwsec_acl_ctx_name(nwsec_policy->key.vrf_id);
    HAL_TRACE_DEBUG("Creating acl ctx {}", ctx_name);
    app_ctxt.acl_ctx= hal::rule_lib_init(ctx_name, &nwsec_rule_config_glbl);

    // Fill nwsec group in the dense rules  with rule id as index.
    // This will be used in comparison and find between insert / delete
    ret = security_policy_add_to_ruledb(nwsec_policy, &app_ctxt.acl_ctx);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to add policy to lib, ret: {}", ret);
        goto end;
    }

    dhl_entry.handle = nwsec_policy->hal_handle;
    dhl_entry.obj   = nwsec_policy;
    cfg_ctxt.app_ctxt = &app_ctxt;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_add_obj(nwsec_policy->hal_handle, &cfg_ctxt,
                             nwsec_policy_create_add_cb,
                             nwsec_policy_create_commit_cb,
                             nwsec_policy_create_abort_cb,
                             nwsec_policy_create_cleanup_cb);
end:
    if (ret == HAL_RET_OK) {
        HAL_API_STATS_INC(HAL_API_SECURITYPOLICY_CREATE_SUCCESS);
    } else {
        HAL_API_STATS_INC(HAL_API_SECURITYPOLICY_CREATE_FAIL);
    }
    nwsec_policy_prepare_rsp(res, ret, nwsec_policy ? nwsec_policy->hal_handle : HAL_HANDLE_INVALID);
    HAL_TRACE_DEBUG("------------------------ API End -----------------------------"); 
    return HAL_RET_OK;
}

// Update policy
//
hal_ret_t
nwsec_policy_make_clone(nwsec_policy_t *policy, nwsec_policy_t **policy_clone)
{
    *policy_clone = nwsec_policy_alloc_init();
    if (*policy_clone == NULL) {
        HAL_TRACE_ERR("Failed to allocate the policy");
        return HAL_RET_ERR;  // Better return value
    }

    memcpy(&(*policy_clone)->key, &policy->key, sizeof(policy->key));

    (*policy_clone)->hal_handle  = policy->hal_handle;
    return HAL_RET_OK;
}
hal_ret_t
validate_nwsec_policy_update(nwsec::SecurityPolicySpec&     spec,
                                nwsec::SecurityPolicyResponse *res)
{
    return HAL_RET_OK;
}

hal_ret_t
nwsec_policy_update_upd_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

hal_ret_t
nwsec_policy_update_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                      ret = HAL_RET_OK;
    dllist_ctxt_t                  *lnode = NULL;
    dhl_entry_t                    *dhl_entry = NULL;
    nwsec_policy_t                 *policy = NULL;
    hal_handle_t                   hal_handle = 0;
    nwsec_policy_upd_app_ctxt_t    *app_ctx = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("{}: invalid cfg_ctxt",
                       __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    app_ctx = (nwsec_policy_upd_app_ctxt_t *) cfg_ctxt->app_ctxt;

    policy = (nwsec_policy_t *) dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("policy handle {}", hal_handle);

    ret = acl::acl_commit(app_ctx->acl_ctx_clone);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Policy commit failed with ret: {}", ret);
        goto end;
    }

    acl::acl_deref(app_ctx->acl_ctx_clone);

    // free the rules in the config db
    nwsec_policy_rules_free(policy);
end:
    return ret;

}

hal_ret_t
nwsec_policy_update_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

hal_ret_t
nwsec_policy_update_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

hal_ret_t
securitypolicy_update(nwsec::SecurityPolicySpec&      spec,
                      nwsec::SecurityPolicyResponse   *res)
{
    hal_ret_t                   ret = HAL_RET_OK;
    nwsec_policy_t              *policy = NULL;
    nwsec_policy_t              *policy_clone  = NULL;
    cfg_op_ctxt_t               cfg_ctxt = { 0 };
    dhl_entry_t                 dhl_entry = { 0 };
    nwsec_policy_upd_app_ctxt_t app_ctx = { 0 };
    const char                  *ctx_name = NULL;
    SecurityPolicyKeyHandle kh = spec.policy_key_or_handle();

    HAL_TRACE_DEBUG("---------------------- API End -------------------");
    HAL_TRACE_DEBUG(" {}: ", __FUNCTION__);

    ret = validate_nwsec_policy_update(spec, res);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}: validation failed", __FUNCTION__);
        goto end;
    }

    // retrieve security_policy object
    policy = security_policy_lookup_key_or_handle(kh);
    if (policy == NULL) {
        HAL_TRACE_ERR("Failed to find the policy");
        ret = HAL_RET_SECURITY_POLICY_NOT_FOUND;
        goto end;
    }

    nwsec_policy_make_clone(policy, (nwsec_policy_t **)&dhl_entry.cloned_obj);
    policy_clone = (nwsec_policy_t *)dhl_entry.cloned_obj;
    ret = extract_policy_from_spec(spec, (nwsec_policy_t *)dhl_entry.cloned_obj);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to extract policy from spec");
        nwsec_policy_free((nwsec_policy_t *)dhl_entry.cloned_obj);
        ret = HAL_RET_HANDLE_INVALID;
        goto end;
    }

    ctx_name = nwsec_acl_ctx_name(policy_clone->key.vrf_id);
    HAL_TRACE_DEBUG("Creating acl ctx {}", ctx_name);
    app_ctx.acl_ctx_clone = acl::acl_create(ctx_name, &nwsec_rule_config_glbl);

    ret = security_policy_add_to_ruledb(policy_clone, &app_ctx.acl_ctx_clone);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to add policy to lib, ret: {}", ret);
        goto end;
    }

    dhl_entry.handle = policy->hal_handle;
    dhl_entry.obj = policy;
    cfg_ctxt.app_ctxt = &app_ctx;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_upd_obj(policy->hal_handle, &cfg_ctxt,
                             nwsec_policy_update_upd_cb,
                             nwsec_policy_update_commit_cb,
                             nwsec_policy_update_abort_cb,
                             nwsec_policy_update_cleanup_cb);

end:
    if (ret == HAL_RET_OK) {
        HAL_API_STATS_INC(HAL_API_SECURITYPOLICY_UPDATE_SUCCESS);
    } else {
        HAL_API_STATS_INC(HAL_API_SECURITYPOLICY_UPDATE_FAIL);
    }

    nwsec_policy_prepare_rsp(res, ret,
                             policy ? policy->hal_handle : HAL_HANDLE_INVALID);
    hal_api_trace(" API End: nwsec_policy update");
    return HAL_RET_OK;
}

// Delete Policy
//
hal_ret_t
validate_nwsec_policy_delete(nwsec::SecurityPolicyDeleteRequest&    req,
                               nwsec::SecurityPolicyDeleteResponse    *res)
{
    return HAL_RET_OK;
}
hal_ret_t
nwsec_policy_delete_del_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    // No pd deletes / delete flows will be added
    //
    return HAL_RET_OK;
}

hal_ret_t
nwsec_policy_delete_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t       ret = HAL_RET_OK;
    dllist_ctxt_t   *lnode  = NULL;
    dhl_entry_t     *dhl_entry = NULL;
    nwsec_policy_t  *policy = NULL;
    hal_handle_t     hal_handle = 0;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("invalid cfg_ctxt");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    policy = (nwsec_policy_t *) dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("delete commit cb");
    // free the rules in the config db
    del_nwsec_policy_from_db(policy);
    nwsec_policy_free(policy);
    hal_handle_free(hal_handle);

end:
    return ret;
}

hal_ret_t
nwsec_policy_delete_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

hal_ret_t
nwsec_policy_delete_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

hal_ret_t
securitypolicy_delete(nwsec::SecurityPolicyDeleteRequest&    req,
                       nwsec::SecurityPolicyDeleteResponse    *res)
{

    hal_ret_t        ret = HAL_RET_OK;
    nwsec_policy_t   *policy = NULL;
    cfg_op_ctxt_t    cfg_ctxt = { 0 };
    dhl_entry_t      dhl_entry = { 0 };
    const char       *ctx_name = NULL;
    const acl_ctx_t  *acl_ctx = NULL;
    SecurityPolicyKeyHandle kh = req.policy_key_or_handle();

    ret = validate_nwsec_policy_delete(req, res);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR(" security policy validate failed ret : {}", ret);
        goto end;
    }

    policy  = security_policy_lookup_key_or_handle(kh);
    if (policy == NULL) {
        HAL_TRACE_ERR("failed to find the policy");
        ret = HAL_RET_SECURITY_POLICY_NOT_FOUND;
        goto end;
    }

    HAL_TRACE_DEBUG("deleting policy id:{}", policy->key.policy_id);
    ctx_name = nwsec_acl_ctx_name(policy->key.vrf_id);
    acl_ctx = acl::acl_get(ctx_name);
    if (acl_ctx == NULL) {
        HAL_TRACE_ERR("Failed to get acl ctx for the policy: {}", policy->key.policy_id);
        ret = HAL_RET_SECURITY_POLICY_NOT_FOUND;
        goto end;
    }
    acl::acl_delete(acl_ctx);

    dhl_entry.handle = policy->hal_handle;
    dhl_entry.obj = policy;
    cfg_ctxt.app_ctxt = NULL;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_del_obj(policy->hal_handle, &cfg_ctxt,
                             nwsec_policy_delete_del_cb,
                             nwsec_policy_delete_commit_cb,
                             nwsec_policy_delete_abort_cb,
                             nwsec_policy_delete_cleanup_cb);

end:
    if (ret == HAL_RET_OK) {
        HAL_API_STATS_INC(HAL_API_SECURITYPOLICY_DELETE_SUCCESS);
    } else {
        HAL_API_STATS_INC(HAL_API_SECURITYPOLICY_DELETE_FAIL);
    }
    res->set_api_status(hal_prepare_rsp(ret));

    return ret;
}

static hal_ret_t
security_policy_rule_spec_build (nwsec_rule_t *rule,
                                 nwsec::SecurityRule *spec)
{
    hal_ret_t               ret = HAL_RET_OK;
    dllist_ctxt_t           *entry = NULL;
    nwsec_policy_appid_t    *nwsec_plcy_appid = NULL;

    spec->set_rule_id(rule->rule_id);
    spec->mutable_action()->set_sec_action(rule->fw_rule_action.sec_action);
    spec->mutable_action()->set_log_action(rule->fw_rule_action.log_action);

    ret = rule_match_spec_build(&rule->fw_rule_match, spec->mutable_match());
    if ( ret != HAL_RET_OK) {
        return ret;
    }

    dllist_for_each(entry, &rule->appid_list_head) {
        nwsec_plcy_appid = dllist_entry(entry, nwsec_policy_appid_t, lentry);
        spec->add_appid(hal::app_redir::appid_to_app(nwsec_plcy_appid->appid));
    }

    return ret;
}

static hal_ret_t
security_policy_spec_build (nwsec_policy_t *policy,
                            nwsec::SecurityPolicySpec *spec)
{
    nwsec_rule_t            *nwsec_rule;
    nwsec::SecurityRule     *spec_rule;
    hal_ret_t               ret = HAL_RET_OK;

    spec->mutable_policy_key_or_handle()->mutable_security_policy_key()->set_security_policy_id(policy->key.policy_id);
    spec->mutable_policy_key_or_handle()->mutable_security_policy_key()->mutable_vrf_id_or_handle()->set_vrf_id(policy->key.vrf_id);

    for (uint32_t i = 0; i < policy->rule_len; i ++) {
        nwsec_rule = policy->dense_rules[i];
        if (nwsec_rule == NULL) {
            continue;
        }
        spec_rule = spec->add_rule();
        if (spec_rule == NULL) {
            return HAL_RET_OOM;
        }
        ret = security_policy_rule_spec_build(nwsec_rule, spec_rule);
        if (ret != HAL_RET_OK) {
            return ret;
        }
    }

    HAL_API_STATS_INC(HAL_API_SECURITYPOLICY_GET_SUCCESS);

    return ret;
}

static inline bool
security_policy_get_ht_cb (void *ht_entry, void *ctxt)
{
    nwsec_policy_t *policy;
    hal_handle_id_ht_entry_t *entry = (hal_handle_id_ht_entry_t *)ht_entry;
    nwsec::SecurityPolicyGetResponseMsg *rsp = (nwsec::SecurityPolicyGetResponseMsg *)ctxt;
    nwsec::SecurityPolicyGetResponse *response = rsp->add_response();
    hal_ret_t ret = HAL_RET_OK;

    policy = (nwsec_policy_t *)hal_handle_get_obj(entry->handle_id);
    
    ret = security_policy_spec_build(policy, response->mutable_spec());
    if (ret == HAL_RET_OK) {
        HAL_TRACE_DEBUG("Policy HT get ok");
        response->set_api_status(types::API_STATUS_OK);
    }

    // return false here, so that we don't terminate the walk
    return false;
}

hal_ret_t
securitypolicy_get (nwsec::SecurityPolicyGetRequest& req,
                    nwsec::SecurityPolicyGetResponseMsg *rsp)
{
    nwsec_policy_t  *policy;
    hal_ret_t       ret = HAL_RET_OK;

    // walk all policies as no key is specified
    if (!req.has_key_or_handle()) {
        g_hal_state->nwsec_policy_ht()->walk(security_policy_get_ht_cb, rsp);
        HAL_API_STATS_INC(HAL_API_SECURITYPOLICY_GET_SUCCESS);
        return HAL_RET_OK;
    }

    auto kh = req.key_or_handle();
    auto response = rsp->add_response();
    if ((policy = security_policy_lookup_key_or_handle(kh)) == NULL) {
        response->set_api_status(types::API_STATUS_NOT_FOUND);
        HAL_API_STATS_INC(HAL_API_SECURITYPOLICY_GET_FAIL);
        return HAL_RET_SECURITY_POLICY_NOT_FOUND;
    }

    ret = security_policy_spec_build(policy, response->mutable_spec());
    if (ret == HAL_RET_OK) {
        response->set_api_status(types::API_STATUS_OK);
        return ret;
    }

    return HAL_RET_OK;
}

hal_ret_t
securitygrouppolicy_create(nwsec::SecurityGroupPolicySpec&    spec,
                           nwsec::SecurityGroupPolicyResponse *rsp)
{
    return HAL_RET_OK;
}

hal_ret_t
securitygrouppolicy_update(nwsec::SecurityGroupPolicySpec&    spec,
                           nwsec::SecurityGroupPolicyResponse *rsp)
{
    return HAL_RET_OK;
}

hal_ret_t
securitygrouppolicy_delete(nwsec::SecurityGroupPolicyDeleteRequest&    spec,
                           nwsec::SecurityGroupPolicyDeleteResponse    *res)
{
    return HAL_RET_OK;
}

hal_ret_t
securitygrouppolicy_get(nwsec::SecurityGroupPolicyGetRequest&       spec,
                        nwsec::SecurityGroupPolicyGetResponseMsg    *res)
{
    return HAL_RET_OK;
}
}    // namespace hal
