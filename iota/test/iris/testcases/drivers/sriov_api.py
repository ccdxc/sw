#! /usr/bin/python3
import iota.harness.api as api
import iota.test.iris.utils.naples_host as host
import iota.test.iris.config.netagent.hw_push_config as hw_config

# Check that the typical VF user commands are supported
# through the driver.
#
#       echo N > /sys/class/net/<PF>/device/sriov_numvfs
#       ip link set <pf> vf <vf_index> vlan <vlan id>
#       ip link set dev <PF> vf <vf-id> trust on
#       ip link set <pf> vf <vf-id> <vf-mac>
#       ip link set <pf> vf <vf-id> max_tx_rate <MAX_RATE_IN_MBIT/S>
#       ip link set <pf> vf <vf_index> spoofchk on|off
#       ip link show <pf>
#       ip link set <pf> vf <vf_index> state auto|enable|disable
#       ip -s link show <pf>
#

def _getSupportedVFs(hostname, intf):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    cmd = "cat /sys/class/net/" + intf + "/device/sriov_totalvfs"
    api.Trigger_AddHostCommand(req, hostname, cmd)

    resp = api.Trigger(req)
    cmd = resp.commands.pop()
    if cmd.exit_code != 0:
        api.Logger.error("Failed to get supported VF total from host %s interface %s" % (hostname, intf))
        api.PrintCommandResults(cmd)
        return 0

    nvfs = int(cmd.stdout.strip())
    return nvfs

def _startVFs(hostname, intf, target_nvfs):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    cmd = "echo " + str(target_nvfs) + " > /sys/class/net/" + intf + "/device/sriov_numvfs"
    api.Trigger_AddHostCommand(req, hostname, cmd)

    resp = api.Trigger(req)
    cmd = resp.commands.pop()
    if cmd.exit_code != 0:
        api.Logger.error("Failed to do %d VFs on host %s interface %s" % (target_nvfs, hostname, intf))
        api.PrintCommandResults(cmd)
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def _setip(hostname, intf, ip):
    # see code from sriov.py
    return api.types.status.SUCCESS

def _ping(hostname, ip):
    # see code from sriov.py
    return api.types.status.SUCCESS

def _checkVFDefaults(hostname, intf, vfid):
    # verify that
    #   mac address is 00:...:00
    #   trust is off
    #   link state is auto
    #   vf 0 MAC 00:00:00:00:00:00, spoof checking off, link-state auto, trust off

    api.Logger.info("Check VF Defaults on host %s interface %s vfid %d" % (hostname, intf, vfid))

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    cmd = "ip link show %s | grep 'vf %d MAC'" % (intf, vfid)
    api.Trigger_AddHostCommand(req, hostname, cmd)

    resp = api.Trigger(req)
    if resp is None:
        api.Logger.error("Failed to trigger command on host %s intf %s" % (hostname, intf))
        api.PrintCommandResults(cmd)
        return None

    cmd = resp.commands.pop()
    if cmd.exit_code != 0:
        api.Logger.error("get vf %d info failed on host %s intf %s" % (vfid, hostname, intf))
        api.PrintCommandResults(cmd)
        return None

    info = cmd.stdout.strip()
    target = "vf %d MAC 00:00:00:00:00:00, spoof checking off, link-state auto, trust off" % vfid
    if info != target:
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def _checkVFAddress(hostname, intf, vfid):
    # see code from sriov.py
    #
    # need to check ifconfig with both trust off (default) and trust on
    #    ifconfig <vf> hw ether <macaddr>
    #    with trust off, should fail with no permission
    #       ip link set dev <PF> vf <vf-id> trust off
    #    with trust on, should succeed
    #       ip link set dev <PF> vf <vf-id> trust on
    #
    # need to check ip command
    #    ip link set <pf> vf <vf-id> <vf-mac>
    return api.types.status.SUCCESS

def _checkVFRate(hostname, intf, vfid):
    # for several rates in Mbps 1 5 10 200 3000 40000 100000
    #       ip link set <pf> vf <vf-id> max_tx_rate <MAX_RATE_IN_MBIT/S>
    #    try iperf or something else to measure data rate
    return api.types.status.SUCCESS

