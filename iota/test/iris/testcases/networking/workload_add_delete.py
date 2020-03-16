#! /usr/bin/python3
import grpc
import json
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.config.netagent.api as agent_api
import iota.test.iris.testcases.security.utils as utils
import pdb

def Setup(tc):
    tc.nwObjects = agent_api.QueryConfigs(kind='Network')
    tc.epObjects = agent_api.QueryConfigs(kind='Endpoint')

    if not agent_api.__config_pushed:
        api.Logger.info("Config objects are not pushed")
        agent_api.UpdateNodeUuidEndpoints(tc.epObjects)

        api.Testbed_ResetVlanAlloc()
        vlan = api.Testbed_AllocateVlan()
        agent_api.UpdateTestBedVlans(tc.nwObjects)

    tc.iters = getattr(tc.args, 'iters', 10)
    return api.types.status.SUCCESS

def Trigger(tc):
    for i in range(0,tc.iters):
        api.Logger.info(" ################### ITER %s  ###################"%(i+1))
        ret = agent_api.DeleteConfigObjects(tc.epObjects)
        if ret != api.types.status.SUCCESS:
            api.Logger.error("Iter: %s Failed to delete network object"%(i+1))
            break
        ret = agent_api.DeleteConfigObjects(tc.nwObjects)
        if ret != api.types.status.SUCCESS:
            api.Logger.error("Iter: %s Failed to delete network object"%(i+1))
            break

        ret = agent_api.PushConfigObjects(tc.nwObjects)
        if ret != api.types.status.SUCCESS:
            api.Logger.error("Iter: %s Failed to push network object"%(i+1))
            break
        ret = agent_api.PushConfigObjects(tc.epObjects)
        if ret != api.types.status.SUCCESS:
            api.Logger.error("Iter: %s Failed to push network object"%(i+1))
            break

    tc.ret = ret
    return ret

def Verify(tc):
    api.ReSetupWorkoads(api.GetWorkloads())
    return tc.ret

def Teardown(tc):
    api.Logger.info("Tearing down ...")
    return api.types.status.SUCCESS
