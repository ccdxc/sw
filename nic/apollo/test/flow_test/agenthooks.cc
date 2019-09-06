//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#include "nic/apollo/agent/hooks.hpp"
#include "nic/apollo/test/flow_test/flow_test.hpp"

using namespace hooks;

extern "C" {

test_params_t g_test_params;
flow_test *g_flow_test_obj;

sdk_ret_t
g_agent_hooks_func (agent_op_t op, void *obj, void *arg)
{
    sdk_ret_t ret = SDK_RET_OK;
    const char *pipeline;

    if (op == INIT_DONE) {
        g_flow_test_obj = new flow_test();
    } else if (op == BATCH_START) {
        pds_epoch_t *epoch = (pds_epoch_t *)obj;
        switch (*epoch) {
        case 1:
            // first call from testapp
            // parse the cfg file now
            pipeline = pipeline_get().c_str();
            memset(&g_test_params, 0, sizeof(test_params_t));
            parse_test_cfg(&g_test_params, pipeline);
            g_test_params.pipeline = pipeline;
            g_flow_test_obj->set_cfg_params(&g_test_params,
                           g_test_params.dual_stack,
                           g_test_params.num_tcp,
                           g_test_params.num_udp,
                           g_test_params.num_icmp,
                           g_test_params.sport_lo,
                           g_test_params.sport_hi,
                           g_test_params.dport_lo,
                           g_test_params.dport_hi);
#if defined(ARTEMIS)
            g_flow_test_obj->set_session_info_cfg_params(
                g_test_params.num_vpcs, g_test_params.num_ip_per_vnic,
                g_test_params.num_remote_mappings, g_test_params.meter_scale,
                TESTAPP_METER_NUM_PREFIXES, g_test_params.num_nh,
                TESTAPP_MAX_SERVICE_TEP, TESTAPP_MAX_REMOTE_SERVICE_TEP);
#endif
            break;

        case PDS_EPOCH_INVALID:
            ret = g_flow_test_obj->create_flows();
            break;

        default:
            break;
        }
    } else if (op == LOCAL_MAPPING_CREATE) {
        g_flow_test_obj->add_local_ep((pds_local_mapping_spec_t *)obj);
    } else if (op == REMOTE_MAPPING_CREATE) {
        g_flow_test_obj->add_remote_ep((pds_remote_mapping_spec_t *)obj);
    }
    return ret;
}

}     // extern "C"
