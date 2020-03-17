#! /usr/bin/python3
import time
import traceback

import iota.harness.api as api
from iota.harness.infra.redfish import redfish_client
from .utils.ping import ping
from .utils.getcfg import getcfg
from .utils.ncsi_ops import check_set_ncsi
from .utils.ncsi_ops import check_ncsi_conn
from iota.test.utils.naples import GetNaplesUptime

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
    try:
        for _iter in range(tc.iterators.count):
            reboot_type = tc.iterators.reboot_types
            api.Logger.info("Iter: %d" % _iter)
            uptime1 = GetNaplesUptime(tc.node_name)
            api.Logger.info("Naples uptime1 %s" % str(uptime1))
            start_time = time.time()
            if reboot_type == "offon":
                api.Logger.info("Powering off the server")
                ret = api.IpmiNodes([tc.node_name], ipmiMethod="off", useNcsi=True)
                if ret != api.types.status.SUCCESS:
                    api.Logger.info("IPMI power off failed")
                    return api.types.status.FAILURE
                api.Logger.info("Server powered off")
                time.sleep(5)
                api.Logger.info("Powering on the server")
                ret = api.IpmiNodes([tc.node_name], ipmiMethod="on", useNcsi=True)
                if ret != api.types.status.SUCCESS:
                    api.Logger.info("IPMI power on failed")
                    return api.types.status.FAILURE
                api.Logger.info("Server powered on")
            else: 
                api.Logger.info("Issuing IPMI server reboot: %s" % reboot_type)
                ret = api.IpmiNodes([tc.node_name], ipmiMethod=reboot_type, useNcsi=True)
                if ret != api.types.status.SUCCESS:
                    api.Logger.info("IPMI server restart failed")
                    return api.types.status.FAILURE
                api.Logger.info("IPMI server restart done")
                
            tc.test_node.WaitForHost()
            elapsed_time = time.time() - start_time
            time.sleep(15)
            uptime2 =  GetNaplesUptime(tc.node_name)
            api.Logger.info("Naples uptime2 %s elapsed time %s" % (str(uptime2), str(elapsed_time)))
            if uptime2 < (uptime1 + elapsed_time):
                api.Logger.error("Naples rebooted upon server reset, exiting")
                return api.types.status.FAILURE
            ret = check_ncsi_conn(tc.cimc_info)
            if ret != api.types.status.SUCCESS:
                api.Logger.error("Unable to connect ot ILO in NCSI mode")
                return api.types.status.FAILURE
            api.Logger.info("IPMI server %s successfull" % reboot_type)
    except:
        api.Logger.error(traceback.format_exc())
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Verify(tc):
    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS