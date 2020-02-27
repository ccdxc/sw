#! /usr/bin/python3
import paramiko
import logging
import traceback

import iota.harness.api as api
from iota.harness.infra.redfish import redfish_client
from iota.harness.infra.redfish.rest.v1 import ServerDownOrUnreachableError
from .utils.getcfg import getcfg
from .utils.ncsi_ops import check_set_ncsi
from iota.test.utils.naples import GetOOBMnicIP


def Setup(tc):
    naples_nodes = api.GetNaplesNodes()
    if len(naples_nodes) == 0:
        api.Logger.error("No naples node found")
        return api.types.status.ERROR
    tc.test_node = naples_nodes[0]
    tc.node_name = tc.test_node.Name()

    tc.cimc_info = tc.test_node.GetCimcInfo()
    if not tc.cimc_info:
        api.Logger.error("CimcInfo is None, exiting")
        return api.types.status.ERROR
    try:
        check_set_ncsi(tc.cimc_info)
    except:
        api.Logger.error(traceback.format_exc())
        return api.types.status.ERROR

    return api.types.status.SUCCESS


def Trigger(tc):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    api.Trigger_AddNaplesCommand(req, tc.node_name, "ifconfig oob_mnic0 up")
    api.Trigger_AddNaplesCommand(req, tc.node_name, "dhclient oob_mnic0", timeout=300)
    
    resp = api.Trigger(req)
    if not resp:
        api.Logger.error("Failed to run cmds")
        return api.types.status.ERROR
    for cmd in resp.commands:
        if cmd.exit_code != 0:
            return api.types.status.ERROR
    
    oob_ip = GetOOBMnicIP(tc.node_name)
    if oob_ip:
        api.Logger.error("oob_mnic got an IP")
        return api.types.status.FAILURE
    return api.types.status.SUCCESS

def Verify(tc):
    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS