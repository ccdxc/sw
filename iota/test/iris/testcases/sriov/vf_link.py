#! /usr/bin/python3
import iota.harness.api as api
import iota.test.utils.naples_host as host
import iota.test.iris.config.netagent.hw_push_config as hw_config
from .sriov_utils import SetVFMac
from .sriov_utils import GetSupportedVFs
from .sriov_utils import CreateVFs
from .sriov_utils import SetVFIp
from .sriov_utils import SetVFLinkState, GetVFLinkState
from .sriov_utils import GetVFNdevState, SetVFNdevState
from .sriov_utils import SetVFTrust
from .sriov_utils import SetPFState, GetPFState

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
    #       ip link set <pf> vf <vf_index> state auto|enable|disable
    # set mac address and ip address
    #    verify ping works by default
    # set link down
    #    verify ping doesn't work
    # set link up
    #    verify ping works
    
    # TODO:
    # Check ping works for each vf

    hostname = tc.host
    pf = tc.pf
    max_vfs = tc.num_vfs
    
    if CreateVFs(hostname, pf, max_vfs) != api.types.status.SUCCESS:
        return api.types.status.ERROR

    for vfid in range(max_vfs):
        api.Logger.info("Check VF %d Link on host %s PF %s" % (vfid, hostname, pf))
        # Set trust on to set the mac addr
        if SetVFTrust(hostname, pf, vfid, "on") != api.types.status.SUCCESS:
                return api.types.status.ERROR
        mac = "00:22:44:66:88:%02x" % vfid
        ret = SetVFMac(hostname, pf, vfid, mac)
        if ret != api.types.status.SUCCESS:
            return api.types.status.ERROR
        
        ret = SetVFIp(hostname, pf, vfid, "112.0.0.1")
        if ret != api.types.status.SUCCESS:
            return api.types.status.ERROR
        failed = False
        
        # DISABLE
        api.Logger.info("Testing VF state DISABLE on VF %d PF %s host %s"
                        % (vfid, pf, hostname))
        state = "disable"
        if SetVFLinkState(hostname, pf, vfid, state) != api.types.status.SUCCESS:
            return api.types.status.FAILURE

        obs_state = GetVFNdevState(hostname, pf, vfid)
        if obs_state != "DOWN":
            api.Logger.error("Configured VF %d state %s but VF Netdev state %s on host %s"
                            % (vfid, state, obs_state, hostname))
            failed = True

        # Enable
        api.Logger.info("Testing VF state ENABLE on VF %d PF %s host %s"
                        % (vfid, pf, hostname))
        state = "enable"
        if SetVFLinkState(hostname, pf, vfid, state) != api.types.status.SUCCESS:
            return api.types.status.FAILURE
        obs_state = GetVFNdevState(hostname, pf, vfid)
        if obs_state != "UP":
            api.Logger.error("Configured VF %d state %s but VF Netdev state %s on host %s"
                            % (vfid, state, obs_state, hostname))
            failed = True

        # AUTO
        api.Logger.info("Testing VF state AUTO on VF %d PF %s host %s"
                        % (vfid, pf, hostname))
        state = "auto"
        if SetVFLinkState(hostname, pf, vfid, state) != api.types.status.SUCCESS:
            return api.types.status.FAILURE

        pf_state = "down"
        if SetPFState(hostname, pf, pf_state) != api.types.status.SUCCESS:
            return api.types.status.FAILURE

        obs_state = GetVFNdevState(hostname, pf, vfid)
        if obs_state != "UP":
            api.Logger.error("Configured VF %d state %s but VF Netdev state %s on host %s"
                            % (vfid, state, obs_state, hostname))
            failed = True

    if failed:
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