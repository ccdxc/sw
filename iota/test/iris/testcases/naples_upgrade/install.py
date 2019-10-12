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
import iota.test.iris.testcases.penctl.common as pencommon
from iota.harness.infra.glopts import GlobalOptions as GlobalOptions

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


def Main(step):
    if GlobalOptions.skip_setup:
        return api.types.status.SUCCESS

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
            ret = common.copyNaplesFwImage(naplesHost, image, "/update/")
            if ret != api.types.status.SUCCESS:
                return ret

    for naplesHost in naplesHosts:
        common.stopTestUpgApp(naplesHost, True)
        ret = __installBinary(naplesHost, "build/aarch64/iris/out/" + common.UPGRADE_TEST_APP + "_bin/" + common.UPGRADE_TEST_APP + ".bin")
        if ret != api.types.status.SUCCESS:
            return ret
        ret = common.copyNaplesFwImage(naplesHost, common.UPGRADE_TEST_APP + ".bin", "/update/")
        if ret != api.types.status.SUCCESS:
            return ret

    req = api.Trigger_CreateExecuteCommandsRequest()
    for n in naplesHosts:
        pencommon.AddPenctlCommand(req, n, "update time")

    api.Trigger(req)

    return api.types.status.SUCCESS
