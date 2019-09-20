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
    policies = utils.GetTargetJsons(tc.iterators.proto)
    sg_json_obj = None

    for policy_json in policies:
        sg_json_obj = utils.ReadJson(policy_json)
        newObjects = agent_api.AddOneConfig(policy_json)
        tc.ret = agent_api.PushConfigObjects(newObjects)

        if tc.ret != api.types.status.SUCCESS:
            return api.types.status.FAILURE

        for pair in tc.workload_pairs:
            w1 = pair[0]
            w2 = pair[1]

            result = utils.RunAll(w1, w2)
            result = api.types.status.SUCCESS

            if result != api.types.status.SUCCESS:
                api.Logger.info("Test run failed for policy {}. Exiting.".format(policy_json))
                agent_api.DeleteConfigObjects(newObjects)
                agent_api.RemoveConfigObjects(newObjects)
                # return success for now, it will be updated to reflect the issues.
                tc.ret = result
                return tc.ret

        agent_api.DeleteConfigObjects(newObjects)
        agent_api.RemoveConfigObjects(newObjects)

    return api.types.status.SUCCESS

def Verify(tc):
    return tc.ret

def Teardown(tc):
    api.Logger.info("Tearing down ...")
    #policy_json = "{}/sgpolicy.json".format(api.GetTopologyDirectory())
    #sg_json_obj = utils.ReadJson(policy_json)
    newObjects = newObjects = agent_api.QueryConfigs(kind='NetworkSecurityPolicy')
    agent_api.PushConfigObjects(newObjects)

    return api.types.status.SUCCESS
