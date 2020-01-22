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

def __execute_pdsctl(host, cmd):
    retval = True

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    api.Trigger_AddNaplesCommand(req, host, cmd)

    resp = api.Trigger(req)
    resp_cmd = resp.commands[0]

    api.PrintCommandResults(resp_cmd)

    return resp_cmd.exit_code == 0, resp_cmd.stdout

def ExecutePdsctlCommand(host, cmd, args=None, yaml=True):
    cmd = __CMDBASE + __CMDSEP + cmd
    if args is not None:
        cmd = cmd + __CMDSEP + args
    if yaml:
        cmd = cmd + __CMDSEP + __CMDFLAG_YAML
    return __execute_pdsctl(host, cmd)

def ExecutePdsctlShowCommand(host, cmd, args=None, yaml=True):
    cmd = __CMDTYPE_SHOW + __CMDSEP + cmd
    return ExecutePdsctlCommand(host, cmd, args, yaml)

def GetObjects(objtype):
    # get object name
    objName = objtype.name.lower()
    # XXX for each host?
    return ExecutePdsctlShowCommand(None, objName)

def GetObject(objtype, keyid):
    # get object name
    objName = objtype.name.lower()
    args = __CMDFLAG_ID + __CMDSEP + keyid
    # XXX for each host?
    return ExecutePdsctlShowCommand(None, objName, args)
