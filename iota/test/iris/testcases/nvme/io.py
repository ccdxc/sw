#! /usr/bin/python3
import iota.harness.api as api

def Setup(tc):
    tc.nodes = api.GetNaplesHostnames()

    return api.types.status.SUCCESS

def Trigger(tc):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    api.Logger.info("Check I/O (Write + Read) on namespace at host {0}".format(tc.nodes[1]))
    api.Trigger_AddHostCommand(req, tc.nodes[1], "head -c {} < /dev/urandom > /root/tmp1.txt".format(tc.iterators.datasize))
    api.Trigger_AddHostCommand(req, tc.nodes[1], "nvme write /dev/{} -z {} -d /root/tmp1.txt".format(tc.iterators.namespace, tc.iterators.datasize))
    api.Trigger_AddHostCommand(req, tc.nodes[1], "nvme read /dev/{} -z {} -d /root/tmp2.txt".format(tc.iterators.namespace, tc.iterators.datasize))
    api.Trigger_AddHostCommand(req, tc.nodes[1], "cmp /root/tmp1.txt /root/tmp2.txt -n {}".format(tc.iterators.datasize))

    tc.resp = api.Trigger(req)

    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS

    api.Logger.info("nvme_io results for the following nodes: {0}".format(tc.nodes))

    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            result = api.types.status.FAILURE

    return result

def Teardown(tc):
    return api.types.status.SUCCESS
