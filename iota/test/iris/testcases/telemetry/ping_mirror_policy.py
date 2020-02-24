#! /usr/bin/python3
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.config.netagent.api as agent_api
import iota.test.iris.testcases.telemetry.utils as utils
import pdb

is_wl_type_bm = False
collector_dest = None

def Setup(tc):
    global is_wl_type_bm
    global collector_dest
    tc.workload_pairs = api.GetRemoteWorkloadPairs()
    tc.workloads = api.GetWorkloads()

    collector_dest = tc.args.collector
    for wl in tc.workloads:
        # for BM type set untag collector
        if api.IsBareMetalWorkloadType(wl.node_name):
            is_wl_type_bm = True
            collector_dest = "10.255.0.2"
            break

    if (is_wl_type_bm == True) and (tc.iterators.proto != "icmp"):
        api.Logger.info("Skipping {} test for baremetal workloads".format(tc.iterators.proto))
        tc.skip = True
        return api.types.status.SUCCESS

    #agent_api.DeleteMirrors()
    return api.types.status.SUCCESS

def Trigger(tc):
    global is_wl_type_bm
    global collector_dest
    policies = utils.GetTargetJsons('mirror', tc.iterators.proto)
    mirror_json_obj = None
    result = api.types.status.SUCCESS
    
    count = 0
    ret_count = 0
    for policy_json in policies:
        #pdb.set_trace()
        #mirror_json_obj = utils.ReadJson(policy_json)
        #agent_api.ConfigureMirror(mirror_json_obj.mirrors, oper = agent_api.CfgOper.ADD)

        verif_json = utils.GetVerifJsonFromPolicyJson(policy_json)
        newObjects = agent_api.AddOneConfig(policy_json)
        if len (newObjects) == 0:
            api.Logger.error("Adding new objects to store failed")
            return api.types.status.FAILURE
        ret = agent_api.PushConfigObjects(newObjects)
        if ret != api.types.status.SUCCESS:
            api.Logger.error("Unable to push mirror objects")
            return api.types.status.FAILURE

        # Get collector
        for wl in tc.workloads:
            if collector_dest == wl.ip_address:
                collector_wl = wl
                break
        ret = utils.RunAll(collector_wl, verif_json, tc, 'mirror')
        result = ret['res']
        ret_count = ret['count']
        count = count + ret_count

        # Update collector
        newObjects = agent_api.QueryConfigs(kind='MirrorSession')

        if is_wl_type_bm == False:
            collector_dest = "192.168.100.102"
        # Get new collector
        for wl in tc.workloads:
            if wl.ip_address == collector_dest:
                collector_wl = wl
                break

        for obj in newObjects:
            obj.spec.collectors[0].export_config.destination = collector_dest
        # Now push the update as we modified
        agent_api.UpdateConfigObjects(newObjects)
        # Rerun the tests
        ret = utils.RunAll(collector_wl, verif_json, tc, 'mirror')
        result = ret['res']
        ret_count = ret['count']
        count = count + ret_count

        # Delete the objects
        agent_api.DeleteConfigObjects(newObjects)
        agent_api.RemoveConfigObjects(newObjects)
        #agent_api.ConfigureMirror(mirror_json_obj.mirrors, oper = agent_api.CfgOper.DELETE)
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

