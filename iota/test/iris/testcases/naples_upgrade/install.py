#! /usr/bin/python3
import pdb
import time
import os
import json
import subprocess
import threading
import iota.harness.api as api
import iota.test.iris.testcases.naples_upgrade.common as common
import iota.protos.pygen.iota_types_pb2 as types_pb2

def __installNaplesFwLatestImage(node, img):

    fullpath = api.GetTopDir() + '/nic/' + img
    api.Logger.info("fullpath for upg image: " + fullpath)
    resp = api.CopyToHost(node, [fullpath], "")
    if resp is None:
        return api.types.status.FAILURE
    if resp.api_response.api_status != types_pb2.API_STATUS_OK:
        api.Logger.error("Failed to copy Drivers to Node: %s" % node)
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def __installBinary(node, img):

    fullpath = api.GetTopDir() + '/nic/' + img
    api.Logger.info("fullpath for binary: " + fullpath)
    resp = api.CopyToHost(node, [fullpath], "")
    if resp is None:
        return api.types.status.FAILURE
    if resp.api_response.api_status != types_pb2.API_STATUS_OK:
        api.Logger.error("Failed to copy Drivers to Node: %s" % node)
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def __copyNaplesFwImage(node, img, path):

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

def Main(step):
    #time.sleep(10)
    naplesHosts = api.GetNaplesHostnames()

    assert(len(naplesHosts) != 0)

    api.ChangeDirectory(common.UPGRADE_ROOT_DIR)
    images = [common.UPGRADE_NAPLES_PKG, common.UPGRADE_NAPLES_PKG_COMPAT_CHECK]
    for image in images:
        for naplesHost in naplesHosts:
            ret = __installNaplesFwLatestImage(naplesHost, image)
            if ret != api.types.status.SUCCESS:
                return ret
            ret = __copyNaplesFwImage(naplesHost, image, "/update/")
            if ret != api.types.status.SUCCESS:
                return ret

    for naplesHost in naplesHosts:
        ret = __installBinary(naplesHost, "build/aarch64/iris/out/" + common.UPGRADE_TEST_APP + "_bin/" + common.UPGRADE_TEST_APP + ".bin")
        if ret != api.types.status.SUCCESS:
            return ret
        ret = __copyNaplesFwImage(naplesHost, common.UPGRADE_TEST_APP + ".bin", "/nic/bin/")
        if ret != api.types.status.SUCCESS:
            return ret

    return api.types.status.SUCCESS
