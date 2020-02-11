#! /usr/bin/python3
import json
import time
import datetime
import iota.harness.api as api
import iota.test.iris.testcases.penctl.common as common
import iota.test.iris.utils.hal_show as hal_show_utils
import iota.test.iris.config.netagent.api as netagent_api
from iota.harness.infra.glopts import GlobalOptions as GlobalOptions

NAPLES_CONFIG_SPEC_LOCAL        = "/tmp/system-config.json"

# Create system config file to enable console with out triggering
# authentication. 
def CreateConfigConsoleNoAuth():
    console_enable = {'console': 'enable'}
    with open(NAPLES_CONFIG_SPEC_LOCAL, 'w') as outfile:
        json.dump(console_enable, outfile, indent=4)

def Main(step):
    policy_mode = getattr(step, "policy_Mode", None)
    agent_nodes = api.GetNaplesHostnames()
    fwd_mode = getattr(step, "fwd_mode", None)
    policy_mode = getattr(step, "policy_Mode", None)

    netagent_api.Init(agent_nodes, hw = True)

    if GlobalOptions.skip_setup:
        # Restore the current profile in cfg DB
        return netagent_api.switch_profile(fwd_mode, policy_mode, push=False)

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

        if common.PenctlGetModeStatus(n) != "NETWORK" or \
           common.PenctlGetTransitionPhaseStatus(n) != "VENICE_REGISTRATION_DONE":
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
                nodes.remove(n)
            elif out["status"]["transition-phase"] == "VENICE_UNREACHABLE":
                api.Logger.info("Reboot pending on node : %s" % n)
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

    api.Logger.info("Switching to profile fwd_mode: %s," \
                    "policy_mode : %s"%(fwd_mode, policy_mode))

    netagent_api.switch_profile(fwd_mode, policy_mode)

    api.Logger.info("Starting SSH server on Nodes")
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

    # api.Logger.info("Verifying system information")
    # req = api.Trigger_CreateExecuteCommandsRequest(serial = False)
    # for n in api.GetNaplesHostnames():
    #     cmd = "curl localhost:8888/api/system/info/"
    #     api.Trigger_AddNaplesCommand(req, n, cmd)

    # resp = api.Trigger(req)
    # for cmd in resp.commands:
    #     api.PrintCommandResults(cmd)
    #     if cmd.exit_code != 0:
    #         api.Logger.error("Agent system get failed : {}".format(cmd.node_name))
    #         result = api.types.status.FAILURE
    #     out = None
    #     try:
    #         out = json.loads(cmd.stdout)
    #     except:
    #         api.Logger.error("Agent System get out failed {}".format(cmd.stdout))
    #         return api.types.status.FAILURE
    #     if out["naples-mode"] != "NETWORK_MANAGED_OOB":
    #         api.Logger.error("Agent not in correct mode: {} {} ".format(cmd.node_name, out["naples-mode"]))
    #         return api.types.status.FAILURE

    api.Logger.info("Removing temp files")
    req = api.Trigger_CreateExecuteCommandsRequest(serial = False)
    for n in api.GetNaplesHostnames():
        # Delete the iota-emulation file created earlied for NMD. Mode change would have passed by now if it had to.
        cmd = "rm -f /data/iota-emulation"
        api.Trigger_AddNaplesCommand(req, n, cmd)

        # Delete device.conf so that during -skip-firmware-upgrade step
        # we dont parse device.conf with hostpin forwarding mode.
        cmd = "rm -f /sysconfig/config0/device.conf"
        api.Trigger_AddNaplesCommand(req, n, cmd)

    resp = api.Trigger(req)
    for cmd in resp.commands:
        api.PrintCommandResults(cmd)

    return api.types.status.SUCCESS
