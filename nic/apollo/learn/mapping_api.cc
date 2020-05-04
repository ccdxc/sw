//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// handle mapping APIs
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/api/internal/pds_mapping.hpp"
#include "nic/apollo/api/utils.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/learn/learn_api.hpp"
#include "nic/apollo/learn/ep_utils.hpp"
#include "nic/apollo/learn/learn_ctxt.hpp"

namespace learn {

static sdk_ret_t
detect_learn_type (learn_ctxt_t *ctxt)
{
    bool is_local;
    ep_learn_type_t learn_type = LEARN_TYPE_INVALID;

    if (ctxt->api_ctxt.mkey->type == PDS_MAPPING_TYPE_L2) {
        is_local = (ctxt->mac_entry != nullptr);
    } else {
        is_local = (ctxt->ip_entry != nullptr);
    }

    // API caller may not distinguish between create and update ops, figure it
    // out by checking if remote mapping entry already exists
    if (!is_local && ctxt->api_ctxt.op == API_OP_CREATE) {
        if (impl::remote_mapping_find(ctxt->api_ctxt.mkey) == SDK_RET_OK) {
            // remote mapping exists, this is update op
            PDS_TRACE_DEBUG("Remote mapping create is changed to update op  %s",
                            ctxt->str());
            ctxt->api_ctxt.op = API_OP_UPDATE;
        }
    }

    switch (ctxt->api_ctxt.op) {
    case API_OP_CREATE:
        if (is_local) {
            // TODO: if this is L2R for IP, do we need to check if the
            // associated MAC is already local or being processed for
            // L2R move in the same batch
            learn_type = LEARN_TYPE_MOVE_L2R;
        } else {
            learn_type = LEARN_TYPE_NEW_REMOTE;
        }
        break;
    case API_OP_UPDATE:
        // corner case, if we have locally learnt this MAC/IP before this
        // is invoked (say mac moved from tep1 -> tep2 -> this tep), we
        // handle this correctly as update API would fail, however, we can
        // check if this is locally known here itself
        if (is_local) {
            learn_type = LEARN_TYPE_NONE;
        } else {
            learn_type = LEARN_TYPE_MOVE_R2R;
        }
        break;
    case API_OP_DELETE:
        if (is_local) {
            // this can happen if we have already processed R2L move while
            // caller tries to delete the MAC/IP, we ignore the delete as learn
            // would have already removed the remote mapping entry
            learn_type = LEARN_TYPE_NONE;
        } else {
            learn_type = LEARN_TYPE_DELETE;
        }
        break;
    default:
        SDK_ASSERT(false);
    }

    if (ctxt->api_ctxt.mkey->type == PDS_MAPPING_TYPE_L2) {
        ctxt->mac_learn_type = learn_type;
        ctxt->ip_learn_type = LEARN_TYPE_INVALID;
    } else {
        ctxt->mac_learn_type = LEARN_TYPE_INVALID;
        ctxt->ip_learn_type = learn_type;
    }
    return SDK_RET_OK;
}

static sdk_ret_t
create_learn_ctxt (pds_mapping_key_t *mkey, learn_ctxt_t *ctxt, api_op_t op)
{
    sdk_ret_t ret;
    ep_mac_key_t *mac_key = &ctxt->mac_key;
    ep_ip_key_t *ip_key = &ctxt->ip_key;;
    pds_obj_key_t vnic_key;
    vnic_entry *vnic;

    ctxt->api_ctxt.mkey = mkey;
    if (mkey->type == PDS_MAPPING_TYPE_L2) {
        MAC_ADDR_COPY(&mac_key->mac_addr, &mkey->mac_addr);
        mac_key->subnet = mkey->subnet;
        ctxt->mac_entry = learn_db()->ep_mac_db()->find(mac_key);
    } else if (mkey->type == PDS_MAPPING_TYPE_L3) {
        ip_key->ip_addr = mkey->ip_addr;
        ip_key->vpc = mkey->vpc;
        ctxt->ip_entry = learn_db()->ep_ip_db()->find(ip_key);
        if (ctxt->ip_entry) {
            ctxt->mac_entry = ctxt->ip_entry->mac_entry();
        }
    } else {
        SDK_ASSERT(false);
    }
    ctxt->api_ctxt.op = op;

    // if local entry exists, save ifindex needed for notifications
    if (ctxt->mac_entry) {
        vnic_key = api::uuid_from_objid(ctxt->mac_entry->vnic_obj_id());
        vnic = vnic_db()->find(&vnic_key);
        if (vnic != nullptr) {
            ctxt->ifindex = api::objid_from_uuid(vnic->host_if());
        }
    }

    // find out learn type and move if any
    ret = detect_learn_type(ctxt);
    if (unlikely(ret != SDK_RET_OK)) {
        return ret;
    }
    return SDK_RET_OK;
}

static inline sdk_ret_t
create_learn_ctxt (pds_remote_mapping_spec_t *spec, learn_ctxt_t *ctxt,
                   api_op_t op)
{
    sdk_ret_t ret;

    ret = create_learn_ctxt(&spec->skey, ctxt, op);
    if (likely(ret == SDK_RET_OK)) {
        ctxt->api_ctxt.spec = spec;
    }
    return ret;
}

sdk_ret_t
process_mapping_api (mapping_key_spec_t key_spec, api_op_t op,
                     pds_batch_ctxt_t bctxt, learn_batch_ctxt_t *lbctxt)
{
    sdk_ret_t ret;
    learn_ctxt_t ctxt = { 0 };
    pds_remote_mapping_spec_t remote_spec;

    ctxt.ctxt_type = LEARN_CTXT_TYPE_API;
    ctxt.bctxt = bctxt;
    ctxt.lbctxt = lbctxt;

    if (op == API_OP_DELETE) {
        ret = create_learn_ctxt(key_spec.skey, &ctxt, op);
    } else {
        if (key_spec.spec->is_local) {
            // local mapping create/update is not yet supported
            SDK_ASSERT(false);
        }
        pds_mapping_spec_to_remote_spec(&remote_spec, key_spec.spec);
        ret = create_learn_ctxt(&remote_spec, &ctxt, op);
    }
    if (ret != SDK_RET_OK) {
        return ret;
    }
    PDS_TRACE_DEBUG("Learn context: %s", ctxt.str());
    ret = process_learn(&ctxt);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to process %s error code %u",
                      ctxt.log_str(ctxt.api_ctxt.mkey->type), ret);
    } else {
        PDS_TRACE_INFO("Processed %s", ctxt.log_str(ctxt.api_ctxt.mkey->type));
    }
    return ret;
}

}   // namespace learn
