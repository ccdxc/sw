#! /usr/bin/python3
import iota.harness.api as api

def Setup(tc):
    tc.nodes = api.GetNaplesHostnames()

    return api.types.status.SUCCESS

def Trigger(tc):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    api.Logger.info("Looking for number of queues supported by controller0 at host {0}".format(tc.nodes[1]))
    api.Trigger_AddHostCommand(req, tc.nodes[1], "nvme get-feature /dev/{} -f 7 | cut -d ':' -f 3".format(tc.iterators.ctrl))

    tc.resp = api.Trigger(req)

    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS

    api.Logger.info("nvme_get_ftr results for the following nodes: {0}".format(tc.nodes))

    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            result = api.types.status.FAILURE
#        elif cmd.stdout != "0x16?\n":
        elif cmd.stdout != "0x7e007e\n":		#Temporary, for testing with SPDK
            result = api.types.status.FAILURE

    return result

def Teardown(tc):
    return api.types.status.SUCCESS
