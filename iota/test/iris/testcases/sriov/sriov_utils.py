import iota.harness.api as api
import traceback
import time

def __execute_cmd(hostname, pf, cmd):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    api.Trigger_AddHostCommand(req, hostname, cmd)
    resp = api.Trigger(req)
    if resp is None:
        api.Logger.error("Failed to trigger command on host %s PF %s" % (hostname, pf))
        api.Logger.error(" cmd=" + cmd)
        raise RuntimeError("Failed")

    return resp.commands.pop()


def GetSupportedVFs(hostname, pf):
    try:
        cmd = "cat /sys/class/net/" + pf + "/device/sriov_totalvfs"
        resp = __execute_cmd(hostname, pf, cmd)
        if resp.exit_code != 0:
            api.Logger.error("Failed to get supported VF total from host %s interface %s"
                             % (hostname, pf))
            api.PrintCommandResults(resp)
            return 0
        nvfs = int(resp.stdout.strip())
    except:
        api.Logger.error(traceback.format_exc())
        return 0

    return nvfs

def DeleteVFs(hostname, pf):
    try:
        # The command will succeed even if numvf = 0 already
        cmd = "echo 0 > /sys/class/net/" + pf + "/device/sriov_numvfs"
        resp = __execute_cmd(hostname, pf, cmd)
        if resp.exit_code != 0:
            api.Logger.error("Failed to delete VFs on host %s interface %s"
                             % (hostname, pf))
            api.PrintCommandResults(resp)
            return api.types.status.FAILURE
    except:
        api.Logger.error(traceback.format_exc())
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def CreateVFs(hostname, pf, num_vfs):
    try:
        if DeleteVFs(hostname, pf) != api.types.status.SUCCESS:
            return api.types.status.FAILURE

        cmd = "echo " + str(num_vfs) + " > /sys/class/net/" + pf + "/device/sriov_numvfs"
        resp = __execute_cmd(hostname, pf, cmd)
        if resp.exit_code != 0:
            api.Logger.error("Failed to do %d VFs on host %s interface %s"
                             % (num_vfs, hostname, pf))
            api.PrintCommandResults(resp)
            return api.types.status.FAILURE
    except:
        api.Logger.error(traceback.format_exc())
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def GetVFName(hostname, pf, vfid):
    # return the vf's netdev name based on the pf pf and the vfid
    #   ls /sys/class/net/<pf>/device/virtfn<vfid>/net
    try:
        cmd = "ls /sys/class/net/%s/device/virtfn%d/net" % (pf, vfid)
        resp = __execute_cmd(hostname, pf, cmd)
        if resp.exit_code != 0:
            api.Logger.error("Failed to get VF name for %s vf %d" % (pf, vfid))
            api.PrintCommandResults(resp)
            return None

        vfname = resp.stdout.strip()
    except:
        api.Logger.error(traceback.format_exc())
        return None

    return vfname

def SetVFIp(hostname, pf, vfid, ip):
    try:
        vf_intf = GetVFName(hostname, pf, vfid)
        cmd = "ifconfig " + vf_intf + " " + ip
        api.Logger.info("Setting host %s interface %s to %s" % (hostname, pf, ip))
        resp = __execute_cmd(hostname, pf, cmd)
        if resp.exit_code != 0:
            api.Logger.error("Failed to set host %s interface %s to %s" % (hostname, pf, ip))
            api.PrintCommandResults(resp)
            return api.types.status.FAILURE
    except:
        api.Logger.error(traceback.format_exc())
        return api.types.status.ERROR

    return api.types.status.SUCCESS

def SetIp(hostname, pf, ip, subnet=24):
    try:
        cmd = "ip addr add %s/%d dev %s" % (ip, subnet, pf)
        api.Logger.info("Setting host %s interface %s to %s/%d"
                        % (hostname, pf, ip, subnet))
        resp = __execute_cmd(hostname, pf, cmd)
        if resp.exit_code != 0:
            api.Logger.error("Failed to set host %s interface %s to %s" % (hostname, pf, ip))
            api.PrintCommandResults(resp)
            return api.types.status.FAILURE
    except:
        api.Logger.error(traceback.format_exc())
        return api.types.status.ERROR

    return api.types.status.SUCCESS

