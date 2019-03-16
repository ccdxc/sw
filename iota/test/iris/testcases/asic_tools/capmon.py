#! /usr/bin/python3
import re
import iota.harness.api as api
import iota.test.iris.testcases.asic_tools.utils as utils

g_req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

def Setup(tc):
    tc.skip = False
    return utils.naples_get(tc)

def base_test(tc):
    cmd = utils.g_path + 'capmon'
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
        if 'capmon' in cmd.command:
            matchObj1 = re.search(r'== PCIe ==', cmd.stdout, 0)
            matchObj2 = re.search(r'== Doorbell ==', cmd.stdout, 0)
            matchObj3 = re.search(r'== TX Scheduler ==', cmd.stdout, 0)
            matchObj4 = re.search(r'== TXDMA ==', cmd.stdout, 0)
            matchObj5 = re.search(r'== RXDMA ==', cmd.stdout, 0)
            matchObj6 = re.search(r'== P4IG ==', cmd.stdout, 0)
            matchObj7 = re.search(r'== P4EG ==', cmd.stdout, 0)
            if None in (matchObj1, matchObj2, matchObj3, matchObj4, matchObj5,
                        matchObj6, matchObj7):
                return api.types.status.FAILURE
    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE
    return base_verify(tc)

def Teardown(tc):
    return api.types.status.SUCCESS
