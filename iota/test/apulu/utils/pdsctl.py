#! /usr/bin/python3
# pdsctl utils
import pdb
import subprocess
import os

from infra.common.logging import logger

import iota.harness.api as api

__CMDBASE  = '/nic/bin/pdsctl'
__CMDSEP  = ' '

__CMDTYPE_SHOW  = 'show'
__CMDTYPE_CLEAR = 'clear'

__CMDFLAG_YAML = '--yaml'
__CMDFLAG_ID   = '--id'

def __execute_pdsctl(node, cmd):
    retval = True

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    api.Trigger_AddNaplesCommand(req, node, cmd)

    resp = api.Trigger(req)
    resp_cmd = resp.commands[0]

    api.PrintCommandResults(resp_cmd)

    return resp_cmd.exit_code == 0, resp_cmd.stdout

def ExecutePdsctlCommand(node, cmd, args=None, yaml=True):
    cmd = __CMDBASE + __CMDSEP + cmd
    if args is not None:
        cmd = cmd + __CMDSEP + args
    if yaml:
        cmd = cmd + __CMDSEP + __CMDFLAG_YAML
    return __execute_pdsctl(node, cmd)

def ExecutePdsctlShowCommand(node, cmd, args=None, yaml=True):
    cmd = __CMDTYPE_SHOW + __CMDSEP + cmd
    return ExecutePdsctlCommand(node, cmd, args, yaml)

def GetObjects(node, objtype):
    # get object name
    objName = objtype.name.lower()
    return ExecutePdsctlShowCommand(node, objName)

