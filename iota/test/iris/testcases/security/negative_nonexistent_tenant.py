#! /usr/bin/python3
import grpc
import json
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.config.netagent.cfg_api as netagent_cfg_api
import iota.test.iris.config.netagent.objects.sgpolicy as sgpolicy_obj
import iota.test.iris.config.netagent.api as agent_api
import iota.test.iris.testcases.security.utils as utils
import pdb
import random

def Setup(tc):
    tc.loop_count = 0
    
    tc.nodes = api.GetNaplesHostnames()
    netagent_cfg_api.DeleteSgPolicies(
    #netagent_cfg_api.ReadJsons()
    tc.sgpolicy = sgpolicy_obj.gl_sg_json_template
        
    return api.types.status.SUCCESS

def Trigger(tc):
    api.Logger.info("Starting")
    #get rules list - it is list of dictionary
    policy_rules = tc.sgpolicy['sgpolicies'][0]['spec']['policy-rules']
    meta = tc.sgpolicy['sgpolicies'][0]['meta']
    meta['name'] = 'default_policies'
    meta['namespace'] = 'default'
    meta['tenant'] = 'idontexist'
    print('Initial policy rules {}'.format(policy_rules))
    print(api.GetWorkloads())
    tc.workload_pairs = api.GetLocalWorkloadPairs()
    print(tc.workload_pairs)
    for pair in tc.workload_pairs:
        w1 = pair[0]
        w2 = pair[1]
        port = str(random.randint(1,65535))
        action = random.choice(utils.action_list)
        rule = utils.Rule((w1, w2, "udp", port),(action))

    print(tc.sgpolicy)
    json_str   = json.dumps(tc.sgpolicy)
    obj = api.parser.ParseJsonStream(json_str)
    agent_api.ConfigureSecurityGroupPolicies(obj.sgpolicies, oper = agent_api.CfgOper.ADD)
    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE
    result = api.types.status.SUCCESS
    return api.types.status.SUCCESS

def Teardown(tc):
    api.Logger.info("Tearing down ...")
    policy_json = "{}/sgpolicy.json".format(api.GetTopologyDirectory())
    sg_json_obj = utils.ReadJson(policy_json)
    agent_api.ConfigureSecurityGroupPolicies(sg_json_obj.sgpolicies, oper = agent_api.CfgOper.ADD)
    return api.types.status.SUCCESS
