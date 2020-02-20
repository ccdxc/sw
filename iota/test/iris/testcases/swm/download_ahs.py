#! /usr/bin/python3

import time
import traceback

import iota.harness.api as api
from iota.harness.infra.redfish import redfish_client
from iota.harness.infra.redfish.rest.v1 import ServerDownOrUnreachableError
from .utils.ahs import download_ahs
from .utils.ncsi_ops import check_set_ncsi
from .utils.common import get_redfish_obj

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
    
    try:
        check_set_ncsi(cimc_info)
        tc.RF = get_redfish_obj(cimc_info, mode="ncsi")
    except:
        api.Logger.error(traceback.format_exc())
        return api.types.status.ERROR

    return api.types.status.SUCCESS

def Trigger(tc):
    try:
        for _iter in range(tc.iterators.count):
            api.Logger.info("Iter %d" %(_iter))
            download_ahs(tc.RF, dtype="DAY", file_path='/tmp/ahs_dump.txt')
            time.sleep(2)
    except:
        api.Logger.error(traceback.format_exc())
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Verify(tc):
    return api.types.status.SUCCESS

def Teardown(tc):
    try:
        if tc.RF:
            tc.RF.logout()
    except:
        api.Logger.error(traceback.format_exc())
        return api.types.status.ERROR

    return api.types.status.SUCCESS