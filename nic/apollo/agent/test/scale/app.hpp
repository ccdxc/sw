//------------------------------------------------------------------------------
// Copyright (c) 2019 Pensando Systems, Inc.
//------------------------------------------------------------------------------

#ifndef __TEST_APP_HPP__
#define __TEST_APP_HPP__

#include "nic/apollo/api/include/pds_tep.hpp"
#include "nic/apollo/api/include/pds_vpc.hpp"
#include "nic/apollo/api/include/pds_subnet.hpp"
#include "nic/apollo/api/include/pds_vnic.hpp"
#include "nic/apollo/api/include/pds_mapping.hpp"
#include "nic/apollo/api/include/pds_policy.hpp"
#include "nic/apollo/api/include/pds_device.hpp"
#include "nic/apollo/api/include/pds_route.hpp"
#include "nic/apollo/api/include/pds_mirror.hpp"
#include "nic/apollo/api/include/pds_meter.hpp"
#include "nic/apollo/api/include/pds_tag.hpp"

sdk_ret_t create_route_table_grpc(pds_route_table_spec_t *rt);
sdk_ret_t create_policy_grpc(pds_policy_spec_t *policy);
sdk_ret_t create_local_mapping_grpc(pds_local_mapping_spec_t *local_spec);
sdk_ret_t create_remote_mapping_grpc(pds_remote_mapping_spec_t *remote_spec);
sdk_ret_t create_vnic_grpc(pds_vnic_spec_t *vnic);
sdk_ret_t create_subnet_grpc(pds_subnet_spec_t *subnet);
sdk_ret_t create_vpc_grpc(pds_vpc_spec_t *vpc);
sdk_ret_t create_tunnel_grpc(uint32_t tep_id, pds_tep_spec_t *tep);
sdk_ret_t create_device_grpc(pds_device_spec_t *device);
sdk_ret_t create_mirror_session_grpc(pds_mirror_session_spec_t *spec);
sdk_ret_t batch_start_grpc(int epoch);
sdk_ret_t batch_commit_grpc(void);
void test_app_init(void);
sdk_ret_t test_app_push_configs(void);
sdk_ret_t create_meter_grpc(pds_meter_spec_t *pds_meter);
sdk_ret_t create_tag_grpc(pds_tag_spec_t *pds_tag);

#endif
