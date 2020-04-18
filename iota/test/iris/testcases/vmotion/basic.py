#! /usr/bin/python3
import pdb

import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
from iota.harness.infra.exceptions import *

def Setup(tc):
    tc.skip = False

    tc.workload_pairs = api.GetRemoteWorkloadPairs()

    if len(tc.workload_pairs) == 0:
        api.Logger.info("Skipping Testcase due to no workload pairs.")
        tc.skip = True

    if api.GetNicMode() != 'hostpin_dvs':
        api.Logger.info("Skipping Testcase: as vmotion nonly on dvs testsuite")
        tc.skip = True

    return api.types.status.SUCCESS

def Trigger(tc):
    if tc.skip: return api.types.status.SUCCESS

    pair1 = tc.workload_pairs[0]
    pair2 = tc.workload_pairs[2]

    # Move pair1 in as-is
    w1 = pair1[0]
    w2 = pair1[1]
    w3 = pair2[0]
    w4 = pair2[1]
    tc.w1_orig_node = w1.node_name
    tc.w2_orig_node = w2.node_name
    tc.w3_orig_node = w3.node_name

    req = api.Trigger_WorkloadMoveRequest()

    api.Trigger_WorkloadMoveAddRequest(req, [w1], w2.node_name, abort_time=20)
    api.Trigger_WorkloadMoveAddRequest(req, [w2], w1.node_name, abort_time=10)
    api.Trigger_WorkloadMoveAddRequest(req, [w3], w4.node_name)

    tc.resp = api.TriggerMove(req)

    return tc.resp

def Verify(tc):
    if tc.skip: return api.types.status.SUCCESS
    if tc.resp is None:
        return api.types.status.FAILURE

    pair1 = tc.workload_pairs[0]
    pair2 = tc.workload_pairs[2]

    # Pair1 move aborted
    w1 = pair1[0]
    w2 = pair1[1]
    w3 = pair2[0]
    if tc.w1_orig_node != w1.node_name:
        return api.types.status.FAILURE

    if tc.w2_orig_node != w2.node_name:
        return api.types.status.FAILURE

    # w3 moved
    if tc.w3_orig_node == w3.node_name:
        return api.types.status.FAILURE

    store_wloads = api.GetWorkloads()
    for s_wl in store_wloads:
        if w1.workload_name == s_wl.workload_name:
            if tc.w1_orig_node != s_wl.node_name:
                return api.types.status.FAILURE
            break

    for s_wl in store_wloads:
        if w2.workload_name == s_wl.workload_name:
            if tc.w2_orig_node != s_wl.node_name:
                return api.types.status.FAILURE
            break

    req = api.Trigger_WorkloadMoveRequest()
    api.Trigger_WorkloadMoveAddRequest(req, [w3], tc.w3_orig_node)
    tc.resp = api.TriggerMove(req)

    return tc.resp

def Teardown(tc):
    return api.types.status.SUCCESS

