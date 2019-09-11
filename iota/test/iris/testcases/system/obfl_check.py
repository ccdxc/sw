#! /usr/bin/python3
import iota.harness.api as api

import string
import time

def parse_ls(output):
    files = {}
    lines = output.split('\n')
    for line in lines:
        api.Logger.info("Parsing line: %s" % (line))
        fields = line.split()
        if len(fields) < 9:
            continue
        files[fields[8]] = int(fields[4])
    return files

def load_log_files(tc):
    req = api.Trigger_CreateExecuteCommandsRequest()
    n = tc.Nodes[0]
    api.Trigger_AddNaplesCommand(req, tc.Nodes[0], "ls -l --color=never /obfl")
    res = api.Trigger(req)
    api.Logger.info(res)
    api.PrintCommandResults(res.commands[0])
    return parse_ls(res.commands[0].stdout)

def compare_files(before, after):
    success = True
    for f, s in after.items():
        if f not in before:
            success = False
            api.Logger.error("New file '%s' in obfl" % (f))
            continue
        ns = before[f]
        if ns > s:
            success = False
            api.Logger.error(
                "File '%s' in obfl increased from %d to %d" % (f, s, ns))
            continue
        api.Logger.info("File '%s' in obfl was %d is %d" % (f, s, ns))
    return success

def Setup(tc):
    tc.Nodes = api.GetNaplesHostnames()
    return api.types.status.SUCCESS

def Trigger(tc):
    before = load_log_files(tc)
    time.sleep(5 * 60)
    after = load_log_files(tc)
    if compare_files(before, after) == False:
        return api.types.status.FAILURE
    return api.types.status.SUCCESS

def Verify(tc):
    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS
