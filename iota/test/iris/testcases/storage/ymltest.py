#! /usr/bin/python3
import time
import pdb
import os

import iota.harness.api as api

def Setup(tc):
    tc.nodes = api.GetNaplesHostnames()
    tc.files = []
    for cfgfile in tc.args.cfg:
        tc.files.append("%s/%s/%s" % (api.GetTopDir(), tc.args.dir, cfgfile))
    tc.files.append("%s/%s/%s" % (api.GetTopDir(), tc.args.dir, tc.args.test))

    for n in tc.nodes:
        tc.files.append("%s/iota/test/iris/testcases/storage/pnsotest_%s.py" % (api.GetTopDir(), api.GetNodeOs(n)))
        resp = api.CopyToHost(n, tc.files)
        if not api.IsApiResponseOk(resp):
            return api.types.status.FAILURE
    return api.types.status.SUCCESS

def Trigger(tc):
    for n in tc.nodes:
        cmd = "./pnsotest_%s.py --wait %d --cfg " % (api.GetNodeOs(n), tc.args.wait)
        for cfgfile in tc.args.cfg:
            cmd += "%s " % cfgfile
        cmd += " --test %s " % tc.args.test

    if getattr(tc.args, "failtest", False):
        cmd += " --failure-test"

    req = api.Trigger_CreateExecuteCommandsRequest()
    api.Trigger_AddHostCommand(req, n, cmd)
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
