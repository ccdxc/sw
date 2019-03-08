//------------------------------------------------------------------------------
// Copyright (c) 2019 Pensando Systems, Inc.
//------------------------------------------------------------------------------

#ifndef __TEST_APP_HPP__
#define __TEST_APP_HPP__

#include "nic/apollo/include/api/pds_tep.hpp"
#include "nic/apollo/include/api/pds_vcn.hpp"
#include "nic/apollo/include/api/pds_subnet.hpp"
#include "nic/apollo/include/api/pds_vnic.hpp"
#include "nic/apollo/include/api/pds_mapping.hpp"
#include "nic/apollo/include/api/pds_policy.hpp"
#include "nic/apollo/include/api/pds_device.hpp"
#include "nic/apollo/include/api/pds_route.hpp"

sdk_ret_t create_route_table_grpc(pds_route_table_spec_t *rt);
sdk_ret_t create_mapping_grpc(pds_mapping_spec_t *mapping);
sdk_ret_t create_vnic_grpc(pds_vnic_spec_t *vnic);
sdk_ret_t create_subnet_grpc(pds_subnet_spec_t *subnet);
sdk_ret_t create_vcn_grpc(pds_vcn_spec_t *vcn);
sdk_ret_t create_tunnel_grpc(pds_tep_spec_t *tep);
sdk_ret_t create_switch_grpc(pds_device_spec_t *device);
void test_app_init(void);
sdk_ret_t test_app_push_configs(void);

#endif
