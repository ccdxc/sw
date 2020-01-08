//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#include "pdsa_vpp_cfg.hpp"
#include <vector>
#include <iterator>

/////// static data
vpp_config_data vpp_config_data::singleton;
vpp_config_batch vpp_config_batch::singleton;
std::list<commit_cbs_t> vpp_config_batch::commit_cbs;

/////// vpp_config_data member functions

// returns the currently configured number of instances of a specific obj id
int
vpp_config_data::size (obj_id_t obj_id) const {
    switch(obj_id) {
    case OBJ_ID_VNIC: return vnic.size();
    case OBJ_ID_SUBNET: return subnet.size();
    case OBJ_ID_DHCP_RELAY: return dhcp_relay.size();
    case OBJ_ID_DHCP_POLICY: return dhcp_policy.size();
    case OBJ_ID_NAT_PORT_BLOCK: return nat_port_block.size();
    case OBJ_ID_SECURITY_PROFILE: return security_profile.size();
    default:
        assert(false);
    }
    return 0;
}

// checks for existence of a key in the configuration. returns true if the key
// exists
bool
vpp_config_data::exists (pds_cfg_msg_t const& cfg_msg) const {

    // since key and spec are a union, and the first element in the spec is
    // the key, we don't verify whether a key or spec is passed here
    switch(cfg_msg.obj_id) {
    case OBJ_ID_VNIC:
        if (vnic.find(cfg_msg.vnic.key.id) != vnic.end()) {
            return true;
        }
        break;

    case OBJ_ID_SUBNET:
        if (subnet.find(cfg_msg.subnet.key.id) != subnet.end()) {
            return true;
        }
        break;

    case OBJ_ID_DHCP_RELAY:
        if (dhcp_relay.find(cfg_msg.dhcp_relay.key.id) != dhcp_relay.end()) {
            return true;
        }
        break;

    case OBJ_ID_DHCP_POLICY:
        if (dhcp_policy.find(cfg_msg.dhcp_policy.key.id) != dhcp_policy.end()) {
            return true;
        }
        break;

    case OBJ_ID_NAT_PORT_BLOCK:
        if (nat_port_block.find(cfg_msg.nat_port_block.key.id) !=
            nat_port_block.end()) {
            return true;
        }
        break;

    case OBJ_ID_SECURITY_PROFILE:
        if (security_profile.find(cfg_msg.security_profile.key.id) !=
            security_profile.end()) {
            return true;
        }
        break;

    default:
        assert(false);
    }
    return false;
}

// retrieves the configured spec associated with the specified key. returns
// true if key found
bool
vpp_config_data::get (pds_cfg_msg_t &cfg_msg) const {
    // initialize iterators to all obj types
    auto vnic_it = vnic.end();
    auto subnet_it = subnet.end();
    auto dhcp_relay_it = dhcp_relay.end();
    auto dhcp_policy_it = dhcp_policy.end();
    auto nat_it = nat_port_block.end();
    auto sec_it = security_profile.end();

    switch(cfg_msg.obj_id) {
    case OBJ_ID_VNIC:
        vnic_it = vnic.find(cfg_msg.vnic.key.id);
        if (vnic_it == vnic.end()) {
            cfg_msg.obj_id = OBJ_ID_NONE;
            return false;
        }
        cfg_msg.vnic.spec = vnic_it->second;
        break;

    case OBJ_ID_SUBNET:
        subnet_it = subnet.find(cfg_msg.subnet.key.id);
        if (subnet_it == subnet.end()) {
            cfg_msg.obj_id = OBJ_ID_NONE;
            return false;
        }
        cfg_msg.subnet.spec = subnet_it->second;
        break;

    case OBJ_ID_DHCP_RELAY:
        dhcp_relay_it = dhcp_relay.find(cfg_msg.dhcp_relay.key.id);
        if (dhcp_relay_it == dhcp_relay.end()) {
            cfg_msg.obj_id = OBJ_ID_NONE;
            return false;
        }
        cfg_msg.dhcp_relay.spec = dhcp_relay_it->second;
        break;

    case OBJ_ID_DHCP_POLICY:
        dhcp_policy_it = dhcp_policy.find(cfg_msg.dhcp_policy.key.id);
        if (dhcp_policy_it == dhcp_policy.end()) {
            cfg_msg.obj_id = OBJ_ID_NONE;
            return false;
        }
        cfg_msg.dhcp_policy.spec = dhcp_policy_it->second;
        break;

    case OBJ_ID_NAT_PORT_BLOCK:
        nat_it = nat_port_block.find(cfg_msg.nat_port_block.key.id);
        if (nat_it == nat_port_block.end()) {
            cfg_msg.obj_id = OBJ_ID_NONE;
            return false;
        }
        cfg_msg.nat_port_block.spec = nat_it->second;
        break;

    case OBJ_ID_SECURITY_PROFILE:
        sec_it = security_profile.find(cfg_msg.security_profile.key.id);
        if (sec_it == security_profile.end()) {
            cfg_msg.obj_id = OBJ_ID_NONE;
            return false;
        }
        cfg_msg.security_profile.spec = sec_it->second;
        break;

    default:
        assert(false);
        return false;
    }

    return true;
}

