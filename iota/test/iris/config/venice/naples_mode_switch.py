#! /usr/bin/python3
import json
import time
import iota.harness.api as api
import iota.test.iris.testcases.penctl.common as common


def Main(step):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    venice_ips = api.GetVeniceMgmtIpAddresses()
    assert(venice_ips)
    uuidMap = api.GetNaplesNodeUuidMap()
    for n in api.GetNaplesHostnames():
        api.Logger.info("Running mode switch on node : %s" % n)
        mode_switch_cmd = "update mode network --network-mode oob --primary-mac {}".format(uuidMap[n]) 
        #mode_switch_cmd = "update mode network --network-mode oob --controllers {} --mgmt-ip {}/16 --default-gw 10.8.0.1 --primary-mac {}".format(" ".join(venice_ips), api.GetNicMgmtIP(n), uuidMap[n])
        common.AddPenctlCommand(req, n, mode_switch_cmd)

    resp = api.Trigger(req)

    for cmd in resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            result = api.types.status.FAILURE

    return api.types.status.SUCCESS

