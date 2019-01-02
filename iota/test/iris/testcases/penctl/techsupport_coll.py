#! /usr/bin/python3
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.protos.pygen.types_pb2 as types_pb2
import iota.test.iris.testcases.penctl.penctldefs as penctldefs
import iota.test.iris.testcases.penctl.common as common
import iota.test.iris.verif.utils.collect_logs_and_cores as collect_logs_cores



def Setup(tc):
    return api.types.status.SUCCESS

def Trigger(tc):
    # Failing TC for collection of logs and cores to happen
    api.Logger.info("Simulating failure of TC: %s ... to generate techsupport" % tc.Name())
    tc.SetStatus(api.types.status.FAILURE)
    resp = collect_logs_cores.Main(tc)
    tc.SetStatus(api.types.status.SUCCESS)
    return resp

def Verify(tc):
    return api.types.status.SUCCESS


def Teardown(tc):
    return api.types.status.SUCCESS
