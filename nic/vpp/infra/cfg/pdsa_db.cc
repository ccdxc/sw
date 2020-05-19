//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//

#include <vector>
#include <iterator>
#include "pdsa_db.hpp"

// static data
vpp_config_data vpp_config_data::singleton;
vpp_config_batch vpp_config_batch::singleton;
vpp_config_cb_registry vpp_config_cb_registry::singleton;

// vpp_config_data member functions

// returns the currently configured number of instances of a specific obj id
int
vpp_config_data::size (obj_id_t obj_id) const {
    switch(obj_id) {
#define _(obj, data) case OBJ_ID_##obj:  return data.size();

    foreach_config_data_element

#undef _
    default:
        assert(false);
    }
    return 0;
}

// returns the size of the cfg_msg object for the specified obj id
int
vpp_config_data::objsize (obj_id_t obj_id) const {
    switch(obj_id) {
#define _(obj, data) case OBJ_ID_##obj:  return sizeof(pds_##data##_info_t);

    foreach_config_data_element

#undef _
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
#define _(obj, data)                                        \
    case OBJ_ID_##obj:                                      \
        if (data.find(cfg_msg.data.key) != data.end()) {    \
            return true;                                    \
        }                                                   \
        break;
        foreach_config_data_element
#undef _

    default:
        // don't assert here, it could be a malformed IPC request
        break;
    }
    return false;
}

// checks for existence of a key in the configuration. returns true if the key
// exists
bool
vpp_config_data::exists (obj_id_t obj_id, pds_obj_key_t const& key) const
{
    switch(obj_id) {
#define _(obj, data)                                        \
    case OBJ_ID_##obj:                                      \
        if (data.find(key) != data.end()) {    \
            return true;                                    \
        }                                                   \
        break;
        foreach_config_data_element
#undef _

    default:
        // don't assert here, it could be a malformed IPC request
        break;
    }
    return false;
}


// retrieves the configured spec associated with the specified key. returns
// true if key found
bool
vpp_config_data::get (pds_cfg_msg_t &cfg_msg) const {
    // initialize iterators to all obj types
#define _(obj, data)                                \
    auto data##_it = data.end();                    \

    foreach_config_data_element
#undef _

    switch(cfg_msg.obj_id) {
#define _(obj, data)                                \
    case OBJ_ID_##obj:                              \
        data##_it = data.find(cfg_msg.data.key);    \
        if (data##_it == data.end()) {              \
            cfg_msg.obj_id = OBJ_ID_NONE;           \
            return false;                           \
        }                                           \
        cfg_msg.data = data##_it->second;           \
        break;

        foreach_config_data_element
#undef _

    default:
        assert(false);
        return false;
    }

    return true;
}

bool
vpp_config_data::get (pds_obj_key_t const& key,
                      pds_cfg_get_rsp_t &reply) const
{
    // initialize iterators to all obj types
#define _(obj, data)                                \
    auto data##_it = data.end();                    \
    const pds_##data##_cfg_msg_t *data##_cfg_msg;

    _(DHCP_POLICY, dhcp_policy)
    _(NAT_PORT_BLOCK, nat_port_block)
    _(SECURITY_PROFILE, security_profile)
#undef _

    switch(reply.obj_id) {
#define _(obj, data)                                    \
    case OBJ_ID_##obj:                                  \
        data##_it = data.find(key);                     \
        if (data##_it == data.end()) {                  \
            return false;                               \
        }                                               \
        data##_cfg_msg = &data##_it->second;            \
        memcpy(&reply.data.spec, &data##_cfg_msg->spec, \
               sizeof(pds_##data##_spec_t));            \
        break;

    _(DHCP_POLICY, dhcp_policy)
    _(NAT_PORT_BLOCK, nat_port_block)
    _(SECURITY_PROFILE, security_profile)
#undef _

    default:
        assert(false);
        return false;
    }

    return true;
}

// walk on all config objects for the given config obj id. returns
void
vpp_config_data::walk (obj_id_t obj_id,
                       pds_cfg_walk_cb cb,
                       void *cb_msg) const {
    pds_cfg_msg_t cfg_msg;
    // initialize iterators to all obj types
#define _(obj, data)                                    \
    auto data##_it = data.begin();                      \

    foreach_config_data_element
#undef _
    cfg_msg.obj_id = obj_id;
    cfg_msg.op = API_OP_NONE;

    switch(obj_id) {
#define _(obj, data)                                    \
    case OBJ_ID_##obj:                                  \
        for (data##_it = data.begin();                  \
             data##_it != data.end(); data##_it++) {    \
            cfg_msg.data = data##_it->second;           \
            (cb) (&cfg_msg, cb_msg);                    \
        }                                               \
        break;

        foreach_config_data_element
#undef _

    default:
        assert(false);
        break;
    }

    return;
}

// inserts/updates the spec associated with the specificed key
void
vpp_config_data::set (pds_cfg_msg_t const& cfg_msg) {
    switch(cfg_msg.obj_id) {
#define _(obj, data)                                  \
    case OBJ_ID_##obj:                                \
        data[cfg_msg.data.spec.key] = cfg_msg.data;   \
        break;

        foreach_config_data_element
#undef _

    default:
        assert(false);
    }
}

// removes the spec associated with the specificed key
void
vpp_config_data::unset (obj_id_t obj_id, pds_cfg_msg_t const& cfg_msg) {
    switch(obj_id) {
#define _(obj, data)                        \
    case OBJ_ID_##obj:                      \
        data.erase(cfg_msg.data.key);       \
        break;

        foreach_config_data_element
#undef _

    default:
        assert(false);
    }
}

// compares the provided keys for the two msgs
static bool
cfg_msg_key_equal (pds_cfg_msg_t const& left, pds_cfg_msg_t const& right) {
    assert(left.obj_id == right.obj_id);

    switch(left.obj_id) {
#define _(obj, data)                                                    \
    case OBJ_ID_##obj:                                                  \
        if (right.op != API_OP_DELETE) {                                \
            return (left.data.spec.key.id == right.data.spec.key.id);   \
        } else {                                                        \
            return (left.data.spec.key.id == right.data.key.id);        \
        }                                                               \
        break;

        foreach_config_data_element
#undef _

    default:
        assert(false);
    }

    return false;
}

// config pool functions to operate on a batch

// returns the number of instances for a specific object type
size_t
vpp_config_batch::maxsize (obj_id_t obj_id) {
    switch(obj_id) {
    case OBJ_ID_DEVICE:
        return PDS_MAX_DEVICE;

    case OBJ_ID_VPC:
        return PDS_MAX_VPC + 1;

    case OBJ_ID_VNIC:
        return PDS_MAX_VNIC;

    case OBJ_ID_SUBNET:
        return PDS_MAX_SUBNET + 1;

    case OBJ_ID_DHCP_POLICY:
        return PDS_MAX_DHCP_POLICY;

    case OBJ_ID_NAT_PORT_BLOCK:
        return PDS_MAX_NAT_PORT_BLOCK;

    case OBJ_ID_SECURITY_PROFILE:
        return PDS_MAX_SECURITY_PROFILE;

    default:
        assert(false);
    }
    return 0;
}

// initialize the current pool sizes, to prepare for a new batch
void
vpp_config_batch::init (void) {
    vpp_config_data &vpp_config = vpp_config_data::get();
    pool_sz.resize(OBJ_ID_MAX);
#define _(obj, data)                                        \
    pool_sz[OBJ_ID_##obj] = vpp_config.size(OBJ_ID_##obj);

    foreach_config_data_element
#undef _
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
vpp_config_batch::publish (void) {
    vpp_config_data &vpp_config = vpp_config_data::get();
    // commit modifications to config data
    for(auto it = batch_op.begin(); it != batch_op.end(); it++) {
        if ((*it).deleted) {
            vpp_config.unset((*it).obj_id, (*it).original);
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

    init();

    for (rsv_count = 0; rsv_count < msglist->num_msgs; rsv_count++) {
        const pds_msg_t *msg = &msglist->msgs[rsv_count];

        if (msg->cmd_msg.id > PDS_MSG_ID_MAX) {
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
    vpp_config_cb_registry &cb_registry = vpp_config_cb_registry::get();
    sdk::sdk_ret_t ret = sdk::SDK_RET_OK;

    publish();

    // commit changes to plugin
    for (; it != batch_op.end(); it++) {
        if ((*it).deleted) {
            // pass the original data in to be deleted
            // if the original existed. no-op otherwise
            if ((*it).original.obj_id != OBJ_ID_NONE) {
                ret = cb_registry.del(&((*it).original));
            }
        } else {
            ret = cb_registry.set(&((*it).modified));
        }
        if (ret != sdk::SDK_RET_OK) {
            break;
        }
    }

    if (it == batch_op.end()) {
        assert(ret == sdk::SDK_RET_OK);
        // all good, activate the configuration
        for(it = batch_op.begin(); it != batch_op.end(); it++) {
            // pass the original data in to be deleted
            // if the original existed. no-op otherwise
            if ((*it).deleted) {
                if ((*it).original.obj_id != OBJ_ID_NONE) {
                    cb_registry.activate(&((*it).original));
                    cb_registry.notify(&((*it).original), true);
                }
            } else {
                cb_registry.activate(&((*it).modified));
                cb_registry.notify(&((*it).modified), false);
            }
        }
        return ret;
    }

    // go backwards from the fail point, and restore original

    do {
        it--;
        // not checking return code during rollback
        if ((*it).original.obj_id == OBJ_ID_NONE) {
            // key didn't exist originally, delete it and activate
            cb_registry.del(&((*it).modified));
            cb_registry.activate(&((*it).modified));
            cb_registry.notify(&((*it).modified), true);
        } else {
            cb_registry.set(&((*it).original));
            cb_registry.activate(&((*it).original));
            cb_registry.notify(&((*it).original), false);
        }
    } while(it != batch_op.begin());

    unpublish();

    return ret;
}

// call the read callback corresponding to the obj id
sdk::sdk_ret_t
vpp_config_cb_registry::read (pds_cfg_get_rsp_t &msg) const {
    auto cb = get_cbs.find(msg.obj_id);

    if (cb == get_cbs.end()) {
        return sdk::SDK_RET_OK;
    }

    switch(msg.obj_id) {
#define _(obj, data)                                \
    case OBJ_ID_##obj:                                 \
        return (cb->second)(&msg.data);

    _(DHCP_POLICY, dhcp_policy)
    _(NAT_PORT_BLOCK, nat_port_block)
    _(SECURITY_PROFILE, security_profile)

#undef _
    default:
        break;
    }
    return sdk::SDK_RET_OK;
}

sdk::sdk_ret_t
vpp_config_cb_registry::read (obj_id_t obj_id, void *info) const {
    auto cb = get_cbs.find(obj_id);

    if (cb == get_cbs.end()) {
        return sdk::SDK_RET_OK;
    }

    return (cb->second)(info);
}

// call the dump callback corresponding to the obj id
sdk::sdk_ret_t
vpp_config_cb_registry::dump (obj_id_t obj_id) const {
    auto cb = dump_cbs.find(obj_id);

    if (cb == dump_cbs.end()) {
        return sdk::SDK_RET_OK;
    }

    return (cb->second)();
}

// call all the set callbacks corresponding to the obj id
sdk::sdk_ret_t
vpp_config_cb_registry::set (const pds_cfg_msg_t *msg) {
    auto cblist = set_cbs.find(msg->obj_id);
    sdk::sdk_ret_t ret_val;

    if (cblist == set_cbs.end()) {
        return sdk::SDK_RET_OK;
    }

    for (auto it = (cblist->second).begin(); it != (cblist->second).end();
         it++) {
        ret_val = (*it)(msg);
        if (ret_val != sdk::SDK_RET_OK) {
            return ret_val;
        }
    }
    return sdk::SDK_RET_OK;
}

// call all the delete callbacks corresponding to the obj id
sdk::sdk_ret_t
vpp_config_cb_registry::del (const pds_cfg_msg_t *msg) {
    auto cblist = del_cbs.find(msg->obj_id);
    sdk::sdk_ret_t ret_val;

    if (cblist == del_cbs.end()) {
        return sdk::SDK_RET_OK;
    }

    for (auto it = (cblist->second).begin();
         it != (cblist->second).end(); it++) {
        ret_val = (*it)(msg);
        if (ret_val != sdk::SDK_RET_OK) {
            return ret_val;
        }
    }
    return sdk::SDK_RET_OK;
}

// call all the activate callbacks corresponding to the obj id
sdk::sdk_ret_t
vpp_config_cb_registry::activate (const pds_cfg_msg_t *msg) {
    auto cblist = act_cbs.find(msg->obj_id);
    sdk::sdk_ret_t ret_val;

    if (cblist == act_cbs.end()) {
        return sdk::SDK_RET_OK;
    }

    for (auto it = (cblist->second).begin();
         it != (cblist->second).end(); it++) {
        ret_val = (*it)(msg);
        if (ret_val != sdk::SDK_RET_OK) {
            return ret_val;
        }
    }
    return sdk::SDK_RET_OK;
}

// call all the notify callbacks corresponding to the obj id
sdk::sdk_ret_t
vpp_config_cb_registry::notify (const pds_cfg_msg_t *msg, bool del) {
    auto cblist = notify_cbs.find(msg->obj_id);

    if (cblist == notify_cbs.end()) {
        return sdk::SDK_RET_OK;
    }

    for (auto it = (cblist->second).begin();
         it != (cblist->second).end(); it++) {
        (*it)(msg, del);
    }
    return sdk::SDK_RET_OK;
}
