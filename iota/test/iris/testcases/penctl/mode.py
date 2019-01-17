#! /usr/bin/python3
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.protos.pygen.types_pb2 as types_pb2
import iota.test.iris.testcases.penctl.penctldefs as penctldefs
import iota.test.iris.testcases.penctl.common as common

def_mode_set_error_strs = [
                           "only hostnames need to be specified as controllers: 'a:a'", 
                           "only hostnames need to be specified as controllers: 'a2:1'",
                           "Not valid mode: a - must be 'network' or 'host'", 
                           "Invalid network mode 'a', must be 'oob' for out of band or 'inband' for inband network management", 
                           "Network-Mode is not applicable if Management-Mode is 'host'", 
                           "Not valid mgmt-ip", 
                           "Not valid host name", 
                           "Not valid mac address", 
                          ]


def Setup(tc):
    tc.Nodes = api.GetNaplesHostnames()
    return api.types.status.SUCCESS

def Trigger(tc):

    req = api.Trigger_CreateExecuteCommandsRequest()
    for n in tc.Nodes:
        common.AddPenctlCommand(req, n, "show mode")

    tc.resp = api.Trigger(req)

    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            return api.types.status.FAILURE
    def checkUpdateMode():
        req = api.Trigger_CreateExecuteCommandsRequest()
        for n in tc.Nodes:
            common.AddPenctlCommand(req, n, "update mode -c a:a")
            common.AddPenctlCommand(req, n, "update mode -c a -c a1 -c a2:1")
            common.AddPenctlCommand(req, n, "update mode -o a")
            common.AddPenctlCommand(req, n, "update mode -k a")
            common.AddPenctlCommand(req, n, "update mode -o host -k a")
            common.AddPenctlCommand(req, n, "update mode -m a")
            common.AddPenctlCommand(req, n, "update mode -n a:")
            common.AddPenctlCommand(req, n, "update mode -p a")
        resp = api.Trigger(req)
        for cmd in resp.commands:
            api.PrintCommandResults(cmd)
            if cmd.exit_code != 1:
                return api.types.status.FAILURE
        for n, cmd in zip(tc.Nodes, resp.commands):
            cmd_resp = list(filter(None, cmd.stdout.split("\n")))
            if not(x in cmd_resp for x in def_mode_set_error_strs):
                api.Logger.error("Update mode response not valid : %s" %(set(def_mode_set_error_strs) - set(cmd_resp)))
                return api.types.status.FAILURE

        return api.types.status.SUCCESS

    validators = [checkUpdateMode]
    for validator in validators:
        ret = validator()
        if ret != api.types.status.SUCCESS:
            return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS
