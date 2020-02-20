#! /usr/bin/python3
import traceback
import time

import iota.harness.api as api
from iota.harness.infra.redfish import redfish_client
from iota.harness.infra.redfish.rest.v1 import ServerDownOrUnreachableError
from .utils.getcfg import getcfg
from .utils.ahs import download_ahs
from threading import Event, Thread
from .utils.ping import ping
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
    try:
        check_set_ncsi(cimc_info)
        # Create a Redfish client object
        tc.RF = get_redfish_obj(cimc_info, mode="ncsi")
    except:
        api.Logger.error(traceback.format_exc())
        return api.types.status.ERROR

    return api.types.status.SUCCESS

def ahs_loop(RF, stop_ev, err_ev):
    try:
        while not stop_ev.isSet():
            download_ahs(RF, dtype="DAY")
            time.sleep(5)
    except:
        err_ev.set()
    return

def Trigger(tc):
    stop_ev = Event()
    err_ev = Event()
    thr = Thread(target=ahs_loop, args=(tc.RF, stop_ev, err_ev,))
    thr.start()
    try:
        for _iter in range(tc.iterators.count):
            if err_ev.isSet() is True:
                raise RuntimeError('AHS error')
            api.Logger.info('Run: %d' % _iter)
            api.Logger.info("Issuing server restart")
            ret = api.RestartNodes([tc.node_name], 'reboot')
            if ret != api.types.status.SUCCESS:
                api.Logger.info("server restart failed")
                return api.types.status.FAILURE
            api.Logger.info("server restart done")
            tc.test_node.WaitForHost()
        stop_ev.set()
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