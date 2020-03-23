//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include <google/protobuf/util/json_util.h>
#include "nic/include/base.hpp"
#include "nic/hal/hal.hpp"
#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "nic/hal/plugins/sfw/cfg/nwsec_group.hpp"
#include "gen/hal/include/hal_api_stats.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/src/utils/if_utils.hpp"
#include "gen/proto/nwsec.pb.h"
#include "nic/hal/plugins/app_redir/app_redir_ctx.hpp"
#include <google/protobuf/util/json_util.h>
#include <fcntl.h>
#include <malloc.h>
#include "nic/sdk/platform/capri/capri_hbm_rw.hpp"
#include "nic/sdk/lib/pal/pal.hpp"
#include "nic/sdk/include/sdk/types.hpp"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "gen/proto/rulestats/rulestats.delphi.hpp"

#define NWSEC_RULE_STATS_SHIFT         6
#define SESSION_MAX_INACTIVITY_TIMEOUT 0xFFFFFFFF

extern sdk::lib::indexer *g_rule_stats_indexer;

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
using nwsec::AppData;



static inline nwsec_rulelist_t * nwsec_rulelist_lookup_by_key(nwsec_policy_t *policy, rule_key_t rule_key);
static inline nwsec_rulelist_t * nwsec_rulelist_alloc_init();

acl::acl_config_t nwsec_rule_config_glbl = { };

//-----------------------------------------------------------------------------
// dump security policy spec
//-----------------------------------------------------------------------------
/*
 * Malloc holds lot of free memory.
 * By trimming the memory the memory is released back
 * used this in cases where we free memory in large blocks
*/
#define TRIM_LOG_ENABLE 0
void
trim_mem_usage (const char *opn)
{
    malloc_trim(0);
#if TRIM_LOG_ENABLE
    char buffer[2048];
    struct mallinfo minfo = mallinfo();
    int fd = 0;
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    fd = open("/data/minfo.txt", O_RDWR | O_APPEND | O_CREAT, mode);
    int len_writ = sprintf(buffer, "%s \n"
                     "Non-mmapped space allocated (bytes)  (arena)       : %d \n"
                     "Number of free chunks                (ordblks)     :  %d \n"
                     "Number of free fastbin blocks        (smblks)      : %d \n"
                     "Number of mmapped regions            (hblks)       : %d \n"
                     "Space allocated in mmapped regions (bytes)(hblkhd) : %d \n"
                     "Maximum total allocated space (bytes) (usmblks)    : %d \n"
                    "Space in freed fastbin blocks (bytes) (fsmblks)    : %d \n"
                     "Total allocated space (bytes)        (uordblks)    : %d \n"
                     "Total free space (bytes)          (fordblks)       : %d \n"
                     "Top-most, releasable space (bytes)   (keepcost)    : %d \n",
                     opn,
                     minfo.arena, minfo.ordblks, minfo.smblks, minfo.hblks, minfo.hblkhd,
                     minfo.usmblks, minfo.fsmblks, minfo.uordblks, minfo.fordblks, minfo.keepcost);
    write(fd, buffer, len_writ);
    memset(buffer, 0, sizeof(buffer));
    malloc_stats();
    close(fd);
#endif
}

hal_ret_t
rule_stats_cb (rule_ctr_t *ctr, bool add)
{
    sdk::lib::indexer::status rs;
    sdk::types::mem_addr_t vaddr, start_addr;
    uint32_t idx;

    if (add) {
        start_addr = asicpd_get_mem_addr(CAPRI_HBM_REG_NWSEC_RULE_STATS);
        SDK_ASSERT(start_addr != INVALID_MEM_ADDRESS);

        rs = g_rule_stats_indexer->alloc(&idx);
        SDK_ASSERT_RETURN((rs == sdk::lib::indexer::SUCCESS), HAL_RET_OOM);

        start_addr += idx * (1 << NWSEC_RULE_STATS_SHIFT);
        auto rule_metrics_ptr = delphi::objects::RuleMetrics::NewRuleMetrics(ctr->rule_key, start_addr);
        SDK_ASSERT(rule_metrics_ptr != NULL);

        sdk::lib::pal_ret_t ret = sdk::lib::pal_physical_addr_to_virtual_addr(start_addr, &vaddr);
        SDK_ASSERT(ret == sdk::lib::PAL_RET_OK);

        ctr->rule_stats = (rule_ctr_data_t *)vaddr;
        ctr->stats_idx = idx;
        bzero(ctr->rule_stats, sizeof(rule_ctr_data_t));
    } else {
        auto rule_metrics_ptr = delphi::objects::RuleMetrics::Find(ctr->rule_key);
        if (rule_metrics_ptr != NULL) {
            delphi::objects::RuleMetrics::Release(rule_metrics_ptr);
            rule_metrics_ptr->Delete();
        }
        g_rule_stats_indexer->free(ctr->stats_idx);
        ctr->rule_stats = NULL;
    }

    return HAL_RET_OK;
}

static inline void
nwsec_spec_dump(void *spec)
{
    std::string cfg;
    Message *msg = (Message *) spec;
    google::protobuf::util::MessageToJsonString(*msg, &cfg);
    HAL_TRACE_VERBOSE("configuration:");
    HAL_TRACE_VERBOSE("{}", cfg.c_str());
}


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
    SDK_ASSERT(entry != NULL);
    ht_entry = (hal_handle_id_ht_entry_t *) entry;
    if (ht_entry == NULL) {
        return NULL;
    }
    nwsec_grp = (nwsec_group_t *)hal_handle_get_obj(ht_entry->handle_id);
    return (void *)&(nwsec_grp->sg_id);
}

uint32_t
nwsec_group_key_size ()
{
    return sizeof(nwsec_group_id_t);
}

