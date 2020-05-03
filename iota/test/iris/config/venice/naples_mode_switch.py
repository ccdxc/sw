#! /usr/bin/python3
import json
import time
import datetime
import iota.harness.api as api
import iota.test.iris.testcases.penctl.common as common


def __get_mgmt_nw():
    if os.environ.get("JOB_ID") is not None:
        return "inband"
    return "oob"

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
        mode_switch_cmd = "update naples --managed-by network --management-network oob --hostname {} --primary-mac {}".format(__get_mgmt_nw(), n, uuidMap[n])
        common.AddPenctlCommand(req, n, mode_switch_cmd)

    resp = api.Trigger(req)

    for cmd in resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            return api.types.status.FAILURE

    nodes = api.GetNaplesHostnames()
    num_retries = 60
    while nodes:
        req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
        for n in nodes:
            #hack for now, need to set date
            api.Logger.info("Checking Tranisition phase for node : %s" % n)
            check_state_cmd = "show naples --json"
            common.AddPenctlCommand(req, n, check_state_cmd)
            resp = api.Trigger(req)
            cmd = resp.commands[0]
            api.PrintCommandResults(cmd)
            if cmd.exit_code != 0:
                return api.types.status.FAILURE
            try:
                out = json.loads(cmd.stdout)
            except:
                api.Logger.error("Penctl output not in Json format {}".format(cmd.stdout))
                return api.types.status.FAILURE
            if out["status"]["transition-phase"] == "REBOOT_PENDING":
                api.Logger.info("Reboot pending on node : %s" % n)
                nodes.remove(n)
            else:
                api.Logger.info("Reboot not pending on node : %s" % n)

        time.sleep(1)
        num_retries = num_retries - 1
        if num_retries == 0:
            api.Logger.error("Reboot pending state not transitioned complete on naples")
            return api.types.status.FAILURE


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
    time.sleep(30)
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

    #Verify Agent in right mode.
    req = api.Trigger_CreateExecuteCommandsRequest(serial = False)
    for n in api.GetNaplesHostnames():
        cmd = "curl localhost:8888/api/system/info/"
        api.Trigger_AddNaplesCommand(req, n, cmd)

    resp = api.Trigger(req)
    venice_urls =[]
    for ip in api.GetVeniceMgmtIpAddresses():
        venice_urls.append(ip + ":9009")
    for cmd in resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            api.Logger.error("Agent system get failed : {}".format(cmd.node_name))
            result = api.types.status.FAILURE
        out = None
        try:
            out = json.loads(cmd.stdout)
        except:
            api.Logger.error("Agent System get out failed {}".format(cmd.stdout))
            return api.types.status.FAILURE
        if out["naples-mode"] != "NETWORK_MANAGED_OOB":
            api.Logger.error("Agent not in correct mode: {} {} ".format(cmd.node_name, out["naples-mode"]))
            return api.types.status.FAILURE
        if set(out["controller-ips"]) != set(venice_urls):
            api.Logger.error("Agent controller IPs don't match: {} {}".format(venice_urls, out["controller-ips"]))
            return api.types.status.FAILURE


    return api.types.status.SUCCESS
