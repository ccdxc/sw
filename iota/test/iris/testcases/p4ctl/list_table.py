#! /usr/bin/python3
import iota.harness.api as api

def Setup(tc):
    tc.Nodes = api.GetNaplesHostnames()
    return api.types.status.SUCCESS

def Trigger(tc):
    cmd = "/nic/tools/p4ctl -c 'debug hardware table list'"
    req = api.Trigger_CreateExecuteCommandsRequest(serial=False)

    for node in tc.Nodes:
        api.Trigger_AddNaplesCommand(req, node, cmd)

    tc.resp = api.Trigger(req)
    return api.types.status.SUCCESS

def Verify(tc):
    table_list = ["flow_hash", "mirror", "rewrite", "flow_info"]
    if tc.resp is None:
        return api.types.status.FAILURE

    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            return api.types.status.FAILURE
        # check sample table names in output
        for t in table_list:
            if t not in cmd.stdout:
                api.Logger.error("Failed to find Table: %s in output"%(t));
                return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS
