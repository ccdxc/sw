//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// uds test app header
///
//----------------------------------------------------------------------------

#ifndef __TEST_APP_HPP__
#define __TEST_APP_HPP__

#include "nic/apollo/api/include/pds_tep.hpp"
#include "nic/apollo/api/include/pds_vpc.hpp"
#include "nic/apollo/api/include/pds_subnet.hpp"
#include "nic/apollo/api/include/pds_vnic.hpp"
#include "nic/apollo/api/include/pds_dhcp.hpp"
#include "nic/apollo/api/include/pds_mapping.hpp"
#include "nic/apollo/api/include/pds_policy.hpp"
#include "nic/apollo/api/include/pds_policer.hpp"
#include "nic/apollo/api/include/pds_device.hpp"
#include "nic/apollo/api/include/pds_route.hpp"
#include "nic/apollo/api/include/pds_mirror.hpp"
#include "nic/apollo/api/include/pds_meter.hpp"
#include "nic/apollo/api/include/pds_tag.hpp"
#include "nic/apollo/api/include/pds_service.hpp"
#include "nic/apollo/api/include/pds_if.hpp"
#include "nic/apollo/api/include/pds_nat.hpp"

sdk_ret_t create_route_table_impl(pds_route_table_spec_t *spec);
sdk_ret_t create_policy_impl(pds_policy_spec_t *spec);
sdk_ret_t create_local_mapping_impl(pds_local_mapping_spec_t *spec);
sdk_ret_t create_remote_mapping_impl(pds_remote_mapping_spec_t *spec);
sdk_ret_t create_vnic_impl(pds_vnic_spec_t *spec);
sdk_ret_t create_subnet_impl(pds_subnet_spec_t *spec);
sdk_ret_t create_vpc_impl(pds_vpc_spec_t *spec = NULL);
sdk_ret_t create_dhcp_policy_impl(pds_dhcp_policy_spec_t *spec);
sdk_ret_t create_nat_port_block_impl(pds_nat_port_block_spec_t *spec = NULL);
sdk_ret_t read_vpc_impl(pds_obj_key_t *key = NULL, pds_vpc_info_t *info = NULL);
sdk_ret_t update_vpc_impl(pds_vpc_spec_t *spec = NULL);
sdk_ret_t delete_vpc_impl(pds_obj_key_t *key = NULL);
sdk_ret_t create_vpc_peer_impl(pds_vpc_peer_spec_t *spec);
sdk_ret_t create_tunnel_impl(uint32_t tep_id, pds_tep_spec_t *spec);
sdk_ret_t create_device_impl(pds_device_spec_t *spec);
sdk_ret_t create_mirror_session_impl(pds_mirror_session_spec_t *spec);
sdk_ret_t create_meter_impl(pds_meter_spec_t *spec);
sdk_ret_t create_tag_impl(pds_tag_spec_t *spec);
sdk_ret_t create_policer_impl(pds_policer_spec_t *spec);
sdk_ret_t create_nexthop_impl(pds_nexthop_spec_t *spec);
sdk_ret_t create_nexthop_group_impl(pds_nexthop_group_spec_t *spec);
sdk_ret_t create_svc_mapping_impl(pds_svc_mapping_spec_t *spec);
sdk_ret_t create_l3_intf_impl(pds_if_spec_t *spec);

pds_batch_ctxt_t batch_start_impl(int epoch);
sdk_ret_t batch_commit_impl(pds_batch_ctxt_t bctxt);
sdk_ret_t batch_abort_impl(pds_batch_ctxt_t bctxt);
sdk_ret_t test_app_push_configs(void);
sdk_ret_t test_app_init(void);
void test_app_deinit(void);

#endif    // __TEST_APP_HPP__
