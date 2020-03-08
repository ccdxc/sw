#! /usr/bin/python3
import traceback
import time

import iota.harness.api as api
from iota.harness.infra.redfish import redfish_client
from iota.harness.infra.redfish.rest.v1 import ServerDownOrUnreachableError
from .utils.getcfg import getcfg
from .utils.ahs import download_ahs
from multiprocessing import Event, Process
from .utils.ping import ping
from .utils.ncsi_ops import check_set_ncsi
from .utils.common import get_redfish_obj

def Setup(tc):
    tc.RF = None
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
        tc.RF = get_redfish_obj(tc.cimc_info, mode="ncsi")
    except:
        api.Logger.error(traceback.format_exc())
        return api.types.status.ERROR

    return api.types.status.SUCCESS

def ahs_loop(RF, fin_ev, stop_ev, err_ev):
    try:
        while True:
            if stop_ev.is_set():
                break
            download_ahs(RF, dtype="DAY")
            time.sleep(5)
    except:
        api.Logger.error(traceback.format_exc())
        err_ev.set()
    finally:
        fin_ev.set()

def Trigger(tc):
    stop_ev = Event()
    err_ev = Event()
    fin_ev = Event()
    thr = Process(target=ahs_loop, args=(tc.RF, fin_ev, stop_ev, err_ev,))
    thr.start()
    err = False
    try:
        for _iter in range(tc.iterators.count):
            if err_ev.is_set() is True:
                raise RuntimeError('AHS error')
            api.Logger.info('Run: %d' % _iter)
            api.Logger.info("Issuing server restart")
            ret = api.IpmiNodes([tc.node_name], ipmiMethod='cycle', useNcsi=True)
            if ret != api.types.status.SUCCESS:
                api.Logger.info("server restart failed")
                return api.types.status.FAILURE
            api.Logger.info("server restart done")
            tc.test_node.WaitForHost()
        
        if err_ev.is_set() is True:
            raise RuntimeError('AHS error')
    except:
        api.Logger.error(traceback.format_exc())
        err = True
    finally:
        stop_ev.set()
        api.Logger.info("Waiting for AHS loop to complete")
        fin_ev.wait(timeout=10)
        thr.terminate()
        thr.join()
    
    if err:
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

    return api.types.status.SUCCESS