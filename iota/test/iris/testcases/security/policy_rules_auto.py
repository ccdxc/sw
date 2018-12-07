#! /usr/bin/python3
import grpc
import json
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.config.netagent.cfg_api as netagent_cfg_api
import iota.test.iris.config.netagent.api as agent_api
import iota.test.iris.testcases.security.utils as utils
import pdb
import time


def Setup(tc):
    tc.workload_pairs = api.GetRemoteWorkloadPairs()

    netagent_cfg_api.DeleteSgPolicies()
    return api.types.status.SUCCESS

def Trigger(tc):
    policies = utils.GetTargetJsons(tc.iterators.proto)
    sg_json_obj = None
    tc.time_matrix = []

    for policy_json in policies:
        api.Logger.info("Running test for {}".format(policy_json))
        sg_json_obj = utils.ReadJson(policy_json)
        start = time.time()
        agent_api.ConfigureSecurityGroupPolicies(sg_json_obj.sgpolicies, oper = agent_api.CfgOper.ADD)
        end = time.time()
        agent_api.ConfigureSecurityGroupPolicies(sg_json_obj.sgpolicies, oper = agent_api.CfgOper.DELETE)
        res_time = {}
        res_time["policy"] = policy_json
        res_time["time"] = end - start
        tc.time_matrix.append(res_time)

        api.Logger.info("For policy {} the time taken is {}".format(policy_json, end - start))

    return api.types.status.SUCCESS

def Verify(tc):
    result = api.types.status.SUCCESS
    api.Logger.info("Result is {}".format(tc.time_matrix))
    return result

def Teardown(tc):
    api.Logger.info("Tearing down ...")
    return api.types.status.SUCCESS
