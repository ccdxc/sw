#! /usr/bin/python3
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.testcases.penctl.penctldefs as penctldefs
import iota.test.iris.testcases.penctl.common as common

def_mode_set_error_strs = [
                            "number of LIFs not supported. --num-lifs should either be 1 or 16",
                            "specified options, --networkMode, --controllers, --default-gw, --dns-servers, --mgmt-ip --primary-mac are not applicable when DSC is in host managed mode",
                            "invalid controller a:a specified. Must be either IP Addresses or FQDNs",
                            "invalid management mode a. Must be either host or network",
                            "host managed mode needs an accompanying feature profile. Specify it with --dsc-profile",
                            "invalid management IP a specified. Must be in CIDR Format",
                            "invalid id a:",
                            "invalid MAC Address a specified",
                            "deleting default profile is disallowed",
                            "specified profile nonExistentProfile doesn't exist"
                          ]


def Setup(tc):
    tc.Nodes = api.GetNaplesHostnames()
    return api.types.status.SUCCESS

def Trigger(tc):

    req = api.Trigger_CreateExecuteCommandsRequest()
    for n in tc.Nodes:
        common.AddPenctlCommand(req, n, "show dsc")

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
            # TODO Add more negative test cases
            common.AddPenctlCommand(req, n, "create dsc-profile -n default -i 42")
            common.AddPenctlCommand(req, n, "delete dsc-profile -n default")
            common.AddPenctlCommand(req, n, "delete dsc-profile -n nonExistentProfile")
            common.AddPenctlCommand(req, n, "update dsc -o host -f scale -c a:a")
            common.AddPenctlCommand(req, n, "update dsc -o network -k oob -c a:a")
            common.AddPenctlCommand(req, n, "update dsc -o network -k inband -c a -c a1 -c a2:1")
            common.AddPenctlCommand(req, n, "update dsc -o a")
            common.AddPenctlCommand(req, n, "update dsc -k a")
            common.AddPenctlCommand(req, n, "update dsc -o network -k a")
            common.AddPenctlCommand(req, n, "update dsc -o network -k oob -m a")
            common.AddPenctlCommand(req, n, "update dsc -o network -n a: -k inband")
            common.AddPenctlCommand(req, n, "update dsc -n a: -f default")
            common.AddPenctlCommand(req, n, "update dsc -o network -k oob -p a")
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
