//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
#include "nic/apollo/agent/hooks.hpp"
#include "nic/apollo/test/flow_test/flow_test.hpp"

using namespace hooks;

extern "C" {

sdk_ret_t
g_agent_hooks_func (agent_op_t op, void *obj, void *arg)
{
    sdk_ret_t ret = SDK_RET_OK;
    static test_params_t test_params;
    static flow_test *flow_test_obj;

    if (op == INIT_DONE) {
        char *pipeline = (char *)obj;
        parse_test_cfg(&test_params, pipeline);
        test_params.pipeline = pipeline;
        flow_test_obj = new flow_test(&test_params);
        flow_test_obj->set_cfg_params(test_params.dual_stack,
                                      test_params.num_tcp,
                                      test_params.num_udp,
                                      test_params.num_icmp,
                                      test_params.sport_lo,
                                      test_params.sport_hi,
                                      test_params.dport_lo,
                                      test_params.dport_hi);
#if defined(ARTEMIS)
        flow_test_obj->set_session_info_cfg_params(
            test_params.num_vpcs, test_params.num_ip_per_vnic,
            test_params.num_remote_mappings, test_params.meter_scale,
            TESTAPP_METER_NUM_PREFIXES, test_params.num_nh,
            TESTAPP_MAX_SERVICE_TEP, TESTAPP_MAX_REMOTE_SERVICE_TEP);
#endif
    } else if (op == BATCH_START) {
        pds_epoch_t *epoch = (pds_epoch_t *)obj;
        if (*epoch == PDS_EPOCH_INVALID) {
            ret = flow_test_obj->create_flows();
        }
    } else if (op == LOCAL_MAPPING_CREATE) {
        flow_test_obj->add_local_ep((pds_local_mapping_spec_t *)obj);
    } else if (op == REMOTE_MAPPING_CREATE) {
        flow_test_obj->add_remote_ep((pds_remote_mapping_spec_t *)obj);
    }
    return ret;
}

}     // extern "C"