static hal_ret_t
validate_nwsec_group_create(nwsec::SecurityGroupSpec& spec,
                            nwsec::SecurityGroupResponse *rsp)
{

    // key-handle field must be set
    if (!spec.has_key_or_handle()) {
        HAL_TRACE_ERR("security group id or handle not set in request");
        rsp->set_api_status(types::API_STATUS_INVALID_ARG);
        return HAL_RET_INVALID_ARG;
    }

    auto kh = spec.key_or_handle();
    if (kh.key_or_handle_case() !=
            SecurityGroupKeyHandle::kSecurityGroupId) {
        // key-handle field set, but securityGroup id not provided
        HAL_TRACE_ERR("security group id not set in"
                      "request");
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
        HAL_TRACE_ERR("invalid cfg_ctxt");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    nwsec_grp = (nwsec_group_t *) dhl_entry->obj;

    HAL_TRACE_DEBUG("policy_id {}",nwsec_grp->sg_id);
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
        HAL_TRACE_ERR("invalid cfg_ctxt");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    nwsec_grp = (nwsec_group_t *) dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("sg_id {} handle {}",
                    nwsec_grp->sg_id,
                    hal_handle);
    //Insert into Hash DB
    ret = add_nwsec_group_to_db(nwsec_grp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("failed to add sg_id {} to db,"
                      "err : {}", nwsec_grp->sg_id,
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
        rsp->mutable_status()->mutable_key_or_handle()->set_security_group_handle(hal_handle);
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
    HAL_TRACE_DEBUG("Creating nwsec group, sg_id {}",
                    spec.key_or_handle().security_group_id());

    nwsec_spec_dump(&spec);
    ret = validate_nwsec_group_create(spec, res);

    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("validation failed, sg_id {} ret: {}",
                      spec.key_or_handle().security_group_id(),
                      ret);
        goto end;
    }
    // Check of the sgid is already present

    // instanstiate the security_policy
    nwsec_grp  = nwsec_group_alloc_init();
    if (nwsec_grp == NULL) {
        HAL_TRACE_ERR("unable to allocate handle/memory"
                      "ret: {}",ret);
        ret = HAL_RET_OOM;
        goto end;
    }

    nwsec_grp->sg_id =  spec.key_or_handle().security_group_id();
    nwsec_grp->hal_handle = hal_handle_alloc(HAL_OBJ_ID_SECURITY_GROUP);
    if (nwsec_grp->hal_handle == HAL_HANDLE_INVALID) {
        HAL_TRACE_ERR("failed to alloc handle for policy_id: {}",nwsec_grp->sg_id);
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
    HAL_TRACE_DEBUG("Updating nwsec group, sg_id {} handle {}",
                    spec.key_or_handle().security_group_id(),
                    spec.key_or_handle().security_group_handle());
    HAL_API_STATS_INC(HAL_API_SECURITYGROUP_UPDATE_SUCCESS);
    return HAL_RET_OK;

}

hal_ret_t
validate_nwsec_group_delete(nwsec::SecurityGroupDeleteRequest& req,
                            nwsec::SecurityGroupDeleteResponseMsg *res)
{
    return HAL_RET_OK;
}

hal_ret_t
nwsec_group_delete_del_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;

}

hal_ret_t
nwsec_group_delete_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t       ret = HAL_RET_OK;
    dllist_ctxt_t   *lnode = NULL;
    dhl_entry_t     *dhl_entry = NULL;
    nwsec_group_t   *nwsec_grp = NULL;
    hal_handle_t    hal_handle = 0;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("invalid cfg_ctxt");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    nwsec_grp = (nwsec_group_t *) dhl_entry->obj;
    hal_handle = dhl_entry->handle;
    ret = del_nwsec_group_from_db(nwsec_grp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_DEBUG("delete commit cb failed for sg_id {}", nwsec_grp->sg_id);
        return ret;
    }
    // free the security group
    hal_handle_free(hal_handle);

end:
    return ret;
}

hal_ret_t
nwsec_group_delete_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;

}

hal_ret_t
nwsec_group_delete_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;

}

hal_ret_t
securitygroup_delete(nwsec::SecurityGroupDeleteRequest& req,
                     nwsec::SecurityGroupDeleteResponseMsg *res)
{
    hal_ret_t                     ret;
    nwsec_group_t                 *nwsec_grp = NULL;
    //nwsec_group_create_app_ctxt_t *app_ctxt = NULL;
    SecurityGroupKeyHandle         kh = req.key_or_handle();


    dhl_entry_t                   dhl_entry = { 0 };
    cfg_op_ctxt_t                 cfg_ctxt = { 0 };
    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("Deleting nwsec group, sg_id {}",
                    req.key_or_handle().security_group_id());
    ret = validate_nwsec_group_delete(req, res);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("nwsec group delete validate failed ret: {}", ret);
        goto end;
    }

    nwsec_grp = nwsec_group_lookup_key_or_handle(kh);
    if (nwsec_grp == NULL) {
        HAL_TRACE_ERR("failed to find the security group");
        ret = HAL_RET_SG_NOT_FOUND;
        goto end;
    }

    dhl_entry.handle = nwsec_grp->hal_handle;
    dhl_entry.obj = nwsec_grp;
    cfg_ctxt.app_ctxt = NULL;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_del_obj(nwsec_grp->hal_handle, &cfg_ctxt,
                             nwsec_group_delete_del_cb,
                             nwsec_group_delete_commit_cb,
                             nwsec_group_delete_abort_cb,
                             nwsec_group_delete_cleanup_cb);
end:
    if (ret == HAL_RET_OK) {
        HAL_TRACE_DEBUG("delete securitygroup id:{} success", nwsec_grp->sg_id);
        HAL_API_STATS_INC(HAL_API_SECURITYGROUP_DELETE_SUCCESS);
    } else {
        HAL_API_STATS_INC(HAL_API_SECURITYGROUP_DELETE_FAIL);
    }
    res->add_api_status(hal_prepare_rsp(ret));
    return ret;
}

hal_ret_t
securitygroup_get(nwsec::SecurityGroupGetRequest& spec,
                  nwsec::SecurityGroupGetResponseMsg *res)
{
    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("Deleting nwsec group, sg_id {}",
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

    SDK_SPINLOCK_LOCK(&nwsec_grp->slock);
    dllist_add(&nwsec_grp->nw_list_head, &nwsec_policy_nw_info->dllist_ctxt);
    SDK_SPINLOCK_UNLOCK(&nwsec_grp->slock);

    HAL_TRACE_DEBUG("add sg => nw, handles:{} => {}",
                    nwsec_grp->hal_handle, nw_handle_id);
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
    SDK_SPINLOCK_LOCK(&nwsec_grp->slock);
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
    SDK_SPINLOCK_UNLOCK(&nwsec_grp->slock);

    HAL_TRACE_DEBUG("del sg =/=> nw, handles:{} =/=> {}",
                    nwsec_grp->hal_handle, nw_handle_id);
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

    SDK_SPINLOCK_LOCK(&nwsec_grp->slock);
    dllist_add(&nwsec_grp->ep_list_head, &nwsec_policy_ep_info->dllist_ctxt);
    SDK_SPINLOCK_UNLOCK(&nwsec_grp->slock);

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


    SDK_SPINLOCK_LOCK(&nwsec_grp->slock);
    dllist_for_each_safe(curr, next,  &nwsec_grp->ep_list_head) {
        nwsec_policy_ep_info  = dllist_entry(curr, hal_handle_id_list_entry_t, dllist_ctxt);
        if (nwsec_policy_ep_info != NULL) {
            if (nwsec_policy_ep_info->handle_id == ep_handle_id) {
                sdk::lib::dllist_del(curr);
                hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_LIST_ENTRY,
                                          nwsec_policy_ep_info);
                SDK_SPINLOCK_UNLOCK(&nwsec_grp->slock);
                return HAL_RET_OK;
            }
        }
    }
    SDK_SPINLOCK_UNLOCK(&nwsec_grp->slock);
    return HAL_RET_NW_HANDLE_NOT_FOUND;
}

// Security Group related functions end;
//
// Security policy begin

