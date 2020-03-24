#! /usr/bin/python3
from datetime import datetime

import iota.harness.api as api

TS_DIR  = "/data/techsupport/"
TS_BIN  = "/nic/bin/techsupport"
TS_CONF = "/nic/conf/techsupport.json"

CHMOD_BASE_CMD = "chmod 644 "

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

def generate_ts_filename(node):
    tstamp = datetime.now().strftime("%Y%m%d-%Hh%Mm%Ss")
    ts_file = f"techsupport-{node}-{tstamp}.tar.gz"
    return ts_file

def get_ts_cmd(ts_file):
    # run techsupport without skipcores option
    ts_cmd = f"{TS_BIN} -c {TS_CONF} -d {TS_DIR} -o {ts_file}"
    return ts_cmd

def trigger_techsupport_requests(tc):
    result = delete_ts_dir(tc)
    if result != True:
        api.Logger.error(f"Failed to delete {TS_DIR} before request")
        return result

    req = api.Trigger_CreateExecuteCommandsRequest()
    cmd_cookies = []
    for node in tc.nodes:
        api.Logger.info(f"Collecting techsupport from {node}")
        ts_filename = generate_ts_filename(node)
        ts_file = TS_DIR + ts_filename
        # initiate techsupport collect request
        ts_cmd = get_ts_cmd(ts_filename)
        api.Logger.verbose(f"Running {ts_cmd} on {node}")
        api.Trigger_AddNaplesCommand(req, node, ts_cmd)
        # set the proper permissions so that we can copy it out
        chmod_cmd = CHMOD_BASE_CMD + ts_file
        api.Trigger_AddNaplesCommand(req, node, chmod_cmd)
        # store the ts info to retrieve later
        cmd_cookies.append((node, ts_file))

    tc.resp = api.Trigger(req)
    tc.cmd_cookies = cmd_cookies
    return result

def collect_techsupports(tc):
    result = True
    if tc.resp is None:
        api.Logger.error("Failed to trigger techsupport requests")
        return False

    cookie_idx = 0
    cmd_iter = iter(tc.resp.commands)
    for ts_cmd, chmod_cmd in zip(cmd_iter, cmd_iter):
        node, ts_file = tc.cmd_cookies[cookie_idx]
        # check if any of the cmd failed
        if any([ts_cmd.exit_code, chmod_cmd.exit_code]):
            api.PrintCommandResults(ts_cmd)
            api.PrintCommandResults(chmod_cmd)
            api.Logger.error(f"Failed to collect techsupport for {node}")
            result = False
        else:
            # copy out the generated techsupport tarball from naples
            api.Logger.debug(f"Copying out {ts_file} from {node} to {tc.dir}")
            api.CopyFromNaples(node, [ts_file], tc.dir, True)
        cookie_idx += 1
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
