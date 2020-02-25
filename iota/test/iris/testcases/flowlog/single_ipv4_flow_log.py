#! /usr/bin/python3
import time
import ipaddress
import pdb

import iota.harness.api as api
from iota.test.iris.utils.shmdump_helper import *

'''
global variables
'''
logsForCommand = None
shmDumpHelper = None
numPkts = 5
server_port = None
client_port = None

def add_command(tc, req, cmd_cookie, cmd, host, bg):
    tc.cmd_cookies.append(cmd_cookie)
    api.Trigger_AddCommand(req, host.node_name, host.workload_name, cmd,
                           background = bg)

def add_naples_command(tc, req, cmd_cookie, cmd, naples):
    tc.cmd_cookies.append(cmd_cookie)
    api.Trigger_AddNaplesCommand(req, naples.node_name, cmd)

def runServerCmd(tc, server_cmd):
    serverReq = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    server_cookie = "Running command at server %s: %s" % (tc.server.ip_address, server_cmd)
    api.Logger.info(server_cookie)
    add_command(tc, serverReq, server_cookie, server_cmd, tc.server, False)
    serverResp = api.Trigger(serverReq)
    if not serverResp:
        api.Logger.error("ERROR: Failed to run server command %s" % server_cmd)
        return api.types.status.FAILURE
    for command in serverResp.commands:
        api.PrintCommandResults(command)
    return api.types.status.SUCCESS

def stopServer(tc):
    return runServerCmd(tc, "pkill -9 iperf3")

def startServer(tc):
    return runServerCmd(tc, "iperf3 --one-of -s -D -p %d" % (server_port))

def Setup(tc):
    tc.cmd_cookies = []
    # Determine the client, server and the naples nodes.
    pairs = api.GetRemoteWorkloadPairs()
    tc.server, tc.client  = pairs[0]
    # Force the naples node to be the client always till the fte bug is fixed.
    if not tc.client.IsNaples():
        tc.client, tc.server = pairs[0]
        assert(tc.client.IsNaples())
    tc.naples = tc.client

    # Create the ShmDumpHelper object
    global shmDumpHelper
    shmDumpHelper = ShmDumpHelper()

    # Assign the ports
    global server_port, client_port
    server_port = api.AllocateUdpPort()
    client_port = api.AllocateUdpPort()
    if server_port and client_port:
        api.Logger.info("Using %d as client port and %d as server port" % (client_port, server_port))

    stopServer(tc)
    return startServer(tc)

#
# Issue "shmdump -file=fwlog_ipc_shm -type=fwlog -full" on the
# naples to snapshot the current state.
# 
def snapshotSharedMem(tc):
    global shmDumpHelper
    napReq = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    cmd_cookie = "shmdump - Snapshot Run"
    cmd = "/nic/bin/shmdump -file=fwlog_ipc_shm -type=fwlog -full"
    add_naples_command(tc, napReq, cmd_cookie, cmd, tc.naples)
    napResp = api.Trigger(napReq)
    if not napResp:
        api.Logger.error("ERROR: No response from naples")
        return api.types.status.FAILURE
    for command in napResp.commands:
        api.PrintCommandResults(command)
    prevIndex = shmDumpHelper.store_shmdump_output(napResp.commands[0])
    if prevIndex is None:
        api.Logger.error("ERROR: Parsing of command output failed")
        return api.types.status.FAILURE
    return api.types.status.SUCCESS

