#! /usr/bin/python3
import paramiko
import logging
import sys
import time
import traceback

import iota.harness.api as api
from iota.harness.infra.redfish import redfish_client
from .utils.ping import ping
from .utils.ncsi_ops import check_set_ncsi
from .utils.nic_ops import get_nic_mode
from .utils.ncsi_ops import set_ncsi_mode
from .utils.dedicated_mode_ops import set_dedicated_mode
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

    if ping(tc.ilo_ip, 3) == api.types.status.SUCCESS:
        tc.initial_mode = "dedicated"
    elif ping(tc.ilo_ncsi_ip, 3) == api.types.status.SUCCESS:
        tc.initial_mode = "ncsi"
    else:
        api.Logger.error('ILO unreachable')
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Trigger(tc):
    max_pings = int(getattr(tc.args, "max_pings", 60))
    mode = tc.initial_mode
    try:
        for _i in range(tc.iterators.count):
            RF = get_redfish_obj(tc.cimc_info, mode=mode)
            obs_mode = get_nic_mode(RF)
            api.Logger.info("Iteration %d: curr_mode %s" % (_i, obs_mode))
            if mode != obs_mode:
                raise RuntimeError("Expected NIC mode %s, observed %s" % (mode, obs_mode))

            next_mode = "dedicated" if mode == "ncsi" else "ncsi"
            if next_mode == "ncsi":
                ret = set_ncsi_mode(RF, mode="dhcp")
            else:
                ret = set_dedicated_mode(RF, mode="dhcp")
            if ret != api.types.status.SUCCESS:
                api.Logger.error("Mode switch from %s -> %s failed" %(mode, next_mode))
                return api.types.status.FAILURE

            api.Logger.info("Switched mode to %s" % (next_mode))
            time.sleep(5)

            if ret == api.types.status.SUCCESS:
                curr_ilo_ip = tc.ilo_ip if next_mode == "dedicated" else tc.ilo_ncsi_ip
                ret = ping(curr_ilo_ip, max_pings)
                if ret != api.types.status.SUCCESS:
                    RF.logout()
                    raise RuntimeError('Unable to ping ILO, Port Switch fail from'
                                      ' %s -> %s' % (mode, next_mode))
                api.Logger.info("Mode switch from %s -> %s successful" % (mode, next_mode))
            else:
                raise RuntimeError('Mode switch config failed')
            mode = next_mode
    except:
        api.Logger.error(traceback.format_exc())
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Verify(tc):
    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS