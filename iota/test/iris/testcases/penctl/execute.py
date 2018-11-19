#! /usr/bin/python3
import re
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.protos.pygen.types_pb2 as types_pb2
import iota.test.iris.testcases.penctl.penctldefs as penctldefs
import iota.test.iris.testcases.penctl.common as common

ansi_escape = re.compile(r'\x1B\[[0-?]*[ -/]*[@-~]')

def Setup(tc):
    tc.Nodes = api.GetNaplesHostnames()
    return api.types.status.SUCCESS

def Trigger(tc):
    tc.cmd_cookies = []
    penctl_req = api.Trigger_CreateExecuteCommandsRequest()
    naples_req = api.Trigger_CreateExecuteCommandsRequest()
    for n in tc.Nodes:
        common.AddPenctlCommand(penctl_req, n, "execute %s" % tc.iterators.command)
        api.Trigger_AddNaplesCommand(naples_req, n, tc.iterators.command)

    tc.penctl_resp = api.Trigger(penctl_req)
    tc.naples_resp = api.Trigger(naples_req)
    return api.types.status.SUCCESS

def Verify(tc):
    if tc.penctl_resp is None or tc.naples_resp is None:
        return api.types.status.FAILURE

    for naples_cmd, penctl_cmd in zip(tc.naples_resp.commands, tc.penctl_resp.commands):
        api.PrintCommandResults(naples_cmd)
        api.PrintCommandResults(penctl_cmd)
        for penctl_line, naples_line in zip(penctl_cmd.stdout.split("\n"), naples_cmd.stdout.split("\r\n")):
            if penctl_line.replace("\\", "") != ansi_escape.sub('', naples_line):
                api.Logger.error("Execute output mismatch expected %s, actual %s" %(naples_line,  penctl_line))
                return api.types.status.FAILURE
    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS
