#! /usr/bin/python3
import grpc
import json
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.config.netagent.api as agent_api
import iota.test.iris.testcases.security.utils as utils
import pdb

def Setup(tc):
    tc.workload_pairs = api.GetRemoteWorkloadPairs()
    agent_api.DeleteSgPolicies()

    return api.types.status.SUCCESS

def Trigger(tc):
    newObjects = agent_api.QueryConfigs(kind='NetworkSecurityPolicy')
    ret = api.types.status.SUCCESS

    for i in range(0,100):
        ret = agent_api.PushConfigObjects(newObjects)
        agent_api.DeleteConfigObjects(newObjects)

        if ret != api.types.status.SUCCESS:
             break        

    agent_api.RemoveConfigObjects(newObjects)
    tc.ret = ret
    return ret

def Verify(tc):
    return tc.ret

def Teardown(tc):
    api.Logger.info("Tearing down ...")
    policy_json = "{}/sgpolicy.json".format(api.GetTopologyDirectory())
    sg_json_obj = utils.ReadJson(policy_json)
    newObjects = agent_api.AddOneConfig(policy_json)
    agent_api.PushConfigObjects(newObjects)

    return api.types.status.SUCCESS