/** Rules within a security policy **/
static inline nwsec_rule_t *
nwsec_rule_alloc(void)
{
    nwsec_rule_t     *rule = NULL;

    rule = (nwsec_rule_t *)
                        g_hal_state->nwsec_rule_slab()->alloc();
    if (rule == NULL) {
        return NULL;
    }
    return rule;
}

// Initialize a nwsec_rule instance
static inline nwsec_rule_t *
nwsec_rule_init (nwsec_rule_t *rule)
{
    if (!rule) {
        return NULL;
    }

    // Slab free will be called when the ref count drops to zero
    ref_init(&rule->ref_count, [] (const ref_t * ref) {
        nwsec_rule_t * rule = container_of(ref, nwsec_rule_t, ref_count);
        if (rule->fw_rule_action.alg == nwsec::APP_SVC_SUN_RPC ||
            rule->fw_rule_action.alg == nwsec::APP_SVC_MSFT_RPC) {
            union hal::alg_options::opt_ *opt = &rule->fw_rule_action.app_options.opt;

            if (opt->sunrpc_opts.program_ids != NULL) {
                HAL_FREE(HAL_MEM_ALLOC_SFW, opt->sunrpc_opts.program_ids);
                opt->sunrpc_opts.program_ids = NULL;
            }

            if (opt->msrpc_opts.uuids != NULL) {
                HAL_FREE(HAL_MEM_ALLOC_SFW, opt->msrpc_opts.uuids);
                opt->msrpc_opts.uuids = NULL;
            }
        }
        rule_match_cleanup(&rule->fw_rule_match);
        g_hal_state->nwsec_rule_slab()->free(rule);
    });
    rule_match_init(&rule->fw_rule_match);
    dllist_reset(&rule->dlentry);
    dllist_reset(&rule->appid_list_head);
    return rule;
}

// allocate and initialize a match_template
static inline nwsec_rule_t *
nwsec_rule_alloc_init()
{
    return nwsec_rule_init(nwsec_rule_alloc());
}

static inline hal_ret_t
add_nwsec_rulelist_to_db (nwsec_policy_t *policy, nwsec_rulelist_t *rule, int rule_index);
static inline hal_ret_t
add_nwsec_rule_to_db (nwsec_policy_t *policy, nwsec_rule_t *rule, int rule_index)
{
    hal_ret_t ret = HAL_RET_OK;

    // find the rule list. if not found allocate one.

    nwsec_rulelist_t *rulelist = nwsec_rulelist_lookup_by_key(policy, rule->rule_id);
    if (rulelist == NULL) {
        HAL_TRACE_VERBOSE("Create rule list:{}", rule->rule_id);
        rulelist = nwsec_rulelist_alloc_init();
        rulelist->rule_id = rule->rule_id;
        ret = add_nwsec_rulelist_to_db(policy, rulelist, rule_index);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_DEBUG("unable to insert rulelist into policy db");
            return HAL_RET_ERR;
        }
    }
    dllist_add(&rulelist->head, &rule->dlentry);
    return HAL_RET_OK;
}
// There is no case where we have to delete a particular rule
// del_nwsec_rule_from_db () not needed for now


//rulelist related changes

bool
nwsec_rulelist_compare_key_func (void *key1, void *key2)
{
    dllist_ctxt_t    lentry;
    SDK_ASSERT((key1 != NULL) && (key2 != NULL));
    // Compare the hash value
    if (!memcmp(key1, key2, sizeof(rule_key_t))) {
        return true;
    }
    return false;
}

void *
nwsec_rulelist_get_key_func (void *entry)
{
    SDK_ASSERT(entry != NULL);
    return &((nwsec_rulelist_t *)entry)->rule_id;
}

uint32_t nwsec_rulelist_key_size (void) {
    return sizeof(rule_key_t);
}

/** Rules within a security policy **/
static inline nwsec_rulelist_t *
nwsec_rulelist_alloc(void)
{
    nwsec_rulelist_t     *rule = NULL;

    rule = (nwsec_rulelist_t *)
                        g_hal_state->nwsec_rulelist_slab()->alloc();
    if (rule == NULL) {
        return NULL;
    }
    return rule;
}

// Initialize a nwsec_rule instance
static inline nwsec_rulelist_t *
nwsec_rulelist_init (nwsec_rulelist_t *rule)
{
    if (!rule) {
        return NULL;
    }

    rule->ht_ctxt.reset();
    // Slab free will be called when the ref count drops to zero
    ref_init(&rule->ref_count, [] (const ref_t * ref) {
        nwsec_rulelist_t * rule = container_of(ref, nwsec_rulelist_t, ref_count);
        //HAL_TRACE_DEBUG("Calling rulelist free");
        g_hal_state->nwsec_rulelist_slab()->free(rule);

    });
    dllist_reset(&rule->head);
    return rule;
}

// allocate and initialize a match_template
static inline nwsec_rulelist_t *
nwsec_rulelist_alloc_init()
{
    return nwsec_rulelist_init(nwsec_rulelist_alloc());
}


static inline hal_ret_t
add_nwsec_rulelist_to_db (nwsec_policy_t *policy, nwsec_rulelist_t *rule, int rule_index)
{
    sdk_ret_t sdk_ret = policy->rules_ht[policy->version]->insert(rule,
                                     &rule->ht_ctxt);
    hal_ret_t ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to install the rule id:{} in the policy {}", rule->rule_id, policy->key.policy_id);
        return ret;
    }

    return HAL_RET_OK;
}

// find a network rule by id??
static inline nwsec_rulelist_t *
nwsec_rulelist_lookup_by_key(nwsec_policy_t *policy, rule_key_t rule_key)
{
    int version = policy->version;
    return ((nwsec_rulelist_t *)policy->rules_ht[version]->lookup(&rule_key));

}

// security policy
static inline nwsec_policy_t *
nwsec_policy_alloc(void)
{
    nwsec_policy_t     *policy = NULL;

    policy = (nwsec_policy_t *)
                        g_hal_state->nwsec_policy_slab()->alloc();
    if (policy == NULL) {
        return NULL;
    }
    return policy;
}

const char *nwsec_acl_ctx_name(vrf_id_t vrf_id)
{
    thread_local static char name[ACL_NAMESIZE];

    std::snprintf(name, sizeof(name), "nwsec-ipv4-rules:%lu", vrf_id);

    return name;
}

// Initialize a nwsec_policy instance
static inline nwsec_policy_t *
nwsec_policy_init (nwsec_policy_t *policy)
{
    if (!policy) {
        return NULL;
    }
    memset(&policy->key, 0, sizeof(policy_key_t));

    for (int i = 0; i < MAX_VERSION; i++) {
        policy->rules_ht[policy->version] = ht::factory(HAL_MAX_NW_SEC_GROUP_CFG,
                                                        hal::nwsec_rulelist_get_key_func,
                                                        hal::nwsec_rulelist_key_size());
        SDK_ASSERT_RETURN((policy->rules_ht[policy->version] != NULL), NULL);
    }
    policy->ht_ctxt.reset();
    policy->acl_ctx = NULL;

    return policy;
}