def doTriggerIcmp(tc):
    global shmDumpHelper, numPkts, logsForCommand
    cmd_cookie = "%s(%s) --> %s(%s)" %\
                 (tc.server.workload_name, tc.server.ip_address,
                  tc.client.workload_name, tc.client.ip_address)
    api.Logger.info("Starting Single-IPv4-ICMP-Flow-Log test from %s" %\
                   (cmd_cookie))

    if snapshotSharedMem(tc) != api.types.status.SUCCESS:
        api.Logger.error("ERROR: Snapshot of the shared memory failed")
        return api.types.status.FAILURE

    hostReq = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    napReq = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    #
    # Add commands to ping the server from the client
    #
    cmd_cookie = "trigger ping: Create case"
    cmd = "ping -c %d %s -I %s" %\
          (numPkts, tc.client.ip_address, tc.server.interface)
    add_command(tc, hostReq, cmd_cookie, cmd, tc.server, False)

    #
    # Issue "halctl clear session" to force delete of the
    # icmp session. This way the shmdump would capture both the
    # flow create and delete events.
    #
    rindex = shmDumpHelper.get_next_read_index()
    clear_cmd_cookie = "Issue halctl clear session"
    clear_cmd = "/nic/bin/halctl clear session"
    cmd_cookie = "shmdump - Second Run"
    cmd = "/nic/bin/shmdump -file=fwlog_ipc_shm -type=fwlog -full -rindex=%s" % rindex
    add_naples_command(tc, napReq, clear_cmd_cookie, clear_cmd, tc.naples)
    add_naples_command(tc, napReq, cmd_cookie, cmd, tc.naples)

    hostResp = api.Trigger(hostReq)
    if not hostResp:
        api.Logger.error("ERROR: Ping failed on the host")
        return api.types.status.FAILURE
    for command in hostResp.commands:
        api.PrintCommandResults(command)
    if hostResp.commands[0].exit_code != 0:
        api.Logger.error("ERROR: Ping failed on the host")
        return api.types.status.FAILURE
    # Issue the commands on the Naples.
    napResp = api.Trigger(napReq)
    if not napResp:
        api.Logger.error("ERROR: No response from naples")
        return api.types.status.FAILURE
    for command in napResp.commands:
        api.PrintCommandResults(command)
    currIndex = shmDumpHelper.store_shmdump_output(napResp.commands[1])
    if currIndex is None:
        api.Logger.error("ERROR: Parsing of command output failed - Second Issue")
        return api.types.status.FAILURE

    # Updates global logsForCommand
    logsForCommand = shmDumpHelper.get_last_parsed_output()
    if not logsForCommand or len(logsForCommand) == 0:
        api.Logger.error("ERROR: Getting difference of outputs failed.")
        return api.types.status.FAILURE
    api.Logger.info("New input to process: %d lines from output" % len(logsForCommand))

    api.Trigger_TerminateAllCommands(hostResp)
    api.Trigger_TerminateAllCommands(napResp)
    return api.types.status.SUCCESS

def doTriggerUdp(tc):
    global shmDumpHelper, logsForCommand
    cmd_cookie = "%s(%s) --> %s(%s)" %\
                 (tc.server.workload_name, tc.server.ip_address,
                  tc.client.workload_name, tc.client.ip_address)
    api.Logger.info("Starting Single-IPv4-UDP-Flow-Log test from %s" %\
                   (cmd_cookie))

    if snapshotSharedMem(tc) != api.types.status.SUCCESS:
        api.Logger.error("ERROR: Snapshot of the shared memory failed")
        return api.types.status.FAILURE

    clientReq = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    napReq = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    client_cookie = "Starting UDP client at %s port %d" % (tc.client.ip_address, client_port)
    client_cmd = "iperf3 -u -c %s -p %d -B %s --cport %d -t 1" % (tc.server.ip_address, server_port, tc.client.ip_address, client_port)
    add_command(tc, clientReq, client_cookie, client_cmd, tc.client, False)
    
    #
    # Issue "halctl clear session" to force delete of the
    # UDP session. This way the shmdump would capture both the
    # flow create and delete events.
    #
    rindex = shmDumpHelper.get_next_read_index()
    clear_cmd_cookie = "Issue halctl clear session"
    clear_cmd = "/nic/bin/halctl clear session"
    cmd_cookie = "shmdump - Second Run"
    cmd = "/nic/bin/shmdump -file=fwlog_ipc_shm -type=fwlog -full -rindex=%s" % rindex
    add_naples_command(tc, napReq, clear_cmd_cookie, clear_cmd, tc.naples)
    add_naples_command(tc, napReq, cmd_cookie, cmd, tc.naples)

    # Start the client
    clientResp = api.Trigger(clientReq)
    if not clientResp:
        api.Logger.error("ERROR: Failed to start UDP client at %s", (tc.client.ip_address))
        return api.types.status.FAILURE
    for command in clientResp.commands:
        api.PrintCommandResults(command)

    # Issue the commands on the Naples.
    napResp = api.Trigger(napReq)
    if not napResp:
        api.Logger.error("ERROR: No response from naples")
        return api.types.status.FAILURE
    for command in napResp.commands:
        api.PrintCommandResults(command)
    currIndex = shmDumpHelper.store_shmdump_output(napResp.commands[1])
    if currIndex is None:
        api.Logger.error("ERROR: Parsing of command output failed - Second Issue")
        return api.types.status.FAILURE

    # Updates global logsForCommand
    logsForCommand = shmDumpHelper.get_last_parsed_output()
    if not logsForCommand or len(logsForCommand) == 0:
        api.Logger.error("ERROR: Getting difference of outputs failed.")
        return api.types.status.FAILURE
    api.Logger.info("New input to process: %d lines from output" % len(logsForCommand))

    api.Trigger_TerminateAllCommands(clientResp)
    api.Trigger_TerminateAllCommands(napResp)
    return api.types.status.SUCCESS

