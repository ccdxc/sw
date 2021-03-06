#! /usr/bin/python3
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.config.netagent.api as agent_api
import iota.test.iris.testcases.telemetry.utils as utils
import iota.test.iris.config.netagent.hw_sec_ip_config as sec_ip_api
import iota.test.iris.testcases.penctl.common as common
import json
import copy
import random
import ipaddress
from collections import defaultdict

is_wl_type_bm = False
wl_sec_ip_info = defaultdict(lambda: dict())
num_collector_at_create = 1

def Setup(tc):
    global is_wl_type_bm
    global wl_sec_ip_info
    tc.workload_pairs = api.GetRemoteWorkloadPairs()
    tc.workloads = api.GetWorkloads()
    result = api.types.status.SUCCESS

    for wl in tc.workloads:
        wl_sec_ip_info[wl.workload_name] = []
        # for BM type set untag collector
        if api.IsBareMetalWorkloadType(wl.node_name):
            is_wl_type_bm = True
        sec_ip_list = sec_ip_api.ConfigWorkloadSecondaryIp(wl, True, tc.iterators.num_collectors)
        wl_sec_ip_info[wl.workload_name] = sec_ip_list
        #for item in sec_ip_list:
        #    api.Logger.info("WL: {} ip: {} ".format(wl.workload_name, item))

    tc.skip = False
    return result


def ConfigMirrorSession(tc, num_collectors, mirror_spec_objects):
    result = api.types.status.SUCCESS

    iteration = 0
    tc.test_iterator_data = defaultdict(lambda: dict())

    for wl in tc.workload_pairs:
        w1 = wl[0]
        w2 = wl[1]

        peer_wl = w1
        local_wl = w2
        if api.IsNaplesNode(w1.node_name):
            peer_wl = w2
            local_wl = w1
        elif api.IsNaplesNode(w2.node_name) != True:
            #one has to be naples
            continue
        if utils.checkIfWorkloadsInSameSubnet(w1,w2) == False:
            #workloads has to be in same subnet to trigger ping
            continue

        api.Logger.info("updated Match rule spec for W1: {} W2: {}, proto: {}, port: {}".format(
                    w1.ip_address, w2.ip_address, tc.iterators.proto, tc.port))

        clonedObject = agent_api.CloneConfigObjects(mirror_spec_objects)
        if len(clonedObject) == 0:
            api.Logger.info("Failed to clone object for iteration: {}".format(iteration))
            result = api.types.status.FAILURE
            break

        utils.generateMirrorSpecConfig(iteration, w1, w2, tc.iterators.proto, tc.port, clonedObject)

        (coll_wl_list, coll_ip_list, coll_type) = utils.updateMirrorCollectorConfig(tc.workloads,
                                  num_collectors, local_wl, wl_sec_ip_info, clonedObject)

        if len(coll_ip_list) == 0:
            api.Logger.error("Unable to find a valid collector")
            result = api.types.status.FAILURE
            break

        result = agent_api.PushConfigObjects(clonedObject, [local_wl.node_name])

        #agent_api.PrintConfigObjects(clonedObject)
        if result != api.types.status.SUCCESS:
            api.Logger.error("Unable to push mirror objects")
            result = api.types.status.FAILURE
            break

        api.Logger.info("collect_wl len: {} collect_dest len: {} ".format(len(coll_wl_list), len(coll_ip_list)))

        tc.test_iterator_data[iteration] = defaultdict(lambda: dict())
        tc.test_iterator_data[iteration]['local_wl'] = local_wl
        tc.test_iterator_data[iteration]['peer_wl'] = peer_wl
        tc.test_iterator_data[iteration]['coll_wl_list'] = coll_wl_list
        tc.test_iterator_data[iteration]['coll_ip_list'] = coll_ip_list
        tc.test_iterator_data[iteration]['coll_type'] = coll_type
        tc.test_iterator_data[iteration]['del_obj'] = clonedObject

        iteration = iteration + 1
        if iteration == tc.iterators.num_mirror_sessions:
            break

    return (result)

def updateMirrorSessionCollectors(tc, num_collectors):
    iteration = 0
    result = api.types.status.SUCCESS
    for iteration in range(tc.iterators.num_mirror_sessions):
        mirror_objects = tc.test_iterator_data[iteration]['del_obj']
        local_wl = tc.test_iterator_data[iteration]['local_wl']
        peer_wl = tc.test_iterator_data[iteration]['peer_wl']
        coll_wl_list = tc.test_iterator_data[iteration]['coll_wl_list']
        coll_ip_list = tc.test_iterator_data[iteration]['coll_ip_list']
        coll_type    = tc.test_iterator_data[iteration]['coll_type']

        utils.generateMirrorCollectorConfig(mirror_objects, num_collectors)

        if num_collectors > len(coll_ip_list):
            api.Logger.info("Collector IP list {} is smaller than requested collectors {} "
                    "for this test! bailing out!!!".format(len(coll_ip_list),num_collectors))
            result = api.types.status.FAILURE
            break

        for obj in mirror_objects:
            old_coll_count = num_collector_at_create
            #api.Logger.info("old_coll_count {} ".format(old_coll_count))
            for c in range(old_coll_count, num_collectors):
                obj.spec.collectors[c].export_config.destination = "{}".format(coll_ip_list[c])
                #api.Logger.info("Updating collector at index {} dst ip {}".format(c, coll_ip_list[c]))

        result = agent_api.UpdateConfigObjects(mirror_objects, [local_wl.node_name])
        #agent_api.PrintConfigObjects(mirror_objects)

        if result != api.types.status.SUCCESS:
            api.Logger.error("Unable to push updates to mirror objects")
            result = api.types.status.FAILURE
            break
    return (result)



