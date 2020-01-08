#! /usr/bin/python3
import time
import iota.harness.api as api
import iota.test.iris.utils.iperf as iperf

__PING_CMD = "ping"
__PING6_CMD = "ping6"

__IPV4_HEADER_SIZE = 20
__IPV6_HEADER_SIZE = 40
__ICMP_HEADER_SIZE = 8
__VLAN_HEADER_SIZE = 4
__IPV4_ENCAP_OVERHEAD = __IPV4_HEADER_SIZE + __ICMP_HEADER_SIZE
__IPV6_ENCAP_OVERHEAD = __IPV6_HEADER_SIZE + __ICMP_HEADER_SIZE

def __sleep(timeout):
    if api.GlobalOptions.dryrun:
        return
    time.sleep(timeout)
    return

def __is_ipv4(af):
    return af == "ipv4"

def __get_ipproto(af):
    if __is_ipv4(af):
        return "v4"
    return "v6"

def __get_workload_address(workload, af):
    if __is_ipv4(af):
        return workload.ip_address
    return workload.ipv6_address

def __ping_addr_substitution(ping_base_cmd, addr):
    ping_addr_options = " %s" %(addr)
    ping_cmd = ping_base_cmd + ping_addr_options
    return ping_cmd

def __get_ping_base_cmd(w, af, packet_size, count, interval, do_pmtu_disc):
    if __is_ipv4(af):
        ping_cmd = __PING_CMD
        packet_size -= __IPV4_ENCAP_OVERHEAD
    else:
        ping_cmd = __PING6_CMD
        packet_size -= __IPV6_ENCAP_OVERHEAD

    if w.uplink_vlan != 0:
        packet_size -= __VLAN_HEADER_SIZE

    if do_pmtu_disc is True:
        if api.GetNodeOs(w.node_name) == "freebsd":
            ping_cmd += " -D "
        else:
            ping_cmd += " -M do"

    ping_cmd += " -A -W 1 -c %d -i %f -s %d " %(count, interval, packet_size)

    return ping_cmd

def pingWorkloads(workload_pairs, af="ipv4", packet_size=64, count=3, interval=0.2, do_pmtu_disc=False):
    cmd_cookies = []

    if not api.IsSimulation():
        req = api.Trigger_CreateAllParallelCommandsRequest()
    else:
        req = api.Trigger_CreateExecuteCommandsRequest(serial = False)
    
    for pair in workload_pairs:
        w1 = pair[0]
        w2 = pair[1]

        ping_base_cmd = __get_ping_base_cmd(w1, af, packet_size, count, interval, do_pmtu_disc)
        addr = __get_workload_address(w2, af)
        ping_cmd = __ping_addr_substitution(ping_base_cmd, addr)

        api.Logger.verbose(" Ping cmd %s " % (ping_cmd))
        api.Trigger_AddCommand(req, w1.node_name, w1.workload_name, ping_cmd)
        cmd_cookies.append(ping_cmd)

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

def iperfWorkloads(workload_pairs, af="ipv4", proto="tcp", packet_size=64):
    serverCmds = []
    clientCmds = []
    cmdDesc = []
    num_sessions = 1
    ipproto = __get_ipproto(af)
    
    if not api.IsSimulation():
        serverReq = api.Trigger_CreateAllParallelCommandsRequest()
        clientReq = api.Trigger_CreateAllParallelCommandsRequest()
    else:
        serverReq = api.Trigger_CreateExecuteCommandsRequest(serial = False)
        clientReq = api.Trigger_CreateExecuteCommandsRequest(serial = False)

    for idx, pairs in enumerate(workload_pairs):
        client = pairs[0]
        server = pairs[1]
        server_addr = __get_workload_address(server, af)
        client_addr = __get_workload_address(client, af)
        port = api.AllocateUdpPort() if proto == 'udp' else api.AllocateTcpPort()
        serverCmd = iperf.ServerCmd(port)
        clientCmd = iperf.ClientCmd(server_addr, port, 1, packet_size, proto, None, ipproto, num_sessions, jsonOut=True)

        cmd_cookie = "Server: %s(%s:%s:%d) <--> Client: %s(%s)" %\
                     (server.workload_name, server_addr, proto, port,\
                      client.workload_name, client_addr)
        api.Logger.verbose("Starting Iperf test %s num-sessions %d" % (cmd_cookie, num_sessions))
        serverCmds.append(serverCmd)
        clientCmds.append(clientCmd)
        cmdDesc.append(cmd_cookie)

        api.Trigger_AddCommand(serverReq, server.node_name, server.workload_name, serverCmd, background = True)
        api.Trigger_AddCommand(clientReq, client.node_name, client.workload_name, clientCmd)

    server_resp = api.Trigger(serverReq)
    #Sleep for some time as bg may not have been started.
    __sleep(30)
    client_resp = api.Trigger(clientReq)
    __sleep(3)
    api.Trigger_TerminateAllCommands(server_resp)
    return [cmdDesc, serverCmds, clientCmds], client_resp

def verifyIPerf(cmd_cookies, response, exit_code=0):
    result = api.types.status.SUCCESS
    conn_timedout = 0
    control_socker_err = 0
    cmdDesc = cmd_cookies[0]
    serverCmds = cmd_cookies[1]
    clientCmds = cmd_cookies[2]
    for idx, cmd in enumerate(response.commands):
        api.Logger.verbose("Iperf Result for %s" % (cmdDesc[idx]))
        api.Logger.verbose("Iperf Server cmd %s" % (serverCmds[idx]))
        api.Logger.verbose("Iperf Client cmd %s" % (clientCmds[idx]))
        if cmd.exit_code != exit_code:
            api.Logger.error("Iperf client exited with error")
            api.PrintCommandResults(cmd)
            if iperf.ConnectionTimedout(cmd.stdout):
                api.Logger.error("Connection timeout, ignoring for now")
                conn_timedout = conn_timedout + 1
                continue
            if iperf.ControlSocketClosed(cmd.stdout):
                api.Logger.error("Control socket cloned, ignoring for now")
                control_socker_err = control_socker_err + 1
                continue
            if iperf.ServerTerminated(cmd.stdout):
                api.Logger.error("Iperf server terminated")
                result = api.types.status.FAILURE
            if not iperf.Success(cmd.stdout):
                api.Logger.error("Iperf failed", iperf.Error(cmd.stdout))
                result = api.types.status.FAILURE
        elif not api.GlobalOptions.dryrun:
            api.Logger.verbose("Iperf Send Rate in Gbps ", iperf.GetSentGbps(cmd.stdout))
            api.Logger.verbose("Iperf Receive Rate in Gbps ", iperf.GetReceivedGbps(cmd.stdout))

    api.Logger.verbose("Iperf test successfull")
    api.Logger.verbose("Number of connection timeouts : {}".format(conn_timedout))
    api.Logger.verbose("Number of control socket errors : {}".format(control_socker_err))
    return result
