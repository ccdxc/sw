#! /usr/bin/python3
import iota.harness.api as api

# Following come from DOL
from apollo.oper.oper import client as OperClient

TS_DIR = "/data/techsupport/"
NUKE_TS_DIR_CMD = f"rm -rf {TS_DIR}"

def delete_ts_dir(naples_nodes):
    result = True
    req = api.Trigger_CreateAllParallelCommandsRequest()
    for node in naples_nodes:
        api.Trigger_AddNaplesCommand(req, node, NUKE_TS_DIR_CMD)
    resp = api.Trigger(req)
    for cmd in resp.commands:
        if cmd.exit_code != 0:
            api.PrintCommandResults(cmd)
            result = False
    return result

def trigger_techsupport_request(tc):
    result = delete_ts_dir(tc.nodes)
    if result != True:
        api.Logger.error(f"Failed to delete {TS_DIR} in naples before copy")
        return result

    # get testcase directory to copy out generated techsupports
    tc_dir = tc.GetLogsDir()
    skipcores = getattr(tc.iterators, "skipcores", False)
    for node in tc.nodes:
        # initiate techsupport collect request
        ts_obj = OperClient.GetTechSupportObject(node)
        ts_obj.SetSkipCores(skipcores)
        ts_file = ts_obj.Collect()
        api.Logger.debug(f"Collecting techsupport from {node} skipcores {skipcores}")
        if ts_file is None:
            api.Logger.error(f"Failed to collect techsupport for {node}")
            result = False
            continue
        api.Logger.debug(f"Copying out {ts_file} from {node} to {tc_dir}")
        # copy out the generated techsupport tarball from naples
        api.CopyFromNaples(node, [ts_file], tc_dir, True)
    return result

def Setup(tc):
    result = api.types.status.SUCCESS
    tc.nodes = api.GetNaplesHostnames()
    if len(tc.nodes) == 0:
        api.Logger.error("No naples nodes found")
        result = api.types.status.FAILURE
    api.Logger.verbose(f"TS: Setup returned {result}")
    return result

def Trigger(tc):
    result = api.types.status.SUCCESS
    if not trigger_techsupport_request(tc):
        result = api.types.status.FAILURE
    api.Logger.verbose(f"TS: Trigger returned {result}")
    return result

def Verify(tc):
    result = api.types.status.SUCCESS
    api.Logger.verbose(f"TS: Verify returned {result}")
    return result

def Teardown(tc):
    result = api.types.status.SUCCESS
    #delete techsupport in naples post copy
    if not delete_ts_dir(tc.nodes):
        api.Logger.error(f"Failed to cleanup {TS_DIR} during teardown")
        result = api.types.status.FAILURE
    api.Logger.verbose(f"TS: Teardown returned {result}")
    return result
