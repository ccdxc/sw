//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// process packets received on learn lif
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/api/include/pds_mapping.hpp"
#include "nic/apollo/api/internal/pds_mapping.hpp"
#include "nic/apollo/api/utils.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/learn/ep_utils.hpp"
#include "nic/apollo/learn/learn_ctxt.hpp"

namespace learn {

using namespace sdk::types;

// wrapper functions to increment counters around mapping APIs
static inline sdk_ret_t
vnic_create (learn_ctxt_t *ctxt, pds_vnic_spec_t *spec)
{
    batch_counters_t *counters = &ctxt->lbctxt->counters;

    counters->vnics[OP_CREATE]++;
    return pds_vnic_create(spec, ctxt->bctxt);
}

static inline sdk_ret_t
vnic_update (learn_ctxt_t *ctxt, pds_vnic_spec_t *spec)
{
    batch_counters_t *counters = &ctxt->lbctxt->counters;

    counters->vnics[OP_UPDATE]++;
    return pds_vnic_update(spec, ctxt->bctxt);
}

static inline sdk_ret_t
local_mapping_create (learn_ctxt_t *ctxt, pds_local_mapping_spec_t *spec)
{
    batch_counters_t *counters = &ctxt->lbctxt->counters;

    if (spec->skey.type == PDS_MAPPING_TYPE_L3) {
        counters->local_ip_map[OP_CREATE]++;
    }
    return pds_local_mapping_create(spec, ctxt->bctxt);
}

static inline sdk_ret_t
local_mapping_update (learn_ctxt_t *ctxt, pds_local_mapping_spec_t *spec)
{
    batch_counters_t *counters = &ctxt->lbctxt->counters;

    if (spec->skey.type == PDS_MAPPING_TYPE_L3) {
        counters->local_ip_map[OP_UPDATE]++;
    }
    return pds_local_mapping_update(spec, ctxt->bctxt);
}

static inline sdk_ret_t
local_mapping_delete (learn_ctxt_t *ctxt, pds_mapping_key_t *key)
{
    batch_counters_t *counters = &ctxt->lbctxt->counters;

    if (key->type == PDS_MAPPING_TYPE_L3) {
        counters->local_ip_map[OP_UPDATE]++;
    }
    return api::pds_local_mapping_delete(key, ctxt->bctxt);
}

static inline sdk_ret_t
remote_mapping_create (learn_ctxt_t *ctxt, pds_remote_mapping_spec_t *spec)
{
    batch_counters_t *counters = &ctxt->lbctxt->counters;

    if (spec->skey.type == PDS_MAPPING_TYPE_L3) {
        counters->remote_ip_map[OP_CREATE]++;
    } else {
        counters->remote_mac_map[OP_CREATE]++;
    }
    return pds_remote_mapping_create(spec, ctxt->bctxt);
}

static inline sdk_ret_t
remote_mapping_update (learn_ctxt_t *ctxt, pds_remote_mapping_spec_t *spec)
{
    batch_counters_t *counters = &ctxt->lbctxt->counters;

    if (spec->skey.type == PDS_MAPPING_TYPE_L3) {
        counters->remote_ip_map[OP_UPDATE]++;
    } else {
        counters->remote_mac_map[OP_UPDATE]++;
    }
    return pds_remote_mapping_update(spec, ctxt->bctxt);
}

static inline sdk_ret_t
remote_mapping_delete (learn_ctxt_t *ctxt, pds_mapping_key_t *key)
{
    batch_counters_t *counters = &ctxt->lbctxt->counters;

    if (key->type == PDS_MAPPING_TYPE_L3) {
        counters->remote_ip_map[OP_DELETE]++;
    } else {
        counters->remote_mac_map[OP_DELETE]++;
    }
    return api::pds_remote_mapping_delete(key, ctxt->bctxt);
}

static sdk_ret_t
process_new_local_mac (learn_ctxt_t *ctxt)
{
    sdk_ret_t ret;
    pds_vnic_spec_t spec = { 0 };
    uint32_t vnic_obj_id;
    impl::learn_info_t *impl = &ctxt->pkt_ctxt.impl_info;

    // allocate MAC entry, VNIC object id and create new vnic
    ret = learn_db()->vnic_obj_id_alloc(&vnic_obj_id);
    if (unlikely(ret != SDK_RET_OK)) {
        PDS_TRACE_ERR("Failed to allocate VNIC object id, error code %u", ret);
        return ret;
    }
    ctxt->mac_entry = ep_mac_entry::factory(&ctxt->mac_key, vnic_obj_id);
    if (unlikely(ctxt->mac_entry == nullptr)) {
        learn_db()->vnic_obj_id_free(vnic_obj_id);
        PDS_TRACE_ERR("Failed to allocate MAC entry");
        return SDK_RET_ERR;
    }

    // TODO: encode learn module specific identifier in uuid
    spec.key = api::uuid_from_objid(vnic_obj_id);
    spec.subnet = ctxt->mac_key.subnet;
    spec.fabric_encap.type = PDS_ENCAP_TYPE_NONE;
    spec.vnic_encap = impl->encap;
    MAC_ADDR_COPY(spec.mac_addr, ctxt->mac_key.mac_addr);
    spec.host_if = api::uuid_from_objid(ctxt->ifindex);

    PDS_TRACE_DEBUG("Creating VNIC %s for EP %s", spec.key.str(), ctxt->str());
    return vnic_create(ctxt, &spec);
}

static sdk_ret_t
process_l2l_move_mac (learn_ctxt_t *ctxt)
{
    pds_vnic_spec_t spec = { 0 };
    impl::learn_info_t *impl = &ctxt->pkt_ctxt.impl_info;

    // update vnic with changed attributes
    spec.key = api::uuid_from_objid(ctxt->mac_entry->vnic_obj_id());
    spec.subnet = ctxt->mac_key.subnet;
    spec.fabric_encap.type = PDS_ENCAP_TYPE_NONE;
    spec.vnic_encap = impl->encap;
    MAC_ADDR_COPY(spec.mac_addr, ctxt->mac_key.mac_addr);
    spec.host_if = api::uuid_from_objid(LIF_IFINDEX(impl->lif));

    PDS_TRACE_DEBUG("Updating VNIC %s for EP %s", spec.key.str(), ctxt->str());
    return vnic_update(ctxt, &spec);
}

static sdk_ret_t
process_r2l_move_mac (learn_ctxt_t *ctxt)
{
    sdk_ret_t ret;
    pds_mapping_key_t mkey;

    ep_mac_to_pds_mapping_key(&ctxt->mac_key, &mkey);
    ret = remote_mapping_delete(ctxt, &mkey);
    if (ret != SDK_RET_OK) {
        return ret;
    }
    // create vnic just like we do for a new mac
    // any remote IP mappings linked to this mac wil be deleted explicitly
    return process_new_local_mac(ctxt);
}

static sdk_ret_t
process_new_local_ip (learn_ctxt_t *ctxt)
{
    pds_local_mapping_spec_t spec{};
    uint32_t vnic_obj_id;

    vnic_obj_id = ctxt->mac_entry->vnic_obj_id();
    ctxt->ip_entry = ep_ip_entry::factory(&ctxt->ip_key, vnic_obj_id);
    if (unlikely(ctxt->ip_entry == nullptr)) {
        PDS_TRACE_ERR("Failed to allocate IP entry for EP %s", ctxt->str());
        return SDK_RET_ERR;
    }

    // create local l3 mapping
    spec.skey.type = PDS_MAPPING_TYPE_L3;
    spec.skey.vpc = ctxt->ip_key.vpc;
    spec.skey.ip_addr = ctxt->ip_key.ip_addr;
    spec.vnic = api::uuid_from_objid(vnic_obj_id);
    spec.subnet = ctxt->mac_key.subnet;
    spec.num_tags = 0;
    MAC_ADDR_COPY(spec.vnic_mac, ctxt->mac_key.mac_addr);

    PDS_TRACE_DEBUG("Creating IP mapping for EP %s", ctxt->str());
    return local_mapping_create(ctxt, &spec);
}

static sdk_ret_t
process_l2l_move_ip (learn_ctxt_t *ctxt)
{
    sdk_ret_t ret;
    pds_mapping_key_t mkey;

    // update local mapping, mapping apis do not support update yet, so do a
    // delete + create instead, since P4 looks up mapping table using key and
    // not index, there should be no disruption to existing flows involving this
    // IP
    ep_ip_to_pds_mapping_key(&ctxt->ip_key, &mkey);
    ret = local_mapping_delete(ctxt, &mkey);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to delete local mapping for EP %s, error code %u",
                      ctxt->str(), ret);
        return ret;
    }

    ret = delete_ip_entry(ctxt->ip_entry, ctxt->ip_entry->mac_entry());
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to delete IP entry for EP %s, error code %u",
                      ctxt->str(), ret);
        return ret;
    }
    ctxt->ip_entry = nullptr;

    // create local IP mapping just like we do for a new IP
    // save current ip_entry, so that we can delete it after the move
    ctxt->pkt_ctxt.old_ip_entry = ctxt->ip_entry;
    ctxt->ip_entry = nullptr;
    return process_new_local_ip(ctxt);
}

static sdk_ret_t
process_r2l_move_ip (learn_ctxt_t *ctxt)
{
    sdk_ret_t ret;
    pds_mapping_key_t mkey;

    ep_ip_to_pds_mapping_key(&ctxt->ip_key, &mkey);
    ret = remote_mapping_delete(ctxt, &mkey);
    if (ret != SDK_RET_OK) {
        return ret;
    }
    // create local IP mapping just like we do for a new IP
    return process_new_local_ip(ctxt);
}

static inline sdk_ret_t
process_new_remote_mac_ip (learn_ctxt_t *ctxt)
{
    return remote_mapping_create(ctxt, ctxt->api_ctxt.spec);
}

// structure to pass batch context to walk callback and get return status
typedef struct cb_args_s {
    sdk_ret_t     ret;
    learn_ctxt_t  *ctxt;
} cb_args_t;

static bool l2r_mac_move_cb (void *entry, void *cb_args)
{
    //send ARP probe and delete mapping
    ep_ip_entry *ip_entry = (ep_ip_entry *)entry;
    cb_args_t *args = (cb_args_t *)cb_args;

    send_arp_probe(ip_entry);
    args->ret = delete_local_ip_mapping(ip_entry, args->ctxt->bctxt);
    if (args->ret != SDK_RET_OK) {
        //stop iterating
        return true;
    }

    // these are local deletes, send ageout event
    // IP address information is not in the context, send it explicitly
    add_ip_to_event_list(ip_entry->key(), args->ctxt, EVENT_ID_IP_AGE);
    return false;
}

