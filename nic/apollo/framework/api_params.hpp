//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains API params definition
///
//----------------------------------------------------------------------------

#ifndef __FRAMEWORK_API_PARAMS_HPP__
#define __FRAMEWORK_API_PARAMS_HPP__

#include "nic/apollo/framework/api.h"
#include "nic/apollo/framework/api.hpp"
#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/api/include/pds_if.hpp"
#include "nic/apollo/api/include/pds_vpc.hpp"
#include "nic/apollo/api/mapping.hpp"
#include "nic/apollo/api/include/pds_mapping.hpp"
#include "nic/apollo/api/include/pds_mirror.hpp"
#include "nic/apollo/api/include/pds_policy.hpp"
#include "nic/apollo/api/include/pds_route.hpp"
#include "nic/apollo/api/include/pds_subnet.hpp"
#include "nic/apollo/api/include/pds_device.hpp"
#include "nic/apollo/api/include/pds_tep.hpp"
#include "nic/apollo/api/include/pds_vnic.hpp"
#include "nic/apollo/api/include/pds_meter.hpp"
#include "nic/apollo/api/include/pds_tag.hpp"
#include "nic/apollo/api/include/pds_service.hpp"
#include "nic/apollo/api/include/pds_nexthop.hpp"
#include "nic/apollo/api/include/pds_policer.hpp"
#include "nic/apollo/api/include/pds_nat.hpp"
#include "nic/apollo/api/include/pds_dhcp.hpp"

namespace api {

typedef enum api_obj_key_type_u {
    API_OBJ_KEY_TYPE_UUID = 0,  // must be 0
    API_OBJ_KEY_TYPE_SKEY = 1,
} api_obj_key_type_t;

/// \brief API specific parameters
struct api_params_s {
    api_obj_key_type_t               key_type;
    union {
        pds_batch_params_t           batch_spec;
        pds_device_spec_t            device_spec;
        pds_obj_key_t                if_key;
        pds_if_spec_t                if_spec;
        pds_obj_key_t                tep_key;
        pds_tep_spec_t               tep_spec;
        pds_obj_key_t                vpc_key;
        pds_vpc_spec_t               vpc_spec;
        pds_obj_key_t                subnet_key;
        pds_subnet_spec_t            subnet_spec;
        pds_obj_key_t                vnic_key;
        pds_vnic_spec_t              vnic_spec;
        pds_obj_key_t                mapping_key;
        // support both primary and 2nd-ary keys for mapping
        struct {
            pds_mapping_key_t        mapping_skey;
            pds_mapping_spec_t       mapping_spec;
        };
        pds_obj_key_t                route_table_key;
        pds_route_table_spec_t       route_table_spec;
        pds_obj_key_t                policy_key;
        pds_policy_spec_t            policy_spec;
        pds_obj_key_t                mirror_session_key;
        pds_mirror_session_spec_t    mirror_session_spec;
        pds_obj_key_t                meter_key;
        pds_meter_spec_t             meter_spec;
        pds_obj_key_t                tag_key;
        pds_tag_spec_t               tag_spec;
        pds_svc_mapping_key_t        svc_mapping_key;
        pds_svc_mapping_spec_t       svc_mapping_spec;
        pds_obj_key_t                vpc_peer_key;
        pds_vpc_peer_spec_t          vpc_peer_spec;
        pds_obj_key_t                nexthop_key;
        pds_nexthop_spec_t           nexthop_spec;
        pds_obj_key_t                nexthop_group_key;
        pds_nexthop_group_spec_t     nexthop_group_spec;
        pds_obj_key_t                policer_key;
        pds_policer_spec_t           policer_spec;
        pds_obj_key_t                nat_port_block_key;
        pds_nat_port_block_spec_t    nat_port_block_spec;
        pds_obj_key_t                dhcp_relay_key;
        pds_dhcp_relay_spec_t        dhcp_relay_spec;
        pds_obj_key_t                dhcp_policy_key;
        pds_dhcp_policy_spec_t       dhcp_policy_spec;
        pds_obj_key_t                security_profile_key;
        pds_security_profile_spec_t  security_profile_spec;
    };
};

slab *api_params_slab(void);
sdk_ret_t api_params_init(void);
void api_params_free(api_params_t *api_params, obj_id_t obj_id,
                     api_op_t api_op);

}    // namespace

#endif    // __FRAMEWORK_API_PARAMS_HPP__
