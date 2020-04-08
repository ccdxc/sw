#! /usr/bin/python3
import iota.harness.api as api

import iota.test.apulu.verif.utils.techsupport_utils as ts_utils

# Following come from DOL
from apollo.oper.oper import client as OperClient

def trigger_techsupport_request(tc):
    result = ts_utils.DeleteTSDir(tc.nodes)
    if result != True:
        api.Logger.error("Failed to clean techsupport in naples before request")
        return result

    # get testcase directory to copy out generated techsupports
    tc_dir = tc.GetLogsDir()
    tc.techsupportTarBalls = []
    for node in tc.nodes:
        # initiate techsupport collect request
        ts_obj = OperClient.GetTechSupportObject(node)
        ts_obj.SetSkipCores(tc.skipcores)
        ts_file = ts_obj.Collect()
        api.Logger.debug(f"Collecting techsupport from {node} skipcores {tc.skipcores}")
        if api.GlobalOptions.dryrun:
            continue
        if ts_file is None:
            api.Logger.error(f"Failed to collect techsupport for {node}")
            result = False
            continue
        api.Logger.debug(f"Copying out {ts_file} from {node} to {tc_dir}")
        # copy out the generated techsupport tarball from naples
        api.CopyFromNaples(node, [ts_file], tc_dir, True)
        tc.techsupportTarBalls.append(tc_dir+ts_utils.GetTechsupportFilename(ts_file))
    api.Logger.verbose(f"TS: collected techsupports {tc.techsupportTarBalls}")
    return result

def Setup(tc):
    result = api.types.status.SUCCESS
    tc.nodes = api.GetNaplesHostnames()
    if len(tc.nodes) == 0:
        api.Logger.error("No naples nodes found")
        result = api.types.status.FAILURE
    tc.skipcores = getattr(tc.iterators, "skipcores", False)
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
    if api.GlobalOptions.dryrun:
        # no techsupport files to verify in case of dryrun
        api.Logger.verbose(f"TS: DryRun Verify returned {result}")
        return result
    if tc.skipcores == ts_utils.IsCoreCollected(tc.techsupportTarBalls):
        api.Logger.error(f"TS: collect core verification failed")
        result = api.types.status.FAILURE
    cores = ts_utils.GetCoreFiles(tc.techsupportTarBalls)
    if cores:
        api.Logger.error(f"TS: core(s) {cores} found in techsupport")
        result = api.types.status.FAILURE
    api.Logger.verbose(f"TS: Verify returned {result}")
    return result

def Teardown(tc):
    result = api.types.status.SUCCESS
    # delete techsupport in naples post copy
    if not ts_utils.DeleteTSDir(tc.nodes):
        api.Logger.error("Failed to cleanup techsupport during teardown")
        result = api.types.status.FAILURE
    api.Logger.verbose(f"TS: Teardown returned {result}")
    return result