// allocate and initialize a match_template
static inline nwsec_policy_t *
nwsec_policy_alloc_init()
{
    return nwsec_policy_init(nwsec_policy_alloc());
}

static inline hal_ret_t
nwsec_policy_rules_free(nwsec_policy_t *policy)
{
    policy->rules_ht[policy->version]->walk(([](void *data, void *ctxt) -> bool {
        dllist_ctxt_t  *curr = NULL, *next = NULL;
        nwsec_rulelist_t *rulelist = (nwsec_rulelist_t *) data;
        if (rulelist == NULL) {

            return true;
        }
        dllist_for_each_safe(curr, next, &rulelist->head) {
            nwsec_rule_t *rule = dllist_entry(curr, nwsec_rule_t, dlentry);
            ref_dec(&rule->ref_count);
        }
        ref_dec(&rulelist->ref_count);
        return false; }), NULL);
    for (int i = 0; i < MAX_VERSION; i++) {
        ht::destroy(policy->rules_ht[i]);
    }


    return HAL_RET_OK;
}

// free
//
static inline hal_ret_t
nwsec_policy_free(nwsec_policy_t *policy)
{
    nwsec_policy_rules_free(policy);
    // Free dense rules one we make it dynamic array
    //HAL_MEM_FREE(policy->dense_rules);
    g_hal_state->nwsec_policy_slab()->free(policy);
    return HAL_RET_OK;
}

static inline hal_ret_t
add_nwsec_policy_to_db (nwsec_policy_t *policy)
{
    hal_ret_t                 ret;
    sdk_ret_t                 sdk_ret;
    hal_handle_id_ht_entry_t  *entry;

    HAL_TRACE_DEBUG("adding policy to hash table");

    entry = (hal_handle_id_ht_entry_t *)g_hal_state->
            hal_handle_id_ht_entry_slab()->alloc();
    if (entry == NULL) {
        return HAL_RET_OOM;
    }

    entry->handle_id = policy->hal_handle;

    sdk_ret = g_hal_state->nwsec_policy_ht()->insert_with_key(&policy->key,
                                                              entry,
                                                              &entry->ht_ctxt);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (sdk_ret != sdk::SDK_RET_OK) {
        HAL_TRACE_ERR("Failed to add security policy  {} to policy db,"
                      "err : {}", policy->key.policy_id, ret);
        hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);
    }
    return ret;

}

static inline hal_ret_t
del_nwsec_policy_from_db (nwsec_policy_t *policy)
{
    hal_handle_id_ht_entry_t  *entry;

    HAL_TRACE_DEBUG("removing policy from hash table");

    entry = (hal_handle_id_ht_entry_t *)
             g_hal_state->nwsec_policy_ht()->remove(&policy->key);
    if (entry) {
        hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);
    }
    return HAL_RET_OK;

}

// find a security policy by sg
nwsec_policy_t *
find_nwsec_policy_by_key(uint32_t policy_id, uint32_t vrf_id)
{
    hal_handle_id_ht_entry_t    *entry;
    policy_key_t                 policy_key = { 0 };
    nwsec_policy_t              *policy;

    policy_key.policy_id = policy_id;
    policy_key.vrf_id = vrf_id;
    entry = (hal_handle_id_ht_entry_t *) g_hal_state->
            nwsec_policy_ht()->lookup(&policy_key);
    if (entry && (entry->handle_id != HAL_HANDLE_INVALID)) {
        SDK_ASSERT(hal_handle_get_from_handle_id(entry->handle_id)->obj_id() == HAL_OBJ_ID_SECURITY_POLICY);
        policy = (nwsec_policy_t *)hal_handle_get_obj(entry->handle_id);
        return policy;
    }
    return NULL;

}

static inline nwsec_policy_t *
find_nwsec_policy_by_handle (hal_handle_t handle)
{
    if (handle == HAL_HANDLE_INVALID) {
        return NULL;
    }
    auto hal_handle = hal_handle_get_from_handle_id(handle);
    if (!hal_handle) {
        HAL_TRACE_DEBUG("failed to find hal object with handle:{}",
                        handle);
        return NULL;
    }
    if (hal_handle->obj_id() != HAL_OBJ_ID_SECURITY_POLICY) {
        HAL_TRACE_DEBUG("failed to find security_group  with handle:{}",
                        handle);
        return NULL;
    }
    return (nwsec_policy_t *)hal_handle->obj();
}

void *
nwsec_policy_get_key_func (void *entry)
{

    hal_handle_id_ht_entry_t   *ht_entry;
    nwsec_policy_t             *policy = NULL;
    SDK_ASSERT(entry != NULL);
    ht_entry = (hal_handle_id_ht_entry_t *) entry;
    if (ht_entry == NULL) {
        return NULL;
    }
    policy  = (nwsec_policy_t *) hal_handle_get_obj(ht_entry->handle_id);
    return (void *)&(policy->key);
}

uint32_t
nwsec_policy_key_size ()
{
    return sizeof(policy_key_t);
}

//
//API to convert MSRPC UUID String to uint8_t array
//
static inline void __str_to_uuid(char *uuid_str, uint8_t *uuid_arr) {
    // Define the UUID Struct
    // to read it right
    typedef struct  uuid_ {
         uint32_t time_lo;
         uint16_t time_mid;
         uint16_t time_hi_vers;
         uint8_t  clock_seq_hi;
         uint8_t  clock_seq_lo;
         uint8_t  node[6];
    } uuid_t;
    uuid_t   *uuid = (uuid_t *)uuid_arr;
    char     *tok  = NULL;
    uint32_t  val = 0;

    tok = strtok(uuid_str, "-");
    if (tok == NULL) return;
    uuid->time_lo = strtoul(tok, NULL, 16);
    tok = strtok(NULL, "-");
    if (tok == NULL) return;
    uuid->time_mid = strtoul(tok, NULL, 16);
    tok = strtok(NULL, "-");
    if (tok == NULL) return;
    uuid->time_hi_vers = strtoul(tok, NULL, 16);
    tok = strtok(NULL, "-");
    if (tok == NULL) return;
    val = strtoul(tok, NULL, 16);
    uuid->clock_seq_hi = (val & 0xFF00)>>8;
    uuid->clock_seq_lo = (val & 0xFF);
    tok = strtok(NULL, "-");
    if (tok == NULL) return;
    uint64_t node = strtoul(tok, NULL, 16);
    for (int idx=5; idx>=0; idx--) {
         uuid->node[idx] = node&0xFF;
         node = node >> 8;
    }
}

//
//API to convert MSRPC UUID to string
//
static inline std::string  __uuid_to_str(uint8_t *uuid) {
    typedef struct  uuid_ {
         uint32_t time_lo;
         uint16_t time_mid;
         uint16_t time_hi_vers;
         uint8_t  clock_seq_hi;
         uint8_t  clock_seq_lo;
         uint8_t  node[6];
    } uuid_t;
    uuid_t  *u = (uuid_t *)uuid;
    std::stringstream ss;
    uint16_t clock_seq = ((u->clock_seq_hi << 8) | u->clock_seq_lo);

    ss << hex << u->time_lo;
    ss << "-";
    ss << setw(4) << setfill('0') << hex << u->time_mid;
    ss << "-";
    ss << setw(4) << setfill('0') << hex << u->time_hi_vers;
    ss << "-";
    ss << setw(4) << setfill('0') << hex << clock_seq;
    ss << "-";
    for (int idx=0; idx<6; idx+=2) {
       uint16_t node = ((u->node[idx] << 8) | u->node[idx+1]);
       ss << setw(4) << setfill('0') << hex << node;
    }

    return ss.str();
}

//nwsec_rule related
hal_ret_t
extract_nwsec_rule_from_spec(nwsec::SecurityRule spec, nwsec_rule_t *rule)
{
    hal_ret_t              ret = HAL_RET_OK;
    //uint32_t             hv = 2166136261; // lns:revisit

    // Action
    rule->fw_rule_action.sec_action = nwsec::SECURITY_RULE_ACTION_ALLOW;
    rule->fw_rule_action.log_action =  nwsec::LOG_NONE;
    rule->rule_id    = spec.rule_id();
    if (spec.has_action()) {
        rule->fw_rule_action.sec_action =  spec.action().sec_action();
        rule->fw_rule_action.log_action = spec.action().log_action();

        // Parse APPs (set alg name only for now)
        rule->fw_rule_action.alg = nwsec::APP_SVC_NONE;
        rule->fw_rule_action.idle_timeout = SESSION_MAX_INACTIVITY_TIMEOUT;
        if (spec.action().has_app_data()) {
            auto app = spec.action().app_data();
            rule->fw_rule_action.alg = app.alg();
            rule->fw_rule_action.idle_timeout = app.idle_timeout();
            union hal::alg_options::opt_ *opt = &rule->fw_rule_action.app_options.opt;
            HAL_TRACE_VERBOSE("Policy_rules::rule_id {} has_app_data: {} idle timeout: {}", rule->rule_id, spec.action().has_app_data(), rule->fw_rule_action.idle_timeout);
            if (app.AppOptions_case() == AppData::kFtpOptionInfo) {
                opt->ftp_opts.allow_mismatch_ip_address = app.ftp_option_info().allow_mismatch_ip_address();
            } else if (app.AppOptions_case() == AppData::kDnsOptionInfo) {
                opt->dns_opts.drop_multi_question_packets = app.dns_option_info().drop_multi_question_packets();
                opt->dns_opts.drop_large_domain_name_packets = app.dns_option_info().drop_large_domain_name_packets();
                opt->dns_opts.drop_long_label_packets = app.dns_option_info().drop_long_label_packets();
                opt->dns_opts.drop_multizone_packets = app.dns_option_info().drop_multizone_packets();
                opt->dns_opts.max_msg_length = app.dns_option_info().max_msg_length();
                opt->dns_opts.query_response_timeout = app.dns_option_info().query_response_timeout();
            } else if (app.AppOptions_case() == AppData::kMsrpcOptionInfo) {
                if (app.msrpc_option_info().data_size()) {
                    opt->msrpc_opts.uuids = (rpc_uuid_t *)HAL_CALLOC(HAL_MEM_ALLOC_SFW,
                                        (sizeof(rpc_uuid_t)*app.msrpc_option_info().data_size()));
                    SDK_ASSERT(opt->msrpc_opts.uuids != NULL);
                    opt->msrpc_opts.uuid_sz = app.msrpc_option_info().data_size();
                    for (int idx=0; idx<app.msrpc_option_info().data_size(); idx++) {
                        __str_to_uuid((char *)app.msrpc_option_info().data(idx).program_id().c_str(),
                                      opt->msrpc_opts.uuids[idx].uuid);
                        opt->msrpc_opts.uuids[idx].timeout = app.msrpc_option_info().data(idx).idle_timeout();
                        HAL_TRACE_VERBOSE("UUID: {} timeout: {} uuid_str: {}",
                                 fte::hex_str((uint8_t*)opt->msrpc_opts.uuids[idx].uuid, MAX_UUID_SZ),
                                 opt->msrpc_opts.uuids[idx].timeout, __uuid_to_str(opt->msrpc_opts.uuids[idx].uuid));
                    }
                }
            } else if (app.AppOptions_case() == AppData::kSunRpcOptionInfo) {
                if (app.sun_rpc_option_info().data_size()) {
                    opt->sunrpc_opts.program_ids = (rpc_programid_t *)HAL_CALLOC(HAL_MEM_ALLOC_SFW,
                                        (sizeof(rpc_programid_t)*app.sun_rpc_option_info().data_size()));
                    SDK_ASSERT(opt->sunrpc_opts.program_ids != NULL);
                    opt->sunrpc_opts.programid_sz = app.sun_rpc_option_info().data_size();
                    for (int idx=0; idx<app.sun_rpc_option_info().data_size(); idx++) {
                         opt->sunrpc_opts.program_ids[idx].program_id = strtoul(\
                                       app.sun_rpc_option_info().data(idx).program_id().c_str(), NULL, 10);
                         opt->sunrpc_opts.program_ids[idx].timeout = app.sun_rpc_option_info().data(idx).idle_timeout();
                         HAL_TRACE_VERBOSE("Program id: {} timeout: {}", opt->sunrpc_opts.program_ids[idx].program_id,
                                                                        opt->sunrpc_opts.program_ids[idx].timeout);
                    }
                }
            } else if (app.AppOptions_case() == AppData::kSipOptions) {
                opt->sip_opts.ctimeout = app.sip_options().ctimeout();
                opt->sip_opts.dscp_code_point = app.sip_options().dscp_code_point();
                opt->sip_opts.media_inactivity_timeout = app.sip_options().media_inactivity_timeout();
                opt->sip_opts.max_call_duration = app.sip_options().max_call_duration();
                opt->sip_opts.t1_timer_value = app.sip_options().t1_timer_value();
                opt->sip_opts.t4_timer_value = app.sip_options().t4_timer_value();
            }
        }
    }

    ret = rule_match_spec_extract(spec.match(), &rule->fw_rule_match);
    if ( ret != HAL_RET_OK) {
        rule_match_cleanup(&rule->fw_rule_match);
        HAL_TRACE_ERR("Failed to retrieve rule_match");
        return ret;
    }

    nwsec_policy_appid_t* nwsec_plcy_appid = NULL;
    uint32_t apps_sz = spec.appid_size();
    HAL_TRACE_VERBOSE("Policy_rules::rule_id {} AppidSize {}", rule->rule_id, apps_sz);
    for (uint32_t apps_cnt = 0; apps_cnt < apps_sz; apps_cnt++) {
        nwsec_plcy_appid = nwsec_policy_appid_alloc_and_init();
        if (nwsec_plcy_appid == NULL) {
            HAL_TRACE_ERR("unable to"
                          "allocate handle/memory"
                          "ret: {}", ret);
            //ToDo:Cleanup the nwsec_plcy_rules allocated till now
            return HAL_RET_OOM;
        }

        uint32_t appid;
        ret = hal::app_redir::app_to_appid(
                  spec.appid(apps_cnt),
                  appid);
        // TODO: Handle resource cleanup before returning
        if(HAL_RET_OK != ret) {
            HAL_TRACE_ERR("unknown app {}",
                          spec.appid(apps_cnt));
            return ret;
        }
        nwsec_plcy_appid->appid = appid;

         //To Do: Check to Get lock on nwsec_plcy_rules ??
        dllist_add_tail(&rule->appid_list_head,
                        &nwsec_plcy_appid->lentry);
    }
    return ret;
}

void
nwsec_rule_free(void *rule)
{
    ref_dec(&((nwsec_rule_t *)rule)->ref_count);
    return;
}

//Rule Db related
hal_ret_t
security_policy_add_to_ruledb( nwsec_policy_t *policy, const acl_ctx_t **acl_ctx)
{
    struct fn_ctx_t {
        const acl_ctx_t **acl_ctx;
        hal_ret_t   ret;
    } fn_ctx = { acl_ctx, HAL_RET_OK };

    policy->rules_ht[policy->version]->walk([](void *data, void *ctxt) -> bool {
            dllist_ctxt_t *curr, *next;
            fn_ctx_t *fn_ctx = (fn_ctx_t *)ctxt;
            nwsec_rulelist_t *rulelist = (nwsec_rulelist_t *)data;
            if (rulelist == NULL) {
                fn_ctx->ret = HAL_RET_ERR;
                HAL_TRACE_ERR("NULL rulelist");
                return true;
            }
            HAL_TRACE_VERBOSE("rule id is {}", rulelist->rule_id);
            dllist_for_each_safe(curr, next, &rulelist->head) {
                nwsec_rule_t *rule = dllist_entry(curr, nwsec_rule_t, dlentry);
                fn_ctx->ret = rule_match_rule_add(fn_ctx->acl_ctx, &rule->fw_rule_match, rule->rule_id, rule->priority, &rule->ref_count);
                if (fn_ctx->ret != HAL_RET_OK) {
                    HAL_TRACE_ERR("rule rule add failed : {}", fn_ctx->ret);
                    return true;
                }
            }
            return false; }, &fn_ctx);
    return fn_ctx.ret;
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
                              nwsec_policy_t                 *nwsec_policy,
                              nwsec::SecurityPolicyResponse  *rsp)
{
    rule_cfg_t *rcfg = rule_cfg_get(nwsec_acl_ctx_name(nwsec_policy->key.vrf_id));
    if (rcfg != NULL) {
        return HAL_RET_POLICY_EXIST;
    }
    return HAL_RET_OK;
}

hal_ret_t
nwsec_policy_create_add_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

hal_ret_t
nwsec_policy_create_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                        ret = HAL_RET_OK;
    dllist_ctxt_t                    *lnode = NULL;
    dhl_entry_t                      *dhl_entry  = NULL;
    nwsec_policy_t                   *policy;
    hal_handle_t                     hal_handle = 0;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("invalid cfg_ctxt");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    policy = (nwsec_policy_t *) dhl_entry->obj;
    hal_handle = dhl_entry->handle;

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
    return HAL_RET_OK;
}

hal_ret_t
nwsec_policy_create_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

hal_ret_t
nwsec_policy_prepare_rsp(nwsec::SecurityPolicyResponse *rsp,
                         hal_ret_t                      ret,
                         hal_handle_t                   hal_handle)
{
    if (ret == HAL_RET_OK) {
        rsp->mutable_policy_status()->mutable_key_or_handle()->set_security_policy_handle(hal_handle);
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
            HAL_TRACE_ERR("unable to"
                          "allocate handle/memory"
                          "ret: {}", ret);
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
        ret = add_nwsec_rule_to_db(policy, nwsec_rule, nwsec_rule->rule_id);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Unable to add to rule to policy db");
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
    rule_lib_cb_t                     cb;

    HAL_TRACE_DEBUG("---------------------- API Start ---------------------");

    nwsec_spec_dump(&spec);
    trim_mem_usage("Before Create");
    // Give up the read lock
    hal_cfg_db_close();

    nwsec_policy = nwsec_policy_alloc_init();
    if (nwsec_policy == NULL) {
        HAL_TRACE_ERR(" unable to"
                      " allocate handle/memory"
                      " ret: {}", ret);
        ret = HAL_RET_OOM;
        goto end;
    }

    nwsec_policy->key.policy_id = spec.key_or_handle().security_policy_key().security_policy_id();
    if (spec.key_or_handle().security_policy_key().vrf_id_or_handle().key_or_handle_case() == kh::VrfKeyHandle::kVrfId) {
        nwsec_policy->key.vrf_id = spec.key_or_handle().security_policy_key().vrf_id_or_handle().vrf_id();
    } else {
        vrf_t *vrf = vrf_lookup_by_handle(spec.key_or_handle().security_policy_key().vrf_id_or_handle().vrf_handle());
        if (!vrf) {
            HAL_TRACE_ERR("Invalid vrf handle {}", spec.key_or_handle().security_policy_key().vrf_id_or_handle().vrf_handle());
            nwsec_policy_free(nwsec_policy);
            ret = HAL_RET_HANDLE_INVALID;
            goto end;
        }
        nwsec_policy->key.vrf_id = vrf->vrf_id;
    }
    ret = validate_nwsec_policy_create(spec, nwsec_policy,res);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("validation failed");
        nwsec_policy_free(nwsec_policy);
        goto end;
    }


    nwsec_policy->hal_handle = hal_handle_alloc(HAL_OBJ_ID_SECURITY_POLICY);
    if (nwsec_policy->hal_handle == HAL_HANDLE_INVALID) {
        HAL_TRACE_ERR("failed to alloc handle for policy_id: {}", nwsec_policy->key.policy_id);
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
    cb.rule_ctr_cb = rule_stats_cb;
    if (hal::is_platform_type_hw()) {
        app_ctxt.acl_ctx= hal::rule_lib_init(ctx_name, &nwsec_rule_config_glbl, &cb);
    } else {
        app_ctxt.acl_ctx= hal::rule_lib_init(ctx_name, &nwsec_rule_config_glbl);
    }

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

    ret = acl::acl_commit(app_ctxt.acl_ctx);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Policy commit failed with ret: {}", ret);
        goto end;
    }
    acl_deref(app_ctxt.acl_ctx);
    //Retake the read lock to make "hal_handle_add_obj" happy
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal_handle_add_obj(nwsec_policy->hal_handle, &cfg_ctxt,
                             nwsec_policy_create_add_cb,
                             nwsec_policy_create_commit_cb,
                             nwsec_policy_create_abort_cb,
                             nwsec_policy_create_cleanup_cb);
end:
    if (ret == HAL_RET_OK) {
        HAL_API_STATS_INC(HAL_API_SECURITYPOLICY_CREATE_SUCCESS);
        session_match_t match = {};
        //match.match_fields |= SESSION_MATCH_SVRF;
        match.match_fields |= SESSION_MATCH_V4_FLOW;
        match.key.svrf_id = nwsec_policy->key.vrf_id;
        session_eval_matching_session(&match);
    } else {
        HAL_API_STATS_INC(HAL_API_SECURITYPOLICY_CREATE_FAIL);
    }
    nwsec_policy_prepare_rsp(res, ret, nwsec_policy ? nwsec_policy->hal_handle : HAL_HANDLE_INVALID);
    HAL_TRACE_DEBUG("------------------------ API End -----------------------------");
    trim_mem_usage("After Create");
    return ret;
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
    return HAL_RET_OK;
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
    SecurityPolicyKeyHandle kh = spec.key_or_handle();

    HAL_TRACE_DEBUG("---------------------- API Start-------------------");
    nwsec_spec_dump(&spec);
    trim_mem_usage("Before update");

    ret = validate_nwsec_policy_update(spec, res);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("validation failed");
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
    // free up cfg db read lock
    hal_cfg_db_close();
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
    app_ctx.acl_ctx = acl::acl_get(ctx_name);
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

    ret = acl::acl_commit(app_ctx.acl_ctx_clone);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Policy commit failed with ret: {}", ret);
        goto end;
    }
    acl::acl_deref(app_ctx.acl_ctx_clone);

    if (app_ctx.acl_ctx) {
        HAL_TRACE_DEBUG("deleted acl");
        acl_deref(app_ctx.acl_ctx);
    }
    //Take up read lock again to make hal_handle_upd_obj happy
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal_handle_upd_obj(policy->hal_handle, &cfg_ctxt,
                             nwsec_policy_update_upd_cb,
                             nwsec_policy_update_commit_cb,
                             nwsec_policy_update_abort_cb,
                             nwsec_policy_update_cleanup_cb);
    nwsec_policy_rules_free(policy);
    g_hal_state->nwsec_policy_slab()->free(policy);

end:
    if (ret == HAL_RET_OK) {
        HAL_API_STATS_INC(HAL_API_SECURITYPOLICY_UPDATE_SUCCESS);
        // On successful update evaluate the sessions
        session_match_t match = {};
        //match.match_fields |= SESSION_MATCH_SVRF;
        match.match_fields |= SESSION_MATCH_V4_FLOW;
        match.key.svrf_id = policy_clone->key.vrf_id;
        session_eval_matching_session(&match);
    } else {
        HAL_API_STATS_INC(HAL_API_SECURITYPOLICY_UPDATE_FAIL);
    }

    nwsec_policy_prepare_rsp(res, ret,
                             policy_clone ? policy_clone->hal_handle : HAL_HANDLE_INVALID);
    hal_api_trace(" API End: nwsec_policy update");
    trim_mem_usage("After update");

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
    //const acl_ctx_t  *acl_ctx = NULL;
    SecurityPolicyKeyHandle kh = req.key_or_handle();
    trim_mem_usage("Before delete");

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
    hal_cfg_db_close();
    rule_lib_delete(ctx_name);

    dhl_entry.handle = policy->hal_handle;
    dhl_entry.obj = policy;
    cfg_ctxt.app_ctxt = NULL;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal_handle_del_obj(policy->hal_handle, &cfg_ctxt,
                             nwsec_policy_delete_del_cb,
                             nwsec_policy_delete_commit_cb,
                             nwsec_policy_delete_abort_cb,
                             nwsec_policy_delete_cleanup_cb);

    nwsec_policy_rules_free(policy);
    g_hal_state->nwsec_policy_slab()->free(policy);
end:
    if (ret == HAL_RET_OK) {
        HAL_API_STATS_INC(HAL_API_SECURITYPOLICY_DELETE_SUCCESS);
        // On success, delete the flows
        session_match_t match = {};
        //match.match_fields |= SESSION_MATCH_SVRF;
        match.match_fields |= SESSION_MATCH_V4_FLOW;
        match.key.svrf_id = policy->key.vrf_id;
        session_eval_matching_session(&match);
    } else {
        HAL_API_STATS_INC(HAL_API_SECURITYPOLICY_DELETE_FAIL);
    }
    res->set_api_status(hal_prepare_rsp(ret));
    trim_mem_usage("After delete");


    return ret;
}

static hal_ret_t
security_policy_rule_spec_build (nwsec_rule_t                          *rule,
                                 nwsec::SecurityRule                   *spec)
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

    spec->mutable_action()->mutable_app_data()->set_alg(rule->fw_rule_action.alg);
    if (rule->fw_rule_action.alg == nwsec::APP_SVC_FTP) {
        spec->mutable_action()->mutable_app_data()->mutable_ftp_option_info()->set_allow_mismatch_ip_address(\
                                      rule->fw_rule_action.app_options.opt.ftp_opts.allow_mismatch_ip_address);
    } else if (rule->fw_rule_action.alg == nwsec::APP_SVC_DNS) {
        spec->mutable_action()->mutable_app_data()->mutable_dns_option_info()->set_drop_multi_question_packets(\
                                      rule->fw_rule_action.app_options.opt.dns_opts.drop_multi_question_packets);
        spec->mutable_action()->mutable_app_data()->mutable_dns_option_info()->set_drop_large_domain_name_packets(\
                                      rule->fw_rule_action.app_options.opt.dns_opts.drop_large_domain_name_packets);
        spec->mutable_action()->mutable_app_data()->mutable_dns_option_info()->set_drop_long_label_packets(\
                                      rule->fw_rule_action.app_options.opt.dns_opts.drop_long_label_packets);
        spec->mutable_action()->mutable_app_data()->mutable_dns_option_info()->set_drop_multizone_packets(\
                                      rule->fw_rule_action.app_options.opt.dns_opts.drop_multizone_packets);
        spec->mutable_action()->mutable_app_data()->mutable_dns_option_info()->set_max_msg_length(\
                                      rule->fw_rule_action.app_options.opt.dns_opts.max_msg_length);
        spec->mutable_action()->mutable_app_data()->mutable_dns_option_info()->set_query_response_timeout(\
                                      rule->fw_rule_action.app_options.opt.dns_opts.query_response_timeout);
    } else if (rule->fw_rule_action.alg == nwsec::APP_SVC_SUN_RPC) {
        for (uint8_t idx = 0; idx < rule->fw_rule_action.app_options.opt.sunrpc_opts.programid_sz; idx++) {
            nwsec::AppData_RPCData* data = spec->mutable_action()->mutable_app_data()->mutable_sun_rpc_option_info()->add_data();

            data->set_program_id(std::to_string(rule->fw_rule_action.app_options.opt.sunrpc_opts.program_ids[idx].program_id));
            data->set_idle_timeout(rule->fw_rule_action.app_options.opt.sunrpc_opts.program_ids[idx].timeout);
        }
    } else if (rule->fw_rule_action.alg == nwsec::APP_SVC_MSFT_RPC) {
        for (uint8_t idx = 0; idx < rule->fw_rule_action.app_options.opt.msrpc_opts.uuid_sz; idx++) {
            nwsec::AppData_RPCData* data = spec->mutable_action()->mutable_app_data()->mutable_msrpc_option_info()->add_data();
            uint8_t *uuid = rule->fw_rule_action.app_options.opt.msrpc_opts.uuids[idx].uuid;
            std::string str;

            str = __uuid_to_str(uuid);
            data->set_program_id(str);
            data->set_idle_timeout(rule->fw_rule_action.app_options.opt.msrpc_opts.uuids[idx].timeout);
        }
    }

    return ret;
}

