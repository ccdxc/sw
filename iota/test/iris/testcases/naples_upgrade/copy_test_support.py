#! /usr/bin/python3
import os
import iota.harness.api as api
import iota.protos.pygen.iota_types_pb2 as types_pb2

def Setup(tc):
    tc.Nodes = api.GetNaplesHostnames()
    ret = api.RestartNodes(tc.Nodes)
    if ret != api.types.status.SUCCESS:
        api.Logger.error("Node restart failed")
        return api.types.status.FAILURE
    return api.types.status.SUCCESS

def_upg_tech_support_files = ["/data/naples-disruptive-upgrade-tech-support.tar.gz", "/data/pre-upgrade-logs.tar"]

def Trigger(tc):
    req = api.Trigger_CreateExecuteCommandsRequest()
    for node in tc.Nodes:
        api.Trigger_AddNaplesCommand(req, node, "ls /data/")
        for file in def_upg_tech_support_files:
            cmd = "chmod 666 %s" % file
            api.Trigger_AddNaplesCommand(req, node, cmd)
    resp = api.Trigger(req)
    for cmd_resp in resp.commands:
        api.PrintCommandResults(cmd_resp)

    for node in tc.Nodes:
        resp = api.CopyFromNaples(node, def_upg_tech_support_files, tc.GetLogsDir())
        if resp == None or resp.api_response.api_status != types_pb2.API_STATUS_OK:
            api.Logger.info("Failed to copy naples-disruptive-upgrade-tech-support.tar.gz or pre-upgrade-logs.tar")
            #return api.types.status.FAILURE
        for file in def_upg_tech_support_files:
            cmd = "mv " + tc.GetLogsDir() + "/" + os.path.basename(file) + " " + tc.GetLogsDir() + "/" + os.path.basename(file) + "-" + node
            #Rename file based on node
            api.Logger.info("executing cmd %s" % cmd)
            os.system(cmd)

    return api.types.status.SUCCESS

def Verify(tc):
    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS
    req = api.Trigger_CreateExecuteCommandsRequest()
    for node in tc.Nodes:
        api.Trigger_AddNaplesCommand(req, node, "rm -rf /data/naples-disruptive-upgrade-tech-support.tar.gz")
        api.Trigger_AddNaplesCommand(req, node, "rm -rf /data/pre-upgrade-logs.tar")
        api.Trigger_AddNaplesCommand(req, node, "rm -rf /data/pre-upgrade-logs.tar")
        api.Trigger_AddNaplesCommand(req, node, "rm /data/sysmgr.json")
    resp = api.Trigger(req)
    for cmd_resp in resp.commands:
        api.PrintCommandResults(cmd_resp)
        if cmd_resp.exit_code != 0:
            api.Logger.error("Deletion failed %s", cmd_resp.command)
    return api.types.status.SUCCESS
