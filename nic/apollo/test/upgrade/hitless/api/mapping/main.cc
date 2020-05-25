//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the mapping upgrade test cases
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/apollo/api/include/pds_subnet.hpp"
#include "nic/apollo/api/include/pds_vnic.hpp"
#include "nic/apollo/test/api/utils/api_base.hpp"
#include "nic/apollo/test/api/utils/base.hpp"
#include "nic/apollo/test/api/utils/batch.hpp"
#include "nic/apollo/test/api/utils/device.hpp"
#include "nic/apollo/test/api/utils/local_mapping.hpp"
#include "nic/apollo/test/api/utils/policer.hpp"
#include "nic/apollo/test/api/utils/subnet.hpp"
#include "nic/apollo/test/api/utils/vnic.hpp"
#include "nic/apollo/test/api/utils/vpc.hpp"
#include "nic/apollo/test/api/utils/nexthop.hpp"
#include "nic/apollo/test/api/utils/if.hpp"
#include "nic/apollo/test/api/utils/utils.hpp"
#include "nic/apollo/test/api/utils/workflow.hpp"

namespace test {
namespace api {

//#define SCALE_DOWN_FOR_DEBUG

//----------------------------------------------------------------------------
// Mapping upg test class
//----------------------------------------------------------------------------

class mapping_upg_test : public pds_test_base {
protected:
    mapping_upg_test() {}
    virtual ~mapping_upg_test() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
    static void SetUpTestCase() {
        if (!agent_mode())
            pds_test_base::SetUpTestCase(g_tc_params);
    }
    static void TearDownTestCase() {
        if (!agent_mode())
            pds_test_base::TearDownTestCase();
    } 
};

static inline void
mapping_upg_setup (void)
{
    pds_batch_ctxt_t bctxt = batch_start();
    sample_device_setup(bctxt);
    sample_vpc_setup(bctxt, PDS_VPC_TYPE_TENANT);
    sample1_subnet_setup(bctxt);
    sample_if_setup(bctxt);
    sample_nexthop_setup(bctxt);
    sample_policer_setup(bctxt);
    vnic_feeder vnic_feeder;
    vnic_feeder.init(int2pdsobjkey(1), int2pdsobjkey(1), k_max_vnic,
                     0x000000030b020a01, PDS_ENCAP_TYPE_DOT1Q,
                     PDS_ENCAP_TYPE_VXLAN, true, false, 0, 0, 5, 0,
                     int2pdsobjkey(20010), int2pdsobjkey(20000));
    many_create(bctxt, vnic_feeder);
    batch_commit(bctxt);
}

static inline void
mapping_upg_teardown (void)
{
    pds_batch_ctxt_t bctxt = batch_start();
    vnic_feeder vnic_feeder;
    vnic_feeder.init(int2pdsobjkey(1), int2pdsobjkey(1), k_max_vnic,
                     0x000000030b020a01, PDS_ENCAP_TYPE_DOT1Q,
                     PDS_ENCAP_TYPE_VXLAN, true, false, 0, 0, 5, 0,
                     int2pdsobjkey(20010), int2pdsobjkey(20000));
    many_delete(bctxt, vnic_feeder);
    sample_policer_teardown(bctxt);
    sample_nexthop_teardown(bctxt);
    sample_if_teardown(bctxt);
    sample1_subnet_teardown(bctxt);
    sample_vpc_teardown(bctxt, PDS_VPC_TYPE_TENANT);
    sample_device_teardown(bctxt);
    batch_commit(bctxt);
}

//----------------------------------------------------------------------------
// Mapping test cases implementation
//----------------------------------------------------------------------------

/// \defgroup MAPPING_UPG_TEST Mapping upg tests
/// @{

// --------------------------- LOCAL MAPPINGS -----------------------
/// \brief Local mapping WF_U_1
/// \ref WF_U_1
TEST_F(mapping_upg_test, local_mapping_workflow_u1) {
    local_mapping_feeder feeder;

    pds_batch_ctxt_t bctxt = batch_start();
    // setup precursor
    mapping_upg_setup();
    // setup local mapping
    feeder.init(k_vpc_key, k_subnet_key, "10.0.0.2/8",
                0x000000030b020a01, PDS_ENCAP_TYPE_VXLAN,
                pdsobjkey2int(k_subnet_key) + 512,
                int2pdsobjkey(1), true, "12.0.0.0/16",
                k_max_vnic, PDS_MAX_VNIC_IP, PDS_MAPPING_TYPE_L3,
                PDS_MAX_TAGS_PER_MAPPING, true);
    // backup
    workflow_u1_s1<local_mapping_feeder>(feeder);

    // tearup precursor
    mapping_upg_teardown();
    // restore
    workflow_u1_s2<local_mapping_feeder>(feeder);

    // setup precursor again
    mapping_upg_setup();
    // config replay
    workflow_u1_s3<local_mapping_feeder>(feeder);
    // tearup precursor
    mapping_upg_teardown();
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
    api_test_program_run(argc, argv);
}
