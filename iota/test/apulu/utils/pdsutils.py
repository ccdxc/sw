#! /usr/bin/python3
# pds utils
import pdb
import os
import iota.harness.api as api

from infra.common.logging import logger

def pdsClearFlows(node_name=None):
    clear_cmd = "/nic/bin/pdsctl clear flow"
    req = api.Trigger_CreateExecuteCommandsRequest(serial=False)
    if node_name:
        api.Trigger_AddNaplesCommand(req, node_name, clear_cmd)
    else:
         for node_name in  api.GetNaplesHostnames():
            api.Trigger_AddNaplesCommand(req, node_name, clear_cmd)
    api.Trigger(req)
    return api.types.status.SUCCESS


def SetPdsLogsLevel(node_name, level):
    cmd = '/nic/bin/pdsctl debug trace --level %s'%level
    req = api.Trigger_CreateExecuteCommandsRequest()

    api.Trigger_AddNaplesCommand(req, node_name, cmd)
    resp = api.Trigger(req)
    api.PrintCommandResults(resp.commands[0])
    if resp.commands[0].exit_code != 0:
        raise Exception("Failed to set the PDS trace level to %s on %s"%(level, node_name))
    return api.types.status.SUCCESS



def GetPdsDefaultLogLevel(node_name):
    # for now there is not pdsctl to get the current trace level,
    # hence returning default trace level as debug
    return 'debug'


def isPdsAlive(node_name=None):
    node_list = []
    if not node_name:
        node_list = api.GetNaplesHostnames()
    else:
        node_list.append(node_name)

    for n in node_list:
        try:
            GetPdsPid(n)
        except:
            return api.types.status.FAILURE
    return api.types.status.SUCCESS


def GetPdsPid(node_name):
    if not node_name:
        raise Exception("Invalid node name Argument")

    cmd = "pidof pdsagent"
    req = api.Trigger_CreateExecuteCommandsRequest()
    api.Trigger_AddNaplesCommand(req, node_name,cmd)
    resp = api.Trigger(req)

    for cmd in resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            raise Exception("Could not find the PDS Agent process on %s"%(node_name))
        else:
            return int(cmd.stdout.strip())

