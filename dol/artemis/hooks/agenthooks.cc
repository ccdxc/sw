//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
#include "nic/apollo/agent/hooks.hpp"
#include "agenthooks.hpp"

using namespace hooks;

extern "C" {

sdk_ret_t
g_agent_hooks_func (agent_op_t op, void *obj, void *arg)
{
    sdk_ret_t ret = SDK_RET_OK;
    static flow_test *flow_test_obj;
    static bool flow_test_cfg_done = false;

    if (op == INIT_DONE) {
        flow_test_obj = new flow_test();
    } else if (op == BATCH_START) {
        pds_epoch_t *epoch = (pds_epoch_t *)obj;
        if (*epoch == PDS_EPOCH_INVALID) {
            ret = flow_test_obj->create_flows();
        }
        if (!flow_test_cfg_done) {
            flow_test_obj->read_config();
            flow_test_cfg_done = true;
        }
    } else if (op == LOCAL_MAPPING_CREATE) {
        flow_test_obj->add_local_ep((pds_local_mapping_spec_t *)obj);
    } else if (op == REMOTE_MAPPING_CREATE) {
        flow_test_obj->add_remote_ep((pds_remote_mapping_spec_t *)obj);
    } else if (op == ROUTE_TABLE_CREATE) {
        flow_test_obj->add_route_table((pds_route_table_spec_t *)obj);
    } else if (op == SUBNET_CREATE) {
        flow_test_obj->add_subnet((pds_subnet_spec_t *)obj);
    } else if (op == SVC_MAPPING_CREATE) {
        flow_test_obj->add_svc_mapping((pds_svc_mapping_spec_t *)obj);
    }
    return ret;
}

}     // extern "C"
