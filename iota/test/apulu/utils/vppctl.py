#! /usr/bin/python3
# vppctl utils
import iota.harness.api as api

__CMDBASE  = '/nic/bin/vppctl'
__CMDSEP  = ' '

__CMDTYPE_ERR = 'errors'
__CMDTYPE_SHOW  = 'show'
__CMDTYPE_CLEAR = 'clear'

vpp_path = 'PATH=$PATH:/nic/bin/; LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/nic/lib/vpp_plugins/:/nic/lib/; export PATH; export LD_LIBRARY_PATH; '

def __execute_vppctl(node, cmd):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    api.Trigger_AddNaplesCommand(req, node, cmd)

    resp = api.Trigger(req)
    resp_cmd = resp.commands[0]

    api.PrintCommandResults(resp_cmd)

    # exit_code always return 0
    return resp_cmd.exit_code == 0, resp_cmd.stdout

def ExecuteVPPctlCommand(node, cmd, args=None):
    cmd = vpp_path + __CMDBASE + __CMDSEP + cmd
    if args is not None:
        cmd = cmd + __CMDSEP + args
    return __execute_vppctl(node, cmd)

def ExecuteVPPctlShowCommand(node, cmd, args=None):
    cmd = __CMDTYPE_SHOW + __CMDSEP + cmd
    return ExecuteVPPctlCommand(node, cmd, args)

def ExecuteVPPctlClearCommand(node, cmd, args=None):
    cmd = __CMDTYPE_CLEAR + __CMDSEP + cmd
    return ExecuteVPPctlCommand(node, cmd, args)

def ParseShowCommand(node_name, type, args=None):
    ret = api.types.status.SUCCESS
    if api.GlobalOptions.dryrun:
        return api.types.status.SUCCESS, None
    res, resp = ExecuteVPPctlShowCommand(node_name, type, args)
    if res != True:
        api.Logger.error(f"Failed to execute show {type} at node {node_name} : {resp}")
        ret = api.types.status.FAILURE

    return ret, resp

def ParseClearCommand(node_name, type, args=None):
    if api.GlobalOptions.dryrun:
        return api.types.status.SUCCESS

    ret, resp = ExecuteVPPctlClearCommand(node_name, type, args)
    if ret != True:
        api.Logger.error(f"Failed to execute clear {type} at node {node_name} : {resp}")
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

