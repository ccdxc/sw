#! /usr/bin/python3
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.config.netagent.api as agent_api
import iota.test.iris.testcases.telemetry.utils as utils
import iota.test.iris.config.netagent.hw_sec_ip_config as sec_ip_api

from collections import defaultdict

is_wl_type_bm = False
wl_sec_ip_info = defaultdict(lambda: dict())

def Setup(tc):
    global is_wl_type_bm
    global wl_sec_ip_info
    tc.workload_pairs = api.GetRemoteWorkloadPairs()
    tc.workloads = api.GetWorkloads()

    for wl in tc.workloads:
        wl_sec_ip_info[wl.workload_name] = []
        # for BM type set untag collector
        if api.IsBareMetalWorkloadType(wl.node_name):
            is_wl_type_bm = True
        sec_ip_list = sec_ip_api.ConfigWorkloadSecondaryIp(wl, True, 2)
        wl_sec_ip_info[wl.workload_name] = sec_ip_list

    #for wl in tc.workloads:
    #    for sec_ip in wl_sec_ip_info[wl.workload_name]:
    #        api.Logger.info("Node: {} WL: {} Sec IP: {}".format(wl.node_name, wl.workload_name, sec_ip))

    #tc.skip = True
    return api.types.status.SUCCESS

def Trigger(tc):
    #if tc.skip: return api.types.status.SUCCESS

    policies = utils.GetTargetJsons('mirror', tc.iterators.proto)
    result = api.types.status.SUCCESS
    
    count = 0
    ret_count = 0
    collector_dest = []
    collector_wl = []
    collector_type = []
    for policy_json in policies:
        collector_dest.clear()
        collector_wl.clear()
        collector_type.clear()

        verif_json = utils.GetVerifJsonFromPolicyJson(policy_json)
        api.Logger.info("Using policy_json = {}".format(policy_json))
        newObjects = agent_api.AddOneConfig(policy_json)
        if len (newObjects) == 0:
            api.Logger.error("Adding new objects to store failed")
            return api.types.status.FAILURE
        ret = agent_api.PushConfigObjects(newObjects)
        if ret != api.types.status.SUCCESS:
            api.Logger.error("Unable to push mirror objects")
            return api.types.status.FAILURE
        utils.DumpMirrorSessions()

        # Get collector to find the workload
        for obj in newObjects:
            for obj_collector in obj.spec.collectors:
                coll_dst = obj_collector.export_config.destination
                coll_type = obj_collector.type
                collector_dest.append(coll_dst)
                collector_type.append(coll_type)
                api.Logger.info(f"export-dest: {coll_dst}, erspan-type: {coll_type}")

        for coll_dst in collector_dest:
            for wl in tc.workloads:
                if (wl.ip_address == coll_dst) or (coll_dst in wl_sec_ip_info[wl.workload_name]):
                    collector_wl.append(wl)

        api.Logger.info("collect_dest len: {} collect_wl len: {}".format(len(collector_dest), len(collector_wl)))
        collector_info = utils.GetMirrorCollectorsInfo(collector_wl, collector_dest, collector_type)
        ret = utils.RunAll(tc, verif_json, 'mirror', collector_info, is_wl_type_bm)
        result = ret['res']
        ret_count = ret['count']
        count = count + ret_count

        if result != api.types.status.SUCCESS:
            api.Logger.info("policy_json = {}, Encountered FAILURE, stopping".format(policy_json))
            # Delete the objects
            agent_api.DeleteConfigObjects(newObjects)
            agent_api.RemoveConfigObjects(newObjects)
            break

        # Update collector
        newObjects = agent_api.QueryConfigs(kind='MirrorSession')
        # mirror config update to local collector is applicable only for ESX topology
        if is_wl_type_bm is False:
            for obj in newObjects:
                if obj.spec.collectors[0].type == utils.ERSPAN_TYPE_2:
                    obj.spec.collectors[0].type = utils.ERSPAN_TYPE_3
                    collector_info[0]['type'] = utils.ERSPAN_TYPE_3
                else:
                    obj.spec.collectors[0].type = utils.ERSPAN_TYPE_2
                    collector_info[0]['type'] = utils.ERSPAN_TYPE_2
                break

            # Now push the update as we modified
            agent_api.UpdateConfigObjects(newObjects)
            utils.DumpMirrorSessions()

            # Rerun the tests
            ret = utils.RunAll(tc, verif_json, 'mirror', collector_info, is_wl_type_bm)
            result = ret['res']
            ret_count = ret['count']
            count = count + ret_count

        # Delete the objects
        agent_api.DeleteConfigObjects(newObjects)
        agent_api.RemoveConfigObjects(newObjects)
        api.Logger.info("policy_json = {}, count = {}, total_count = {}".format(policy_json, ret_count, count))
        if result != api.types.status.SUCCESS:
            api.Logger.info("policy_json = {}, Encountered FAILURE, stopping".format(policy_json))
            break
    tc.SetTestCount(count)
    collector_dest.clear()
    collector_wl.clear()
    return result

def Verify(tc):
    result = api.types.status.SUCCESS
    return result

def Teardown(tc):
    api.Logger.info("Tearing down ...")
    for wl in tc.workloads:
        sec_ip_api.ConfigWorkloadSecondaryIp(wl, False, 2)
        wl_sec_ip_info[wl.workload_name].clear()
    return api.types.status.SUCCESS

