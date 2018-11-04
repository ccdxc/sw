#! /usr/bin/python3

import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.config.netagent.cfg_api as netagent_cfg_api

def Setup(tc):
    tc.loop_count = tc.args.count
    api.Logger.info("Loop count set to ", tc.loop_count)
    return api.types.status.SUCCESS

def Trigger(tc):
    api.Logger.info("Starting add-del loop")
    for x in range(tc.loop_count):
        api.Logger.info("Iteration ", x+1)
        netagent_cfg_api.AddSgPolicies()
        netagent_cfg_api.DeleteSgPolicies()
    return api.types.status.SUCCESS

def Verify(tc):
    #if tc.resp is None:
        #api.Logger.Info("Validating ...")
        #return api.types.status.FAILURE

    result = api.types.status.SUCCESS
    return result

def Teardown(tc):
    api.Logger.info("Tearing down ...")
    return api.types.status.SUCCESS
