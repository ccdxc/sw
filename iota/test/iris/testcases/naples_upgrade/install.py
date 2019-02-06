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

def __installNaplesFwImage(node):

    fullpath = api.GetTopDir() + '/' + common.UPGRADE_NAPLES_PKG

    resp = api.CopyToHost(node, [fullpath], "")
    if resp is None:
        return api.types.status.FAILURE
    if resp.api_response.api_status != types_pb2.API_STATUS_OK:
        api.Logger.error("Failed to copy Drivers to Node: %s" % node)
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def __modifyNaplesFwImage(node):

    def untar(node):
        req = api.Trigger_CreateExecuteCommandsRequest()
        api.Trigger_AddHostCommand(req, node, "tar xvf naples_fw.tar")
        resp = api.Trigger(req)
        for cmd in resp.commands:
            api.PrintCommandResults(cmd)
            if cmd.exit_code != 0:
                return api.types.status.FAILURE
        return api.types.status.SUCCESS


    def removePkgFile(node):
        req = api.Trigger_CreateExecuteCommandsRequest()
        api.Trigger_AddHostCommand(req, node, "rm -rf naples_fw.tar")
        resp = api.Trigger(req)
        for cmd in resp.commands:
            api.PrintCommandResults(cmd)
            if cmd.exit_code != 0:
                return api.types.status.FAILURE
        return api.types.status.SUCCESS

    def modifyManifest(node):
        req = api.Trigger_CreateExecuteCommandsRequest()
        api.Trigger_AddHostCommand(req, node, "cat MANIFEST | jq '.build_user = \"iota-upgrade-user\"' > MANIFEST.temp 2>&1")
        api.Trigger_AddHostCommand(req, node, "mv MANIFEST.temp MANIFEST")
        resp = api.Trigger(req)
        for cmd in resp.commands:
            api.PrintCommandResults(cmd)
            if cmd.exit_code != 0:
                return api.types.status.FAILURE
            print(cmd.stdout)
        return api.types.status.SUCCESS

    def tar(node):
        req = api.Trigger_CreateExecuteCommandsRequest()
        api.Trigger_AddHostCommand(req, node, "tar cvf naples_fw.tar kernel.img MANIFEST system.img u-boot.img")
        resp = api.Trigger(req)
        for cmd in resp.commands:
            api.PrintCommandResults(cmd)
            if cmd.exit_code != 0:
                return api.types.status.FAILURE
        return api.types.status.SUCCESS

    def removeImgFiles(node):
        req = api.Trigger_CreateExecuteCommandsRequest()
        api.Trigger_AddHostCommand(req, node, "rm -rf kernel.img MANIFEST system.img u-boot.img")
        resp = api.Trigger(req)
        for cmd in resp.commands:
            api.PrintCommandResults(cmd)
            if cmd.exit_code != 0:
                return api.types.status.FAILURE
        return api.types.status.SUCCESS

    steps = [untar, removePkgFile, modifyManifest, tar, removeImgFiles]
    for step in steps:
        ret = step(node)
        if ret != api.types.status.SUCCESS:
            return api.types.status.FAILURE

    return api.types.status.SUCCESS

def __copyNaplesFwImage(node):

    copy_cmd = "sshpass -p pen123 scp -o ConnectTimeout=20 -o StrictHostKeyChecking=no naples_fw.tar root@{}:/update/".format(api.GetNicIntMgmtIP(node))
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
    for naplesHost in naplesHosts:
        ret = __installNaplesFwImage(naplesHost)
        if ret != api.types.status.SUCCESS:
            return ret
        ret = __modifyNaplesFwImage(naplesHost)
        if ret != api.types.status.SUCCESS:
            return ret
        ret = __copyNaplesFwImage(naplesHost)
        if ret != api.types.status.SUCCESS:
            return ret

    return api.types.status.SUCCESS
