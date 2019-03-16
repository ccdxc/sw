#! /usr/bin/python3
import re
import iota.harness.api as api
import iota.test.iris.testcases.asic_tools.utils as utils

g_req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

def Setup(tc):
    tc.skip = False
    return utils.naples_get(tc)

def base_test(tc):
    cmd = utils.g_path + 'capview'
    api.Trigger_AddNaplesCommand(g_req, tc.naples_node, cmd)
    api.Logger.info("cmd - %s" % cmd)

def Trigger(tc):
    if tc.skip:
        return api.types.status.SUCCESS
    base_test(tc)
    tc.resp = api.Trigger(g_req)
    return api.types.status.SUCCESS

def base_verify(tc):
    for cmd in tc.resp.commands:
        if 'capview' in cmd.command:
            matchObj = re.search(r'>', cmd.stdout, 0)
            if matchObj is None:
                return api.types.status.FAILURE
    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE
    return base_verify(tc)

def Teardown(tc):
    return api.types.status.SUCCESS
