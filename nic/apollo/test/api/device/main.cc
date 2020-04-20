//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the all device test cases
///
//----------------------------------------------------------------------------

#include "nic/apollo/test/api/utils/device.hpp"
#include "nic/apollo/test/api/utils/workflow.hpp"

namespace test {
namespace api {

//----------------------------------------------------------------------------
// Device test class
//----------------------------------------------------------------------------

class device_test : public pds_test_base {
protected:
    device_test() {}
    virtual ~device_test() {}
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

//----------------------------------------------------------------------------
// Device test cases implementation
//----------------------------------------------------------------------------

/// \defgroup DEVICE_TEST Device Tests
/// @{

/// \brief Device WF_B1
/// \ref WF_B1
TEST_F(device_test, device_workflow_b1) {
    if (!apulu()) return;

    device_feeder feeder;
    feeder.init("2001:1::1", "00:02:01:00:00:01", "2001:1::2", false, false,
                0, false, PDS_DEVICE_PROFILE_DEFAULT,
                PDS_MEMORY_PROFILE_DEFAULT, PDS_DEV_OPER_MODE_HOST);
    workflow_b1<device_feeder>(feeder);
}

/// \brief Device WF_B2
/// \ref WF_B2
TEST_F(device_test, device_workflow_b2) {
    if (!apulu()) return;

    device_feeder feeder1, feeder1A;
    feeder1.init("1.0.0.1", "00:02:01:00:00:01", "1.0.0.2", false, false, 0,
                 false, PDS_DEVICE_PROFILE_DEFAULT,
                 PDS_MEMORY_PROFILE_DEFAULT, PDS_DEV_OPER_MODE_HOST);
    feeder1A.init("2.0.0.1", "00:02:01:00:0A:0B", "2.0.0.2", false, false, 0,
                  false, PDS_DEVICE_PROFILE_DEFAULT,
                  PDS_MEMORY_PROFILE_DEFAULT, PDS_DEV_OPER_MODE_HOST);
    workflow_b2<device_feeder>(feeder1, feeder1A);
}

/// \brief Device WF_1
/// \ref WF_1
TEST_F(device_test, device_workflow_1) {
    device_feeder feeder;
    feeder.init("1.0.0.1", "00:02:01:00:00:01", "1.0.0.2", false, false, 0,
                false, PDS_DEVICE_PROFILE_DEFAULT,
                PDS_MEMORY_PROFILE_DEFAULT, PDS_DEV_OPER_MODE_HOST);
    workflow_1<device_feeder>(feeder);

    if (apulu()) {
        feeder.init("2001:1::1", "00:02:01:00:00:01", "2001:1::2", false, false,
                    0, false, PDS_DEVICE_PROFILE_DEFAULT,
                    PDS_MEMORY_PROFILE_DEFAULT, PDS_DEV_OPER_MODE_HOST);
        workflow_1<device_feeder>(feeder);
    }
}

/// \brief Device WF_2
/// \ref WF_2
TEST_F(device_test, device_workflow_2) {
    device_feeder feeder;
    feeder.init("1.0.0.1", "00:02:01:00:00:01", "1.0.0.2", false, false, 0,
                false, PDS_DEVICE_PROFILE_DEFAULT,
                PDS_MEMORY_PROFILE_DEFAULT, PDS_DEV_OPER_MODE_HOST);
    workflow_2<device_feeder>(feeder);

    if (apulu()) {
        feeder.init("2001:1::1", "00:02:01:00:00:01", "2001:1::2", false, false,
                    0, false, PDS_DEVICE_PROFILE_DEFAULT,
                    PDS_MEMORY_PROFILE_DEFAULT, PDS_DEV_OPER_MODE_HOST);
        workflow_2<device_feeder>(feeder);
    }
}

/// \brief Device WF_4
/// \ref WF_4
TEST_F(device_test, device_workflow_4) {
    device_feeder feeder;
    feeder.init("1.0.0.1", "00:02:01:00:00:01", "1.0.0.2", false, false, 0,
                false, PDS_DEVICE_PROFILE_DEFAULT,
                PDS_MEMORY_PROFILE_DEFAULT, PDS_DEV_OPER_MODE_HOST);
    workflow_4<device_feeder>(feeder);

    if (apulu()) {
        feeder.init("2001:1::1", "00:02:01:00:00:01", "2001:1::2", false, false,
                    0, false, PDS_DEVICE_PROFILE_DEFAULT,
                    PDS_MEMORY_PROFILE_DEFAULT, PDS_DEV_OPER_MODE_HOST);
        workflow_4<device_feeder>(feeder);
    }
}

/// \brief Device WF_6
/// \ref WF_6
TEST_F(device_test, device_workflow_6) {
    device_feeder feeder1, feeder1A, feeder1B;
    feeder1.init("1.0.0.1", "00:02:01:00:00:01", "1.0.0.2", false, false, 0,
                 false, PDS_DEVICE_PROFILE_DEFAULT,
                 PDS_MEMORY_PROFILE_DEFAULT, PDS_DEV_OPER_MODE_HOST);
    feeder1A.init("1.0.0.1", "00:02:01:00:0A:0B", "111.0.0.111", false, false,
                  0, false, PDS_DEVICE_PROFILE_DEFAULT,
                  PDS_MEMORY_PROFILE_DEFAULT, PDS_DEV_OPER_MODE_HOST);
    feeder1B.init("1.0.0.1", "00:02:01:0A:0B:0C", "99.99.99.99", false, false,
                  0, false, PDS_DEVICE_PROFILE_DEFAULT,
                  PDS_MEMORY_PROFILE_DEFAULT, PDS_DEV_OPER_MODE_HOST);
    workflow_6<device_feeder>(feeder1, feeder1A, feeder1B);

    if (apulu()) {
        feeder1.init("2001:1::1", "00:02:01:00:00:01", "2001:1::2", false,
                     false, 0, false, PDS_DEVICE_PROFILE_DEFAULT,
                     PDS_MEMORY_PROFILE_DEFAULT, PDS_DEV_OPER_MODE_HOST);
        feeder1A.init("2001:1::1", "00:02:01:00:0A:0B", "2002:1::2", false,
                      false, 0, false, PDS_DEVICE_PROFILE_DEFAULT,
                      PDS_MEMORY_PROFILE_DEFAULT, PDS_DEV_OPER_MODE_HOST);
        feeder1B.init("2001:1::1", "00:02:01:0A:0B:0C", "2003:1::2", false,
                      false, 0, false, PDS_DEVICE_PROFILE_DEFAULT,
                      PDS_MEMORY_PROFILE_DEFAULT, PDS_DEV_OPER_MODE_HOST);
        workflow_6<device_feeder>(feeder1, feeder1A, feeder1B);
    }
}

/// \brief Device WF_7
/// \ref WF_7
TEST_F(device_test, device_workflow_7) {
    device_feeder feeder1, feeder1A, feeder1B;
    feeder1.init("1.0.0.1", "00:02:01:00:00:01", "1.0.0.2", false, false,
                 0, false, PDS_DEVICE_PROFILE_DEFAULT,
                 PDS_MEMORY_PROFILE_DEFAULT, PDS_DEV_OPER_MODE_HOST);
    feeder1A.init("1.0.0.1", "00:02:01:00:0A:0B", "111.0.0.111", false, false,
                  0, false, PDS_DEVICE_PROFILE_DEFAULT,
                  PDS_MEMORY_PROFILE_DEFAULT, PDS_DEV_OPER_MODE_HOST);
    feeder1B.init("1.0.0.1", "00:02:01:0A:0B:0C", "99.99.99.99", false, false,
                  0, false, PDS_DEVICE_PROFILE_DEFAULT,
                  PDS_MEMORY_PROFILE_DEFAULT, PDS_DEV_OPER_MODE_HOST);
    workflow_7<device_feeder>(feeder1, feeder1A, feeder1B);

    if (apulu()) {
        feeder1.init("2001:1::1", "00:02:01:00:00:01", "2001:1::2", false, false,
                     0, false, PDS_DEVICE_PROFILE_DEFAULT,
                     PDS_MEMORY_PROFILE_DEFAULT, PDS_DEV_OPER_MODE_HOST);
        feeder1A.init("2001:1::1", "00:02:01:00:0A:0B", "2002:1::2", false, false,
                      0, false, PDS_DEVICE_PROFILE_DEFAULT,
                      PDS_MEMORY_PROFILE_DEFAULT, PDS_DEV_OPER_MODE_HOST);
        feeder1B.init("2001:1::1", "00:02:01:0A:0B:0C", "2003:1::2", false, false,
                      0, false, PDS_DEVICE_PROFILE_DEFAULT,
                      PDS_MEMORY_PROFILE_DEFAULT, PDS_DEV_OPER_MODE_HOST);
        workflow_7<device_feeder>(feeder1, feeder1A, feeder1B);
    }
}

/// \brief Device WF_8
/// \ref WF_8
TEST_F(device_test, device_workflow_8) {
    if (!apulu()) return;

    device_feeder feeder1, feeder1A, feeder1B;
    feeder1.init("1.0.0.1", "00:02:01:00:00:01", "1.0.0.2", false, false, 0,
                 false, PDS_DEVICE_PROFILE_DEFAULT,
                 PDS_MEMORY_PROFILE_DEFAULT, PDS_DEV_OPER_MODE_HOST);
    feeder1A.init("1.0.0.1", "00:02:01:00:0A:0B", "111.0.0.111", false, false,
                  0, false, PDS_DEVICE_PROFILE_DEFAULT,
                  PDS_MEMORY_PROFILE_DEFAULT, PDS_DEV_OPER_MODE_HOST);
    feeder1B.init("1.0.0.1", "00:02:01:0A:0B:0C", "99.99.99.99", false, false,
                  0, false, PDS_DEVICE_PROFILE_DEFAULT,
                  PDS_MEMORY_PROFILE_DEFAULT, PDS_DEV_OPER_MODE_HOST);
    workflow_8<device_feeder>(feeder1, feeder1A, feeder1B);

    if (apulu()) {
        feeder1.init("2001:1::1", "00:02:01:00:00:01", "2001:1::2", false, false,
                     0, false, PDS_DEVICE_PROFILE_DEFAULT,
                     PDS_MEMORY_PROFILE_DEFAULT, PDS_DEV_OPER_MODE_HOST);
        feeder1A.init("2001:1::1", "00:02:01:00:0A:0B", "2002:1::2", false, false,
                      0, false, PDS_DEVICE_PROFILE_DEFAULT,
                      PDS_MEMORY_PROFILE_DEFAULT, PDS_DEV_OPER_MODE_HOST);
        feeder1B.init("2001:1::1", "00:02:01:0A:0B:0C", "2003:1::2", false, false,
                      0, false, PDS_DEVICE_PROFILE_DEFAULT,
                      PDS_MEMORY_PROFILE_DEFAULT, PDS_DEV_OPER_MODE_HOST);
        workflow_8<device_feeder>(feeder1, feeder1A, feeder1B);
    }
}

/// \brief Device WF_9
/// \ref WF_9
TEST_F(device_test, device_workflow_9) {
    if (!apulu()) return;

    device_feeder feeder1, feeder1A;
    feeder1.init("1.0.0.1", "00:02:01:00:00:01", "1.0.0.2", false, false, 0,
                 false, PDS_DEVICE_PROFILE_DEFAULT,
                 PDS_MEMORY_PROFILE_DEFAULT, PDS_DEV_OPER_MODE_HOST);
    feeder1A.init("1.0.0.1", "00:02:01:00:0A:0B", "111.0.0.111", false, false,
                  0, false, PDS_DEVICE_PROFILE_DEFAULT,
                  PDS_MEMORY_PROFILE_DEFAULT, PDS_DEV_OPER_MODE_HOST);
    workflow_9<device_feeder>(feeder1, feeder1A);

    if (apulu()) {
        feeder1.init("2001:1::1", "00:02:01:00:00:01", "2001:1::2", false,
                     false, 0, false, PDS_DEVICE_PROFILE_DEFAULT,
                     PDS_MEMORY_PROFILE_DEFAULT, PDS_DEV_OPER_MODE_HOST);
        feeder1A.init("2001:1::1", "00:02:01:00:0A:0B", "2002:1::2", false,
                      false, 0, false, PDS_DEVICE_PROFILE_DEFAULT,
                      PDS_MEMORY_PROFILE_DEFAULT, PDS_DEV_OPER_MODE_HOST);
        workflow_9<device_feeder>(feeder1, feeder1A);
    }
}

/// \brief Device WF_N_1
/// \ref WF_N_1
TEST_F(device_test, device_workflow_neg_1) {
    if (!apulu()) return;

    device_feeder feeder;
    feeder.init("1.0.0.1", "00:02:01:00:00:01", "1.0.0.2", false, false, 0,
                false, PDS_DEVICE_PROFILE_DEFAULT,
                PDS_MEMORY_PROFILE_DEFAULT, PDS_DEV_OPER_MODE_HOST);
    workflow_neg_1<device_feeder>(feeder);

    if (apulu()) {
        feeder.init("2001:1::1", "00:02:01:00:00:01", "2001:1::2", false, false,
                    0, false, PDS_DEVICE_PROFILE_DEFAULT,
                    PDS_MEMORY_PROFILE_DEFAULT, PDS_DEV_OPER_MODE_HOST);
        workflow_neg_1<device_feeder>(feeder);
    }
}

/// \brief Device WF_N_3
/// \ref WF_N_3
TEST_F(device_test, device_workflow_neg_3) {
    device_feeder feeder;
    feeder.init("1.0.0.1", "00:02:01:00:00:01", "1.0.0.2", false, false, 0,
                false, PDS_DEVICE_PROFILE_DEFAULT,
                PDS_MEMORY_PROFILE_DEFAULT, PDS_DEV_OPER_MODE_HOST);
    workflow_neg_3<device_feeder>(feeder);

    if (apulu()) {
        feeder.init("2001:1::1", "00:02:01:00:00:01", "2001:1::2", false, false,
                    0, false, PDS_DEVICE_PROFILE_DEFAULT,
                    PDS_MEMORY_PROFILE_DEFAULT, PDS_DEV_OPER_MODE_HOST);
        workflow_neg_3<device_feeder>(feeder);
    }
}

/// \brief Device WF_N_5
/// \ref WF_N_5
TEST_F(device_test, DISABLED_device_workflow_neg_5) {
    device_feeder feeder1, feeder1A;
    feeder1.init("1.0.0.1", "00:02:01:00:00:01", "1.0.0.2", false, false, 0,
                 false, PDS_DEVICE_PROFILE_DEFAULT,
                 PDS_MEMORY_PROFILE_DEFAULT, PDS_DEV_OPER_MODE_HOST);
    feeder1A.init("1.0.0.1", "00:02:01:00:0A:0B", "111.0.0.111", false, false,
                  0, false, PDS_DEVICE_PROFILE_DEFAULT,
                  PDS_MEMORY_PROFILE_DEFAULT, PDS_DEV_OPER_MODE_HOST);
    workflow_neg_5<device_feeder>(feeder1, feeder1A);

    if (apulu()) {
        feeder1.init("2001:1::1", "00:02:01:00:00:01", "2001:1::2", false,
                     false, 0, false, PDS_DEVICE_PROFILE_DEFAULT,
                     PDS_MEMORY_PROFILE_DEFAULT, PDS_DEV_OPER_MODE_HOST);
        feeder1A.init("2001:1::1", "00:02:01:00:0A:0B", "2002:1::2", false,
                      false, 0, false, PDS_DEVICE_PROFILE_DEFAULT,
                      PDS_MEMORY_PROFILE_DEFAULT, PDS_DEV_OPER_MODE_HOST);
        workflow_neg_5<device_feeder>(feeder1, feeder1A);
    }
}

//---------------------------------------------------------------------
// Non templatized test cases
//---------------------------------------------------------------------

/// \brief update device ip
TEST_F(device_test, device_update_device_ip) {
    if (!apulu()) return;

    device_feeder feeder;
    pds_device_spec_t spec = {0};

    // init
    feeder.init("1.0.0.1", "00:02:01:00:00:01", "1.0.0.2", false, false, 0,
                 false, PDS_DEVICE_PROFILE_DEFAULT,
                 PDS_MEMORY_PROFILE_DEFAULT, PDS_DEV_OPER_MODE_HOST);
    device_create(feeder);

    // trigger
    spec.device_ip_addr = feeder.spec.device_ip_addr;
    increment_ip_addr(&spec.device_ip_addr, 1);
    device_update(feeder, &spec, DEVICE_ATTR_DEVICE_IP);

    // validate
    device_read(feeder, SDK_RET_OK);

    // cleanup
    device_delete(feeder);
    device_read(feeder, SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update device mac
TEST_F(device_test, device_update_device_mac) {
    if (!apulu()) return;

    device_feeder feeder;
    pds_device_spec_t spec = {0};
    uint64_t mac;

    // init
    feeder.init("1.0.0.1", "00:02:01:00:00:01", "1.0.0.2", false, false, 0,
                 false, PDS_DEVICE_PROFILE_DEFAULT,
                 PDS_MEMORY_PROFILE_DEFAULT, PDS_DEV_OPER_MODE_HOST);
    device_create(feeder);

    // trigger
    mac = MAC_TO_UINT64(feeder.spec.device_mac_addr);
    mac = mac + 1;
    MAC_UINT64_TO_ADDR(spec.device_mac_addr, mac);
    device_update(feeder, &spec, DEVICE_ATTR_DEVICE_MAC);

    // validate
    device_read(feeder, SDK_RET_OK);

    // cleanup
    device_delete(feeder);
    device_read(feeder, SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update gateway ip
TEST_F(device_test, device_update_gateway_ip) {
    if (!apulu()) return;

    device_feeder feeder;
    pds_device_spec_t spec = {0};

    // init
    feeder.init("1.0.0.1", "00:02:01:00:00:01", "1.0.0.2", false, false, 0,
                 false, PDS_DEVICE_PROFILE_DEFAULT,
                 PDS_MEMORY_PROFILE_DEFAULT, PDS_DEV_OPER_MODE_HOST);
    device_create(feeder);

    // trigger
    spec.gateway_ip_addr = feeder.spec.gateway_ip_addr;
    increment_ip_addr(&spec.gateway_ip_addr, 1);
    device_update(feeder, &spec, DEVICE_ATTR_GATEWAY_IP);

    // validate
    device_read(feeder, SDK_RET_OK);

    // cleanup
    device_delete(feeder);
    device_read(feeder, SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update bridging enable flag
TEST_F(device_test, device_update_bridging_en) {
    if (!apulu()) return;

    device_feeder feeder;
    pds_device_spec_t spec = {0};

    // init
    feeder.init("1.0.0.1", "00:02:01:00:00:01", "1.0.0.2", false, false, 0,
                 false, PDS_DEVICE_PROFILE_DEFAULT,
                 PDS_MEMORY_PROFILE_DEFAULT, PDS_DEV_OPER_MODE_HOST);
    device_create(feeder);

    // trigger
    spec.bridging_en = true;
    device_update(feeder, &spec, DEVICE_ATTR_BRIDGING_EN);

    // validate
    device_read(feeder, SDK_RET_OK);

    // cleanup
    device_delete(feeder);
    device_read(feeder, SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update learning enable flag
TEST_F(device_test, device_update_learning_en) {
    if (!apulu()) return;

    device_feeder feeder;
    pds_device_spec_t spec = {0};

    // init
    feeder.init("1.0.0.1", "00:02:01:00:00:01", "1.0.0.2", false, false, 0,
                 false, PDS_DEVICE_PROFILE_DEFAULT,
                 PDS_MEMORY_PROFILE_DEFAULT, PDS_DEV_OPER_MODE_HOST);
    device_create(feeder);

    // trigger
    spec.learning_en = true;
    device_update(feeder, &spec, DEVICE_ATTR_LEARNING_EN);

    // validate
    device_read(feeder, SDK_RET_OK);

    // cleanup
    device_delete(feeder);
    device_read(feeder, SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update learning age timeout
TEST_F(device_test, device_update_learn_age_timeout) {
    if (!apulu()) return;

    device_feeder feeder;
    pds_device_spec_t spec = {0};

    // init
    feeder.init("1.0.0.1", "00:02:01:00:00:01", "1.0.0.2", false, false, 0,
                 false, PDS_DEVICE_PROFILE_DEFAULT,
                 PDS_MEMORY_PROFILE_DEFAULT, PDS_DEV_OPER_MODE_HOST);
    device_create(feeder);

    // trigger
    spec.learn_age_timeout = feeder.spec.learn_age_timeout + 1;
    device_update(feeder, &spec, DEVICE_ATTR_LEARN_AGE_TIME_OUT);

    // validate
    device_read(feeder, SDK_RET_OK);

    // cleanup
    device_delete(feeder);
    device_read(feeder, SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update overlay routing enable flag
TEST_F(device_test, device_update_overlay_routing_en) {
    if (!apulu()) return;

    device_feeder feeder;
    pds_device_spec_t spec = {0};

    // init
    feeder.init("1.0.0.1", "00:02:01:00:00:01", "1.0.0.2", false, false, 0,
                 false, PDS_DEVICE_PROFILE_DEFAULT,
                 PDS_MEMORY_PROFILE_DEFAULT, PDS_DEV_OPER_MODE_HOST);
    device_create(feeder);

    // trigger
    spec.overlay_routing_en = true;
    device_update(feeder, &spec, DEVICE_ATTR_OVERLAY_ROUTING_EN);

    // validate
    device_read(feeder, SDK_RET_OK);

    // cleanup
    device_delete(feeder);
    device_read(feeder, SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update device profile
TEST_F(device_test, device_update_device_profile) {
    if (!apulu()) return;

    device_feeder feeder;
    pds_device_spec_t spec = {0};

    // init
    feeder.init("1.0.0.1", "00:02:01:00:00:01", "1.0.0.2", false, false, 0,
                 false, PDS_DEVICE_PROFILE_DEFAULT,
                 PDS_MEMORY_PROFILE_DEFAULT, PDS_DEV_OPER_MODE_HOST);
    device_create(feeder);

    // trigger
    spec.device_profile = PDS_DEVICE_PROFILE_2PF;
    device_update(feeder, &spec, DEVICE_ATTR_DEVICE_PROFILE);

    // validate
    device_read(feeder, SDK_RET_OK);

    // cleanup
    device_delete(feeder);
    device_read(feeder, SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update memory profile
TEST_F(device_test, device_update_memory_profile) {
    if (!apulu()) return;

    device_feeder feeder;
    pds_device_spec_t spec = {0};

    // init
    feeder.init("1.0.0.1", "00:02:01:00:00:01", "1.0.0.2", false, false, 0,
                 false, PDS_DEVICE_PROFILE_DEFAULT,
                 PDS_MEMORY_PROFILE_DEFAULT, PDS_DEV_OPER_MODE_HOST);
    device_create(feeder);

    // trigger
    // nochange as of now we have only one profile
    spec.memory_profile = PDS_MEMORY_PROFILE_DEFAULT;
    device_update(feeder, &spec, DEVICE_ATTR_MEMORY_PROFILE);

    // validate
    device_read(feeder, SDK_RET_OK);

    // cleanup
    device_delete(feeder);
    device_read(feeder, SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update device oper mode
TEST_F(device_test, device_update_dev_oper_mode) {
    if (!apulu()) return;

    device_feeder feeder;
    pds_device_spec_t spec = {0};

    // init
    feeder.init("1.0.0.1", "00:02:01:00:00:01", "1.0.0.2", false, false, 0,
                 false, PDS_DEVICE_PROFILE_DEFAULT,
                 PDS_MEMORY_PROFILE_DEFAULT, PDS_DEV_OPER_MODE_HOST);
    device_create(feeder);

    // trigger
    spec.dev_oper_mode = PDS_DEV_OPER_MODE_BITW;
    device_update(feeder, &spec, DEVICE_ATTR_DEV_OPER_MODE);

    // validate
    device_read(feeder, SDK_RET_OK);

    // cleanup
    device_delete(feeder);
    device_read(feeder, SDK_RET_ENTRY_NOT_FOUND);
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
