#! /usr/bin/python3
import pdb
import time
import iota.harness.api as api
import iota.protos.pygen.types_pb2 as types_pb2
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2

macvlan_test_script = 'macvlans.sh'
linux_test_dir = api.GetTopDir() + '/platform/drivers/linux/eth/ionic/tests/'
source_file = linux_test_dir + macvlan_test_script

def Setup(tc):
    # Find the name of the target host
    names = api.GetNaplesHostnames()
    name = names[0]
    if api.GetNodeOs(name) != "linux":
        return api.types.status.SUCCESS

    # Copy the test script to the target host
    resp = api.CopyToHost(name, [source_file], "")
    if resp is None:
        api.Logger.error("None resp on Failed copy %s to Host %s" % (source_file, name))
        return api.types.status.FAILURE
    if resp.api_response.api_status != types_pb2.API_STATUS_OK:
        api.Logger.error("Failed to copy %s to Host %s" % (source_file, name))
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Trigger(tc):
    names = api.GetNaplesHostnames()
    name = names[0]
    if api.GetNodeOs(name) != "linux":
        return api.types.status.SUCCESS

    # Set up to run the macvlan offload test shellscript
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    # Get the workload interface information
    pairs = api.GetRemoteWorkloadPairs()
    pair = pairs[0]
    w1 = pair[0]
    w2 = pair[1]

    tc.cmd_descr = "Server: %s %s --> %s" % (w1.interface, w1.ip_address, w2.ip_address)
    api.Logger.info("Starting macvlan offload test from %s" % (tc.cmd_descr))

    basecmd = './%s %s %s %s' % (macvlan_test_script, w1.interface, w1.ip_address, w2.ip_address)
    api.Trigger_AddHostCommand(req, w1.node_name, basecmd, background = False)

    # Send the commands
    tc.resp = api.Trigger(req)

    return api.types.status.SUCCESS

def Verify(tc):
    names = api.GetNaplesHostnames()
    name = names[0]
    if api.GetNodeOs(name) != "linux":
        return api.types.status.SUCCESS

    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS

    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            result = api.types.status.FAILURE
    return result

