#! /usr/bin/python3
import grpc
import json
import pdb
import random
import sys
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.config.netagent.api as agent_api
import iota.test.iris.testcases.security.utils as utils
import iota.test.iris.utils.ip_rule_db.rule_db.rule_db as db
import iota.test.iris.utils.ip_rule_db.test as test
import iota.test.iris.utils.ip_rule_db.util.proto as proto_util
from iota.test.iris.testcases.aging.aging_utils import *

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

def copyTestCommand(tc):
    test_reject_file = os.path.join(os.path.dirname(__file__),
                                    "test_reject.py")

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    path_to_copy = "test_reject_script"
    for w in tc.workload_dict.keys():
        api.Logger.info("Copying file %s to %s:%s"%
                        (test_reject_file, w.workload_name,
                         path_to_copy))
        resp = api.CopyToWorkload(w.node_name, w.workload_name,
                                  [test_reject_file], path_to_copy)

        if resp is None:
            api.Logger.error("Failed to copy test reject command to %s"%w.workload_name)
            return False
        api.Trigger_AddCommand(req, w.node_name, w.workload_name,
                               "yes | mv %s/test_reject.py /usr/local/bin"%(path_to_copy))
        api.Trigger_AddCommand(req, w.node_name, w.workload_name,
                               "yes | chmod 755 /usr/local/bin/test_reject.py")

    trig_resp = api.Trigger(req)
    for cmd in trig_resp.commands:
        api.PrintCommandResults(cmd)

    return True

def addPktFltrRuleOnEp(tc, enable=True):
    '''
    On endpoint, REJECT rule is installed to prevent TCP RST
    response to TCP SYN packets. This ensures the
    accurate rule hit counting.

    NOTE: By default iptables sends 'port-unreachable' for REJECT action.
    '''
    req = api.Trigger_CreateExecuteCommandsRequest(serial = False)
    for w in tc.workload_dict.keys():
        api.Trigger_AddCommand(req, w.node_name, w.workload_name,
                               "iptables -%s INPUT -p tcp -i eth1 -j REJECT"%
                               ("A" if enable else "D"))

    trig_resp = api.Trigger(req)
    result = 0
    for cmd in trig_resp.commands:
        api.PrintCommandResults(cmd)
        result |= cmd.exit_code

    return False if result else True

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
    tc.scale = tc.iterators.scale
    result = test.execute()
    api.Logger.info("ip rule table module sanity check - %s"%
                    ("SUCCESS" if result else "FAIL"))
    if not result:
        return api.types.status.FAILURE

    if not copyTestCommand(tc):
        return api.types.status.FAILURE

    if not addPktFltrRuleOnEp(tc, enable=True):
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

        scale = 0
        while scale < tc.scale :
            for w1, dst_workload_list in tc.workload_dict.items():
                for w2 in dst_workload_list:
                    seed += 1
                    if scale >= tc.scale :
                        break

                    # If src and dst workload are behind same Naples,
                    # then only one db sees the packet.
                    if w1.node_name == w2.node_name:
                        w1_db = rule_db_map.get(w1.node_name, None)
                        w2_db = None
                    else:
                        w1_db = rule_db_map.get(w1.node_name, None)
                        w2_db = rule_db_map.get(w2.node_name, None)

                    api.Logger.info("(%s/%s) Running between w1: %s(%s) and w2: %s(%s)"%
                                    (scale+1, tc.scale, w1.ip_address, w1.workload_name,
                                     w2.ip_address,
                                     w2.workload_name))
                    tc.ret = utils.RunAll(1, w1, w2, w1_db, w2_db, FilterAndAlter, seed=seed)
                    if tc.ret != api.types.status.SUCCESS:
                        agent_api.DeleteConfigObjects(newObjects)
                        agent_api.RemoveConfigObjects(newObjects)
                        return tc.ret

                    scale += 1
            utils.clearNaplesSessions()

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

        if agent_api.DeleteConfigObjects(newObjects):
            api.Logger.error("Failed to delete config object for %s"%policy_json)

        if agent_api.RemoveConfigObjects(newObjects):
            api.Logger.error("Failed to remove config object for %s"%policy_json)

    return api.types.status.SUCCESS

def Verify(tc):
    return tc.ret

def Teardown(tc):
    api.Logger.info("Tearing down ...")
    #policy_json = "{}/sgpolicy.json".format(api.GetTopologyDirectory())
    #sg_json_obj = utils.ReadJson(policy_json)
    newObjects = agent_api.QueryConfigs(kind='NetworkSecurityPolicy')
    agent_api.PushConfigObjects(newObjects)
    addPktFltrRuleOnEp(tc, enable=False)
    return api.types.status.SUCCESS