static bool del_ip_entry_cb (void *entry, void *cookie)
{
    learn_ctxt_t *ctxt = (learn_ctxt_t *)cookie;
    ep_ip_entry *ip_entry = (ep_ip_entry *)entry;

    add_to_delete_list(ip_entry, ctxt->mac_entry, &ctxt->lbctxt->del_objs);
    return false;
}

static sdk_ret_t
process_l2r_move_mac (learn_ctxt_t *ctxt)
{
    sdk_ret_t ret;
    cb_args_t cb_args;
    ep_mac_entry *mac_entry = ctxt->mac_entry;

    // send ARP probes and delete all IP mappings associated with this MAC
    // need to do this before deleting the VNIC
    cb_args.ctxt = ctxt;
    mac_entry->walk_ip_list(l2r_mac_move_cb, &cb_args);
    ret = cb_args.ret;
    if (unlikely(ret != SDK_RET_OK)) {
        PDS_TRACE_ERR("Failed to process IP mappings associated with %s EP %s "
                      "error code %u", ctxt->str(),
                      mac_entry->key2str().c_str(), ret);
        return ret;
    }

    // delete vnic
    ret = delete_vnic(mac_entry, ctxt->bctxt);
    if (unlikely(ret != SDK_RET_OK)) {
        PDS_TRACE_ERR("Failed to add VNIC delete API to batch for %s EP %s "
                      "error code %u", ctxt->str(),
                      mac_entry->key2str().c_str(), ret);
        return ret;
    }

    // create remote mapping
    ret = remote_mapping_create(ctxt, ctxt->api_ctxt.spec);
    if (unlikely(ret != SDK_RET_OK)) {
        return ret;
    }

    // add IP entries to delete list
    mac_entry->walk_ip_list(del_ip_entry_cb, ctxt);

    // add MAC entry to delete list
    add_to_delete_list(mac_entry, &ctxt->lbctxt->del_objs);

    // add to broadcast event list
    add_mac_to_event_list(ctxt, EVENT_ID_MAC_MOVE_L2R);

    return SDK_RET_OK;
}

