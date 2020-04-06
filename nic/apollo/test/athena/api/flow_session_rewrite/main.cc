
//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the all flow session rewrite test cases for athena
///
//----------------------------------------------------------------------------
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/test/api/utils/base.hpp"
#include "nic/apollo/api/include/athena/pds_flow_session_rewrite.h"
#include "nic/apollo/test/athena/api/flow_session_rewrite/utils.hpp"

namespace test {
namespace api {

//----------------------------------------------------------------------------
// SESSION REWRITE test class
//----------------------------------------------------------------------------

class flow_session_rewrite_test : public pds_test_base {
protected:
    flow_session_rewrite_test() {}
    virtual ~flow_session_rewrite_test() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
    static void SetUpTestCase() {
        pds_test_base::SetUpTestCase(g_tc_params);
    }
    static void TearDownTestCase() {
        pds_test_base::TearDownTestCase();
    }
};

//----------------------------------------------------------------------------
// Flow session rewrite test cases implementation
//----------------------------------------------------------------------------

/// \defgroup FLOW SESSION REWRITE Flow session rewrite tests
/// @{

/// \brief Flow session rewrite tests
TEST_F(flow_session_rewrite_test, flow_session_rewrite_crud) {
    pds_flow_session_rewrite_spec_t spec = { 0 };
    pds_flow_session_rewrite_key_t key = { 0 };
    pds_flow_session_rewrite_info_t info = { 0 };

    spec.key.session_rewrite_id = 2;
    fill_data(&spec.data, 2, REWRITE_NAT_TYPE_IPV4_SNAT, ENCAP_TYPE_L2);
    SDK_ASSERT(pds_flow_session_rewrite_create(&spec) == 
            PDS_RET_OK);

    key.session_rewrite_id = 2;
    SDK_ASSERT(pds_flow_session_rewrite_read(&key, &info) == 
            PDS_RET_OK);
    SDK_ASSERT(memcmp(&info.spec.data, &spec.data,
               sizeof(pds_flow_session_rewrite_data_t)) == 0);

    memset(&spec, 0, sizeof(spec));
    spec.key.session_rewrite_id = 5;
    fill_data(&spec.data, 5, REWRITE_NAT_TYPE_IPV4_DNAT, ENCAP_TYPE_MPLSOUDP);
    SDK_ASSERT(pds_flow_session_rewrite_create(&spec) == 
            PDS_RET_OK);

    memset(&info, 0, sizeof(info));
    key.session_rewrite_id = 5;
    SDK_ASSERT(pds_flow_session_rewrite_read(&key, &info) == 
            PDS_RET_OK);
    SDK_ASSERT(memcmp(&info.spec.data, &spec.data,
               sizeof(pds_flow_session_rewrite_data_t)) == 0);

    key.session_rewrite_id = 5;
    SDK_ASSERT(pds_flow_session_rewrite_delete(&key) == PDS_RET_OK);

    memset(&info, 0, sizeof(info));
    key.session_rewrite_id = 5;
    // FIXME: need a method to distinguish between no encap and no entry case
    SDK_ASSERT(pds_flow_session_rewrite_read(&key, &info) ==
               PDS_RET_ERR);

    memset(&spec, 0, sizeof(spec));
    spec.key.session_rewrite_id = 20;
    fill_data(&spec.data, 20, REWRITE_NAT_TYPE_IPV6_SNAT, ENCAP_TYPE_MPLSOUDP);
    SDK_ASSERT(pds_flow_session_rewrite_create(&spec) == 
            PDS_RET_OK);

    memset(&info, 0, sizeof(info));
    key.session_rewrite_id = 20;
    SDK_ASSERT(pds_flow_session_rewrite_read(&key, &info) == 
            PDS_RET_OK);
    SDK_ASSERT(memcmp(&info.spec.data, &spec.data,
               sizeof(pds_flow_session_rewrite_data_t)) == 0);

    memset(&spec, 0, sizeof(spec));
    spec.key.session_rewrite_id = 32;
    fill_data(&spec.data, 32, REWRITE_NAT_TYPE_NONE, ENCAP_TYPE_L2);
    SDK_ASSERT(pds_flow_session_rewrite_create(&spec) == 
            PDS_RET_OK);

    memset(&info, 0, sizeof(info));
    key.session_rewrite_id = 32;
    SDK_ASSERT(pds_flow_session_rewrite_read(&key, &info) == 
            PDS_RET_OK);
    SDK_ASSERT(memcmp(&info.spec.data, &spec.data,
               sizeof(pds_flow_session_rewrite_data_t)) == 0);

    memset(&spec, 0, sizeof(spec)); 
    spec.key.session_rewrite_id = PDS_FLOW_SESSION_REWRITE_ID_MAX;
    fill_data(&spec.data, PDS_FLOW_SESSION_REWRITE_ID_MAX,
              REWRITE_NAT_TYPE_NONE, ENCAP_TYPE_L2);
    SDK_ASSERT(pds_flow_session_rewrite_create(&spec) == 
            PDS_RET_INVALID_ARG);

    memset(&info, 0, sizeof(info));
    key.session_rewrite_id = 0;
    fill_data(&spec.data, 0, REWRITE_NAT_TYPE_NONE, ENCAP_TYPE_L2);
    SDK_ASSERT(pds_flow_session_rewrite_read(&key, &info) == 
            PDS_RET_INVALID_ARG);

    memset(&spec, 0, sizeof(spec)); 
    spec.key.session_rewrite_id = 11;
    fill_data(&spec.data, 11, REWRITE_NAT_TYPE_IPV4_SDPAT, ENCAP_TYPE_GENEVE);
    SDK_ASSERT(pds_flow_session_rewrite_create(&spec) == PDS_RET_OK);

    memset(&info, 0, sizeof(info));
    key.session_rewrite_id = 11;
    SDK_ASSERT(pds_flow_session_rewrite_read(&key, &info) == 
            PDS_RET_OK);
    SDK_ASSERT(memcmp(&info.spec.data, &spec.data,
               sizeof(pds_flow_session_rewrite_data_t)) == 0);

    memset(&spec, 0, sizeof(spec)); 
    spec.key.session_rewrite_id = 11;
    fill_data(&spec.data, 11, REWRITE_NAT_TYPE_IPV4_DNAT, ENCAP_TYPE_L2);
    SDK_ASSERT(pds_flow_session_rewrite_update(&spec) == 
            PDS_RET_OK);

    memset(&info, 0, sizeof(info));
    key.session_rewrite_id = 11;
    SDK_ASSERT(pds_flow_session_rewrite_read(&key, &info) == 
            PDS_RET_OK);
    SDK_ASSERT(memcmp(&info.spec.data, &spec.data,
               sizeof(pds_flow_session_rewrite_data_t)) == 0);

    memset(&spec, 0, sizeof(spec)); 
    spec.key.session_rewrite_id = 24;
    fill_data(&spec.data, 24, REWRITE_NAT_TYPE_MAX, ENCAP_TYPE_GENEVE);
    SDK_ASSERT(pds_flow_session_rewrite_create(&spec) == 
            PDS_RET_INVALID_ARG);

    memset(&spec, 0, sizeof(spec)); 
    spec.key.session_rewrite_id = 34;
    fill_data(&spec.data, 34, REWRITE_NAT_TYPE_IPV6_SNAT, ENCAP_TYPE_MAX);
    SDK_ASSERT(pds_flow_session_rewrite_create(&spec) == 
            PDS_RET_INVALID_ARG);
}

/// @}

}    // namespace api
}    // namespace test

//----------------------------------------------------------------------------
// Entry point
//----------------------------------------------------------------------------

/// @private
int
main (int argc, char **argv)
{
    return api_test_program_run(argc, argv);
}
