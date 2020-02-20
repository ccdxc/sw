#! /usr/bin/python3
import time
import traceback

import iota.harness.api as api
from iota.harness.infra.redfish import redfish_client
from iota.harness.infra.redfish.rest.v1 import AuthMethod
from iota.harness.infra.redfish.rest.v1 import ServerDownOrUnreachableError
from .utils.getcfg import getcfg
from .utils.ncsi_ops import switch_ncsi_ports
from .utils.ping import ping
from .utils.ncsi_ops import check_set_ncsi
from .utils.common import get_redfish_obj

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
        check_set_ncsi(tc.cimc_info)
    except:
        api.Logger.error(traceback.format_exc())
        return api.types.status.ERROR

    return api.types.status.SUCCESS

def Trigger(tc):
    max_pings = int(getattr(tc.args, "max_pings", 60))
    try:
        for _iter in range(tc.iterators.count):
            api.Logger.info("Iteration %d" % _iter)
            RF = get_redfish_obj(tc.cimc_info, mode="ncsi")
            ret = switch_ncsi_ports(RF)
            if ret == api.types.status.SUCCESS:
                time.sleep(10)
                ret = ping(tc.ilo_ncsi_ip, max_pings)
                if ret != api.types.status.SUCCESS:
                    api.Logger.error("Unable to ping ILO after NCSI channel switch")
                    return api.types.status.FAILURE
            else:
                api.Logger.error("Config failed for NCSI channel switch")
                return api.types.status.ERROR
            api.Logger.info("NCSI channel switch successful")
    except:
        api.Logger.error(traceback.format_exc())
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Verify(tc):
    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS