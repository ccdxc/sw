#! /usr/bin/python3
import iota.harness.api as api

def __execute_naples_cmd(naples_nodes, cmds):
    result = True
    req = api.Trigger_CreateAllParallelCommandsRequest()
    for node in naples_nodes:
        for cmd in cmds:
            api.Trigger_AddNaplesCommand(req, node, cmd)
    resp = api.Trigger(req)
    for cmd in resp.commands:
        if cmd.exit_code != 0:
            api.PrintCommandResults(cmd)
            result = False
    return result

def ChangeFilesPermission(naples_nodes, files, perm='644'):
    chmodCmd = f"chmod {perm} "
    chmodCmds = map((lambda x: chmodCmd + x), files)
    return __execute_naples_cmd(naples_nodes, chmodCmds)

def DeleteDirectories(naples_nodes, dirs):
    deleteCmd = "rm -rf "
    deleteCmds = map((lambda x: deleteCmd + x), dirs)
    return __execute_naples_cmd(naples_nodes, deleteCmds)