def InjectTestTrafficAndValidateCapture(tc, num_mirror_sessions, collector_count):
    ret_count = 0
    ret = {}
    result = api.types.status.SUCCESS
    for iteration in range(num_mirror_sessions):
        local_wl = tc.test_iterator_data[iteration]['local_wl']
        peer_wl = tc.test_iterator_data[iteration]['peer_wl']
        coll_wl_list = tc.test_iterator_data[iteration]['coll_wl_list']
        coll_ip_list = tc.test_iterator_data[iteration]['coll_ip_list']
        coll_type    = tc.test_iterator_data[iteration]['coll_type']
        api.Logger.info("Running traffic validation iteration: {} {}-->{}".format(iteration,
                    local_wl.workload_name, peer_wl.workload_name))

        utils.setSourceWorkloadsUpLinkVlan(local_wl.uplink_vlan)
        collector_info = utils.GetMirrorCollectorsInfo(coll_wl_list, coll_ip_list, coll_type)
        result = utils.RunCmd(local_wl, tc.iterators.proto, peer_wl,
                              peer_wl.ip_address, tc.port, collector_info, 'mirror', is_wl_type_bm)
        if result != api.types.status.SUCCESS:
            api.Logger.error("Failed in packet validation at collector: {}".format(iteration))
            break

        ret_count += 2

    ret['res'] = result
    ret['count'] = ret_count

    return ret


def Trigger(tc):
    #if tc.skip: return api.types.status.SUCCESS

    result = api.types.status.SUCCESS
    count = 0

    policies = utils.GetTargetJsons('mirror', 'crud')
    for policy_json in policies:
        api.Logger.info("Policy File: {}".format(policy_json))
        mirror_spec_objects = agent_api.AddOneConfig(policy_json)
        if len (mirror_spec_objects) == 0:
            api.Logger.info("Policy object len {}".format(len(mirror_spec_objects)))
            continue
        verif_json = utils.GetVerifJsonFromPolicyJson(policy_json)
        #create mirror sessions with 1 collector
        utils.generateMirrorCollectorConfig(mirror_spec_objects, num_collector_at_create)
        port = random.randint(100, 10000)
        tc.port = port

        result  = ConfigMirrorSession(tc, num_collector_at_create, mirror_spec_objects)
        if result != api.types.status.SUCCESS:
            api.Logger.info("Failed in Mirror session configuration")
            agent_api.RemoveConfigObjects(mirror_spec_objects)
            break

        ret = InjectTestTrafficAndValidateCapture(tc, tc.iterators.num_mirror_sessions, num_collector_at_create)

        result = ret['res']
        ret_count = ret['count']
        count = count + ret_count

        if (result == api.types.status.SUCCESS) and (tc.iterators.num_collectors > num_collector_at_create):
            #update mirror sessions with num_collectors
            result = updateMirrorSessionCollectors(tc, tc.iterators.num_collectors)
            if result != api.types.status.SUCCESS:
                api.Logger.info("Failed in Mirror session configuration")
            else:
                ret = InjectTestTrafficAndValidateCapture(tc, tc.iterators.num_mirror_sessions, tc.iterators.num_collectors)
                result = ret['res']
                ret_count = ret['count']
                count = count + ret_count

        #remove all but one mirror session and check the collectors are not deleted
        if (result == api.types.status.SUCCESS):
            for iteration in range(1, tc.iterators.num_mirror_sessions):
                obj = tc.test_iterator_data[iteration]['del_obj']
                agent_api.DeleteConfigObjects(obj)
                agent_api.RemoveConfigObjects(obj)
                tc.test_iterator_data[iteration] = {}

            ret = InjectTestTrafficAndValidateCapture(tc, 1, tc.iterators.num_collectors)
            result = ret['res']
            ret_count = ret['count']
            count = count + ret_count

        for iteration in range(tc.iterators.num_mirror_sessions):
            if tc.test_iterator_data[iteration]:
                obj = tc.test_iterator_data[iteration]['del_obj']
                agent_api.DeleteConfigObjects(obj)
                agent_api.RemoveConfigObjects(obj)
                tc.test_iterator_data[iteration] = {}

        agent_api.RemoveConfigObjects(mirror_spec_objects)

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
    for wl in tc.workloads:
        sec_ip_api.ConfigWorkloadSecondaryIp(wl, False, tc.iterators.num_collectors)
        wl_sec_ip_info[wl.workload_name].clear()
    if tc.skip: return api.types.status.SUCCESS
    return api.types.status.SUCCESS

