#! /usr/bin/python3
import time
import pdb
import os

import iota.harness.api as api
import iota.test.iris.testcases.storage.pnsodefs as pnsodefs
import iota.test.iris.testcases.storage.pnsoutils as pnsoutils

def Setup(tc):
    pnsoutils.Setup(tc)

    tc.nodes = api.GetNaplesHostnames()
    for n in tc.nodes:
        tc.files.append("%s/pnsotest_%s.py" % (tc.tcdir, api.GetNodeOs(n)))
        api.Logger.info("Copying testyml files to Node:%s" % n)
        resp = api.CopyToHost(n, tc.files)
        if not api.IsApiResponseOk(resp):
            return api.types.status.FAILURE
    return api.types.status.SUCCESS

def Trigger(tc):
    for n in tc.nodes:
        cmd = "./pnsotest_%s.py --wait %d --cfg blocksize.yml globals.yml --test %s" % (api.GetNodeOs(n), tc.args.wait, tc.args.test)
    if getattr(tc.args, "failtest", False):
        cmd += " --failure-test"

    req = api.Trigger_CreateExecuteCommandsRequest()
    api.Trigger_AddHostCommand(req, n, "dmesg -c > /dev/null")
    api.Trigger_AddHostCommand(req, n, cmd)
    api.Trigger_AddHostCommand(req, n, "dmesg")
    api.Logger.info("Running PNSO test %s" % cmd)

    tc.resp = api.Trigger(req)
    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            api.Logger.error("Command failed: %s" % cmd.command)
            result = api.types.status.FAILURE

    return result

def Teardown(tc):
    return api.types.status.SUCCESS

def RunTest(tc):
    if Setup(tc) == api.types.status.SUCCESS  and \
         Trigger(tc) == api.types.status.SUCCESS  and Verify(tc) == api.types.status.SUCCESS:
        return api.types.status.SUCCESS

    return api.types.status.SUCCESS
