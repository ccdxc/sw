#! /usr/bin/python3
import grpc
import json
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.config.netagent.api as agent_api
import iota.test.iris.testcases.security.utils as utils
import iota.test.iris.utils.ip_rule_db.rule_db.rule_db as db
import iota.test.iris.utils.ip_rule_db.test as test
from iota.test.iris.testcases.aging.aging_utils import *
import pdb
import random
import sys

def FilterAndAlter(pkt, src_w, dst_w, **kwargs):
    '''
    This function is argument to @RunAll method.
    Given packet header fields, source workload and destination
    workload, it applies some logic and returns Boolean
    as filter result. Based on business logic, it can also modify the
    packet header fields
    '''
    pkt["sip"] = src_w.ip_address
    pkt["dip"] = dst_w.ip_address
    return False

def setupWorkloadDict(tc):
    workload_pairs = api.GetRemoteWorkloadPairs()
    workload_pairs.extend(api.GetLocalWorkloadPairs())
    naples_node_name_list = api.GetNaplesHostnames()
    w_list = []

    for w1,w2 in workload_pairs:
        if w1.node_name in naples_node_name_list or \
           w2.node_name in naples_node_name_list:
            w_list.append((w1,w2))
            api.Logger.info("Adding Naples workload w1: %s and w2: %s to list"%
                            (w1.workload_name, w2.workload_name))
        else:
            api.Logger.debug("Skipping non Naples workload w1: %s and w2: %s"%
                             (w1.workload_name, w2.workload_name))
    tc.workload_dict = workload_dict = {}
    for w1,w2 in w_list:
        l = workload_dict.get(w1, [])
        l.append(w2)
        workload_dict[w1] = l

    for w1, dst_workload_list in tc.workload_dict.items():
        l = [w2.workload_name for w2 in dst_workload_list]
        api.Logger.info("%s => %s"%(w1.workload_name, ",".join(l)))

def Setup(tc):
    setupWorkloadDict(tc)
    agent_api.DeleteSgPolicies()
    tc.scale = len(tc.workload_dict) \
               if tc.iterators.scale < len(tc.workload_dict) \
                  else tc.iterators.scale

    result = test.execute()
    api.Logger.info("ip rule table module sanity check - %s"%
                    ("SUCCESS" if result else "FAIL"))
    if not result:
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Trigger(tc):
    policies = utils.GetTargetJsons(tc.iterators.proto)
    sg_json_obj = None

    # Generate the random seed
    seed = random.randrange(sys.maxsize)
    api.Logger.info("Seed val: %s"%seed)

    for policy_json in policies:
        sg_json_obj = utils.ReadJson(policy_json)
        newObjects = agent_api.AddOneConfig(policy_json)
        agent_api.DeleteConfigObjects(newObjects)
        tc.ret = agent_api.PushConfigObjects(newObjects)
        rule_db_map = utils.SetupLocalRuleDbPerNaple(policy_json)
        if tc.ret != api.types.status.SUCCESS:
            return api.types.status.FAILURE

        while tc.scale > 0:
            for w1, dst_workload_list in tc.workload_dict.items():
                seed += 1
                for w2 in dst_workload_list:

                    # If src and dst workload are behind same Naples,
                    # then only one db sees the packet.
                    if w1.node_name == w2.node_name:
                        w1_db = rule_db_map.get(w1.node_name, None)
                        w2_db = None
                    else:
                        w1_db = rule_db_map.get(w1.node_name, None)
                        w2_db = rule_db_map.get(w2.node_name, None)

                    api.Logger.info("Running between w1: %s(%s) and w2: %s(%s)"%
                                    (w1.ip_address, w1.workload_name, w2.ip_address,
                                     w2.workload_name))
                    utils.RunAll(1, w1, w2, w1_db, w2_db, FilterAndAlter, seed=seed)

                    tc.scale -= 1

                utils.clearNaplesSessions(w1.node_name)

        for node,db in rule_db_map.items():
            result = utils.compareStats(db, node, tc.iterators.proto)
            api.Logger.info("Comparison of rule stats for Node %s - %s"%
                            (node, "SUCCESS" \
                             if result == api.types.status.SUCCESS \
                             else "FAIL"))
            if result != api.types.status.SUCCESS:
                agent_api.DeleteConfigObjects(newObjects)
                agent_api.RemoveConfigObjects(newObjects)
                tc.ret = result
                return tc.ret

        agent_api.DeleteConfigObjects(newObjects)
        agent_api.RemoveConfigObjects(newObjects)

    return api.types.status.SUCCESS

def Verify(tc):
    return tc.ret

def Teardown(tc):
    api.Logger.info("Tearing down ...")
    #policy_json = "{}/sgpolicy.json".format(api.GetTopologyDirectory())
    #sg_json_obj = utils.ReadJson(policy_json)
    newObjects = agent_api.QueryConfigs(kind='NetworkSecurityPolicy')
    agent_api.PushConfigObjects(newObjects)

    return api.types.status.SUCCESS
