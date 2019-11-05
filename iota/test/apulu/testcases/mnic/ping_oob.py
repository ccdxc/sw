#! /usr/bin/python3
import time
import iota.harness.api as api

#
# In this test, the host pings naples oob_mnic0 interface.  It assumes the host
# non-naples mgmt interface and naples oob_mnic0 are on the same subnet.
#
# This is different from iris ping_oob.  It does not ping between the oob_mnic0
# interfaces of naples pairs.
#

def Setup(tc):
    return api.types.status.SUCCESS

def Trigger(tc):
    #import pdb; pdb.set_trace()
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    for node in api.GetNodes():
        if not node.IsNaples():
            continue

        api.Trigger_AddHostCommand(req, node.Name(),
                'ping -c{count} {oob_mnic0_ip}'.format(
                    oob_mnic0_ip = node.GetNicMgmtIP(),
                    count = 3))

    tc.resp = api.Trigger(req)

    return api.types.status.SUCCESS

def Verify(tc):
    if not tc.resp.commands:
        api.Logger.info('No commands were issued')
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code:
            result = api.types.status.FAILURE

    return result

def Teardown(tc):
    return api.types.status.SUCCESS

