#! /usr/bin/python3
import grpc
import json
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.config.netagent.api as agent_api
import iota.test.iris.testcases.security.utils as utils
import pdb
import time

def Setup(tc):
    tc.workload_pairs = api.GetRemoteWorkloadPairs()
    agent_api.DeleteSgPolicies()

    return api.types.status.SUCCESS

def Trigger(tc):
    policies = utils.GetTargetJsons("netagent-expansion")
    sg_json_obj = None

    for policy_json in policies:
        sg_json_obj = utils.ReadJson(policy_json)
        newObjects = agent_api.AddOneConfig(policy_json)
        start = time.time()
        tc.ret = agent_api.PushConfigObjects(newObjects)
        end = time.time()
        diff = end - start

        if diff > 120:
            api.Logger.info("Time taken to push configs is {} seconds.")
            tc.ret = api.types.status.FAILURE

        agent_api.DeleteConfigObjects(newObjects)
        agent_api.RemoveConfigObjects(newObjects)

        if tc.ret == api.types.status.FAILURE:
            return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Verify(tc):
    return tc.ret

def Teardown(tc):
    api.Logger.info("Tearing down ...")
    newObjects = newObjects = agent_api.QueryConfigs(kind='NetworkSecurityPolicy')
    agent_api.PushConfigObjects(newObjects)

    return api.types.status.SUCCESS
