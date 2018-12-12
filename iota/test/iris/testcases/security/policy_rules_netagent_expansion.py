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
    sg_json_obj = None
    tc.time_matrix = []
    tc.policy_json = "{}/{}_expansion_policy.json".format(utils.GetProtocolDirectory("netagent-expansion"), tc.iterators.apps)

    api.Logger.info("Running test for {}".format(tc.policy_json))
    sg_json_obj = utils.ReadJson(tc.policy_json)
    start = time.time()
    agent_api.ConfigureSecurityGroupPolicies(sg_json_obj.sgpolicies, oper = agent_api.CfgOper.ADD)
    end = time.time()
    agent_api.ConfigureSecurityGroupPolicies(sg_json_obj.sgpolicies, oper = agent_api.CfgOper.DELETE)
    tc.res_time = end - start

    if tc.res_time > 300 :
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Verify(tc):
    if tc.res_time > 180 :
        api.Logger.info("Test Failed for config {}. It took {} seconds.".format(tc.policy_json, tc.res_time))
        return api.types.status.FAILURE

    return api.types.status.SUCCESS 

def Teardown(tc):
    api.Logger.info("Tearing down ...")
    policy_json = "{}/sgpolicy.json".format(api.GetTopologyDirectory())
    sg_json_obj = utils.ReadJson(policy_json)
    agent_api.ConfigureSecurityGroupPolicies(sg_json_obj.sgpolicies, oper = agent_api.CfgOper.ADD)
    return api.types.status.SUCCESS
