#! /usr/bin/python3
import json
import time
import datetime
import iota.harness.api as api
import iota.test.iris.testcases.penctl.common as common
import iota.test.iris.utils.hal_show as hal_show_utils
from iota.harness.infra.glopts import GlobalOptions as GlobalOptions

NAPLES_CONFIG_SPEC_LOCAL        = "/tmp/system-config.json"

# Create system config file to enable console with out triggering
# authentication. 
def CreateConfigConsoleNoAuth():
    console_enable = {'console': 'enable'}
    with open(NAPLES_CONFIG_SPEC_LOCAL, 'w') as outfile:
        json.dump(console_enable, outfile, indent=4)

def Main(step):
    if GlobalOptions.skip_setup:
        #No mode switch required for skeip setup
        return api.types.status.SUCCESS
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    uuidMap = api.GetNaplesNodeUuidMap()
    nodes = api.GetNaplesHostnames()
    for n in nodes:
        # Touch a file to indicate to NMD that the current mode is emulation
        cmd = "touch /data/iota-emulation"
        api.Trigger_AddNaplesCommand(req, n, cmd)
        # Make sure console is enabled
        CreateConfigConsoleNoAuth()
        api.CopyToNaples(n, [NAPLES_CONFIG_SPEC_LOCAL], "")
        cmd = "mv /system-config.json /sysconfig/config0/system-config.json"
        api.Trigger_AddNaplesCommand(req, n, cmd)

        if common.PenctlGetModeStatus(n) != "NETWORK" or common.PenctlGetTransitionPhaseStatus(n) != "VENICE_REGISTRATION_DONE":
            api.Logger.info("Host [{}] is in HOST mode. Initiating mode change.".format(n))
            ret = common.SetNaplesModeOOB_Static(n, "1.1.1.1", "1.1.1.2/24")
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
            time.sleep(30)
            common.AddPenctlCommand(req, n, check_state_cmd)
            api.Trigger_AddNaplesCommand(req, n, "touch /data/no_watchdog")
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
            if not hal_show_utils.IsNaplesForwardingModeClassic(n):
                api.Logger.info("Dataplane already in HOSTPIN mode. Skipping node [{}] for reboot.".format(n))
                reboot_nodes.append(n)
                nodes.remove(n)
            elif out["status"]["transition-phase"] == "VENICE_UNREACHABLE":
                api.Logger.info("Reboot pending on node : %s" % n)
                reboot_nodes.append(n)
                nodes.remove(n)
            elif out["status"]["transition-phase"] == "VENICE_REGISTRATION_DONE":
                api.Logger.info("Node already transitioned : %s" % n)
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
    enable_sshd = "system enable-sshd"
    copy_key = "update ssh-pub-key -f ~/.ssh/id_rsa.pub"
    for n in api.GetNaplesHostnames():
        #hack for now, need to set date
        cmd = "date -s '{}'".format(datetime.datetime.utcnow().strftime("%Y-%m-%d %H:%M:%S"))
        api.Trigger_AddNaplesCommand(req, n, cmd)
        common.AddPenctlCommand(req, n, enable_sshd)
        common.AddPenctlCommand(req, n, copy_key)

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
        cmd = "curl localhost:8888/api/system/info/"
        api.Trigger_AddNaplesCommand(req, n, cmd)

        # Delete the iota-emulation file created earlied for NMD. Mode change would have passed by now if it had to.
        cmd = "rm -f /data/iota-emulation"
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