def ClearIps(hostname, pf):
    try:
        cmd = "ip addr flush dev %s" % (pf)
        api.Logger.info("Clearing IPs on host %s interface %s"
                        % (hostname, pf))
        resp = __execute_cmd(hostname, pf, cmd)
        if resp.exit_code != 0:
            api.Logger.error("Failed to Clear IPs on host %s interface %s"
                             % (hostname, pf))
            api.PrintCommandResults(resp)
            return api.types.status.FAILURE
    except:
        api.Logger.error(traceback.format_exc())
        return api.types.status.ERROR

    return api.types.status.SUCCESS

def ping_check(hostname, pf, ip, retries=5):
    timeout = 4
    try:
        for _i in range(retries):
            api.Logger.info("Ping attempt %d: ip %s" % (_i, ip))
            cmd = "ping -c 1 -I %s -W %d %s" % (pf, timeout, ip)
            resp = __execute_cmd(hostname, pf, cmd)
            if resp.exit_code == 0:
                api.Logger.info("Ping successful for ip %s" % (ip))
                return api.types.status.SUCCESS
    except:
        api.Logger.error(traceback.format_exc())
        return api.types.status.FAILURE
    return api.types.status.FAILURE

def CheckVFDefaults(hostname, pf, vfid, mac):
    # verify that
    #   mac address is 00:...:00
    #   trust is off
    #   link state is auto
    #   vf 0 MAC 00:00:00:00:00:00, spoof checking off, link-state auto, trust off
    try:
        cmd = "ip link show %s | grep 'vf %d MAC'" % (pf, vfid)
        resp = __execute_cmd(hostname, pf, cmd)
        if resp.exit_code != 0:
            api.Logger.error("get VF %d info failed on host %s PF %s"
                             % (vfid, hostname, pf))
            api.PrintCommandResults(resp)
            return None

        info = resp.stdout.strip()
        
        target = "vf %d MAC %s, spoof checking off, link-state auto, trust off" % (vfid, mac)
        if info != target:
            api.Logger.error("vf settings check failed host %s PF %s" % (vfid, hostname, pf))
            api.Logger.error("    should have: " + target)
            api.Logger.error("    got        : " + info)
            return api.types.status.FAILURE
    except:
        api.Logger.error(traceback.format_exc())
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def GetVFNdevMac(hostname, pf, vfid):
    try:
        vf_intf = GetVFName(hostname, pf, vfid)
        cmd = "cat /sys/class/net/%s/address" % (vf_intf)

        resp = __execute_cmd(hostname, pf, cmd)
        if resp.exit_code != 0:
            api.Logger.error("Get vf netdev %s mac failed on host %s"
                             % (vf_intf, hostname))
            api.PrintCommandResults(resp)
            if resp.stderr.find("RTNETLINK answers: Input/output error") != -1:
                api.Logger.error("Is the VF API not supported in this FW?")
                return None
    except:
        api.Logger.error(traceback.format_exc())
        return None

    return resp.stdout.strip()

def SetVFNdevMac(hostname, pf, vfid, mac):
    try:
        vf_intf = GetVFName(hostname, pf, vfid)
        cmd = "ifconfig %s hw ether %s" % (vf_intf, mac)
        resp = __execute_cmd(hostname, pf, cmd)
        if resp.exit_code != 0:
            api.Logger.error("Set vf netdev %s mac failed on host %s "
                             % (vfid, hostname))
            api.PrintCommandResults(resp)
            return api.types.status.FAILURE
    except:
        api.Logger.error(traceback.format_exc())
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def GetVFMac(hostname, pf, vfid):
    try:
        cmd = "ip link show %s | grep 'vf %d' | awk '{ print $4 }'" % (pf, vfid)

        resp = __execute_cmd(hostname, pf, cmd)
        if resp.exit_code != 0:
            api.Logger.error("get vf %d info failed on host %s PF %s" % (vfid, hostname, pf))
            api.PrintCommandResults(resp)
            if resp.stderr.find("RTNETLINK answers: Input/output error") != -1:
                api.Logger.error("Is the VF API not supported in this FW?")
                return None
    except:
        api.Logger.error(traceback.format_exc())
        return None
    
    return resp.stdout.replace(',', '').strip()