static inline sdk_ret_t
process_l2r_move_ip (learn_ctxt_t *ctxt)
{
    sdk_ret_t ret;

    // delete local mapping, add remote mapping
    ret = local_mapping_delete(ctxt, ctxt->api_ctxt.mkey);
    if (ret != SDK_RET_OK) {
        return ret;
    }
    ret = remote_mapping_create(ctxt, ctxt->api_ctxt.spec);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    // add ip entry to delete list
    add_to_delete_list(ctxt->ip_entry, ctxt->mac_entry,
                       &ctxt->lbctxt->del_objs);

    // add to broadcast event list
    add_ip_to_event_list(ctxt, EVENT_ID_IP_MOVE_L2R);

    return SDK_RET_OK;
}

static inline sdk_ret_t
process_r2r_move_mac_ip (learn_ctxt_t *ctxt)
{
    sdk_ret_t ret;

    // TODO: return remote_mapping_update(ctxt, ctxt->api_ctxt.spec);
    // mapping table does not support update yet

    ret = remote_mapping_delete(ctxt, ctxt->api_ctxt.mkey);
    if (ret != SDK_RET_OK) {
        return ret;
    }
    return remote_mapping_create(ctxt, ctxt->api_ctxt.spec);
}

static inline sdk_ret_t
process_delete_mac (learn_ctxt_t *ctxt)
{
    sdk_ret_t ret;

    ret = remote_mapping_delete(ctxt, ctxt->api_ctxt.mkey);
    if (ret == SDK_RET_OK) {
        add_del_event_to_list(ctxt);
    }
    return ret;
}

// caller ensures key type is L3
static vnic_entry *
ip_mapping_key_to_vnic (pds_mapping_key_t *key)
{
    sdk_ret_t ret;
    ep_mac_key_t mac_key;
    ep_mac_entry *mac_entry;
    pds_remote_mapping_info_t info;

    ret = api::pds_remote_mapping_read(key, &info);
    if (ret == SDK_RET_ENTRY_NOT_FOUND) {
        return nullptr;
    }
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to read remote mapping for key (%s, %s) with err "
                      "code %u", key->vpc.str(), ipaddr2str(&key->ip_addr),
                      ret);
        return nullptr;
    }
    MAC_ADDR_COPY(&mac_key.mac_addr, &info.spec.vnic_mac);
    mac_key.subnet = info.spec.subnet;
    mac_entry = learn_db()->ep_mac_db()->find(&mac_key);
    if (mac_entry) {
        pds_obj_key_t vnic_key = api::uuid_from_objid(mac_entry->vnic_obj_id());
        return vnic_db()->find(&vnic_key);
    }
    return nullptr;
}

static sdk_ret_t
process_delete_ip (learn_ctxt_t *ctxt)
{
    sdk_ret_t ret;
    pds_mapping_key_t *mkey = ctxt->api_ctxt.mkey;

    // if the MAC associated with this IP is local, API caller may be deleting
    // remote IP mappings after the MAC did a R2L move, send ARP request before
    // deleting to proactively check if IP has also moved along with the MAC

    // TODO: check if API caller can send MAC and subnet for deletes so that we
    // can look up learn database to check if MAC associated with this IP
    // has moved R2L
    vnic_entry *vnic = ip_mapping_key_to_vnic(mkey);
    if (vnic) {
        send_arp_probe(vnic, mkey->ip_addr.addr.v4_addr);
    }
    ret = remote_mapping_delete(ctxt, mkey);
    if (ret == SDK_RET_OK) {
        add_del_event_to_list(ctxt);
    }
    return ret;
}

static inline void
update_counters (learn_ctxt_t *ctxt, ep_learn_type_t learn_type,
                 pds_mapping_type_t mtype)
{
    int idx = LEARN_TYPE_CTR_IDX(learn_type);
    batch_counters_t *counters = &ctxt->lbctxt->counters;

    if (mtype == PDS_MAPPING_TYPE_L2) {
        counters->mac_learns[idx]++;
    } else {
        counters->ip_learns[idx]++;
    }
}

#define DISPATCH_MAC_OR_IP(func, ctxt, mtype, ret)      \
do {                                                    \
    if (mtype == PDS_MAPPING_TYPE_L2) {                 \
        ret = func##_mac(ctxt);                         \
    } else {                                            \
        ret = func##_ip(ctxt);                          \
    }                                                   \
} while(0);

static sdk_ret_t
process_learn_dispatch (learn_ctxt_t *ctxt, ep_learn_type_t learn_type,
                        pds_mapping_type_t mtype)
{
    sdk_ret_t ret = SDK_RET_ERR;

    switch (learn_type) {
    case LEARN_TYPE_NEW_LOCAL:
        DISPATCH_MAC_OR_IP(process_new_local, ctxt, mtype, ret);
        break;
    case LEARN_TYPE_MOVE_L2L:
        DISPATCH_MAC_OR_IP(process_l2l_move, ctxt, mtype, ret);
        break;
    case LEARN_TYPE_MOVE_R2L:
        DISPATCH_MAC_OR_IP(process_r2l_move, ctxt, mtype, ret);
        break;
    case LEARN_TYPE_NEW_REMOTE:
        ret = process_new_remote_mac_ip(ctxt);
        break;
    case LEARN_TYPE_MOVE_L2R:
        DISPATCH_MAC_OR_IP(process_l2r_move, ctxt, mtype, ret);
        break;
    case LEARN_TYPE_MOVE_R2R:
        ret = process_r2r_move_mac_ip(ctxt);
        break;
    case LEARN_TYPE_DELETE:
        DISPATCH_MAC_OR_IP(process_delete, ctxt, mtype, ret);
        break;
    case LEARN_TYPE_NONE:
        ret = SDK_RET_OK;
    default:
        break;
    }
    update_counters(ctxt, learn_type, mtype);
    return ret;
}

sdk_ret_t
process_learn (learn_ctxt_t *ctxt)
{
    sdk_ret_t ret = SDK_RET_OK;
    bool learn_mac = false;
    bool learn_ip = false;

    if (ctxt->ctxt_type == LEARN_CTXT_TYPE_PKT) {
        // if we are learning from packet, we may have both MAC and IP
        // to be learnt
        learn_mac = true;
        if (ctxt->ip_learn_type != LEARN_TYPE_INVALID) {
            learn_ip = true;
        }
    } else if (ctxt->ctxt_type == LEARN_CTXT_TYPE_API) {
        // if we are processing mapping API, key is either MAC or IP
        if (ctxt->api_ctxt.mkey->type == PDS_MAPPING_TYPE_L2) {
            learn_mac = true;
        } else {
            learn_ip = true;
        }
    } else {
        // should never reach here
        SDK_ASSERT(false);
    }

    if (learn_mac) {
        PDS_TRACE_DEBUG("Processing %s", ctxt->log_str(PDS_MAPPING_TYPE_L2));
        ret = process_learn_dispatch(ctxt, ctxt->mac_learn_type,
                                     PDS_MAPPING_TYPE_L2);
    }
    if (ret == SDK_RET_OK && learn_ip) {
        PDS_TRACE_DEBUG("Processing %s", ctxt->log_str(PDS_MAPPING_TYPE_L3));
        ret = process_learn_dispatch(ctxt, ctxt->ip_learn_type,
                                          PDS_MAPPING_TYPE_L3);
    }
    return ret;
}

}    // namespace learn
