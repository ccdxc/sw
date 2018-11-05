#! /usr/bin/python3
import time
import pdb

import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.testcases.storage.pnsodefs as pnsodefs

def Setup(tc):
    tc.nodes = api.GetNaplesHostnames()
    tc.dmesg_commands = []
    tc.output_commands = []
    return api.types.status.SUCCESS

def Trigger(tc):
    req = api.Trigger_CreateExecuteCommandsRequest()
    for n in tc.nodes:
        api.Trigger_AddHostCommand(req, n, "dmesg -c 2>&1 > /dev/null")
        api.Trigger_AddHostCommand(req, n, "rmmod pencake", background = False,
                                   rundir = pnsodefs.PNSO_DRIVER_DIR)
        api.Trigger_AddHostCommand(req, n, "rmmod sonic", background = False,
                                   rundir = pnsodefs.PNSO_DRIVER_DIR)
        api.Trigger_AddHostCommand(req, n, "insmod sonic.ko core_count=2", background = False,
                                   rundir = pnsodefs.PNSO_DRIVER_DIR)
        api.Trigger_AddHostCommand(req, n, "insmod pencake.ko repeat=1", background = False,
                                   rundir = pnsodefs.PNSO_DRIVER_DIR)
        api.Trigger_AddHostCommand(req, n, "sleep 10")

        cmd = api.Trigger_AddHostCommand(req, n, "dmesg | tail -n 100")
        tc.dmesg_commands.append(cmd)

        for c in range(1, 5):
            output = api.Trigger_AddHostCommand(req, n, "cat /sys/module/pencake/status/%d" % c)
            tc.output_commands.append(output)

    tc.resp = api.Trigger(req)
    return api.types.status.SUCCESS

def __verify_PencakeInsmodDmesg(dmesg_output):
    lines = dmesg_output.split('\n')
    idx = lines[-2].find(pnsodefs.PNSO_PENCAKE_SUCCESS_MSG)
    if idx == -1:
        return api.types.status.FAILURE
    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE
    result = api.types.status.SUCCESS

    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            return api.types.status.FAILURE

    for dm_cmd in tc.dmesg_commands:
        ret = __verify_PencakeInsmodDmesg(dm_cmd.stdout)
        if ret != api.types.status.SUCCESS:
            api.Logger.error("Error find Pencake Success Message. Node: %s" % dm_cmd.node_name)
            api.Logger.error("ExpectedMessage = %s" % pnsodefs.PNSO_PENCAKE_SUCCESS_MSG)
            result = ret

    #for out_cmd in tc.output_commands:
    #    obj = api.parser.ParseJsonStream(out_cmd.stdout)
    #    if obj == None:
    #        api.Logger.error("Failed to parse JSON output. Command = %s" % out_cmd.command)
    #        result = api.types.status.FAILURE

    return result

def Teardown(tc):
    return api.types.status.SUCCESS
