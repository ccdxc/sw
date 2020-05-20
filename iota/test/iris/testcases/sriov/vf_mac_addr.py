#! /usr/bin/python3
import iota.harness.api as api
import iota.test.utils.naples_host as host
import iota.test.iris.config.netagent.hw_push_config as hw_config
from .sriov_utils import CheckVFDefaults
from .sriov_utils import SetVFMac
from .sriov_utils import GetVFMac
from .sriov_utils import GetSupportedVFs
from .sriov_utils import CreateVFs
from .sriov_utils import SetVFTrust


def Setup(tc):
    host = None
    for _node in api.GetNaplesHostnames():
        if api.IsNaplesNode(_node) and api.GetNodeOs(_node) == "linux":
            host = _node
            break
    
    if not host:
        api.Logger.error("Unable to find a Naples node with linux os")
        return api.types.status.ERROR

    tc.host = host
    tc.pf = api.GetNaplesHostInterfaces(host)[0]
    tc.num_vfs = GetSupportedVFs(tc.host, tc.pf)
    api.Logger.info("Host %s PF %s supports %d VFs" % (tc.host, tc.pf, tc.num_vfs))

    if tc.num_vfs == 0:
        api.Logger.warn("Max supported VFs on host %s is 0, expected non-zero" % host)
        return api.types.status.ERROR

    if CreateVFs(tc.host, tc.pf, tc.num_vfs)!= api.types.status.SUCCESS:
        return api.types.status.ERROR

    return api.types.status.SUCCESS


def Trigger(tc):
    hostname = tc.host
    pf = tc.pf
    max_vfs = tc.num_vfs

    
    num_vfs = min([max_vfs, 255])
    if CreateVFs(hostname, pf, num_vfs) != api.types.status.SUCCESS:
        return api.types.status.ERROR

    api.Logger.info("Check VF set mac host %s PF %s" % (hostname, pf))

    # set it to something and check it, then set it back to 00
    for vfid in range(num_vfs):
        mac = "00:22:44:66:88:%02x" % vfid
        api.Logger.info('Setting mac addr %s on VF %d' % (mac, vfid))
        if SetVFTrust(hostname, pf, vfid, "on") != api.types.status.SUCCESS:
            return api.types.status.ERROR
        if SetVFMac(hostname, pf, vfid, mac) != api.types.status.SUCCESS:
            api.Logger.error("set vf %d mac failed on host %s pf %s" % (vfid, hostname, pf))
        
        vf_mac = GetVFMac(hostname, pf, vfid)
        if vf_mac is None:
            api.Logger.error("Failed to read vf %d mac on host %s pf %s" % (vfid, hostname, pf))
            return api.types.status.FAILURE
        if vf_mac != mac:
            api.Logger.error("Configured mac %s but found mac %s" % (mac, vf_mac))
            return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Verify(tc):
    return api.types.status.SUCCESS

def Teardown(tc):
    hostname = tc.host
    os_name = api.GetNodeOs(hostname)
    # restore the workloads on the one host we tested
    if host.UnloadDriver(os_name, hostname, "eth") is api.types.status.FAILURE:
        return api.types.status.FAILURE
    if host.LoadDriver(os_name, hostname) is api.types.status.FAILURE:
        return api.types.status.FAILURE

    hw_config.ReAddWorkloads(hostname)

    return api.types.status.SUCCESS