// inserts/updates the spec associated with the specificed key
void
vpp_config_data::set (pds_cfg_msg_t const& cfg_msg) {
    switch(cfg_msg.obj_id) {
    case OBJ_ID_VNIC:
        vnic[cfg_msg.vnic.spec.key.id] = cfg_msg.vnic.spec;
        break;

    case OBJ_ID_SUBNET:
        subnet[cfg_msg.subnet.spec.key.id] = cfg_msg.subnet.spec;
        break;

    case OBJ_ID_DHCP_RELAY:
        dhcp_relay[cfg_msg.dhcp_relay.spec.key.id] = cfg_msg.dhcp_relay.spec;
        break;

    case OBJ_ID_DHCP_POLICY:
        dhcp_policy[cfg_msg.dhcp_policy.spec.key.id] = cfg_msg.dhcp_policy.spec;
        break;

    case OBJ_ID_NAT_PORT_BLOCK:
        nat_port_block[cfg_msg.nat_port_block.spec.key.id] =
            cfg_msg.nat_port_block.spec;
        break;

    case OBJ_ID_SECURITY_PROFILE:
        security_profile[cfg_msg.security_profile.spec.key.id] =
            cfg_msg.security_profile.spec;
        break;

    default:
        assert(false);
    }
}

// removes the spec associated with the specificed key
void
vpp_config_data::unset (obj_id_t obj_id, pds_cfg_msg_t const& cfg_msg) {
    switch(obj_id) {
    case OBJ_ID_VNIC:
        vnic.erase(cfg_msg.vnic.key.id);
        break;

    case OBJ_ID_SUBNET:
        subnet.erase(cfg_msg.subnet.key.id);
        break;

    case OBJ_ID_DHCP_RELAY:
        dhcp_relay.erase(cfg_msg.dhcp_relay.key.id);
        break;

    case OBJ_ID_DHCP_POLICY:
        dhcp_policy.erase(cfg_msg.dhcp_policy.key.id);
        break;

    case OBJ_ID_NAT_PORT_BLOCK:
        nat_port_block.erase(cfg_msg.nat_port_block.key.id);
        break;

    case OBJ_ID_SECURITY_PROFILE:
        security_profile.erase(cfg_msg.security_profile.key.id);
        break;

    default:
        assert(false);
    }
}

// compares the provided keys for the two msgs
static bool
cfg_msg_key_equal (pds_cfg_msg_t const& left, pds_cfg_msg_t const& right) {
    assert(left.obj_id == right.obj_id);

    switch(left.obj_id) {
    case OBJ_ID_VNIC:
        if (right.op != API_OP_DELETE) {
            return (left.vnic.spec.key.id == right.vnic.spec.key.id);
        } else {
            return (left.vnic.spec.key.id == right.vnic.key.id);
        }
        break;

    case OBJ_ID_SUBNET:
        if (right.op != API_OP_DELETE) {
            return (left.subnet.spec.key.id == right.subnet.spec.key.id);
        } else {
            return (left.subnet.spec.key.id == right.subnet.key.id);
        }
        break;

    case OBJ_ID_DHCP_RELAY:
        if (right.op != API_OP_DELETE) {
            return (left.dhcp_relay.spec.key.id ==
                    right.dhcp_relay.spec.key.id);
        } else {
            return (left.dhcp_relay.spec.key.id == right.dhcp_relay.key.id);
        }
        break;

    case OBJ_ID_DHCP_POLICY:
        if (right.op != API_OP_DELETE) {
            return (left.dhcp_policy.spec.key.id ==
                    right.dhcp_policy.spec.key.id);
        } else {
            return (left.dhcp_policy.spec.key.id == right.dhcp_policy.key.id);
        }
        break;

    case OBJ_ID_NAT_PORT_BLOCK:
        if (right.op != API_OP_DELETE) {
            return (left.nat_port_block.spec.key.id ==
                    right.nat_port_block.spec.key.id);
        } else {
            return (left.nat_port_block.spec.key.id ==
                    right.nat_port_block.key.id);
        }
        break;

    case OBJ_ID_SECURITY_PROFILE:
        if (right.op != API_OP_DELETE) {
            return (left.security_profile.spec.key.id ==
                    right.security_profile.spec.key.id);
        } else {
            return (left.security_profile.spec.key.id ==
                    right.security_profile.key.id);
        }
        break;

    default:
        assert(false);
    }

    return false;
}

//////// config pool functions to operate on a batch

// returns the number of instances for a specific object type
size_t
vpp_config_batch::maxsize (obj_id_t obj_id) {
    switch(obj_id) {
    case OBJ_ID_VNIC:
        return PDS_MAX_VNIC;

    case OBJ_ID_SUBNET:
        return PDS_MAX_SUBNET;

    case OBJ_ID_DHCP_RELAY:
        return PDS_MAX_DHCP_RELAY;

    case OBJ_ID_DHCP_POLICY:
        return PDS_MAX_DHCP_POLICY;

    case OBJ_ID_NAT_PORT_BLOCK:
        return PDS_MAX_NAT_PORT_BLOCK;

    case OBJ_ID_SECURITY_PROFILE:
        return 1;

    default:
        assert(false);
    }
    return 0;
}

// initialize the current pool sizes, to prepare for a new batch
void
vpp_config_batch::init (void) {
    vpp_config_data &vpp_config = vpp_config_data::get();

    pool_sz[OBJ_ID_VNIC] = vpp_config.size(OBJ_ID_VNIC);
    pool_sz[OBJ_ID_SUBNET] = vpp_config.size(OBJ_ID_SUBNET);
    pool_sz[OBJ_ID_DHCP_RELAY] = vpp_config.size(OBJ_ID_DHCP_RELAY);
    pool_sz[OBJ_ID_DHCP_POLICY] = vpp_config.size(OBJ_ID_DHCP_POLICY);
    pool_sz[OBJ_ID_NAT_PORT_BLOCK] = vpp_config.size(OBJ_ID_NAT_PORT_BLOCK);
    pool_sz[OBJ_ID_SECURITY_PROFILE] = vpp_config.size(OBJ_ID_SECURITY_PROFILE);
}

// checks for the existence of the specified key either in batch or config db
bool
vpp_config_batch::exists (pds_cfg_msg_t const& msg) const {

    // first, search the operations list to enable dedup
    for (auto it = batch_op.begin(); it != batch_op.end(); it++) {
        if ((*it).obj_id != msg.obj_id) {
            continue;
        }
        if (cfg_msg_key_equal((*it).modified, msg)) {
            if ((*it).deleted) {
                return false;
            } else {
                return true;
            }
        }
    }

    // next check the config
    vpp_config_data &vpp_config = vpp_config_data::get();
    return vpp_config.exists(msg);
}

// prepare to insert a new spec. check if a new object can be accomodated
bool
vpp_config_batch::reserve (pds_cfg_msg_t const& msg) {
    // check size, we shouldn't exceed max
    if ((pool_sz[msg.obj_id] + 1) > maxsize(msg.obj_id)) {
        return false;
    }

    pool_sz[msg.obj_id] += 1;

    return true;
}

// add a new set operation to batch (can be a create or update)
void
vpp_config_batch::set (pds_cfg_msg_t const& msg) {
    batch_op_t current_op;

    // deduplicate, if necessary
    for (auto it = batch_op.begin(); it != batch_op.end(); it++) {
        if ((*it).obj_id != msg.obj_id) {
            continue;
        }
        if (cfg_msg_key_equal((*it).modified, msg)) {
            (*it).deleted = false;
            (*it).modified = msg;
            return;
        }
    }

    // create new batch object
    memset(&current_op, 0, sizeof(batch_op_t));
    current_op.obj_id = msg.obj_id;
    current_op.deleted = false;
    current_op.modified = msg;
    current_op.original = msg; // only key & obj_id are relevant

    // cache the original data
    vpp_config_data &vpp_config = vpp_config_data::get();
    vpp_config.get(current_op.original);

    batch_op.push_back(current_op);
}

// add a new del operation to batch
void
vpp_config_batch::del (obj_id_t obj_id, pds_cfg_msg_t const& msg) {
    batch_op_t current_op;

    pool_sz[msg.obj_id] -= 1;

    // deduplicate, if necessary
    for (auto it = batch_op.begin(); it != batch_op.end(); it++) {
        if ((*it).obj_id != obj_id) {
            continue;
        }
        if (cfg_msg_key_equal((*it).modified, msg)) {
            (*it).deleted = true;
            (*it).modified = msg;
            return;
        }
    }

    // create new batch object
    memset(&current_op, 0, sizeof(batch_op_t));
    current_op.obj_id = obj_id;
    current_op.deleted = true;
    current_op.modified = msg;
    current_op.original = msg; // only key & obj_id are relevant

    // cache the original data
    vpp_config_data &vpp_config = vpp_config_data::get();
    vpp_config.get(current_op.original);

    batch_op.push_back(current_op);
}

void
vpp_config_batch::clear (void) {
    batch_op.clear();
    pool_sz.clear();
}

void
vpp_config_batch::register_cbs (obj_id_t id,
                               pds_cfg_set_cb set_cb_fn,
                               pds_cfg_del_cb del_cb_fn,
                               pds_cfg_act_cb act_cb_fn) {
    commit_cbs_t cbs;

    cbs.obj_id = id;
    cbs.set_cb = set_cb_fn;
    cbs.del_cb = del_cb_fn;
    cbs.act_cb = act_cb_fn;
    vpp_config_batch::commit_cbs.push_back(cbs);
}

void
vpp_config_batch::publish (void) {
    vpp_config_data &vpp_config = vpp_config_data::get();
    // commit modifications to config data
    for(auto it = batch_op.begin(); it != batch_op.end(); it++) {
        if ((*it).deleted) {
            vpp_config.unset((*it).obj_id, (*it).modified);
        } else {
            vpp_config.set((*it).modified);
        }
    }
}

void
vpp_config_batch::unpublish (void) {
    vpp_config_data &vpp_config = vpp_config_data::get();
    // restore original to config data
    for(auto it = batch_op.begin(); it != batch_op.end(); it++) {
        if ((*it).original.obj_id == OBJ_ID_NONE) {
            vpp_config.unset((*it).obj_id, (*it).modified);
        } else {
            vpp_config.set((*it).original);
        }
    }
}

