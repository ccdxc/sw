#! /usr/bin/python3
import paramiko
import logging
import sys

import iota.harness.api as api
from iota.harness.infra.redfish import redfish_client
from iota.harness.infra.redfish.rest.v1 import ServerDownOrUnreachableError
from .utils.getcfg import getcfg
from .utils.vmedia_ops import mount_vmedia
from .utils.vmedia_ops import eject_vmedia
from .utils.ncsi_ops import check_set_ncsi

logging.basicConfig(stream=sys.stdout, level=logging.INFO)

def Setup(tc):
    naples_nodes = api.GetNaplesNodes()
    if len(naples_nodes) == 0:
        api.Logger.error("No naples node found")
        return api.types.status.ERROR
    tc.test_node = naples_nodes[0]
    tc.node_name = tc.test_node.Name

    cimc_info = tc.test_node.GetCimcInfo()
    if not cimc_info:
        api.Logger.error("CimcInfo is None, exiting")
        return api.types.status.ERROR
        
    tc.ilo_ip = cimc_info.GetIp()
    tc.ilo_ncsi_ip = cimc_info.GetNcsiIp()
    tc.ilo_uname = cimc_info.GetUsername()
    tc.ilo_password = cimc_info.GetPassword()
    try:
        #check_set_ncsi(tc.cfg)
        # Create a Redfish client object
        tc.RF = redfish_client(base_url="https://%s" % (tc.ilo_ncsi_ip),
                               username=tc.ilo_username,
                               password=tc.ilo_password)
        # Login with the Redfish client
        tc.RF.login()

    except ServerDownOrUnreachableError:
        api.Logger.error("{} ILO ip not reachable or does not support RedFish".format(tc.cfg['NCSI_IP']))
        return api.types.status.ERROR
    except Exception as e:
        api.Logger.error(str(e))
        return api.types.status.ERROR

    return api.types.status.SUCCESS


def Trigger(tc):
    mgmt_ip = "192.168.68.1"
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    ping_cmd = "ping -I oob_mnic0 -c3 %s" % (mgmt_ip)
    api.Trigger_AddNaplesCommand(req, tc.node_name, "dhclient oob_mnic0")
    api.Trigger_AddNaplesCommand(req, tc.node_name, "ifconfig oob_mnic0")
    api.Trigger_AddNaplesCommand(req, tc.node_name, ping_cmd)
    resp = api.Trigger(req)
    for cmd in resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            return api.types.status.FAILURE
    return api.types.status.SUCCESS

def Verify(tc):
    return api.types.status.SUCCESS

def Teardown(tc):
    try:
        if tc.mounted is True:
            eject_vmedia(tc.RF)
        tc.RF.logout()
    except Exception as e:
        api.Logger.error(str(e))
        return api.types.status.ERROR

    return api.types.status.SUCCESS