def doTriggerTcp(tc):
    global shmDumpHelper, logsForCommand
    cmd_cookie = "%s(%s) --> %s(%s)" %\
                 (tc.server.workload_name, tc.server.ip_address,
                  tc.client.workload_name, tc.client.ip_address)
    api.Logger.info("Starting Single-IPv4-TCP-Flow-Log test from %s" %\
                   (cmd_cookie))

    if snapshotSharedMem(tc) != api.types.status.SUCCESS:
        api.Logger.error("ERROR: Snapshot of the shared memory failed")
        return api.types.status.FAILURE

    clientReq = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    napReq = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    client_cookie = "Starting TCP client at %s port %d" % (tc.client.ip_address, client_port)
    client_cmd = "iperf3 -c %s -p %d -B %s --cport %d -t 1" % (tc.server.ip_address, server_port, tc.client.ip_address, client_port)
    add_command(tc, clientReq, client_cookie, client_cmd, tc.client, False)
    
    #
    # Issue "halctl clear session" to force delete of the
    # TCP session. This way the shmdump would capture both the
    # flow create and delete events.
    #
    rindex = shmDumpHelper.get_next_read_index()
    clear_cmd_cookie = "Issue halctl clear session"
    clear_cmd = "/nic/bin/halctl clear session"
    cmd_cookie = "shmdump - Second Run"
    cmd = "/nic/bin/shmdump -file=fwlog_ipc_shm -type=fwlog -full -rindex=%s" % rindex
    add_naples_command(tc, napReq, clear_cmd_cookie, clear_cmd, tc.naples)
    add_naples_command(tc, napReq, cmd_cookie, cmd, tc.naples)

    # Start the client
    clientResp = api.Trigger(clientReq)
    if not clientResp:
        api.Logger.error("ERROR: Failed to start TCP client at %s", tc.client.ip_address)
        return api.types.status.FAILURE
    for command in clientResp.commands:
        api.PrintCommandResults(command)

    # Issue the commands on the Naples.
    napResp = api.Trigger(napReq)
    if not napResp:
        api.Logger.error("ERROR: No response from naples")
        return api.types.status.FAILURE
    for command in napResp.commands:
        api.PrintCommandResults(command)
    currIndex = shmDumpHelper.store_shmdump_output(napResp.commands[1])
    if currIndex is None:
        api.Logger.error("ERROR: Parsing of command output failed - Second Issue")
        return api.types.status.FAILURE

    # Updates global logsForCommand
    logsForCommand = shmDumpHelper.get_last_parsed_output()
    if not logsForCommand or len(logsForCommand) == 0:
        api.Logger.error("ERROR: Getting difference of outputs failed.")
        return api.types.status.FAILURE
    api.Logger.info("New input to process: %d lines from output" % len(logsForCommand))

    api.Trigger_TerminateAllCommands(clientResp)
    api.Trigger_TerminateAllCommands(napResp)
    return api.types.status.SUCCESS

def Trigger(tc):
    if tc.iterators.proto == 'icmp':
        return doTriggerIcmp(tc)
    elif tc.iterators.proto == 'udp':
        return doTriggerUdp(tc)
    else:
        return doTriggerTcp(tc)

def verifyIcmp(tc):
    global logsForCommand
    global numPkts
    global shmDumpHelper

    if not logsForCommand:
        return api.types.status.FAILURE
    # We force the naples to be client however for icmp we ping the client
    # from the server. So naples (client) would see the first packet from
    # the wire.
    direction = 'from-uplink'
    return shmDumpHelper.verifyIcmp(logsForCommand, src=tc.server.ip_address,
                                    dst=tc.client.ip_address, direction=direction,
                                    statsMatch=True, nPackets=numPkts)

def verifyUdp(tc):
    global logsForCommand
    global shmDumpHelper

    if not logsForCommand:
        return api.types.status.FAILURE
    direction = 'from-uplink' if tc.naples == tc.server else 'from-host'
    return shmDumpHelper.verifyUdp(logsForCommand, src=tc.client.ip_address,
                                   dst=tc.server.ip_address, sport=str(client_port),
                                   dport=str(server_port), direction=direction)

def verifyTcp(tc):
    global logsForCommand
    global shmDumpHelper

    if not logsForCommand:
        return api.types.status.FAILURE
    direction = 'from-uplink' if tc.naples == tc.server else 'from-host'
    return shmDumpHelper.verifyTcp(logsForCommand, src=tc.client.ip_address,
                                   dst=tc.server.ip_address, sport=str(client_port),
                                   dport=str(server_port), direction=direction)

def Verify(tc):
    if tc.iterators.proto == 'icmp':
        return verifyIcmp(tc)
    elif tc.iterators.proto == 'udp':
        return verifyUdp(tc)
    else:
        return verifyTcp(tc)

def Teardown(tc):
    stopServer(tc)
    return api.types.status.SUCCESS

