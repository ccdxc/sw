//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the vpc test utility routines implementation
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/apollo/test/utils/utils.hpp"
#include "nic/apollo/test/utils/vpc_peer.hpp"
#include "nic/apollo/test/utils/vpc.hpp"

namespace api_test {

vpc_peer_util::vpc_peer_util(vpc_peer_stepper_seed_t *seed) {
    this->key.id = seed->key.id;
    this->vpc1.id = seed->vpc1.id;
    this->vpc2.id = seed->vpc2.id;
}

vpc_peer_util::~vpc_peer_util() {}

sdk::sdk_ret_t
vpc_peer_util::create(void) const {
    pds_vpc_peer_spec_t spec;

    memset(&spec, 0, sizeof(spec));
    spec.key.id = this->key.id;
    spec.vpc1.id = this->vpc1.id;
    spec.vpc2.id = this->vpc2.id;

    return (pds_vpc_peer_create(&spec));
}

sdk::sdk_ret_t
vpc_peer_util::read(pds_vpc_peer_info_t *info) const {
    sdk_ret_t rv;
    pds_vpc_peer_key_t key;

    memset(&key, 0, sizeof(pds_vpc_peer_key_t));
    memset(info, 0, sizeof(pds_vpc_peer_info_t));

    key.id = this->key.id;
    rv = pds_vpc_peer_read(&key, info);
    if ((rv = pds_vpc_peer_read(&key, info)) != SDK_RET_OK) {
        return rv;
    }

    return sdk::SDK_RET_OK;
}

sdk::sdk_ret_t
vpc_peer_util::update(void) const {
    pds_vpc_peer_spec_t spec;

    memset(&spec, 0, sizeof(spec));
    spec.key.id = this->key.id;
    spec.vpc1.id = this->vpc1.id;
    spec.vpc2.id = this->vpc2.id;
    // return (pds_vpc_peer_update(&spec));
    return sdk::SDK_RET_OK;
}

sdk::sdk_ret_t
vpc_peer_util::del(void) const {
    pds_vpc_peer_key_t key = {};

    key.id = this->key.id;
    return (pds_vpc_peer_delete(&key));
}

static inline void
vpc_peer_stepper_seed_increment (vpc_peer_stepper_seed_t *seed, int width)
{
    seed->key.id += width;
    seed->vpc1.id = seed->vpc2.id + width;
    seed->vpc2.id = seed->vpc1.id + width;
}

static inline sdk::sdk_ret_t
vpc_peer_util_object_stepper (vpc_peer_stepper_seed_t *init_seed,
                              utils_op_t op, sdk_ret_t expected_result)
{
    sdk::sdk_ret_t rv = sdk::SDK_RET_OK;
    vpc_peer_stepper_seed_t seed = {0};
    pds_vpc_peer_info_t info = {};
    uint32_t width = 1;
    uint32_t num_objs = init_seed->num_vpc_peers;

    vpc_peer_util::stepper_seed_init(&seed, init_seed->key, init_seed->vpc1,
                                     init_seed->vpc2, init_seed->num_vpc_peers);

    for (uint32_t idx = 0; idx < num_objs; idx++) {
        vpc_peer_util vpc_peer_obj(&seed);
        switch (op) {
        case OP_MANY_CREATE:
            rv = vpc_peer_obj.create();
            break;
        case OP_MANY_READ:
            rv = vpc_peer_obj.read(&info);
            break;
        case OP_MANY_UPDATE:
            rv = vpc_peer_obj.update();
            break;
        case OP_MANY_DELETE:
            rv = vpc_peer_obj.del();
            break;
        default:
            return sdk::SDK_RET_INVALID_OP;
        }
        if (rv != expected_result) {
            return sdk::SDK_RET_ERR;
        }
        vpc_peer_stepper_seed_increment(&seed, width);
    }
    return sdk::SDK_RET_OK;
}

sdk::sdk_ret_t
vpc_peer_util::many_create(vpc_peer_stepper_seed_t *seed) {
    return (vpc_peer_util_object_stepper(seed, OP_MANY_CREATE, sdk::SDK_RET_OK));
}

sdk::sdk_ret_t
vpc_peer_util::many_read(vpc_peer_stepper_seed_t *seed, sdk_ret_t expected_res) {
    return (vpc_peer_util_object_stepper(seed, OP_MANY_READ, expected_res));
}

sdk::sdk_ret_t
vpc_peer_util::many_update(vpc_peer_stepper_seed_t *seed) {
    return (vpc_peer_util_object_stepper(seed, OP_MANY_UPDATE, sdk::SDK_RET_OK));
}

sdk::sdk_ret_t
vpc_peer_util::many_delete(vpc_peer_stepper_seed_t *seed) {
    return (vpc_peer_util_object_stepper(seed, OP_MANY_DELETE, sdk::SDK_RET_OK));
}

void
vpc_peer_util::stepper_seed_init (vpc_peer_stepper_seed_t *seed,
                                  pds_vpc_peer_key_t key,
                                  pds_vpc_key_t vpc1,
                                  pds_vpc_key_t vpc2,
                                  uint32_t num_vpc_peers) {
    seed->key.id = key.id;
    seed->vpc1.id = vpc1.id;
    seed->vpc2.id = vpc2.id;
    seed->num_vpc_peers = num_vpc_peers;
}

}    // namespace api_test
