#! /usr/bin/python3
from datetime import datetime
import re

import iota.harness.api as api

TS_DIR = "/data/techsupport/"
TS_SCRIPT = "/nic/tools/collect_techsupport.sh"
TS_SCRIPT_OP_PATTERN = 'Techsupport collected at (\S+)\r\n'

CHMOD_BASE_CMD = "chmod "

class __TCData: pass

def delete_ts_dir(tc):
    result = True
    req = api.Trigger_CreateAllParallelCommandsRequest()
    for node in tc.nodes:
        api.Trigger_AddNaplesCommand(req, node, tc.delete_cmd)
    resp = api.Trigger(req)
    try:
        for cmd in resp.commands:
            if cmd.exit_code != 0:
                api.PrintCommandResults(cmd)
                result = False
    except Exception as e:
        api.Logger.error("Exception {} ".format(str(e)))
    return result

def get_techsupport_file(cmd_output):
    api.Logger.verbose(f"techsupport script output {cmd_output}")
    matches = re.search(TS_SCRIPT_OP_PATTERN, cmd_output)
    ts_file = matches.groups()[0] if matches else None
    return ts_file

def trigger_techsupport_requests(tc):
    result = delete_ts_dir(tc)
    if result != True:
        api.Logger.error(f"Failed to delete {TS_DIR} before request")
        return result

    req = api.Trigger_CreateAllParallelCommandsRequest()
    for node in tc.nodes:
        api.Logger.info(f"Collecting techsupport from {node}")
        api.Trigger_AddNaplesCommand(req, node, TS_SCRIPT)
    tc.resp = api.Trigger(req)
    return True

def fix_file_permission(node, file, perm='644'):
    cmd = f"{CHMOD_BASE_CMD} {perm} {file}"
    req = api.Trigger_CreateAllParallelCommandsRequest()
    api.Trigger_AddNaplesCommand(req, node, cmd)
    resp = api.Trigger(req)
    if resp and resp.commands[0].exit_code == 0:
        return True
    return False

def collect_techsupports(tc):
    result = True
    if tc.resp is None:
        api.Logger.error("Failed to trigger techsupport requests")
        return False

    for cmd in tc.resp.commands:
        node = cmd.node_name
        if cmd.exit_code != 0:
            api.PrintCommandResults(cmd)
            api.Logger.error(f"Failed to collect techsupport for {node}")
            result = False
        else:
            ts_file = get_techsupport_file(cmd.stdout)
            if ts_file is None:
                api.Logger.error(f"Failed to parse techsupport op for {node}")
                result = False
                continue
            if not fix_file_permission(node, ts_file):
                api.Logger.error(f"Failed to fix perm for techsupport on {node}")
                result = False
                continue
            # copy out the generated techsupport tarball from naples
            api.Logger.debug(f"Copying out {ts_file} from {node} to {tc.dir}")
            api.ChangeDirectory("/")
            api.CopyFromNaples(node, [ts_file], tc.dir, True)
    return result

def __setup(tc):
    tc.nodes = api.GetNaplesHostnames()
    if len(tc.nodes) == 0:
        api.Logger.error("No naples node found")
        return False
    tc.delete_cmd = f"rm -rf {TS_DIR}"
    return True

def __trigger(tc):
    return trigger_techsupport_requests(tc)

def __verify(tc):
    return collect_techsupports(tc)

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
    if not __setup(tc) or not __trigger(tc) or not __verify(tc) or not __teardown(tc):
       return api.types.status.FAILURE
    return api.types.status.SUCCESS

def Main(tc):
    api.Logger.verbose("Collecting TechSupport")
    res = __collect_techsupport(tc)
    api.Logger.verbose(f"Collecting TechSupport - result {res}")
    return res
