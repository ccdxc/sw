#! /usr/bin/python3
import pdb
import time
import os
import json
import subprocess
import threading
import iota.harness.api as api
import iota.protos.pygen.types_pb2 as types_pb2
import iota.test.iris.testcases.naples_upgrade.common as common

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

def __copyNaplesFwImage(node, img):

    copy_cmd = "sshpass -p pen123 scp -o ConnectTimeout=20 -o StrictHostKeyChecking=no {} root@{}:/update/".format(img, api.GetNicIntMgmtIP(node))
    req = api.Trigger_CreateExecuteCommandsRequest()
    api.Trigger_AddHostCommand(req, node, copy_cmd)
    resp = api.Trigger(req)
    for cmd in resp.commands:
        if cmd.exit_code != 0:
            api.Logger.error("Copy to failed %s" % cmd.command)
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
            ret = __copyNaplesFwImage(naplesHost, image)
            if ret != api.types.status.SUCCESS:
                return ret

    return api.types.status.SUCCESS
