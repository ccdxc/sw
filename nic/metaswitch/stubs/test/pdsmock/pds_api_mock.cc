//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include "nic/metaswitch/stubs/test/pdsmock/pds_api_mock.hpp"
#include "nic/metaswitch/stubs/test/pdsmock/pds_api_mock_validate.hpp"

#include "nic/apollo/api/include/pds_init.hpp"
#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/api/include/pds_if.hpp"
#include "nic/apollo/api/include/pds_mapping.hpp"
#include "nic/apollo/api/include/pds_nexthop.hpp"
#include "nic/apollo/api/include/pds_route.hpp"
#include "nic/apollo/api/include/pds_tep.hpp"
#include "nic/apollo/api/include/pds_vnic.hpp"
#include "nic/apollo/api/include/pds_vpc.hpp"
#include "nic/apollo/test/utils/nexthop.hpp"
#include "nic/apollo/test/utils/nexthop_group.hpp"

#include "nic/sdk/platform/fru/fru.hpp"
#include "nic/apollo/api/include/pds_init.hpp"
#include "nic/sdk/include/sdk/platform.hpp"

#include "nic/metaswitch/stubs/hals/pdsa_li.hpp"
#include "nic/metaswitch/stubs/common/pdsa_cookie.hpp"
#include "nic/metaswitch/stubs/common/pdsa_state.hpp"
#include "nic/metaswitch/stubs/hals/pdsa_hal_init.hpp"
#include <thread>

pds_batch_ctxt_t pds_batch_start(pds_batch_params_t *batch_params) 
{
    auto pds_mock = dynamic_cast<pdsa_test::pds_mock_t*>(pdsa_test::test_params()->test_output);
    if (pds_mock == nullptr) {
        // Not running in mock test mode - return dummy
        auto pds_mock = new pdsa_test::pds_mock_t(true);
        pds_mock->cookie = (void*) batch_params->cookie;
        return (uint64_t) pds_mock;
    }
    pds_mock->cookie = (void*) batch_params->cookie;
    return ((pds_batch_ctxt_t) pds_mock);
}

sdk_ret_t pds_batch_commit(pds_batch_ctxt_t bctxt) 
{
    // Gtest mode
    auto pds_mock = (pdsa_test::pds_mock_t*) bctxt;
    int count = 0;
    auto& pds_ret_status =  pds_mock->pds_ret_status;

    if (pds_mock->sim) {
        std::thread cb(pdsa_stub::hal_callback, true, (uint64_t)pds_mock->cookie);
        cb.detach();
        return SDK_RET_OK;
    }

    if (pds_mock->mock_pds_batch_commit_fail_) {
        return SDK_RET_ERR;
    }
    if (pds_mock->expected_pds.size() != pds_mock->rcvd_pds.size()) {
        std::cout << "Batch size mismatch - Expected: "
                  << pds_mock->expected_pds.size() << " Received:" 
                  << pds_mock->rcvd_pds.size()  << std::endl;
        pds_ret_status = false;
        return SDK_RET_OK;
    }
    for (auto& expected_pds: pds_mock->expected_pds) {
        auto& rcvd_pds = pds_mock->rcvd_pds[count];
        if (expected_pds.op != rcvd_pds.op) {
            std::cout << "Op Mismatch - Expected: " << expected_pds.op 
                      << "  Received: " << rcvd_pds.op << std::endl;
            pds_ret_status = false;
            return SDK_RET_OK;
        }
        switch (expected_pds.obj_id) {
        case OBJ_ID_TEP: 
        {
            if (!pdsa_test::pds_tep_mock_validate (expected_pds, rcvd_pds)) {
                pds_ret_status = false;
                return SDK_RET_OK;
            }
            break;
        }
        case OBJ_ID_NEXTHOP_GROUP:
        {
            if (!pdsa_test::pds_nhgroup_mock_validate (expected_pds, rcvd_pds)) {
                pds_ret_status = false;
                return SDK_RET_OK;
            }
            break;
        }
        case OBJ_ID_IF: 
        {
            if (!pdsa_test::pds_if_mock_validate (expected_pds, rcvd_pds)) {
                pds_ret_status = false;
                return SDK_RET_OK;
            }
            break;
        }
        default:
            break;
        }
        ++count;
    }
    if (pds_mock->mock_pds_batch_async_fail_) {
        pds_ret_status = false;
        return SDK_RET_OK;
    }
    pds_ret_status = true;
    return SDK_RET_OK;
}

sdk_ret_t pds_batch_destroy(pds_batch_ctxt_t bctxt) {
    return SDK_RET_OK;
}

sdk_ret_t pds_if_create(pds_if_spec_s *spec,
                        pds_batch_ctxt_t bctxt) {
    auto pds_mock = (pdsa_test::pds_mock_t*) bctxt;
    if (pds_mock->mock_pds_spec_op_fail_) {
        return SDK_RET_ENTRY_EXISTS;
    }
    pds_mock->rcvd_pds.emplace_back(OBJ_ID_IF, API_OP_CREATE);
    pds_mock->rcvd_pds.back().intf = *spec;
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
    auto pds_mock = (pdsa_test::pds_mock_t*) bctxt;
    if (pds_mock->mock_pds_spec_op_fail_) {
        return SDK_RET_ENTRY_EXISTS;
    }
    pds_mock->rcvd_pds.emplace_back(OBJ_ID_IF, API_OP_UPDATE);
    pds_mock->rcvd_pds.back().intf = *spec;
    return SDK_RET_OK;
}

