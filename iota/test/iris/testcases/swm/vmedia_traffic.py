#! /usr/bin/python3
import paramiko
import logging
import sys
import time
import traceback

import iota.harness.api as api
from iota.harness.infra.redfish import redfish_client
from iota.harness.infra.redfish.rest.v1 import ServerDownOrUnreachableError
from .utils.getcfg import getcfg
from .utils.vmedia_ops import mount_vmedia
from .utils.vmedia_ops import eject_vmedia
from .utils.ncsi_ops import check_set_ncsi
from .utils.common import get_redfish_obj

logging.basicConfig(stream=sys.stdout, level=logging.INFO)

VMEDIA_PATH = "http://package-mirror.test.pensando.io/iota/swm_test/test_vmedia_iperf.iso"

def __execute_cmd(node_name, cmd, timeout=None):
    req = api.Trigger_CreateExecuteCommandsRequest(serial=True)
    if timeout:
        api.Trigger_AddHostCommand(req, node_name, cmd, timeout=timeout)
    else:
        api.Trigger_AddHostCommand(req, node_name, cmd)

    resp = api.Trigger(req)
    if resp is None:
        raise RuntimeError("Failed to trigger on host %s cmd %s" % (node_name, cmd))
    return resp

def Setup(tc):
    tc.mounted = False
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
        
    tc.ilo_ip = cimc_info.GetIp()
    tc.ilo_ncsi_ip = cimc_info.GetNcsiIp()
    try:
        check_set_ncsi(cimc_info)
        # Create a Redfish client object
        tc.RF = get_redfish_obj(cimc_info, mode="ncsi")
        # Mount vmedia
        if mount_vmedia(tc.RF, VMEDIA_PATH) != api.types.status.SUCCESS:
            api.Logger.error("Mounting vmedia unsuccessful")
            return api.types.status.ERROR
        api.Logger.info("Vmedia mount success")
        time.sleep(10)
        tc.mounted = True
    except:
        api.Logger.error(traceback.format_exc())
        return api.types.status.ERROR

    return api.types.status.SUCCESS

def _run_vmedia_traffic(node_name):
    retries = 10
    for _i in range(retries):
        cddev = "/dev/sr0"
        cmd = "ls %s" % cddev
        resp = __execute_cmd(node_name, cmd)
        cmd = resp.commands.pop()
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            cddev = "/dev/sr1"
            cmd = "ls %s" % cddev
            resp = __execute_cmd(node_name, cmd)
            cmd = resp.commands.pop()
            api.PrintCommandResults(cmd)
            if cmd.exit_code != 0:
                api.Logger.info("Device not available %s" % cddev)
                if _i < (retries - 1):
                    api.Logger.info("Retrying after 30s...")
                    time.sleep(30)
                continue
        api.Logger.info("Vmedia is mapped to device %s" % (cddev))
        cmd = "dd if=%s of=/dev/null bs=1M" % cddev
        resp = __execute_cmd(node_name, cmd, timeout=3600)
        cmd = resp.commands.pop()
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            raise RuntimeError("Vmedia traffic test is not successfull")
        return
    raise RuntimeError("Vmedia device was not detected on the host after %d retries"
                       % (retries))
    

def Trigger(tc):
    try:
        _run_vmedia_traffic(tc.node_name)
    except:
        api.Logger.error(traceback.format_exc())
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Verify(tc):
    return api.types.status.SUCCESS

def Teardown(tc):
    try:
        if tc.mounted is True:
            if eject_vmedia(tc.RF) != api.types.status.SUCCESS:
                api.Logger.error("Ejecting vmedia unsuccessful")
        if tc.RF:
            tc.RF.logout()
    except:
        api.Logger.error(traceback.format_exc())

    return api.types.status.SUCCESS