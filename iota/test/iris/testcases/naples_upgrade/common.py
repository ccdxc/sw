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
    api.Trigger_AddNaplesCommand(req, node, "LD_LIBRARY_PATH=/platform/lib:/nic/lib /update/{} {}".format(UPGRADE_TEST_APP + ".bin", param), background = True)
    resp = api.Trigger(req)
    for cmd_resp in resp.commands:
        api.PrintCommandResults(cmd_resp)
        if cmd_resp.exit_code != 0:
            api.Logger.error("Creating core failed {}".format(cmd_resp.command))
            return api.types.status.FAILURE

    return api.types.status.SUCCESS

def killTestUpgApp(node):
    req = api.Trigger_CreateExecuteCommandsRequest()
    api.Trigger_AddNaplesCommand(req, node, "cat /run/testupgapp.pid | xargs kill -9")
    resp = api.Trigger(req)
    for cmd_resp in resp.commands:
        api.PrintCommandResults(cmd_resp)
        if cmd_resp.exit_code != 0:
            api.Logger.error("Stopping {}".format(cmd_resp.command))
            return api.types.status.FAILURE
    return api.types.status.SUCCESS

def stopTestUpgApp(node, unreg):
    killTestUpgApp(node)
    time.sleep(2)
    if unreg:
        startTestUpgApp(node, "unregupgapp")
    time.sleep(2)
    killTestUpgApp(node)
    time.sleep(2)

    return api.types.status.SUCCESS

def GetNaplesMgmtIP():
    return  "169.254.0.1"

def GetNaplesMgmtPort():
    return  "8888"

def copyNaplesFwImage(node, img, path):

    copy_cmd = "sshpass -p pen123 scp -o ConnectTimeout=20 -o StrictHostKeyChecking=no {} root@{}:{}".format(img, api.GetNicIntMgmtIP(node), path)
    req = api.Trigger_CreateExecuteCommandsRequest()
    api.Trigger_AddHostCommand(req, node, copy_cmd)
    resp = api.Trigger(req)
    for cmd in resp.commands:
        if cmd.exit_code != 0:
            api.Logger.error("Copy to failed %s" % cmd.command)
            return api.types.status.FAILURE

    req = api.Trigger_CreateExecuteCommandsRequest()
    api.Trigger_AddNaplesCommand(req, node, "chmod 777 {}/{}".format(path, img))
    resp = api.Trigger(req)
    for cmd_resp in resp.commands:
        api.PrintCommandResults(cmd_resp)
        if cmd_resp.exit_code != 0:
            api.Logger.error("Creating core failed %s", cmd_resp.command)
            return api.types.status.FAILURE

    return api.types.status.SUCCESS