sdk_ret_t pds_if_delete(pds_if_key_t *key,
                        pds_batch_ctxt_t bctxt) {
    auto pds_mock = (pdsa_test::pds_mock_t*) bctxt;
    pds_mock->rcvd_pds.emplace_back(OBJ_ID_IF, API_OP_DELETE);
    pds_mock->rcvd_pds.back().intf.key = *key;
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
    return SDK_RET_OK;
}

sdk_ret_t pds_nexthop_read(pds_nexthop_key_t *key, pds_nexthop_info_t *info) {
    return SDK_RET_OK;
}

sdk_ret_t pds_nexthop_update(pds_nexthop_spec_s *spec,
                             pds_batch_ctxt_t bctxt) {
    return SDK_RET_OK;
}

sdk_ret_t pds_nexthop_delete(pds_nexthop_key_t *key,
                             pds_batch_ctxt_t bctxt) {
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
    auto pds_mock = (pdsa_test::pds_mock_t*) bctxt;
    pds_mock->rcvd_pds.emplace_back(OBJ_ID_TEP, API_OP_CREATE);
    pds_mock->rcvd_pds.back().tep = *spec;
    return SDK_RET_OK;
}

sdk_ret_t pds_tep_read(pds_tep_key_t *key, pds_tep_info_t *info) {
    return SDK_RET_OK;
}

sdk_ret_t pds_tep_update(pds_tep_spec_s *spec,
                         pds_batch_ctxt_t bctxt) {
    auto pds_mock = (pdsa_test::pds_mock_t*) bctxt;
    if (pds_mock->mock_pds_spec_op_fail_) {
        return SDK_RET_ENTRY_EXISTS;
    }
    pds_mock->rcvd_pds.emplace_back(OBJ_ID_TEP, API_OP_UPDATE);
    pds_mock->rcvd_pds.back().tep = *spec;
    return SDK_RET_OK;
}

sdk_ret_t pds_tep_delete(pds_tep_key_t *key,
                         pds_batch_ctxt_t bctxt) {
    auto pds_mock = (pdsa_test::pds_mock_t*) bctxt;
    pds_mock->rcvd_pds.emplace_back(OBJ_ID_TEP, API_OP_DELETE);
    pds_mock->rcvd_pds.back().tep.key = *key;
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

sdk_ret_t
pds_policy_delete (pds_policy_key_t *key,
                   pds_batch_ctxt_t bctxt)
{
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
    auto pds_mock = (pdsa_test::pds_mock_t*) bctxt;
    if (pds_mock->mock_pds_spec_op_fail_) {
        return SDK_RET_ENTRY_EXISTS;
    }
    pds_mock->rcvd_pds.emplace_back(OBJ_ID_NEXTHOP_GROUP, API_OP_CREATE);
    pds_mock->rcvd_pds.back().nhgroup = *spec;
    return SDK_RET_OK;
}

sdk_ret_t pds_nexthop_group_read(pds_nexthop_key_t *key, 
                                 pds_nexthop_group_info_t *info) {
    return SDK_RET_OK;
}

sdk_ret_t pds_nexthop_group_update(pds_nexthop_group_spec_s *spec,
                                   pds_batch_ctxt_t bctxt) {
    auto pds_mock = (pdsa_test::pds_mock_t*) bctxt;
    if (pds_mock->mock_pds_spec_op_fail_) {
        return SDK_RET_ENTRY_EXISTS;
    }
    pds_mock->rcvd_pds.emplace_back(OBJ_ID_NEXTHOP_GROUP, API_OP_UPDATE);
    pds_mock->rcvd_pds.back().nhgroup = *spec;
    return SDK_RET_OK;
}

sdk_ret_t pds_nexthop_group_delete(pds_nexthop_group_key_t *key,
                                   pds_batch_ctxt_t bctxt) {
    auto pds_mock = (pdsa_test::pds_mock_t*) bctxt;
    pds_mock->rcvd_pds.emplace_back(OBJ_ID_NEXTHOP_GROUP, API_OP_DELETE);
    pds_mock->rcvd_pds.back().nhgroup.key = *key;
    return SDK_RET_OK;
}

sdk_ret_t pds_subnet_create(pds_subnet_spec_s *spec,
                            pds_batch_ctxt_t bctxt) {
    return SDK_RET_OK;
}

namespace sdk {
namespace lib {
catalog* catalog::factory(std::string catalog_file_path,
                          std::string catalog_file_name,
                          platform_type_t platform)
{
    static catalog g_ctlg_;
    g_ctlg_.catalog_db_.num_fp_ports = 1;
    return &g_ctlg_;
}
catalog::~catalog() {};
} // End namespace lib
} // End namespace sdk

sdk_ret_t pds_init(pds_init_params_s *params) {
    static sdk::lib::catalog* ctlg = sdk::lib::catalog::factory();
    api::g_pds_state.set_catalog(ctlg);
    return SDK_RET_OK;
}

sdk_ret_t pds_device_delete(pds_batch_ctxt_t bctxt) {
    return SDK_RET_OK;
}

sdk_ret_t pds_teardown() {
    return SDK_RET_OK;
}

namespace sdk {
namespace platform {
int readFruKey(std::string key, std::string &value)
{
    // Fill some dummy values
    if (key == NUMMACADDR_KEY) {
        value = "25";
        return 0;
    } 
    if (key == MACADDRESS_KEY) {
        value = "00:00:cd:00:00:01";
        return 0;
    } 
    return -1;
}
}
}
namespace api {
    pds_state g_pds_state;
    pds_state::pds_state() { }
    pds_state::~pds_state() { }
}
