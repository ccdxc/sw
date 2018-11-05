#! /usr/bin/python3
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.protos.pygen.types_pb2 as types_pb2
import iota.test.iris.testcases.storage.pnsodefs as pnsodefs
import os

def Setup(tc):
    tc.nodes = api.GetNaplesHostnames()
    return api.types.status.SUCCESS

def Trigger(tc):
    fullpath = api.GetTopDir() + '/' + tc.args.package
    for n in tc.nodes:
        resp = api.CopyToHost(n, [fullpath], pnsodefs.PNSO_DEST_DIR)
        if resp is None:
            return api.types.status.FAILURE
        if resp.api_response.api_status != types_pb2.API_STATUS_OK:
            api.Logger.error("Failed to copy Drivers to Node: %s" % n)
            return api.types.status.FAILURE

    pkgname = os.path.basename(tc.args.package)
    req = api.Trigger_CreateExecuteCommandsRequest()
    for n in tc.nodes:
        api.Trigger_AddHostCommand(req, n, "tar xf %s" % pkgname,
                                   background = False, 
                                   rundir = pnsodefs.PNSO_DEST_DIR)
        api.Trigger_AddHostCommand(req, n, "make modules", background = False,
                                   rundir = pnsodefs.PNSO_DRIVER_DIR)
        api.Trigger_AddHostCommand(req, n, "ls sonic.ko", background = False,
                                   rundir = pnsodefs.PNSO_DRIVER_DIR)
    tc.resp = api.Trigger(req)
    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS
