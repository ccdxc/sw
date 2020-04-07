#! /usr/bin/python3
import iota.harness.api as api

def Setup(tc):
    tc.nodes = api.GetNaplesHostnames()

    return api.types.status.SUCCESS

def Trigger(tc):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    api.Trigger_AddHostCommand(req, tc.nodes[1], "cd /root/pynvme; python3 -B -m pytest --pciaddr=1.1.0.2 scripts/pynvme_scripts/%s" %(tc.iterators.testcase))

    tc.resp = api.Trigger(req)

    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS

    api.Logger.info("nvme_pynvme results for the following nodes: {0}".format(tc.nodes))
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        #TBD: Decide on detecting failures with pynvme

    return result

def Teardown(tc):
    return api.types.status.SUCCESS
