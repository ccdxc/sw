#! /usr/bin/python3
import pdb
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
from iota.test.iris.utils import vmotion_utils

def __get_workload_pairs():
    pairs = []
    for w1 in api.GetWorkloads():
        for w2 in api.GetWorkloads():
            if id(w1) == id(w2) or w1.uplink_vlan != w2.uplink_vlan:
                    continue
            pairs.append((w1, w2))
    return pairs

def Setup(tc):
    tc.workload_pairs = __get_workload_pairs()
    if getattr(tc.args, 'vmotion_enable', False):
        wloads = []
        for pair in tc.workload_pairs:
            wloads.append(pair[1])
        vmotion_utils.PrepareWorkloadVMotion(tc, wloads)

    return api.types.status.SUCCESS

def Trigger(tc):
    api.Logger.info("Running IP Normalization tests ...")
    req = api.Trigger_CreateExecuteCommandsRequest(serial = False)
    tc.cmd_cookies = []

    for pair in tc.workload_pairs:
        w1 = pair[0]
        w2 = pair[1]
        cmd_cookie = "%s(%s) --> %s(%s)" % \
                (w1.workload_name, w1.ip_address, w2.workload_name, w2.ip_address)
        api.Logger.info("Injecting IP pkt with DF flag set from %s" % (cmd_cookie))
        tc.cmd_cookies.append(cmd_cookie)
        api.Trigger_AddCommand(req, w1.node_name, w1.workload_name,
                               "hping3 -c 1 -0 --dontfrag %s" % w2.ip_address)
    tc.resp = api.Trigger(req)
    return api.types.status.SUCCESS

def Verify(tc):
    if getattr(tc.args, 'vmotion_enable', False):
        vmotion_utils.PrepareWorkloadRestore(tc)

    api.Logger.info("Validating ...")
    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS
    return result

def Teardown(tc):
    api.Logger.info("Tearing down ...")
    return api.types.status.SUCCESS
