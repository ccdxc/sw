//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_INFRA_CFG_PDSA_DB_HPP__
#define __VPP_INFRA_CFG_PDSA_DB_HPP__

#include <map>
#include <unordered_map>
#include <vector>
#include <list>
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/core/msg.h"
#include "nic/apollo/api/core/msg.h"

// callback function prototype
typedef sdk::sdk_ret_t (*pds_cfg_set_cb)(const pds_cfg_msg_t *msg);
typedef sdk::sdk_ret_t (*pds_cfg_del_cb)(const pds_cfg_msg_t *msg);
typedef sdk::sdk_ret_t (*pds_cfg_act_cb)(const pds_cfg_msg_t *msg);

// function prototypes
int pds_cfg_register_callbacks(obj_id_t id,
                               pds_cfg_set_cb set_cb_fn,
                               pds_cfg_del_cb del_cb_fn,
                               pds_cfg_act_cb act_cb_fn);

typedef struct {
    obj_id_t      obj_id;
    bool          deleted;
    pds_cfg_msg_t modified;
    pds_cfg_msg_t original;
} batch_op_t;

typedef struct {
    obj_id_t       obj_id;
    pds_cfg_set_cb set_cb;
    pds_cfg_del_cb del_cb;
    pds_cfg_act_cb act_cb;
} commit_cbs_t;

// represents all the configuration data that is required by all VPP plugins.
// this is the authoritative copy, and plugins are notified (through
// registered callbacks) when this config changes
class vpp_config_data {
    // containers are ordered map, instance to cfg msg
    std::unordered_map<pds_obj_key_t, pds_vpc_cfg_msg_t, pds_obj_key_hash> vpc;
    std::unordered_map<pds_obj_key_t, pds_vnic_cfg_msg_t, pds_obj_key_hash> vnic;
    std::unordered_map<pds_obj_key_t, pds_subnet_cfg_msg_t, pds_obj_key_hash> subnet;
    std::unordered_map<pds_obj_key_t, pds_dhcp_relay_cfg_msg_t, pds_obj_key_hash> dhcp_relay;
    std::unordered_map<pds_obj_key_t, pds_dhcp_policy_cfg_msg_t, pds_obj_key_hash> dhcp_policy;
    std::unordered_map<pds_obj_key_t, pds_nat_port_block_cfg_msg_t, pds_obj_key_hash> nat_port_block;
    std::unordered_map<pds_obj_key_t, pds_security_profile_cfg_msg_t, pds_obj_key_hash> security_profile;
    static vpp_config_data singleton;

public:
    // instance accessor
    static vpp_config_data &get(void) { return singleton; }

    // constant operations
    int size(obj_id_t obj_id) const;
    bool exists(pds_cfg_msg_t const& cfg_msg) const;
    bool get(pds_cfg_msg_t &value) const;

    // modifiers
    void set(pds_cfg_msg_t const& cfg_msg);
    void unset(obj_id_t obj_id, pds_cfg_msg_t const& cfg_msg);
};


// list of operations in a single msglist received from
// HAL. each instance in the msglist has a batch_op_t entry in here, which is
// executed once all entries are created.
class vpp_config_batch {
    std::list<batch_op_t> batch_op;
    std::map<obj_id_t, size_t> pool_sz;

    static std::list<commit_cbs_t> commit_cbs;
    static vpp_config_batch singleton;

private:
    // internal operations
    void init(void);
    void publish(void);
    void unpublish(void);
    bool reserve(pds_cfg_msg_t const& msg);
    sdk::sdk_ret_t add(pds_cfg_msg_t const& cfg_msg);
    void set(pds_cfg_msg_t const& msg);
    void del(obj_id_t obj_id, pds_cfg_msg_t const& msg);

    // constant operations
    bool exists(pds_cfg_msg_t const& msg) const;
    static size_t maxsize(obj_id_t ob_id);

public:
    static void register_cbs(obj_id_t id,
                             pds_cfg_set_cb set_cb_fn,
                             pds_cfg_del_cb del_cb_fn,
                             pds_cfg_act_cb act_cb_fn);

    static vpp_config_batch &get(void) { return singleton; }
public:
    sdk::sdk_ret_t create(const pds_msg_list_t *msglist);
    sdk::sdk_ret_t commit(void);
    void clear(void);
};


#endif // __VPP_INFRA_CFG_PDSA_DB_HPP__