def SetVFMac(hostname, pf, vfid, mac):
    try:
        cmd = "ip link set %s vf %d mac %s" % (pf, vfid, mac)
        resp = __execute_cmd(hostname, pf, cmd)
        if resp.exit_code != 0:
            api.Logger.error("get vf %d info failed on host %s PF %s" % (vfid, hostname, pf))
            api.PrintCommandResults(resp)
            if resp.stderr.find("RTNETLINK answers: Input/output error") != -1:
                api.Logger.error("Is the VF API not supported in this FW?")
            return api.types.status.FAILURE
        
        # ip link set does not update VF's netdev, so using ifconfig to update it.
        vf_intf = GetVFName(hostname, pf, vfid)
        cmd = "ifconfig %s hw ether %s" % (vf_intf, mac)

        resp = __execute_cmd(hostname, pf, cmd)
        if resp.exit_code != 0:
            api.Logger.error("Failed to set mac addr for vf_intf %s on host %s"
                            % (vf_intf, hostname))
            api.PrintCommandResults(resp)
            return api.types.status.FAILURE
    except:
        api.Logger.error(traceback.format_exc())
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def SetVFRate(hostname, pf, vfid, rate):
    cmd = "ip link set %s vf %s max_tx_rate %s" % (pf, vfid, rate)
    try:
        resp = __execute_cmd(hostname, pf, cmd)
        if resp.exit_code != 0:
            api.Logger.error("Set VF %d max_tx_rate failed on host %s PF %s"
                             % (vfid, hostname, pf))
            api.PrintCommandResults(resp)
            return api.types.status.FAILURE
    except:
        api.Logger.error(traceback.format_exc())
        return api.types.status.FAILURE
    
    return api.types.status.SUCCESS

def GetVFRate(hostname, pf, vfid):
    cmd = "ip link show %s | grep 'vf %d' | grep -oP 'max_tx_rate\s+\d+'" % (pf, vfid)
    try:
        resp = __execute_cmd(hostname, pf, cmd)
        if resp.exit_code != 0:
            api.Logger.error("Get VF %d max_tx_rate failed on host %s PF %s"
                             % (vfid, hostname, pf))
            api.PrintCommandResults(resp)
            return None
        obs_rate = int(resp.stdout.split()[-1])
        return obs_rate
    except:
        api.Logger.error(traceback.format_exc())
        return None

def SetVFLinkState(hostname, pf, vfid, state):
    cmd = "ip link set %s vf %d state %s" % (pf, vfid, state)
    try:
        resp = __execute_cmd(hostname, pf, cmd)
        time.sleep(1)
        if resp.exit_code != 0:
            api.Logger.error("Set vf %d link state %s failed on host %s PF %s"
                             % (vfid, state, hostname, pf))
            api.PrintCommandResults(resp)
            return api.types.status.FAILURE
    except:
        api.Logger.error(traceback.format_exc())
        return api.types.status.FAILURE
    
    return api.types.status.SUCCESS

def GetVFLinkState(hostname, pf, vfid):
    cmd = "ip link show %s | grep 'vf %d' | grep -oP 'link-state\s+\w+'" % (pf, vfid)
    try:
        resp = __execute_cmd(hostname, pf, cmd)
        if resp.exit_code != 0:
            api.Logger.error("Get vf %d link state failed on host %s PF %s"
                             % (vfid, hostname, pf))
            api.PrintCommandResults(resp)
            return None

        obs_state = resp.stdout.split()[-1]
        return obs_state
    except:
        api.Logger.error(traceback.format_exc())
        return None

def GetVFNdevState(hostname, pf, vfid):
    vf_intf = GetVFName(hostname, pf, vfid)
    cmd = "ip link show %s | grep -m1 -oP 'state\s+\w+'" % (vf_intf)
    try:
        resp = __execute_cmd(hostname, pf, cmd)
        if resp.exit_code != 0:
            api.Logger.error("Get vf %d netdev state failed on host %s PF %s"
                             % (vfid, hostname, pf))
            api.PrintCommandResults(resp)
            return None

        obs_state = resp.stdout.split()[-1]
        return obs_state
    except:
        api.Logger.error(traceback.format_exc())
        return None

def SetVFNdevState(hostname, pf, vfid, state):
    vf_intf = GetVFName(hostname, pf, vfid)
    cmd = "ip link set %s %s" % (vf_intf, state)
    try:
        resp = __execute_cmd(hostname, pf, cmd)
        time.sleep(1)
        if resp.exit_code != 0:
            api.Logger.error("Get vf %d netdev state %s failed on host %s PF %s"
                             % (vfid, state, hostname, pf))
            api.PrintCommandResults(resp)
            return api.types.status.FAILURE
    except:
        api.Logger.error(traceback.format_exc())
        return api.types.status.FAILURE
    
    return api.types.status.SUCCESS

def GetPFState(hostname, pf):
    cmd = "ip link show %s | grep -m1 -oP 'state\s+\w+'" % (pf)
    try:
        resp = __execute_cmd(hostname, pf, cmd)
        if resp.exit_code != 0:
            api.Logger.error("Get PF %s state failed on host %s" % (pf, hostname))
            api.PrintCommandResults(resp)
            return None

        obs_state = resp.stdout.split()[-1]
        return obs_state
    except:
        api.Logger.error(traceback.format_exc())
        return None

def SetPFState(hostname, pf, state):
    cmd = "ip link set %s %s" % (pf, state)
    try:
        resp = __execute_cmd(hostname, pf, cmd)
        time.sleep(1)
        if resp.exit_code != 0:
            api.Logger.error("Set PF %s state %s failed on host %s"
                             % (pf, state, hostname))
            api.PrintCommandResults(resp)
            return api.types.status.FAILURE

    except:
        api.Logger.error(traceback.format_exc())
        return api.types.status.FAILURE
    return api.types.status.SUCCESS

def GetVFTrust(hostname, pf, vfid):
    cmd = "ip link show %s | grep -m1 -oP 'trust\s+\w+'" % (pf)
    try:
        resp = __execute_cmd(hostname, pf, cmd)
        if resp.exit_code != 0:
            api.Logger.error("Get VF %d trust failed on host %s PF %s"
                             % (vfid, hostname, pf))
            api.PrintCommandResults(resp)
            return None

        obs_trust = resp.stdout.split()[-1]
        return obs_trust
    except:
        api.Logger.error(traceback.format_exc())
        return None

def SetVFTrust(hostname, pf, vfid, trust):
    cmd = "ip link set %s vf %d trust %s" % (pf, vfid, trust)
    try:
        resp = __execute_cmd(hostname, pf, cmd)
        time.sleep(1)
        if resp.exit_code != 0:
            api.Logger.error("Set VF %d trust %s failed on host %s PF %s"
                             % (vfid, trust, hostname, pf))
            api.PrintCommandResults(resp)
            return api.types.status.FAILURE

    except:
        api.Logger.error(traceback.format_exc())
        return api.types.status.FAILURE
    return api.types.status.SUCCESS

def GetVFRxStats(hostname, pf, vfid):
    cmd = "ip -s link show %s | grep -A 4 'vf %d' | awk 'NR == 3  { print $2 }'" % (pf, vfid)
    try:
        resp = __execute_cmd(hostname, pf, cmd)
        if resp.exit_code != 0:
            api.Logger.error("Get VF %d Rx stats failed on host %s PF %s"
                             % (vfid, hostname, pf))
            api.PrintCommandResults(resp)
            return -1

        rx_pkts = int(resp.stdout)
        return rx_pkts
    except:
        api.Logger.error(traceback.format_exc())
        return -1

def GetVFTxStats(hostname, pf, vfid):
    cmd = "ip -s link show %s | grep -A 4 'vf %d' | awk 'NR == 5  { print $2 }'"\
          % (pf, vfid)
    try:
        resp = __execute_cmd(hostname, pf, cmd)
        if resp.exit_code != 0:
            api.Logger.error("Get VF %d Tx stats failed on host %s PF %s"
                             % (vfid, hostname, pf))
            api.PrintCommandResults(resp)
            return -1

        tx_pkts = int(resp.stdout)
        return tx_pkts
    except:
        api.Logger.error(traceback.format_exc())
        return -1