def _checkVFLink(hostname, intf, vfid):
    #       ip link set <pf> vf <vf_index> state auto|enable|disable
    # set mac address and ip address
    #    verify ping works by default
    # set link down
    #    verify ping doesn't work
    # set link up
    #    verify ping works
    return api.types.status.SUCCESS

def _checkVFStats(hostname, intf, vfid):
    # check that there have been packets showing up after previous tests
    #       ip -s link show <pf>
    return api.types.status.SUCCESS

def _checkVFSpoof(hostname, intf, vfid):
    #       ip link set <pf> vf <vf_index> spoofchk on|off
    # verify spoofchk on by default
    # get current Tx packet count
    # generate and attempt to sent packet with bogus src mac
    # verify that packet count is unchanged
    #
    # disable spoofchk
    # verify spoof off
    # get current Tx packet count
    # generate and attempt to sent packet with bogus src mac
    # verify that packet count is incremented
    #
    # enable spoofchk
    # verify spoof on
    # get current Tx packet count
    # generate and attempt to sent packet with bogus src mac
    # verify that packet count is unchanged
    #
    return api.types.status.SUCCESS

def _checkVFvlan(hostname, intf, vfid):
    #       ip link set <pf> vf <vf_index> vlan <vlan id>
    # verify there is no vlan by default
    #    verify ping succeeds
    # add port vlan id
    #    verify ping fails
    # add port vlan on peer
    #    verify ping succeeds
    # remove port vlan on local and peer
    #    verify ping succeeds
    #
    return api.types.status.SUCCESS

def Setup(tc):
    tc.nodes = api.GetNaplesHostnames()
    hostname = tc.nodes[0]

    tc.os = api.GetNodeOs(hostname)
    tc.intf = api.GetNaplesHostInterfaces(hostname)[0]

    if tc.os != "linux":
        return api.types.status.SUCCESS

    tc.num_vfs = _getSupportedVFs(hostname, tc.intf)
    if tc.num_vfs == 0:
        api.Logger.warn("0 VFs supported on host %s interface %s, expected non-zero" % (hostname, tc.intf))
        return api.types.status.SUCCESS

    ret = _startVFs(hostname, tc.intf, tc.num_vfs)
    return ret

def Trigger(tc):
    if tc.os != "linux":
        return api.types.status.SUCCESS
    if tc.num_vfs == 0:
        return api.types.status.SUCCESS

    hostname = tc.nodes[0]
    intf = tc.intf
    vfid = tc.num_vfs - 1
    api.Logger.info("SR-IOV API test on host %s interface %s vfid %d" % (hostname, intf, vfid))

    fail = 0
    ret = _checkVFDefaults(hostname, intf, vfid)
    if ret != api.types.status.SUCCESS:
        fail = fail + 1

    ret = _checkVFAddress(hostname, intf, vfid)
    if ret != api.types.status.SUCCESS:
        fail = fail + 1

    ret = _checkVFRate(hostname, intf, vfid)
    if ret != api.types.status.SUCCESS:
        fail = fail + 1

    ret = _checkVFLink(hostname, intf, vfid)
    if ret != api.types.status.SUCCESS:
        fail = fail + 1

    ret = _checkVFStats(hostname, intf, vfid)
    if ret != api.types.status.SUCCESS:
        fail = fail + 1

    ret = _checkVFSpoof(hostname, intf, vfid)
    if ret != api.types.status.SUCCESS:
        fail = fail + 1

    ret = _checkVFvlan(hostname, intf, vfid)
    if ret != api.types.status.SUCCESS:
        fail = fail + 1

    if fail != 0:
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Verify(tc):
    hostname = tc.nodes[0]

    # restore the workloads on the one host we tested
    if host.UnloadDriver(tc.os, hostname, "eth") is api.types.status.FAILURE:
        return api.types.status.FAILURE
    if host.LoadDriver(tc.os, hostname) is api.types.status.FAILURE:
        return api.types.status.FAILURE

    hw_config.ReAddWorkloads(hostname)

    return api.types.status.SUCCESS
