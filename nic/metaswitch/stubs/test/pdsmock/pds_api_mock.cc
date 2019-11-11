#include  <stdio.h>
#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/api/include/pds_if.hpp"
#include "nic/apollo/api/include/pds_mapping.hpp"
#include "nic/apollo/api/include/pds_nexthop.hpp"
#include "nic/apollo/api/include/pds_route.hpp"
#include "nic/apollo/api/include/pds_tep.hpp"
#include "nic/apollo/api/include/pds_vnic.hpp"
#include "nic/apollo/api/include/pds_vpc.hpp"
#include "nic/metaswitch/stubs/test/utils/base.hpp"
#include "nic/apollo/test/utils/nexthop.hpp"


extern api_test::pdsa_test_params_t g_test_params;

pds_batch_ctxt_t pds_batch_start(pds_batch_params_t *batch_params) {
    return 0;
}

sdk_ret_t pds_batch_commit(pds_batch_ctxt_t bctxt) {
    return SDK_RET_OK;
}

sdk_ret_t pds_batch_destroy(pds_batch_ctxt_t bctxt) {
    return SDK_RET_OK;
}

sdk_ret_t pds_if_create(pds_if_spec_s *spec,
                        pds_batch_ctxt_t bctxt) {
    return SDK_RET_OK;
}

sdk_ret_t pds_if_read(pds_if_key_t *key, pds_if_info_t *info) {
    return SDK_RET_OK;
}

sdk_ret_t pds_if_read_all(if_read_cb_t cb, void *ctxt) {
    return SDK_RET_OK;
}

sdk_ret_t pds_if_update(pds_if_spec_s *spec,
                        pds_batch_ctxt_t bctxt) {
    return SDK_RET_OK;
}

sdk_ret_t pds_if_delete(pds_if_key_t *key,
                        pds_batch_ctxt_t bctxt) {
    return SDK_RET_OK;
}


sdk_ret_t pds_local_mapping_create(pds_local_mapping_spec_s *spec,
                                   pds_batch_ctxt_t bctxt) {
    return SDK_RET_OK;
}

sdk_ret_t pds_local_mapping_read(pds_mapping_key_t *key,
                                 pds_local_mapping_info_t *info) {
    return SDK_RET_OK;
}

sdk_ret_t pds_local_mapping_update(pds_local_mapping_spec_s *spec,
                                   pds_batch_ctxt_t bctxt) {
    return SDK_RET_OK;
}

sdk_ret_t pds_local_mapping_delete(pds_mapping_key_t *key,
                                   pds_batch_ctxt_t bctxt) {
    return SDK_RET_OK;
}

sdk_ret_t pds_remote_mapping_create(pds_remote_mapping_spec_s *spec,
                                    pds_batch_ctxt_t bctxt) {
    return SDK_RET_OK;
}

sdk_ret_t pds_remote_mapping_read(pds_mapping_key_t *key,
                                  pds_remote_mapping_info_t *info) {
    return SDK_RET_OK;
}

sdk_ret_t pds_remote_mapping_update(pds_remote_mapping_spec_s *spec,
                                    pds_batch_ctxt_t bctxt) {
    return SDK_RET_OK;
}

sdk_ret_t pds_remote_mapping_delete(pds_mapping_key_t *key,
                                    pds_batch_ctxt_t bctxt) {
    return SDK_RET_OK;
}

sdk_ret_t pds_nexthop_create(pds_nexthop_spec_s *spec,
                             pds_batch_ctxt_t bctxt) {
    printf("In mock nhcreate\n");
    if (!g_test_params.nh.spec_compare(spec)) {
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

sdk_ret_t pds_nexthop_read(pds_nexthop_key_t *key, pds_nexthop_info_t *info) {
    if (!g_test_params.nh.key_compare(key)) {
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

sdk_ret_t pds_nexthop_update(pds_nexthop_spec_s *spec,
                             pds_batch_ctxt_t bctxt) {
    if (!g_test_params.nh.spec_compare(spec)) {
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

sdk_ret_t pds_nexthop_delete(pds_nexthop_key_t *key,
                             pds_batch_ctxt_t bctxt) {
    if (!g_test_params.nh.key_compare(key)) {
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

sdk_ret_t pds_route_table_create(pds_route_table_spec_s *spec,
                                 pds_batch_ctxt_t bctxt) {
    return SDK_RET_OK;
}

sdk_ret_t pds_route_table_read(pds_route_table_key_t *key,
                               pds_route_table_info_t *info) {
    return SDK_RET_OK;
}

sdk_ret_t pds_route_table_update(pds_route_table_spec_s *spec,
                                 pds_batch_ctxt_t bctxt) {
    return SDK_RET_OK;
}

sdk_ret_t pds_route_table_delete(pds_route_table_key_t *key,
                                 pds_batch_ctxt_t bctxt) {
    return SDK_RET_OK;
}

sdk_ret_t pds_tep_create(pds_tep_spec_s *spec,
                         pds_batch_ctxt_t bctxt) {
    return SDK_RET_OK;
}

sdk_ret_t pds_tep_read(pds_tep_key_t *key, pds_tep_info_t *info) {
    return SDK_RET_OK;
}

sdk_ret_t pds_tep_update(pds_tep_spec_s *spec,
                         pds_batch_ctxt_t bctxt) {
    return SDK_RET_OK;
}

sdk_ret_t pds_tep_delete(pds_tep_key_t *key,
                         pds_batch_ctxt_t bctxt) {
    return SDK_RET_OK;
}

sdk_ret_t pds_vnic_create(pds_vnic_spec_s *spec,
                          pds_batch_ctxt_t bctxt) {
    return SDK_RET_OK;
}

sdk_ret_t pds_vnic_read(pds_vnic_key_t *key, pds_vnic_info_t *info) {
    return SDK_RET_OK;
}

sdk_ret_t pds_vnic_update(pds_vnic_spec_s *spec,
                          pds_batch_ctxt_t bctxt) {
    return SDK_RET_OK;
}

sdk_ret_t pds_vnic_delete(pds_vnic_key_t *key,
                          pds_batch_ctxt_t bctxt) {
    return SDK_RET_OK;
}

sdk_ret_t pds_vpc_create(pds_vpc_spec_s *spec,
                         pds_batch_ctxt_t bctxt) {
    return SDK_RET_OK;
}

sdk_ret_t pds_vpc_read(pds_vpc_key_t *key, pds_vpc_info_t *info) {
    return SDK_RET_OK;
}

sdk_ret_t pds_vpc_update(pds_vpc_spec_s *spec,
                         pds_batch_ctxt_t bctxt) {
    return SDK_RET_OK;
}

sdk_ret_t pds_vpc_delete(pds_vpc_key_t *key,
                         pds_batch_ctxt_t bctxt) {
    return SDK_RET_OK;
}

sdk_ret_t pds_tag_create(pds_tag_spec_s *spec,
                         pds_batch_ctxt_t bctxt) {
    return SDK_RET_OK;
}

sdk_ret_t pds_meter_create(pds_meter_spec_s *spec,
                           pds_batch_ctxt_t bctxt) {
    return SDK_RET_OK;
}

sdk_ret_t pds_mirror_session_update(pds_mirror_session_spec_s *spec,
                                    pds_batch_ctxt_t bctxt) {
    return SDK_RET_OK;
}

sdk_ret_t pds_policy_create(pds_policy_spec_s *spec,
                            pds_batch_ctxt_t bctxt) {
    return SDK_RET_OK;
}

sdk_ret_t pds_device_read(pds_device_info_s *spec) {
    return SDK_RET_OK;
}

sdk_ret_t pds_vpc_peer_create(pds_vpc_peer_spec_s *spec,
                              pds_batch_ctxt_t bctxt) {
    return SDK_RET_OK;
}

sdk_ret_t pds_device_create(pds_device_s *spec,
                            pds_batch_ctxt_t bctxt) {
    return SDK_RET_OK;
}

sdk_ret_t pds_mirror_session_create(pds_mirror_session_spec_s *spec,
                                    pds_batch_ctxt_t bctxt) {
    return SDK_RET_OK;
}

sdk_ret_t pds_subnet_delete(pds_subnet_key_s *spec,
                            pds_batch_ctxt_t bctxt) {
    return SDK_RET_OK;
}

sdk_ret_t pds_svc_mapping_create(pds_svc_mapping_spec_s *spec,
                                 pds_batch_ctxt_t bctxt) {
    return SDK_RET_OK;
}

sdk_ret_t pds_mirror_session_read(pds_mirror_session_key_s *spec,
                                  pds_mirror_session_info_s *info) {
    return SDK_RET_OK;
}

sdk_ret_t pds_mirror_session_delete(pds_mirror_session_key_s *spec,
                                    pds_batch_ctxt_t bctxt) {
    return SDK_RET_OK;
}

sdk_ret_t pds_nexthop_group_create(pds_nexthop_group_spec_s *spec,
                                   pds_batch_ctxt_t bctxt) {
    return SDK_RET_OK;
}

sdk_ret_t pds_nexthop_group_read(pds_nexthop_group_key_t *key,
                                 pds_nexthop_group_info_t *info) {
    return SDK_RET_OK;
}

sdk_ret_t pds_nexthop_group_update(pds_nexthop_group_spec_s *spec,
                                   pds_batch_ctxt_t bctxt) {
    return SDK_RET_OK;
}

sdk_ret_t pds_nexthop_group_delete(pds_nexthop_group_key_t *key,
                                   pds_batch_ctxt_t bctxt) {
    return SDK_RET_OK;
}

sdk_ret_t pds_subnet_create(pds_subnet_spec_s *spec,
                            pds_batch_ctxt_t bctxt) {
    return SDK_RET_OK;
}

sdk_ret_t pds_init(pds_init_params_s *params) {
    return SDK_RET_OK;
}

sdk_ret_t pds_device_delete(pds_batch_ctxt_t bctxt) {
    return SDK_RET_OK;
}

sdk_ret_t pds_teardown() {
    return SDK_RET_OK;
}

namespace api {
    pds_state g_pds_state;
    pds_state::pds_state() { }
    pds_state::~pds_state() { }
}
