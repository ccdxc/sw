#! /usr/bin/python3
import json
import time
import datetime
import iota.harness.api as api
import iota.test.iris.testcases.penctl.common as common


def Main(step):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    venice_ips = api.GetVeniceMgmtIpAddresses()
    assert(venice_ips)
    uuidMap = api.GetNaplesNodeUuidMap()
    for n in api.GetNaplesHostnames():
        #hack for now, need to set date
        cmd = "date -s '{}'".format(datetime.datetime.utcnow().strftime("%Y-%m-%d %H:%M:%S"))
        api.Trigger_AddNaplesCommand(req, n, cmd)
        api.Logger.info("Running mode switch on node : %s" % n)
        mode_switch_cmd = "update mode --management-mode network --network-mode oob --hostname {} --primary-mac {}".format(n, uuidMap[n])
        common.AddPenctlCommand(req, n, mode_switch_cmd)

    resp = api.Trigger(req)

    for cmd in resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            result = api.types.status.FAILURE


    reboot_nodes = []
    for n in api.GetNaplesHostnames():
        reboot_nodes.append(n)

    ret = api.RestartNodes(reboot_nodes)
    if ret != api.types.status.SUCCESS:
        api.Logger.error("Node restart failed")
        return api.types.status.FAILURE



    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    for n in api.GetNaplesHostnames():
        #hack for now, need to set date
        cmd = "date -s '{}'".format(datetime.datetime.utcnow().strftime("%Y-%m-%d %H:%M:%S"))
        api.Trigger_AddNaplesCommand(req, n, cmd)

    resp = api.Trigger(req)

    for cmd in resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            result = api.types.status.FAILURE

    #Give some time for naples to admit to venice after mode switch
    #This is required for now as Iota is setting time manually to make TLS happy
    time.sleep(10)
    #Check whether naples has switch mode succesfully
    req = api.Trigger_CreateExecuteCommandsRequest(serial = False)
    for n in api.GetNaplesHostnames():
        cmd = "cat /sysconfig/config0/app-start.conf  | grep hostpin"
        api.Trigger_AddNaplesCommand(req, n, cmd)

    resp = api.Trigger(req)
    for cmd in resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            api.Logger.error("Mode switch failed on node : {}".format(cmd.node_name))
            result = api.types.status.FAILURE

    return api.types.status.SUCCESS
