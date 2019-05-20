//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the mirror test utility routines implementation
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/apollo/test/utils/utils.hpp"
#include "nic/apollo/test/utils/mirror.hpp"

namespace api_test {

mirror_session_util::mirror_session_util(mirror_session_stepper_seed_t *seed) {
    this->key.id = seed->key.id;
    this->snap_len = 100;
    this->type = seed->type;

    if (seed->type == PDS_MIRROR_SESSION_TYPE_RSPAN) {
        this->rspan_spec.interface = seed->interface;
        memcpy(&this->rspan_spec.encap, &seed->encap, sizeof(pds_encap_t));
    } else if (seed->type == PDS_MIRROR_SESSION_TYPE_ERSPAN) {
        memcpy(&this->erspan_spec.dst_ip, &seed->dst_ip, sizeof(ip_addr_t));
        memcpy(&this->erspan_spec.src_ip, &seed->src_ip, sizeof(ip_addr_t));
        this->erspan_spec.vpc.id = seed->vpc_id;
        this->erspan_spec.dscp = seed->dscp;
        this->erspan_spec.span_id = seed->span_id;
    }
}

sdk::sdk_ret_t
mirror_session_util::create(void) {
    pds_mirror_session_spec_t spec = {0};

    spec.key.id = key.id;
    spec.type =type;
    spec.snap_len = snap_len;
    if(spec.type == PDS_MIRROR_SESSION_TYPE_RSPAN) {
        spec.rspan_spec = rspan_spec;
    } else if (spec.type == PDS_MIRROR_SESSION_TYPE_ERSPAN) {
        spec.erspan_spec = erspan_spec;
    }

    return pds_mirror_session_create(&spec);
}

sdk::sdk_ret_t
mirror_session_util::read(pds_mirror_session_info_t *info) {
    sdk_ret_t rv;
    pds_mirror_session_key_t key;

    memset(&key, 0, sizeof(pds_mirror_session_key_t));
    memset(info, 0, sizeof(pds_mirror_session_info_t));

    key.id = this->key.id;
    if ((rv = pds_mirror_session_get(&key, info)) != sdk::SDK_RET_OK)
        return rv;

    if (capri_mock_mode())
        return sdk::SDK_RET_OK;

    // validate mirror session type
    if (info->spec.type != this->type) {
        return sdk::SDK_RET_ERR;
    }
    // validate snap_len
    if (info->spec.snap_len != this->snap_len) {
        return sdk::SDK_RET_ERR;
    }
    if (info->spec.type == PDS_MIRROR_SESSION_TYPE_RSPAN) {
        // validate rspan spec
        if (info->spec.rspan_spec.interface !=
            this->rspan_spec.interface) {
            return sdk::SDK_RET_ERR;
        }
        if (info->spec.rspan_spec.encap.type !=
            this->rspan_spec.encap.type) {
            return sdk::SDK_RET_ERR;
        }
        if (info->spec.rspan_spec.encap.val.vlan_tag !=
            this->rspan_spec.encap.val.vlan_tag) {
            return sdk::SDK_RET_ERR;
        }
    } else if (info->spec.type == PDS_MIRROR_SESSION_TYPE_ERSPAN) {
        // validate erspan spec
        if (info->spec.erspan_spec.src_ip.addr.v4_addr !=
            this->erspan_spec.src_ip.addr.v4_addr) {
            return sdk::SDK_RET_ERR;
        }
        if (info->spec.erspan_spec.dst_ip.addr.v4_addr !=
            this->erspan_spec.dst_ip.addr.v4_addr) {
            return sdk::SDK_RET_ERR;
        }
    }

    return sdk::SDK_RET_OK;
}

sdk::sdk_ret_t
mirror_session_util::update(void) {
    // TODO
    return SDK_RET_OK;
}

sdk::sdk_ret_t
mirror_session_util::del(void) {
    return pds_mirror_session_delete(&this->key);
}

static inline sdk::sdk_ret_t
mirror_session_util_object_stepper(mirror_session_stepper_seed_t *seed,
                                   uint32_t num_mirror_sessions,
                                   utils_op_t op,
                                   sdk_ret_t expected_result = sdk::SDK_RET_OK)
{
    sdk::sdk_ret_t rv = sdk::SDK_RET_OK;
    pds_mirror_session_info_t info = {};
    mirror_session_stepper_seed_t local_seed;

    if (seed->key.id == 0) seed->key.id = 1;
    memcpy(&local_seed, seed, sizeof(mirror_session_stepper_seed_t));

    for (uint32_t idx = seed->key.id;
         idx < seed->key.id + num_mirror_sessions;
         idx++) {

        local_seed.key.id = idx;
        mirror_session_util ms_obj(&local_seed);
        switch (op) {
        case OP_MANY_CREATE:
            rv = ms_obj.create();
            break;
        case OP_MANY_DELETE:
            rv = ms_obj.del();
            break;
        case OP_MANY_READ:
            rv = ms_obj.read(&info);
            break;
        default:
            return sdk::SDK_RET_INVALID_OP;
        }

        if (rv != expected_result)
            return sdk::SDK_RET_ERR;

        if (local_seed.type == PDS_MIRROR_SESSION_TYPE_RSPAN) {
            if (local_seed.encap.type == PDS_ENCAP_TYPE_DOT1Q)
                local_seed.encap.val.vlan_tag++;
            else if (local_seed.encap.type == PDS_ENCAP_TYPE_QINQ) {
                local_seed.encap.val.qinq_tag.c_tag++;
                local_seed.encap.val.qinq_tag.s_tag++;
            }
        } else if (local_seed.type == PDS_MIRROR_SESSION_TYPE_ERSPAN) {
            local_seed.span_id++;
            local_seed.dscp++;
        }
    }

    return sdk::SDK_RET_OK;
}

sdk::sdk_ret_t
mirror_session_util::many_create(mirror_session_stepper_seed_t *seed,
                                 uint32_t num_mirror_sessions) {
    return (mirror_session_util_object_stepper(seed, num_mirror_sessions,
                                               OP_MANY_CREATE));
}

sdk::sdk_ret_t
mirror_session_util::many_read(mirror_session_stepper_seed_t *seed,
                     uint32_t num_mirror_sessions,
                     sdk::sdk_ret_t exp_result) {
    return (mirror_session_util_object_stepper(seed, num_mirror_sessions,
                                               OP_MANY_READ, exp_result));
}

sdk::sdk_ret_t
mirror_session_util::many_delete(mirror_session_stepper_seed_t *seed,
                                 uint32_t num_mirror_sessions) {
    return (mirror_session_util_object_stepper(seed, num_mirror_sessions,
                                               OP_MANY_DELETE));
}

}    // namespace api_test