// add a new operation to the batch list
sdk::sdk_ret_t
vpp_config_batch::add (pds_cfg_msg_t const& cfg_msg) {
    bool ret;

    switch(cfg_msg.op) {
    case API_OP_CREATE:
        ret = exists(cfg_msg);
        if (ret) {
            return sdk::SDK_RET_ENTRY_EXISTS;
        }

        ret = reserve(cfg_msg);
        if (!ret) {
            return sdk::SDK_RET_NO_RESOURCE;
        }

        set(cfg_msg);
        break;

    case API_OP_UPDATE:
        ret = exists(cfg_msg);
        if (!ret) {
            return sdk::SDK_RET_ENTRY_NOT_FOUND;
        }

        set(cfg_msg);
        break;

    case API_OP_DELETE:
        ret = exists(cfg_msg);
        if (!ret) {
            return sdk::SDK_RET_ENTRY_NOT_FOUND;
        }

        del(cfg_msg.obj_id, cfg_msg);
        break;

    default:
        break;

    }
    return sdk::SDK_RET_OK;
}

// stage one of batch processing. reserve resources for all messages in batch.
sdk::sdk_ret_t
vpp_config_batch::create (const pds_msg_list_t *msglist) {
    uint32_t rsv_count;
    sdk::sdk_ret_t ret = sdk::SDK_RET_OK;

    for (rsv_count = 0; rsv_count < msglist->num_msgs; rsv_count++) {
        const pds_msg_t *msg = &msglist->msgs[rsv_count];

        if (msg->id > PDS_MSG_ID_MAX) {
            ret = sdk::SDK_RET_INVALID_ARG;
            break;
        }

        ret = add(msg->cfg_msg);
        if(ret != sdk::SDK_RET_OK) {
            break;
        }
    }

    return ret;
}

// stage two of batch processing. commit, and rollback on errors
sdk::sdk_ret_t
vpp_config_batch::commit (void) {
    auto it = batch_op.begin();
    sdk::sdk_ret_t ret = sdk::SDK_RET_OK;

    publish();

    // commit changes to plugin
    for (; it != batch_op.end(); it++) {
        for (auto cit = commit_cbs.begin(); cit != commit_cbs.end(); cit++) {
            if ((*cit).obj_id != (*it).obj_id) {
                continue;
            }
            if ((*it).deleted) {
                // pass the original data in to be deleted
                // if the original existed. no-op otherwise
                if ((*it).original.obj_id != OBJ_ID_NONE) {
                    ret = ((*cit).del_cb)(&((*it).original));
                }
            } else {
                ret = ((*cit).set_cb)(&((*it).modified));
            }
            if (ret != sdk::SDK_RET_OK) {
                break;
            }
        }
        if (ret != sdk::SDK_RET_OK) {
            break;
        }
    }

    if (it == batch_op.end()) {
        assert(ret == sdk::SDK_RET_OK);
        // all good, activate the configuration
        for(it = batch_op.begin(); it != batch_op.end(); it++) {
            for (auto cit = commit_cbs.begin();
                 cit != commit_cbs.end(); cit++) {
                if ((*cit).obj_id != (*it).obj_id) {
                    continue;
                }
                if ((*cit).act_cb == NULL) {
                    continue;
                }
                // pass the original data in to be deleted
                // if the original existed. no-op otherwise
                if ((*it).deleted) {
                    if ((*it).original.obj_id != OBJ_ID_NONE) {
                        ((*cit).act_cb)(&((*it).original));
                    }
                } else {
                    ((*cit).act_cb)(&((*it).modified));
                }
            }
        }
        return ret;
    }

    // go backwards from the fail point, and restore original

    do {
        it--;
        for (auto cit = commit_cbs.begin(); cit != commit_cbs.end(); cit++) {
            if ((*cit).obj_id != (*it).obj_id) {
                continue;
            }
            // not checking return code during rollback
            if ((*it).original.obj_id == OBJ_ID_NONE) {
                // key didn't exist originally, delete it and activate
                ((*cit).del_cb)(&((*it).modified));
                if ((*cit).act_cb) {
                    ((*cit).act_cb)(&((*it).modified));
                }
            } else {
                ((*cit).set_cb)(&((*it).original));
                if ((*cit).act_cb) {
                    ((*cit).act_cb)(&((*it).original));
                }
            }
        }
    } while(it != batch_op.begin());

    unpublish();

    return ret;
}
