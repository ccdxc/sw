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
from .sriov_utils import SetPFState, GetPFState
from .sriov_utils import SetIp
from .sriov_utils import ping_check
from .sriov_utils import SetVFNdevMac
from .sriov_utils import GetVFTxStats, GetVFRxStats
from iota.test.iris.utils.subif_utils import getRemoteHostIntfWorkloadsPairs
import iota.test.iris.utils.iperf as iperf
import time
from .sriov_utils import ClearIps
from .sriov_utils import DeleteVFs
from .sriov_utils import SetVFTrust
from .sriov_utils import GetVFName


def Setup(tc):
    host1 = None
    host2 = None
    api.Logger.info("Nodes: %s" % api.GetWorkloadNodeHostnames())
    for _node in api.GetWorkloadNodeHostnames():
        if not host1 and api.IsNaplesNode(_node) and api.GetNodeOs(_node) == "linux":
            host1 = _node
        else:
            host2 = _node
        
    if not host1:
        api.Logger.error("Unable to find a Naples node with linux os")
        return api.types.status.ERROR
    if not host2:
        api.Logger.error("Unable to find a node with linux os")
        return api.types.status.ERROR

    tc.host1 = host1
    tc.host2 = host2
    tc.pf_1 = api.GetWorkloadNodeHostInterfaces(tc.host1)[0]
    tc.pf_2 = api.GetWorkloadNodeHostInterfaces(tc.host2)[0]

    api.Logger.info("Host interface pair[%s, %s]" % (tc.pf_1, tc.pf_2))

    tc.num_vfs = GetSupportedVFs(tc.host1, tc.pf_1)
    api.Logger.info("Host %s PF %s supports %d VFs" % (tc.host1, tc.pf_1, tc.num_vfs))

    if tc.num_vfs == 0:
        api.Logger.warn("Max supported VFs on host %s is 0, expected non-zero" % host)
        return api.types.status.ERROR

    if CreateVFs(tc.host1, tc.pf_1, tc.num_vfs)!= api.types.status.SUCCESS:
        return api.types.status.ERROR
    
    tc.vfid = 0
    tc.vf_intf = GetVFName(tc.host1, tc.pf_1, tc.vfid)
    tc.remote_intf = tc.pf_2

    if SetVFTrust(tc.host1, tc.pf_1, tc.vfid, "on") != api.types.status.SUCCESS:
            return api.types.status.ERROR
    # Assign mac addr to VF
    if SetVFNdevMac(host1, tc.pf_1, tc.vfid, "00:22:44:66:88:a1") != api.types.status.SUCCESS:
        return api.types.status.ERROR

    # Assign Ips to the vf interface and corresponding PF on the remote node.
    tc.vf_ip = "30.0.0.1"
    tc.remote_ip = "30.0.0.2"
    if SetIp(host1, tc.vf_intf, tc.vf_ip, 24) != api.types.status.SUCCESS:
        return api.types.status.ERROR
    if SetIp(host2, tc.remote_intf, tc.remote_ip, 24) != api.types.status.SUCCESS:
        return api.types.status.ERROR
    if SetVFNdevState(tc.host1, tc.pf_1, tc.vfid, "up") != api.types.status.SUCCESS:
        return api.types.status.ERROR
    if ping_check(tc.host1, tc.vf_intf, tc.remote_ip, retries=5) != api.types.status.SUCCESS:
        api.Logger.error("Unable to ping the remote interface")
        return api.types.status.ERROR

    return api.types.status.SUCCESS


def Trigger(tc):
    # check that there have been packets showing up after previous tests
    #       ip -s link show <pf>
    
    packet_count = 1000
    mode = tc.iterators.mode
    if mode == "RX":
        pkt_cnt_before = GetVFRxStats(tc.host1, tc.pf_1, tc.vfid)
    else:
        pkt_cnt_before = GetVFTxStats(tc.host1, tc.pf_1, tc.vfid)
    
    if pkt_cnt_before == -1:
        api.Logger.error("Getting VF %s stats failed" % mode)
        return api.types.status.ERROR

    reverse = True if mode == "RX" else False
    servercmd = iperf.ServerCmd(server_ip=tc.remote_ip, port=7777)
    clientcmd = iperf.ClientCmd(tc.remote_ip, packet_count=packet_count, client_ip=tc.vf_ip,
                                jsonOut=True, port=7777, proto='udp', reverse=reverse)

    sreq = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    creq = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    api.Trigger_AddHostCommand(sreq, tc.host2, servercmd, background=True)
    api.Trigger_AddHostCommand(creq, tc.host1, clientcmd, timeout=30)

    server_resp = api.Trigger(sreq)
    if not server_resp:
        api.Logger.error("Unable to execute server command")
        return api.types.status.ERROR
    time.sleep(5)

    client_resp = api.Trigger(creq)
    if not client_resp:
        api.Logger.error("Unable to execute client command")
        return api.types.status.ERROR
    resp = client_resp.commands.pop()
    if resp.exit_code != 0:
        api.Logger.error("Iperf client failed with exit code %d" % resp.exit_code)
        api.PrintCommandResults(resp)
        return api.types.status.ERROR
    if not iperf.Success(resp.stdout):
        api.Logger.error("Iperf failed with error: %s" % iperf.Error(resp.stdout))
        return api.types.status.ERROR
        
    if mode == "RX":
        pkt_cnt_after = GetVFRxStats(tc.host1, tc.pf_1, tc.vfid)
    else:
        pkt_cnt_after = GetVFTxStats(tc.host1, tc.pf_1, tc.vfid)

    delta_pkt_cnt = pkt_cnt_after - pkt_cnt_before
    if packet_count > delta_pkt_cnt:
        api.Logger.error("Incorrect %s stats, expected %d observed %d"
                        % (mode, packet_count, delta_pkt_cnt))
        return api.types.status.FAILURE   
        
    return api.types.status.SUCCESS

def Verify(tc):
    return api.types.status.SUCCESS

def Teardown(tc):
    # Clear the ip settings on the remote interface
    ClearIps(tc.host2, tc.pf_2)

    # Delte created VFs
    DeleteVFs(tc.host1, tc.pf_1)

    for hostname in [tc.host1, tc.host2]:
        os_name = api.GetNodeOs(hostname)
        # restore the workloads on the one host we tested
        if host.UnloadDriver(os_name, hostname, "eth") is api.types.status.FAILURE:
            return api.types.status.FAILURE
        if host.LoadDriver(os_name, hostname) is api.types.status.FAILURE:
            return api.types.status.FAILURE

        hw_config.ReAddWorkloads(hostname)

    return api.types.status.SUCCESS