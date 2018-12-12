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
    sip = getattr(tc.iterators, "sip", 1)
    dip = getattr(tc.iterators, "dip", 1)

    for i in range(1, 9):
        policy_json = "{}/{}_{}_{}_expansion_policy.json".format(utils.GetProtocolDirectory("halcfg-expansion"), sip, dip, i)
        res = {}

        api.Logger.info("Running test for {}".format(policy_json))
        sg_json_obj = utils.ReadJson(policy_json)
        start = time.time()
        agent_api.ConfigureSecurityGroupPolicies(sg_json_obj.sgpolicies, oper = agent_api.CfgOper.ADD)
        end = time.time()
        agent_api.ConfigureSecurityGroupPolicies(sg_json_obj.sgpolicies, oper = agent_api.CfgOper.DELETE)
        res_time = end - start

        res["policy"] = policy_json 
        res["time"] = res_time
        tc.time_matrix.append(res)

        if res_time > 300:
            return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Verify(tc):
    for res in tc.time_matrix:
        if res["time"] > 180 :
            api.Logger.info("Test Failed for config {}. It took {} seconds.".format(res["policy"], res["time"]))
            return api.types.status.FAILURE

    return api.types.status.SUCCESS 

def Teardown(tc):
    api.Logger.info("Tearing down ...")
    policy_json = "{}/sgpolicy.json".format(api.GetTopologyDirectory())
    sg_json_obj = utils.ReadJson(policy_json)
    agent_api.ConfigureSecurityGroupPolicies(sg_json_obj.sgpolicies, oper = agent_api.CfgOper.ADD)
    return api.types.status.SUCCESS
