#! /usr/bin/python3
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.config.netagent.api as agent_api
import iota.test.iris.testcases.telemetry.utils as utils
import pdb


def Setup(tc):
    tc.workload_pairs = api.GetRemoteWorkloadPairs()
    tc.workloads = api.GetWorkloads()

    #agent_api.DeleteMirrors()
    return api.types.status.SUCCESS

def Trigger(tc):
    policies = utils.GetTargetJsons('flowmon', tc.iterators.proto)
    mirror_json_obj = None
    result = api.types.status.SUCCESS
    
    count = 0
    ret_count = 0
    for policy_json in policies:
        collector_dest = None
        #pdb.set_trace()
        verif_json = utils.GetVerifJsonFromPolicyJson(policy_json)
        newObjects = agent_api.AddOneConfig(policy_json)
        if len (newObjects) == 0:
            api.Logger.error("Adding new objects to store failed")
            return api.types.status.FAILURE
        ret = agent_api.PushConfigObjects(newObjects)
        if ret != api.types.status.SUCCESS:
            api.Logger.error("Unable to push flowmon objects")
            return api.types.status.FAILURE

        # Get collector to find the workload
        for obj in newObjects:
            collector_dest = obj.spec.exports[0].destination
            api.Logger.info("export-dest: {} ".format(collector_dest))

        # Get collector
        for wl in tc.workloads:
            if collector_dest == wl.ip_address:
                collector_wl = wl
                break
        ret = utils.RunAll(collector_wl, verif_json, tc, 'flowmon')
        result = ret['res']
        ret_count = ret['count']
        count = count + ret_count
        agent_api.DeleteConfigObjects(newObjects)
        agent_api.RemoveConfigObjects(newObjects)
        api.Logger.info("policy_json = {}, count = {}, total_count = {}".format(policy_json, ret_count, count))
        if result != api.types.status.SUCCESS:
            api.Logger.info("policy_json = {}, Encountered FAILURE, stopping".format(policy_json))
            break
    tc.SetTestCount(count)
    return result

def Verify(tc):
    result = api.types.status.SUCCESS
    return result

def Teardown(tc):
    api.Logger.info("Tearing down ...")
    return api.types.status.SUCCESS

