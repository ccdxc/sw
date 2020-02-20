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

    req = api.Trigger_WorkloadMoveRequest()

    for pair in tc.workload_pairs:
        w1 = pair[0]
        w2 = pair[1]
        api.Trigger_WorkloadMoveAddRequest(req, [w1], w2.node_name)
        api.Trigger_WorkloadMoveAddRequest(req, [w2], w1.node_name)
        break

    tc.resp = api.TriggerMove(req)


    return api.types.status.SUCCESS

def Verify(tc):
    if tc.skip: return api.types.status.SUCCESS
    if tc.resp is None:
        return api.types.status.FAILURE
    
    return tc.resp


def Teardown(tc):
    return api.types.status.SUCCESS
