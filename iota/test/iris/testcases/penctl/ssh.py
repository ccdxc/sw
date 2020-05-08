#! /usr/bin/python3
import time
import iota.harness.api as api
import iota.test.iris.testcases.penctl.common as common


def Setup(tc):
    tc.Nodes = api.GetNaplesHostnames()
    return api.types.status.SUCCESS

def Trigger(tc):
    req = api.Trigger_CreateExecuteCommandsRequest()
    for n in tc.Nodes:
        common.AddPenctlCommand(req, n, "%s" % (tc.iterators.option))

    tc.resp = api.Trigger(req)

    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            return api.types.status.FAILURE

    time.sleep(5)
    req = api.Trigger_CreateExecuteCommandsRequest()
    for n in tc.Nodes:
        cmd = "sshpass -p pen123 ssh -o StrictHostKeyChecking=no root@{} ls -al /".format(common.GetNaplesMgmtIP(n))
        api.Trigger_AddHostCommand(req, n, cmd)
    tc.resp = api.Trigger(req)
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            if "disable-sshd" in tc.iterators.option:
                req = api.Trigger_CreateExecuteCommandsRequest()
                for n in tc.Nodes:
                    common.AddPenctlCommand(req, n, "system enable-sshd")
                tc.resp = api.Trigger(req)
                continue
            if "enable-sshd" in tc.iterators.option:
                return api.types.status.FAILURE
    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS
