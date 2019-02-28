#! /usr/bin/python3
import iota.harness.api as api

__IPV4_HEADER_SIZE = 20
__IPV6_HEADER_SIZE = 40
__ICMP_HEADER_SIZE = 8
__IPV4_ENCAP_OVERHEAD = __IPV4_HEADER_SIZE + __ICMP_HEADER_SIZE
__IPV6_ENCAP_OVERHEAD = __IPV6_HEADER_SIZE + __ICMP_HEADER_SIZE


def pingAllRemoteWloadPairs(workload_pairs=None, mtu=64, af="ipv4", count=3, interval=0.2, do_pmtu_disc=False):
    """ Caller is supposed to pass appropriate args - No validation to be done here """
    cmd_cookies = []
    do_v4_ping = False
    do_v6_ping = False

    if af.find("ipv4") is not -1:
        do_v4_ping = True
        packet_size = mtu - __IPV4_ENCAP_OVERHEAD
        v4_ping_base_cmd = "ping -A -W 1 -c %d -i %f -s %d " %(count, interval, packet_size)

    if af.find("ipv6") is not -1:
        do_v6_ping = True
        packet_size = mtu - __IPV6_ENCAP_OVERHEAD
        v6_ping_base_cmd = "ping6 -A -W 1 -c %d -i %f -s %d " %(count, interval, packet_size)

    if workload_pairs is None:
        workload_pairs = api.GetRemoteWorkloadPairs()

    if not api.IsSimulation():
        req = api.Trigger_CreateAllParallelCommandsRequest()
    else:
        req = api.Trigger_CreateExecuteCommandsRequest(serial = False)
    for pair in workload_pairs:
        w1 = pair[0]
        w2 = pair[1]

        if do_pmtu_disc is True:
            if api.GetNodeOs(w1.node_name) == "freebsd":
                pmtu_disc_cmd = " -D "
            else:
                pmtu_disc_cmd = " -M do "
        else:
            pmtu_disc_cmd = ""

        if do_v4_ping is True:
            cmd_cookie = v4_ping_base_cmd + pmtu_disc_cmd + "-S %s %s" %(w1.ip_address, w2.ip_address)
            api.Logger.verbose("Ping test cmd %s from %s(%s %s) --> %s(%s %s)" % (cmd_cookie, w1.workload_name, w1.ip_address, w1.interface, w2.workload_name, w2.ip_address, w2.interface))
            api.Trigger_AddCommand(req, w1.node_name, w1.workload_name, cmd_cookie)
            cmd_cookies.append(cmd_cookie)

        if do_v6_ping is True:
            cmd_cookie = v6_ping_base_cmd + pmtu_disc_cmd + "-S %s %s" %(w1.ipv6_address, w2.ipv6_address)
            api.Logger.verbose("Ping6 test cmd %s from %s(%s %s) --> %s(%s %s)" % (cmd_cookie, w1.workload_name, w1.ipv6_address, w1.interface, w2.workload_name, w2.ipv6_address, w2.interface))
            api.Trigger_AddCommand(req, w1.node_name, w1.workload_name, cmd_cookie)
            cmd_cookies.append(cmd_cookie)

    resp = api.Trigger(req)
    return cmd_cookies, resp

def verifyPing(cmd_cookies, response, exit_code=0):
    result = api.types.status.SUCCESS
    if response is None:
        api.Logger.error("verifyPing failed - no response")
        return api.types.status.FAILURE
    commands = response.commands
    cookie_idx = 0
    for cmd in commands:
        if cmd.exit_code != exit_code:
            api.Logger.error("verifyPing failed for %s" % (cmd_cookies[cookie_idx]))
            api.PrintCommandResults(cmd)
            result = api.types.status.FAILURE
        cookie_idx += 1
    return result
