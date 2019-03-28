//------------------------------------------------------------------------------
// Copyright (c) 2019 Pensando Systems, Inc.
//------------------------------------------------------------------------------

#ifndef __TEST_APP_HPP__
#define __TEST_APP_HPP__

#include "nic/apollo/api/include/pds_tep.hpp"
#include "nic/apollo/api/include/pds_vcn.hpp"
#include "nic/apollo/api/include/pds_subnet.hpp"
#include "nic/apollo/api/include/pds_vnic.hpp"
#include "nic/apollo/api/include/pds_mapping.hpp"
#include "nic/apollo/api/include/pds_policy.hpp"
#include "nic/apollo/api/include/pds_device.hpp"
#include "nic/apollo/api/include/pds_route.hpp"

sdk_ret_t create_route_table_grpc(pds_route_table_spec_t *rt);
sdk_ret_t create_mapping_grpc(pds_mapping_spec_t *mapping);
sdk_ret_t create_vnic_grpc(pds_vnic_spec_t *vnic);
sdk_ret_t create_subnet_grpc(pds_subnet_spec_t *subnet);
sdk_ret_t create_vcn_grpc(pds_vcn_spec_t *vcn);
sdk_ret_t create_tunnel_grpc(uint32_t tep_id, pds_tep_spec_t *tep);
sdk_ret_t create_device_grpc(pds_device_spec_t *device);
sdk_ret_t batch_start_grpc(void);
sdk_ret_t batch_commit_grpc(void);
void test_app_init(void);
sdk_ret_t test_app_push_configs(void);

#endif
