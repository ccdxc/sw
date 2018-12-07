#! /usr/bin/python3
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.config.netagent.cfg_api as netagent_cfg_api
import iota.test.iris.config.netagent.api as agent_api
import iota.test.iris.testcases.telemetry.utils as utils
import pdb

def Setup(tc):
    tc.workload_pairs = api.GetRemoteWorkloadPairs()
    tc.workloads = api.GetWorkloads()

    netagent_cfg_api.DeleteMirrors()
    return api.types.status.SUCCESS

def Trigger(tc):
    policies = utils.GetTargetJsons(tc.iterators.proto)
    mirror_json_obj = None
    result = api.types.status.SUCCESS
    
    count = 0
    ret_count = 0
    for policy_json in policies:
        #pdb.set_trace()
        mirror_json_obj = utils.ReadJson(policy_json)
        verif_json = utils.GetVerifJsonFromPolicyJson(policy_json)
        agent_api.ConfigureMirror(mirror_json_obj.mirrors, oper = agent_api.CfgOper.ADD)
        # Get collector
        for wl in tc.workloads:
            if tc.args.collector == wl.ip_address:
                collector_wl = wl
                break
        ret = utils.RunAll(collector_wl, verif_json, tc)
        result = ret['res']
        ret_count = ret['count']
        count = count + ret_count
        agent_api.ConfigureMirror(mirror_json_obj.mirrors, oper = agent_api.CfgOper.DELETE)
        api.Logger.info("policy_json = {}, count = {}, total_count = {}".format(policy_json, ret_count, count))
    return result

def Verify(tc):
    result = api.types.status.SUCCESS
    return result

def Teardown(tc):
    api.Logger.info("Tearing down ...")
    return api.types.status.SUCCESS