static hal_ret_t
security_policy_spec_build (nwsec_policy_t               *policy,
                            nwsec::SecurityPolicySpec    *spec,
                            nwsec::SecurityPolicyStats   *stats,
                            nwsec::SecurityPolicyStatus  *status)
{
    hal_ret_t               ret = HAL_RET_OK;

    spec->mutable_key_or_handle()->mutable_security_policy_key()->set_security_policy_id(policy->key.policy_id);
    spec->mutable_key_or_handle()->mutable_security_policy_key()->mutable_vrf_id_or_handle()->set_vrf_id(policy->key.vrf_id);

    const char *ctx_name = nwsec_acl_ctx_name(policy->key.vrf_id);
    rule_cfg_t *rcfg = rule_cfg_get(ctx_name);
    struct fn_ctx_t {
        nwsec::SecurityPolicySpec   *spec;
        nwsec::SecurityPolicyStats  *stats;
        nwsec::SecurityPolicyStatus *status;
        hal_ret_t                     ret;
        rule_cfg_t                   *rcfg;
    } fn_ctx = { spec, stats, status, HAL_RET_OK, rcfg };
    if (rcfg == NULL) {
        HAL_TRACE_ERR("Couldnt find rule config for: {}", ctx_name);
        HAL_API_STATS_INC(HAL_API_SECURITYPOLICY_GET_FAIL);
        return HAL_RET_ERR;
    }

    policy->rules_ht[policy->version]->walk_safe([](void *data, void *ctxt) -> bool {
        dllist_ctxt_t             *curr, *next;
        nwsec::SecurityRule       *spec_rule;
        nwsec::SecurityRuleStats  *rule_stats;
        nwsec::SecurityRuleStatus *rule_status;
        fn_ctx_t *fn_ctx = (fn_ctx_t *)ctxt;
        nwsec_rulelist_t *rulelist = (nwsec_rulelist_t *)data;
        if (rulelist == NULL) {
            fn_ctx->ret = HAL_RET_ERR;
            return true;
        }
        rule_stats = fn_ctx->stats->add_rule_stats();
        rule_stats->set_rule_id(rulelist->rule_id);

        rule_ctr_t *ctr = rule_ctr_get(fn_ctx->rcfg, rulelist->rule_id);
        if (ctr) {
            rule_stats->set_num_hits(ctr->rule_stats->total_hits);
            rule_stats->set_num_tcp_hits(ctr->rule_stats->tcp_hits);
            rule_stats->set_num_udp_hits(ctr->rule_stats->udp_hits);
            rule_stats->set_num_icmp_hits(ctr->rule_stats->icmp_hits);
        }

        dllist_for_each_safe(curr, next, &rulelist->head) {
            nwsec_rule_t *rule = dllist_entry(curr, nwsec_rule_t, dlentry);
            spec_rule = fn_ctx->spec->add_rule();
            if (spec_rule == NULL) {
                fn_ctx->ret =   HAL_RET_OOM;
                return true;
            }
            rule_status = fn_ctx->status->add_rule_status();
            if (rule_status == NULL) {
                fn_ctx->ret = HAL_RET_OOM;
                return true;
            }
            rule_status->set_rule_id(rule->rule_id);
            rule_status->set_priority(rule->priority);

            fn_ctx->ret = security_policy_rule_spec_build(rule, spec_rule);
            if (fn_ctx->ret != HAL_RET_OK) {
                return true;
            }
        }
        return false; }, &fn_ctx);

    if (fn_ctx.ret == HAL_RET_OK) {
        HAL_API_STATS_INC(HAL_API_SECURITYPOLICY_GET_SUCCESS);
    }

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

    ret = security_policy_spec_build(policy, response->mutable_spec(), response->mutable_pol_stats(), response->mutable_status());
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

    ret = security_policy_spec_build(policy, response->mutable_spec(), response->mutable_pol_stats(), response->mutable_status());
    if (ret == HAL_RET_OK) {
        response->set_api_status(types::API_STATUS_OK);
        return ret;
    }

    return HAL_RET_OK;
}

bool
securitypolicy_is_allow (vrf_id_t svrf_id, hal::ipv4_tuple *acl_key, session::FlowAction action)
{
    hal::nwsec_rule_t *nwsec_rule;
    hal_ret_t ret = HAL_RET_OK;
    const hal::ipv4_rule_t *rule = NULL;
    const acl::acl_ctx_t *acl_ctx = NULL;


    bool default_policy = NWSEC_POLICY_DEFAULT;
    const char *ctx_name = nwsec_acl_ctx_name(svrf_id);
    acl_ctx = acl::acl_get(ctx_name);
    if (acl_ctx == NULL) {
        HAL_TRACE_DEBUG("No policy on this vrf - use default deny");
        if (action != session::FlowAction::FLOW_ACTION_DROP) {
            return false;
        } else {
            //Already session is deny continue to deny
            return true;
        }
    }

    ret = acl_classify(acl_ctx, (const uint8_t *)acl_key, (const acl_rule_t **)&rule,0x01);
    if (ret == HAL_RET_OK) {
        if (rule != NULL) {
            acl::ref_t *rc;
            rc = get_rule_data((acl_rule_t *) rule);
            nwsec_rule = (hal::nwsec_rule_t *)RULE_MATCH_USER_DATA(rc, nwsec_rule_t, ref_count);
            HAL_TRACE_DEBUG("Rule id : {} rule action: {} session action: {}", nwsec_rule->rule_id, nwsec_rule->fw_rule_action.sec_action, action);
            if (nwsec_rule->fw_rule_action.sec_action != (nwsec::SecurityAction)action) {
                if (acl_ctx) {
                    acl_deref(acl_ctx);
                }
                return false;
            } else {
                if (acl_ctx) {
                    acl_deref(acl_ctx);
                 }
                return true;
            }
        }
    }
    if (acl_ctx) {
        acl_deref(acl_ctx);
    }
    return default_policy;
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
