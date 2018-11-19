#! /usr/bin/python3
import pdb
import os
import iota.harness.api as api
import iota.protos.pygen.types_pb2 as types_pb2
import iota.test.iris.testcases.penctl.common as common


def __penctl_exec(node):
    return "penctl.linux" if api.GetNodeOs(node) == "linux" else "penctl.freebsd"

def __installPenCtl(node):

    fullpath = api.GetTopDir() + '/' + common.PENCTL_PKG

    resp = api.CopyToHost(node, [fullpath], "")
    if resp is None:
        return api.types.status.FAILURE
    if resp.api_response.api_status != types_pb2.API_STATUS_OK:
        api.Logger.error("Failed to copy Drivers to Node: %s" % node)
        return api.types.status.FAILURE

    req = api.Trigger_CreateExecuteCommandsRequest()
    api.Trigger_AddHostCommand(req, node, "tar -xvf %s" % os.path.basename(common.PENCTL_PKG),
                           background = False)

    #Create a symlink at top level
    execName = __penctl_exec(node)
    realPath = "realpath %s/%s " % (common.PENCTL_DEST_DIR, execName)
    api.Trigger_AddHostCommand(req, node, realPath, background = False)

    resp = api.Trigger(req)

    for cmd in resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            return api.types.status.FAILURE

    common.PENCTL_EXEC[node] = resp.commands[1].stdout.split("\n")[0]

    return api.types.status.SUCCESS

def __installNaplesFwImage(node):

    fullpath = api.GetTopDir() + '/' + common.PENCTL_NAPLES_PKG

    resp = api.CopyToHost(node, [fullpath], "")
    if resp is None:
        return api.types.status.FAILURE
    if resp.api_response.api_status != types_pb2.API_STATUS_OK:
        api.Logger.error("Failed to copy Drivers to Node: %s" % node)
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Main(step):
    #time.sleep(120)
    naplesHosts = api.GetNaplesHostnames()

    assert(len(naplesHosts) != 0)

    api.ChangeDirectory(common.PENCTL_ROOT_DIR)
    for naplesHost in naplesHosts:
        ret = __installPenCtl(naplesHost)
        if ret != api.types.status.SUCCESS:
            return ret
        ret = __installNaplesFwImage(naplesHost)
        if ret != api.types.status.SUCCESS:
            return ret

    return api.types.status.SUCCESS
