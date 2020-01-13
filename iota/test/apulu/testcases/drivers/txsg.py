#! /usr/bin/python3
import time
import iota.harness.api as api
import iota.test.utils.iperf as iperf

# Testing tx sg through ethtool
#
#  Initial state:
#     expect to see
#        from ethtool -k
#           sg is on
#              $ ethtool -k eno1 | grep scatt
#              scatter-gather: on
#                     tx-scatter-gather: on
#                     tx-scatter-gather-fraglist: off [fixed]
#
#  Test feature on
#     see sg stats increment when given a particular load of traffic
#       ethtool --set-priv-flags enp181s0 sw-dbg-stats on
#       ethtool -S enp181s0 | grep _sg_ | grep -v ": 0"
#       ifconfig enp181s0 10.0.0.58
#       iperf3 -c 10.0.0.22
#       ethtool -S enp181s0 | grep _sg_ | grep -v ": 0"
#
#  Test feature off
#     set feature off in ethtool, verify -k sees it off
#     run traffic load, verify sg stats don't change
#  Test feature restored
#     set feature on in ethtool, verify -k sees it on
#     see sg stats increment when given a particular load of traffic
#

def Setup(tc):
    api.Logger.info("Ethtool Tx SG")
    tc.nodes = api.GetNaplesHostnames()
    tc.os = api.GetNodeOs(tc.nodes[0])

    return api.types.status.SUCCESS


def __setDebugStatsOn(host, intf):
    req = api.Trigger_CreateExecuteCommandsRequest(serial=True)

    api.Logger.info("Enable debug stats: %s" % intf)
    api.Trigger_AddHostCommand(req, host, "ethtool --set-priv-flags %s sw-dbg-stats on" % intf)
    resp = api.Trigger(req)

    cmd = resp.commands[0]
    api.PrintCommandResults(cmd)

    if cmd.exit_code != 0:
        api.Logger.error("Bad exit code %d on interface %s" % (cmd.exit_code, intf))
        return api.types.status.FAILURE

    return api.types.status.SUCCESS


def __getSgStats(host, intf):
    req = api.Trigger_CreateExecuteCommandsRequest(serial=True)

    api.Logger.info("Get SG stats: %s" % intf)

    # Get the sg stats from ethtool and strip out those with 0 counts, and
    # strip out the counter for "no sg elems used"
    api.Trigger_AddHostCommand(req, host, "ethtool -S %s | grep _sg_ | grep -v ': 0' | grep -v _0:" % intf)
    resp = api.Trigger(req)

    cmd = resp.commands[0]
    api.PrintCommandResults(cmd)

    # don't check cmd.exit_code because an empty grep will return error,
    # but empty results isn't necessarily an error in this usage.

    return cmd.stdout


def __setTxsgOnoff(host, intf, onoff):
    req = api.Trigger_CreateExecuteCommandsRequest(serial=True)

    api.Logger.info("Set Txsg %s: %s" % (onoff, intf))
    api.Trigger_AddHostCommand(req, host, "ethtool -K %s sg %s" % (intf, onoff))
    resp = api.Trigger(req)

    cmd = resp.commands[0]
    api.PrintCommandResults(cmd)

    if cmd.exit_code != 0:
        api.Logger.error("Bad exit code %d on interface %s" % (cmd.exit_code, intf))
        api.Logger.info(cmd.stderr)
        return api.types.status.FAILURE

    return api.types.status.SUCCESS


def __runTraffic(intf):

    api.Logger.info("Run traffic: %s" % intf)

    client = None
    server = None
    clientCmd = None
    serverCmd = None
    clientReq = None
    serverReq = None

    for pairs in api.GetRemoteWorkloadPairs():
        client = pairs[0]
        api.Logger.error("Comparing client interface %s with %s" % (client.interface, intf))
        if client.interface == intf:
            server = pairs[1]
            break

    if server is None:
        api.Logger.error("No workload found for interface %s" % intf)
        return api.types.status.FAILURE

    cmd_descr = "Server: %s(%s) <--> Client: %s(%s)" %\
                   (server.workload_name, server.ip_address,\
                    client.workload_name, client.ip_address)
    api.Logger.info("Starting Iperf test from %s" % cmd_descr)

    port = api.AllocateTcpPort()
    serverCmd = iperf.ServerCmd(port)
    clientCmd = iperf.ClientCmd(server.ip_address, port)

    serverReq = api.Trigger_CreateExecuteCommandsRequest(serial=False)
    api.Trigger_AddCommand(serverReq, server.node_name, server.workload_name,
                           serverCmd, background=True)

    clientReq = api.Trigger_CreateExecuteCommandsRequest(serial=False)
    api.Trigger_AddCommand(clientReq, client.node_name, client.workload_name,
                           clientCmd)

    # Server runs in the background
    server_resp = api.Trigger(serverReq)

    # Sleep for some time as bg may not have been started.
    time.sleep(5)
    client_resp = api.Trigger(clientReq)

    # Stop the backgrounded server
    term_resp = api.Trigger_TerminateAllCommands(server_resp)

    # We don't bother checking the iperf results, we just wanted traffic
    # so just check that the commands succeeded
    ret = api.types.status.SUCCESS
    for cmd in server_resp.commands:
        if cmd.exit_code != 0:
            ret = api.types.status.FAILURE
    for cmd in client_resp.commands:
        if cmd.exit_code != 0:
            ret = api.types.status.FAILURE
    for cmd in term_resp.commands:
        if cmd.exit_code != 0:
            ret = api.types.status.FAILURE

    return ret


def Trigger(tc):
    if tc.os != 'linux':
        return api.types.status.SUCCESS

    hosts = api.GetNaplesHostnames()
    intfs = api.GetNaplesHostInterfaces(tc.nodes[0])

    statsPre = None
    statsOn = None
    statsOff = None
    fail = 0

    for intf in intfs:
        api.Logger.info("Trying interface %s" % intf)

        # Turn on debug stats so we can see the sg stats
        ret = __setDebugStatsOn(hosts[0], intf)
        if ret != api.types.status.SUCCESS:
            fail += 1
            #continue

        # First, make sure it is "on" and we see sg counts
        ret = __setTxsgOnoff(hosts[0], intf, "on")
        if ret != api.types.status.SUCCESS:
            fail += 1
            #continue

        statsPre = __getSgStats(hosts[0], intf)
        ret = __runTraffic(intf)
        if ret != api.types.status.SUCCESS:
            fail += 1
            #continue

        statsOn = __getSgStats(hosts[0], intf)
        if statsOn != None and statsOn == statsPre:
            api.Logger.error("No Tx SG counts when SG on - Failed for interface %s" % intf)
            fail += 1
            #continue

        # Next, turn it off and check that the counts don't change
        ret = __setTxsgOnoff(hosts[0], intf, "off")
        if ret != api.types.status.SUCCESS:
            fail += 1

        statsPre = __getSgStats(hosts[0], intf)
        ret = __runTraffic(intf)
        if ret != api.types.status.SUCCESS:
            fail += 1

        statsOff = __getSgStats(hosts[0], intf)
        if statsOff != statsPre:
            api.Logger.error("Tx SG counts changed even though SG off - Failed for interface %s" % intf)
            api.Logger.error("Before: %s" % statsPre)
            api.Logger.error("After : %s" % statsOff)
            fail += 1

        # Last, turn it back on and check that the counts change
        ret = __setTxsgOnoff(hosts[0], intf, "on")
        if ret != api.types.status.SUCCESS:
            fail += 1

        ret = __runTraffic(intf)
        if ret != api.types.status.SUCCESS:
            fail += 1

        statsBackOn = __getSgStats(hosts[0], intf)
        if statsBackOn == statsOn:
            api.Logger.error("Tx SG counts didn't change when reenabled - Failed for interface %s" % intf)
            api.Logger.error("Before: %s" % statsOn)
            api.Logger.error("After : %s" % statsBackOn)
            fail += 1

        # For some reason between the deep magic in setting up GetRemoteWorkloads,
        # and my own ineptness with Python, testing beyond the first interface
        # fails because I can't get a properly connected client/server workload
        # pair.  So for now, we'll jump out here after the first interface.
        break

    if fail > 0:
        api.Logger.error("Fail %d" % fail)
        return api.types.status.FAILURE

    return api.types.status.SUCCESS


def Verify(tc):
    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS
