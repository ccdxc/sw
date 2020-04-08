#! /usr/bin/python3
from datetime import datetime
import re

import iota.harness.api as api
import iota.test.apulu.utils.naples as naples_utils
import iota.test.apulu.verif.utils.techsupport_utils as ts_utils

TS_SCRIPT = "/nic/tools/collect_techsupport.sh"
TS_SCRIPT_OP_PATTERN = 'Techsupport collected at (\S+)\r\n'

class __TCData: pass

def get_techsupport_file(cmd_output):
    api.Logger.verbose(f"techsupport script output {cmd_output}")
    matches = re.search(TS_SCRIPT_OP_PATTERN, cmd_output)
    ts_file = matches.groups()[0] if matches else None
    return ts_file

def trigger_techsupport_requests(tc):
    result = ts_utils.DeleteTSDir(tc.nodes)
    if result != True:
        api.Logger.error("Failed to clean techsupport in naples before request")
        return result

    req = api.Trigger_CreateAllParallelCommandsRequest()
    for node in tc.nodes:
        api.Logger.info(f"Collecting techsupport from {node}")
        api.Trigger_AddNaplesCommand(req, node, TS_SCRIPT)
    tc.resp = api.Trigger(req)
    return True

def collect_techsupports(tc):
    result = True
    if tc.resp is None:
        api.Logger.error("Failed to trigger techsupport requests")
        return False

    tc.techsupportTarBalls = []
    for cmd in tc.resp.commands:
        node = cmd.node_name
        if cmd.exit_code != 0:
            api.PrintCommandResults(cmd)
            api.Logger.error(f"Failed to collect techsupport for {node}")
            result = False
        else:
            ts_file = get_techsupport_file(cmd.stdout)
            if api.GlobalOptions.dryrun:
                continue
            if ts_file is None:
                api.Logger.error(f"Failed to parse techsupport op for {node}")
                result = False
                continue
            if not naples_utils.ChangeFilesPermission([node], [ts_file]):
                api.Logger.error(f"Failed to fix perm for techsupport on {node}")
                result = False
                continue
            # copy out the generated techsupport tarball from naples
            api.Logger.debug(f"Copying out {ts_file} from {node} to {tc.dir}")
            api.ChangeDirectory("/")
            api.CopyFromNaples(node, [ts_file], tc.dir, True)
            tc.techsupportTarBalls.append(tc.dir+ts_utils.GetTechsupportFilename(ts_file))
    return result

def __setup(tc):
    tc.nodes = api.GetNaplesHostnames()
    if len(tc.nodes) == 0:
        api.Logger.error("No naples node found")
        return False
    return True

def __trigger(tc):
    return trigger_techsupport_requests(tc)

def __verify(tc):
    if not collect_techsupports(tc):
        return False
    if api.GlobalOptions.dryrun:
        # no techsupport files to verify in case of dryrun
        api.Logger.verbose(f"DryRun: nothing to be verified")
        return True
    if not ts_utils.IsCoreCollected(tc.techsupportTarBalls):
        api.Logger.error(f"Core directory is not collected in techsupport")
        return False
    cores = ts_utils.GetCoreFiles(tc.techsupportTarBalls)
    if cores:
        api.Logger.error(f"Core files {cores} found in techsupport")
        return False
    return True

def __teardown(tc):
    # delete techsupport in naples post copy
    result = ts_utils.DeleteTSDir(tc.nodes)
    if result != True:
        api.Logger.error("Failed to cleanup techsupport during teardown")
    return result

def __collect_techsupport(testcase):
    tc = __TCData()
    if testcase:
        # techsupport collection as part of testcase failure
        tc.dir = testcase.GetLogsDir()
    else:
        # techsupport collection as part of testsuite teardown
        tc.dir = api.GetTestsuiteLogsDir()
    tc.dir += "/"
    if not __setup(tc) or not __trigger(tc) or not __verify(tc) or not __teardown(tc):
       return api.types.status.FAILURE
    return api.types.status.SUCCESS

def Main(tc):
    api.Logger.verbose("Collecting TechSupport")
    res = __collect_techsupport(tc)
    api.Logger.info(f"Collecting TechSupport - result {res}")
    return res
