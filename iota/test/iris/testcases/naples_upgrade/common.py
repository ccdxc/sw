import subprocess
import json
import time
import os
import iota.harness.api as api
import iota.harness.infra.store as store
import iota.test.iris.testcases.drivers.cmd_builder as cmd_builder
import iota.test.iris.utils.naples_host as naples_host_util
import iota.protos.pygen.iota_types_pb2 as types_pb2

UPGRADE_ROOT_DIR = 'upgrade_bin'

UPGRADE_NAPLES_PKG = "naples_fw.tar"
UPGRADE_NAPLES_PKG_COMPAT_CHECK = "naples_upg_fw.tar"

UPGRADE_TEST_APP = "testupgapp"

def startTestUpgApp(node, param):
    stopTestUpgApp(node, False)
    #/data/sysmgr.json
    data = [
               {
                   "name": "{}".format(UPGRADE_TEST_APP),
                   "command": "/update/{} {}".format(UPGRADE_TEST_APP + ".bin", param),
                   "dependencies": [
                       {
                           "kind": "service",
                           "service-name": "delphi"
                       },
                       {
                           "kind": "service",
                           "service-name": "hal"
                       }
                   ],
                   "flags": ["save_stdout_on_crash"],
                   "timeout": 3000.0
               }
           ]
    # Write JSON file
    with open("sysmgr.json", "w") as write_file:
        json.dump(data, write_file)

    # Read JSON file
    with open('sysmgr.json') as data_file:
        data_loaded = json.load(data_file)
    print(data == data_loaded)

    fullpath = api.GetTopDir() + '/iota/sysmgr.json'
    api.Logger.info("fullpath for binary: " + fullpath)
    resp = api.CopyToHost(node, [fullpath], "")
    if resp is None:
        return api.types.status.FAILURE
    if resp.api_response.api_status != types_pb2.API_STATUS_OK:
        api.Logger.error("Failed to copy Drivers to Node: %s" % node)
        return api.types.status.FAILURE

    ret = copyNaplesFwImage(node, "sysmgr.json", "/data/")
    if ret != api.types.status.SUCCESS:
        return ret

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
    if os.path.exists("sysmgr.json"):
        os.remove("sysmgr.json")
        api.Logger.info("sysmgr.json file removed")
    else:
        api.Logger.info("The file sysmgr.json does not exist")

    req = api.Trigger_CreateExecuteCommandsRequest()
    api.Trigger_AddNaplesCommand(req, node, "cat /run/testupgapp.pid | xargs kill -9")
    api.Trigger_AddNaplesCommand(req, node, "rm /data/sysmgr.json")
    resp = api.Trigger(req)
    for cmd_resp in resp.commands:
        api.PrintCommandResults(cmd_resp)
        if cmd_resp.exit_code != 0:
            api.Logger.error("Stopping {}".format(cmd_resp.command))
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

def GetNaplesMgmtIP(n):
    return  api.GetNicIntMgmtIP(n)

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
