#! /usr/bin/python3
import iota.harness.api as api
from iota.harness.infra.redfish import redfish_client
from iota.harness.infra.redfish.rest.v1 import ServerDownOrUnreachableError
from .utils.ping import ping
from .utils.getcfg import getcfg
from .utils.ncsi_ops import check_set_ncsi
from iota.test.iris.testcases.penctl.common import GetNaplesFruJson
from iota.test.iris.testcases.penctl.common import GetNaplesFrequency

import json

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
    try:
        check_set_ncsi(cimc_info)
    except ServerDownOrUnreachableError:
        api.Logger.error("{} ILO ip not reachable or does not support RedFish"
                         % tc.ilo_ncsi_ip)
        return api.types.status.ERROR
    except Exception as e:
        api.Logger.error(str(e))
        return api.types.status.ERROR

    return api.types.status.SUCCESS

def Trigger(tc):
    try:
        api.Logger.info("Issuing server reset")
        ret = api.RestartNodes([tc.node_name], 'reboot')
        if ret != api.types.status.SUCCESS:
            api.Logger.error("server reset failed")
            return api.types.status.FAILURE

        tc.test_node.WaitForHost()
        api.Logger.info("Server reboot successful")
        
        naples_freq = GetNaplesFrequency(tc.node_name)
        naples_fru = GetNaplesFruJson(tc.node_name)
        if naples_fru['product-name'] == "NAPLES 25" and naples_freq != 416:
            api.Logger.error("Frequency is %d not 416"
                             % naples_freq)
            return api.types.status.FAILURE 
    except Exception as e:
        api.Logger.error(str(e))
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Verify(tc):
    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS