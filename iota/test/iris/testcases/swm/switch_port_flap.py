#! /usr/bin/python3
import traceback

import iota.harness.api as api
from iota.harness.infra.redfish import redfish_client
from iota.harness.infra.redfish.rest.v1 import ServerDownOrUnreachableError
from .utils.ping import ping
from .utils.getcfg import getcfg
from .utils.ncsi_ops import check_set_ncsi
from .utils.ncsi_ops import check_ncsi_conn
import time

def Setup(tc):
    naples_nodes = api.GetNaplesNodes()
    if len(naples_nodes) == 0:
        api.Logger.error("No naples node found")
        return api.types.status.ERROR
    tc.test_node = naples_nodes[0]
    tc.node_name = tc.test_node.Name()

    cimc_info = tc.test_node.GetCimcInfo()
    if not cimc_info:
        api.Logger.error("CimcInfo is None, exiting")
        return api.types.status.ERROR
        
    tc.ilo_ip = cimc_info.GetIp()
    tc.ilo_ncsi_ip = cimc_info.GetNcsiIp()
    tc.cimc_info = cimc_info
    try:
        check_set_ncsi(cimc_info)
    except:
        api.Logger.error(traceback.format_exc())
        return api.types.status.ERROR

    return api.types.status.SUCCESS

def Trigger(tc):
    try:
        for _i in range(tc.iterators.count):
            api.Logger.info("Iter: %d" % _i)
            ret = check_ncsi_conn(tc.cimc_info)
            if ret != api.types.status.SUCCESS:
                api.Logger.error("ILO not accessible in NCSI mode")
                return api.types.status.ERROR
            api.Logger.info("Flapping data ports")
            ret = api.FlapDataPorts([tc.node_name], num_ports_per_node=2, down_time=30,
                                    flap_count=1)
            if ret != api.types.status.SUCCESS:
                api.Logger.error("Data ports flap not successfull")
                return api.types.status.ERROR
            api.Logger.info("Data ports flap successfull")

            ret = check_ncsi_conn(tc.cimc_info)
            if ret != api.types.status.SUCCESS:
                api.Logger.error("ILO not accessible in NCSI mode")
                return api.types.status.ERROR
    except:
        api.Logger.error(traceback.format_exc())
        return api.types.status.ERROR
    return api.types.status.SUCCESS

def Verify(tc):
    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS