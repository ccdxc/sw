#! /usr/bin/python3
import grpc
import json
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.config.netagent.cfg_api as netagent_cfg_api
import iota.test.iris.config.netagent.api as agent_api
import iota.test.iris.testcases.security.utils as utils
import pdb

def Setup(tc):
    tc.workload_pairs = api.GetRemoteWorkloadPairs()

    netagent_cfg_api.DeleteSgPolicies()
    return api.types.status.SUCCESS

def Trigger(tc):

    api.Logger.info("BARUN TRIGGER PROTO = {} PAIRS {}".format(tc.iterators.proto, tc.workload_pairs))
    policies = utils.GetTargetJsons(tc.iterators.proto)
    sg_json_obj = None

    for policy_json in policies:
        sg_json_obj = utils.ReadJson(policy_json)
        verif_json = utils.GetVerifJsonFromPolicyJson(policy_json)
        agent_api.ConfigureSecurityGroupPolicies(sg_json_obj.sgpolicies, oper = agent_api.CfgOper.ADD)
        for pair in tc.workload_pairs:
            w1 = pair[0]
            w2 = pair[1]
            result = utils.RunAll(w1, w2, verif_json)
            if result != api.types.status.SUCCESS:
                return api.types.status.FAILURE
        agent_api.ConfigureSecurityGroupPolicies(sg_json_obj.sgpolicies, oper = agent_api.CfgOper.DELETE)

    return api.types.status.FAILURE

def Verify(tc):
    result = api.types.status.SUCCESS
    return result

def Teardown(tc):
    api.Logger.info("Tearing down ...")
    return api.types.status.SUCCESS
