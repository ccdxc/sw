#! /usr/bin/python3
import time
import json
import iota.harness.api as api

CPLD_REV8_FILE_PATH = "/data/naples100_02_rev8.bin"
CPLD_REV9_FILE_PATH = "/data/naples100_rev9.bin.bin"

def Setup(tc):
    return api.types.status.SUCCESS

def Trigger(tc):
    api.Logger.info("Starting CPLD Upgrade test")

    tc.Nodes = api.GetNaplesHostnames()
    api.Logger.info("The number of hosts {}" .format(len(tc.Nodes)))

    reboot_nodes = []
    for node in tc.Nodes:
        req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
        api.Trigger_AddNaplesCommand(req, node, "/nic/tools/fwupdate -l")
        resp = api.Trigger(req)

        for cmd in resp.commands:
            req_cpldapp = api.Trigger_CreateExecuteCommandsRequest(serial = True)
            try:
                out = json.loads(cmd.stdout)
            except:
                api.Logger.error("Error using fwupdate {}".format(cmd.stdout))
                return api.types.status.FAILURE
            revision = out["cpld"]["bitfile"]["version"]
            api.Logger.info("{} :: CPLD revision is {}".format(cmd.node_name, revision))
            if revision == "8":
                api.Trigger_AddNaplesCommand(req_cpldapp, node, "LD_LIBRARY_PATH=/platform/lib/ /platform/bin/cpldapp -writeflash /data/naples100_rev9.bin", timeout=300)
            elif revision == "9":
                api.Trigger_AddNaplesCommand(req_cpldapp, node, "LD_LIBRARY_PATH=/platform/lib/ /platform/bin/cpldapp -writeflash /data/naples100_02_rev8.bin", timeout=300)
            else:
                api.Logger.error("Revision not supported continue")
                continue
            api.Trigger_AddNaplesCommand(req_cpldapp, node, "touch /tmp/cpldreset")

            resp_cpldapp = api.Trigger(req_cpldapp)
            for cmd_cpldapp in resp_cpldapp.commands:
                if cmd_cpldapp.exit_code == 0:
                        api.Logger.info("Cpld written successfully ")
                        reboot_nodes.append(node)

    ret = api.RestartNodes(reboot_nodes)
    if ret != api.types.status.SUCCESS:
        api.Logger.error("Node restart failed")
        return api.types.status.FAILURE

    api.Logger.info("Hosts rebooted successfully")
    for node_afterreboot in tc.Nodes:
        req_afterreboot = api.Trigger_CreateExecuteCommandsRequest(serial = True)
        api.Trigger_AddNaplesCommand(req_afterreboot, node_afterreboot, "/nic/tools/fwupdate -l")
        resp_afterreboot = api.Trigger(req_afterreboot)

        for cmd_afterreboot in resp_afterreboot.commands:
            try:
                out = json.loads(cmd_afterreboot.stdout)
            except:
                api.Logger.error("Error using fwupdate {}".format(cmd_afterreboot.stdout))
                return api.types.status.FAILURE
            api.Logger.info("{} :: CPLD revision is {}".format(cmd_afterreboot.node_name, out["cpld"]["bitfile"]["version"]))
    return api.types.status.SUCCESS

def Verify(tc):
    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS

