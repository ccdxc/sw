#! /usr/bin/python3
# pdsctl utils
import pdb
import subprocess
import os

from infra.common.logging import logger

import iota.test.apulu.config.utils as utils

__CMDTYPE_SHOW  = ' show '
__CMDTYPE_CLEAR = ' clear '

__CMDFLAG_YAML = ' --yaml '
__CMDFLAG_ID   = ' --id '

def __get_pdsctl_path():
    rel_path = "nic/build/aarch64/%s/bin/pdsctl" % (utils.GetPipelineName())
    abs_path = os.path.join(os.environ['WS_TOP'], rel_path)
    return abs_path

def __execute_pdsctl(cmd):
    retval = True
    try:
        op = subprocess.Popen(cmd, stdout=subprocess.PIPE, shell=True)
        output = op.stdout.read().decode('utf-8', 'ignore')
        # pdsctl does not exit with non-zero code in case of failure
        # so check for "rpc error" in output and fix retval
        if 'rpc error' in output:
            retval = False
    except subprocess.CalledProcessError as e:
        output = "Command execution failed."
        retval = False
    logger.info("pdsctl: command[%s], output[%s], retval[%d]" \
                % (cmd, output, retval))
    return retval, output

def ExecutePdsctlCommand(cmd, args=None, yaml=True):
    pdsctl = __get_pdsctl_path()
    finalCmd = pdsctl + cmd
    if args is not None:
        finalCmd = finalCmd + ' ' + args
    if yaml:
        finalCmd = finalCmd + __CMDFLAG_YAML
    return __execute_pdsctl(finalCmd)

def ExecutePdsctlShowCommand(cmd, args=None, yaml=True):
    cmd = __CMDTYPE_SHOW + cmd
    return ExecutePdsctlCommand(cmd, args, yaml)

def GetObjects(objtype):
    # get object name
    objName = objtype.name.lower()
    return ExecutePdsctlShowCommand(objName)

def GetObject(objtype, keyid):
    # get object name
    objName = objtype.name.lower()
    args = __CMDFLAG_ID + keyid
    return ExecutePdsctlShowCommand(objName, args)
