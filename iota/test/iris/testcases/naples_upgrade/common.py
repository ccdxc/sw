import subprocess
import json
import time
import iota.harness.api as api
import iota.harness.infra.store as store
import iota.test.iris.testcases.drivers.cmd_builder as cmd_builder
import iota.test.iris.utils.naples_host as naples_host_util

UPGRADE_ROOT_DIR = 'upgrade_bin'

UPGRADE_NAPLES_PKG = "naples_fw.tar"
UPGRADE_NAPLES_PKG_COMPAT_CHECK = "naples_upg_fw.tar"

UPGRADE_TEST_APP = "testupgapp"

def startTestUpgApp(node, param):
    req = api.Trigger_CreateExecuteCommandsRequest()
    api.Trigger_AddNaplesCommand(req, node, "LD_LIBRARY_PATH=/platform/lib:/nic/lib /nic/bin/{} {}".format(UPGRADE_TEST_APP, param), background = True)
    resp = api.Trigger(req)
    for cmd_resp in resp.commands:
        api.PrintCommandResults(cmd_resp)
        if cmd_resp.exit_code != 0:
            api.Logger.error("Creating core failed %s", cmd_resp.command)
            return api.types.status.FAILURE

    return api.types.status.SUCCESS

def stopTestUpgApp(node):
    req = api.Trigger_CreateExecuteCommandsRequest()
    api.Trigger_AddNaplesCommand(req, node, "cat /run/testupgapp.pid | xargs kill -9")
    resp = api.Trigger(req)
    for cmd_resp in resp.commands:
        api.PrintCommandResults(cmd_resp)
        if cmd_resp.exit_code != 0:
            api.Logger.error("Stopping %s", cmd_resp.command)
            return api.types.status.FAILURE

    return api.types.status.SUCCESS
