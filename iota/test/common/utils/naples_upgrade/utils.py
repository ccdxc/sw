import subprocess
import json
import time
import os
import iota.harness.api as api
import iota.protos.pygen.iota_types_pb2 as types_pb2

def GetNaplesMgmtPort():
    return  "8888"

def copyNaplesFwImage(node, img, path):
    copy_cmd = "sshpass -p pen123 scp -o ConnectTimeout=20 -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no {} root@{}:{}".format(img, api.GetNicIntMgmtIP(node), path)
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

def installNaplesFwLatestImage(node, img):

    fullpath = api.GetTopDir() + '/nic/' + img
    api.Logger.info("fullpath for upg image: " + fullpath)
    resp = api.CopyToHost(node, [fullpath], "")
    if resp is None:
        return api.types.status.FAILURE
    if resp.api_response.api_status != types_pb2.API_STATUS_OK:
        api.Logger.error("Failed to copy Drivers to Node: %s" % node)
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def installBinary(node, img):

    fullpath = api.GetTopDir() + '/nic/' + img
    api.Logger.info("fullpath for binary: " + fullpath)
    resp = api.CopyToHost(node, [fullpath], "")
    if resp is None:
        return api.types.status.FAILURE
    if resp.api_response.api_status != types_pb2.API_STATUS_OK:
        api.Logger.error("Failed to copy Drivers to Node: %s" % node)
        return api.types.status.FAILURE

    return api.types.status.SUCCESS
