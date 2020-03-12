#! /usr/bin/python3
from iota.harness.infra.glopts import GlobalOptions as GlobalOptions
import iota.harness.api as api

# Following come from DOL
from apollo.oper.oper import client as OperClient

TS_DIR = "/data/techsupport/"

class __TCData: pass

def delete_ts_dir(tc):
    result = True
    req = api.Trigger_CreateAllParallelCommandsRequest()
    for node in tc.nodes:
        api.Trigger_AddNaplesCommand(req, node, tc.delete_cmd)
    resp = api.Trigger(req)
    for cmd in resp.commands:
        if cmd.exit_code != 0:
            api.PrintCommandResults(cmd)
            result = False
    return result

def trigger_techsupport_request(tc):
    result = delete_ts_dir(tc)
    if result != True:
        api.Logger.error(f"Failed to delete {TS_DIR} before copy")
        return result

    req = api.Trigger_CreateAllParallelCommandsRequest()
    for node in tc.nodes:
        # initiate techsupport collect request
        ts_file = OperClient.GetTechSupport(node)
        api.Logger.debug(f"Collected techsupport for {node} at {ts_file} -> {tc.dir}")
        if ts_file is None:
            api.Logger.error(f"Failed to collect techsupport for {node}")
            continue
        # copy out the generated techsupport tarball from naples
        api.CopyFromNaples(node, [ts_file], tc.dir, True)
    resp = api.Trigger(req)
    for cmd in resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            api.Logger.error("Failed to copy techsupport")
            result = False
    return result

def __setup(tc):
    tc.nodes = api.GetNaplesHostnames()
    if len(tc.nodes) == 0:
        api.Logger.error("No naples nodes found")
        return False
    tc.delete_cmd = "rm -rf %s" % (TS_DIR)
    return True

def __trigger(tc):
    return trigger_techsupport_request(tc)

def __teardown(tc):
    #delete techsupport in naples post copy
    result = delete_ts_dir(tc)
    if result != True:
        api.Logger.error(f"Failed to delete {TS_DIR} during teardown")
    return result

def __collect_techsupport(testcase):
    tc = __TCData()
    if testcase:
        # techsupport collection as part of testcase failure
        tc.dir = testcase.GetLogsDir()
    else:
        # techsupport collection as part of testsuite teardown
        tc.dir = api.GetTestsuiteLogsDir()
    if not __setup(tc) or not __trigger(tc) or not __teardown(tc):
       return api.types.status.FAILURE
    return api.types.status.SUCCESS

def Main(tc):
    api.Logger.verbose("Collecting TechSupport")
    res = __collect_techsupport(tc)
    api.Logger.verbose(f"Collecting TechSupport - result {res}")
    return res
