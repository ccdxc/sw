#! /usr/bin/python3
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.protos.pygen.types_pb2 as types_pb2
import iota.test.iris.testcases.penctl.penctldefs as penctldefs
import os

def Setup(tc):
    tc.Nodes = api.GetNaplesHostnames()
    return api.types.status.SUCCESS

def Trigger(tc):
    fullpath = api.GetTopDir() + '/' + tc.args.package
    logpath = api.GetTopDir() + '/'
    for n in tc.Nodes:
        resp = api.CopyToHost(n, [fullpath], penctldefs.PENCTL_ROOT_DIR)
        if resp is None:
            return api.types.status.FAILURE
        if resp.api_response.api_status != types_pb2.API_STATUS_OK:
            api.Logger.error("Failed to copy Drivers to Node: %s" % n)
            return api.types.status.FAILURE

    #mgmt_ip = api.GetNaplesMgmtIpAddresses()[0]
    mgmt_ip = "1.0.0.2"

    if api.GetNodeOs(tc.Nodes[0]) == "linux":
        penctl = "./penctl.linux "
    else:
        penctl = "./penctl.freebsd "

    req = api.Trigger_CreateExecuteCommandsRequest()
    for n in tc.Nodes:
        api.Trigger_AddHostCommand(req, n, "tar -xvf %s" % os.path.basename(tc.args.package),
                                   background = False,
                                   rundir = penctldefs.PENCTL_ROOT_DIR)
        api.Trigger_AddHostCommand(req, n, penctl + "-h", background = False,
                                   rundir = penctldefs.PENCTL_DEST_DIR)
        api.Trigger_AddHostCommand(req, n, penctl + "get logs -m nmd --ip %s | tail -n 20" % mgmt_ip, background = False,
                                   rundir = penctldefs.PENCTL_DEST_DIR)
        api.Trigger_AddHostCommand(req, n, penctl + "get logs -m nmd --ip %s > nmd.log" % mgmt_ip, background = False,
                                   rundir = penctldefs.PENCTL_DEST_DIR)
        api.Trigger_AddHostCommand(req, n, penctl + "get running-package --ip " + mgmt_ip, background = False,
                                   rundir = penctldefs.PENCTL_DEST_DIR)
    tc.resp = api.Trigger(req)

    for n in tc.Nodes:
        resp = api.CopyFromHost(n, [penctldefs.PENCTL_DEST_DIR + "/nmd.log"], "%s/%s_nmd.log" % (tc.GetLogsDir(), n))

    # File is copied to local machine
    #req2 = api.Trigger_CreateExecuteCommandsRequest()
    #for n in tc.Nodes:
    #    api.Trigger_AddHostCommand(req2, n, "tail -n 20 %s/%s_nmd.log" % (logpath, n) , background = False,
    #                               rundir = penctldefs.PENCTL_DEST_DIR)
    #tc.resp = api.Trigger(req2)

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
