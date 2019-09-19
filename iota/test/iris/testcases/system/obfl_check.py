#! /usr/bin/python3
import iota.harness.api as api

import string
import time

INGORE_FILES = [ 'sysmond' ]

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

def clean_log_files(tc):
    req = api.Trigger_CreateExecuteCommandsRequest()
    n = tc.Nodes[0]
    api.Trigger_AddNaplesCommand(req, tc.Nodes[0], "rm -f /obfl/*.log.[0-9]")
    res = api.Trigger(req)
    api.Logger.info(res)
    api.PrintCommandResults(res.commands[0])

def load_log_files(tc):
    req = api.Trigger_CreateExecuteCommandsRequest()
    n = tc.Nodes[0]
    api.Trigger_AddNaplesCommand(req, tc.Nodes[0], "ls -l --color=never /obfl")
    res = api.Trigger(req)
    api.Logger.info(res)
    api.PrintCommandResults(res.commands[0])
    return parse_ls(res.commands[0].stdout)

def is_ignored(filename):
    for ignored_file in INGORE_FILES:
        if ignored_file in filename:
            return True
    return False

def compare_files(before, after):
    success = True
    for filename, new_size in after.items():
        if is_ignored(filename):
            api.Logger.info("Ignoring '%s'" % (filename))
            continue
        if filename not in before:
            success = False
            api.Logger.error("New file '%s' in obfl" % (filename))
            continue
        old_size = before[filename]
        if new_size != old_size:
            success = False
            api.Logger.error(
                "File '%s' in obfl increased from %d to %d" %
                (filename, old_size, new_size))
            continue
        api.Logger.info("File '%s' in obfl was %d is %d" %
                        (filename, old_size, new_size))
    return success

def Setup(tc):
    tc.Nodes = api.GetNaplesHostnames()
    return api.types.status.SUCCESS

def Trigger(tc):
    clean_log_files(tc)
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
