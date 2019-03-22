#! /usr/bin/python3
import json
import time
import datetime
import iota.harness.api as api
import iota.test.iris.testcases.penctl.common as common


def Main(step):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    uuidMap = api.GetNaplesNodeUuidMap()
    nodes = api.GetNaplesHostnames()
    for n in nodes:
        if common.PenctlGetModeStatus(n) != "NETWORK" or common.PenctlGetTransitionPhaseStatus(n) != "VENICE_REGISTRATION_DONE":
            api.Logger.info("Host [{}] is in HOST mode. Initiating mode change.".format(n))
            ret = common.SetNaplesModeOOB_Static(n, "1.1.1.1", "2.2.2.2/24")
            if ret == None:
                return api.types.status.FAILURE

        #hack for now, need to set date
        cmd = "date -s '{}'".format(datetime.datetime.utcnow().strftime("%Y-%m-%d %H:%M:%S"))
        api.Trigger_AddNaplesCommand(req, n, cmd)

    resp = api.Trigger(req)

    for cmd in resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            return api.types.status.FAILURE

    num_retries = 60
    reboot_nodes = []
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
                reboot_nodes.append(n)
                nodes.remove(n)
            else:
                api.Logger.info("Reboot not pending on node : %s" % n)

        time.sleep(1)
        num_retries = num_retries - 1
        if num_retries == 0:
            api.Logger.error("Reboot pending state not transitioned complete on naples")
            return api.types.status.FAILURE

    ret = api.RestartNodes(reboot_nodes)
    if ret != api.types.status.SUCCESS:
        api.Logger.error("Node restart failed")
        return api.types.status.FAILURE

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    for n in nodes:
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
    for n in nodes:
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
    for n in nodes:
        cmd = "curl localhost:9007/api/system/info/"
        api.Trigger_AddNaplesCommand(req, n, cmd)

    resp = api.Trigger(req)
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


    return api.types.status.SUCCESS
