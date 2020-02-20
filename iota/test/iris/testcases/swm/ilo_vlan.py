#! /usr/bin/python3

import time
import traceback

import iota.harness.api as api
from iota.harness.infra.redfish import redfish_client
from iota.harness.infra.redfish.rest.v1 import ServerDownOrUnreachableError
from .utils.ahs import download_ahs
from .utils.ncsi_ops import check_set_ncsi
from .utils.common import get_redfish_obj
from .utils.nic_ops import enable_vlan_mode
from .utils.nic_ops import get_nic_obj

def Setup(tc):
    tc.RF = None
    naples_nodes = api.GetNaplesNodes()
    if len(naples_nodes) == 0:
        api.Logger.error("No naples node found, exiting...")
        return api.types.status.ERROR
    tc.test_node = naples_nodes[0]
    tc.node_name = tc.test_node.Name()

    cimc_info = tc.test_node.GetCimcInfo()
    if not cimc_info:
        api.Logger.error("CimcInfo is None, exiting")
        return api.types.status.ERROR
    vlan_base = api.Testbed_GetVlanBase()
    if not vlan_base:
        api.Logger.error("Base vlan not specified for the testbed")
        return api.types.status.ERROR
    vlan_count = api.Testbed_GetVlanCount()
    if vlan_count == 1:
        api.Logger.error("Native vlan cannot be used for this test")
        return api.types.status.ERROR
    tc.vlan = vlan_base + 1

    try:
        check_set_ncsi(cimc_info)
        tc.RF = get_redfish_obj(cimc_info, mode="ncsi")
    except:
        api.Logger.error(traceback.format_exc())
        return api.types.status.ERROR

    return api.types.status.SUCCESS

def Trigger(tc):
    try:
        nic_obj = get_nic_obj(tc.RF, nic_type="ncsi")
        if not nic_obj:
            raise RuntimeError("Unable to get shared mode NIC obj")
        if enable_vlan_mode(tc.RF, nic_obj, tc.vlan) != api.types.status.SUCCESS:
            api.Logger.error("Failed to enable vlan on ILO")
            return api.types.status.ERROR
        time.sleep(10)
    except:
        api.Logger.error(traceback.format_exc())
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Verify(tc):
    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS