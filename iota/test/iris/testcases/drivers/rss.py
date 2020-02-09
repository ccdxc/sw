#! /usr/bin/python3
from itertools import product
import ipaddress
from bitstring import BitArray
from collections import Counter
import iota.harness.api as api
import iota.test.iris.utils.iperf as iperf
import iota.test.utils.naples_host as host
import iota.test.iris.config.netagent.hw_push_config as hw_config
import pdb
import iota.harness.infra.utils.toeplitz as toeplitz
from iota.harness.infra.utils.toeplitz import *


def generate_flows(tc, num_flows, num_queues):
    queue_used = Counter()
    port_used = set()
    hash_mask = (1 << 7) - 1

    for q in range(num_queues):
        queue_used[q] = 0

    for server_port, client_port in product(range(10000, 19999), range(20000, 29999)):

        if server_port in port_used or client_port in port_used:
            continue

        hdr = (tc.server_ip,tc.client_ip, server_port, client_port)
        flow_hash = toeplitz.toeplitz_hash(toeplitz.toeplitz_input(*hdr, rss_type_num=tc.rss_enum),
                                          BitArray(bytes=toeplitz.toeplitz_symmetric_key))

        queue = (flow_hash & hash_mask) % num_queues
        if queue_used[queue] != num_flows:
            queue_used[queue] += 1
            yield (queue, server_port, client_port)

        port_used.add(client_port)
        port_used.add(server_port)

        if all(queue_used[q] == num_flows for q in queue_used):
            return api.types.status.SUCCESS

    else:
        api.Logger.info(f"Queue used: {queue_used}")
        api.Logger.error("Not enough combinations to generate all flows!")
        return api.types.status.FAILURE


def send_iperf_cmd(tc, client_port, server_port, test_packet_count):
    """
    send iperf to the receiving node
    use specific client/server ports to sensure packets land in the desired queue
    """
    serverReq = None
    clientReq = None

    serverReq = api.Trigger_CreateExecuteCommandsRequest(serial=False)
    clientReq = api.Trigger_CreateExecuteCommandsRequest(serial=False)

    cmd_descr = f"Server: [({tc.server.workload_name})({tc.server_ip})] <--> Client: [({tc.client.workload_name})({tc.client_ip})]" 
    tc.cmd_descr.append(cmd_descr)

    server_cmd = None
    client_cmd = None

    server_cmd = iperf.ServerCmd(server_port)
    client_cmd = iperf.ClientCmd(tc.server_ip, server_port, time=10, proto=tc.proto, jsonOut=True,
                                ipproto=tc.tc_ip_proto, num_of_streams=tc.num_sessions,
                                client_ip=tc.client_ip, client_port=client_port, packet_count=test_packet_count,
                                bandwidth='10m')

    tc.server_cmds.append(server_cmd)
    tc.client_cmds.append(client_cmd)
    api.Logger.info(f"Iperf:  Server [{tc.server.node_name}.{tc.server.interface}]")
    api.Logger.info(f"        Client [{tc.client.node_name}.{tc.client.interface}]")
    
    api.Trigger_AddCommand(serverReq, tc.server.node_name, tc.server.workload_name,
                           server_cmd, background=True)

    api.Trigger_AddCommand(clientReq, tc.client.node_name, tc.client.workload_name,
                           client_cmd)


    server_resp = api.Trigger(serverReq)
    #Sleep for some time as bg may not have been started.
    time.sleep(10)

    tc.iperf_client_resp = api.Trigger(clientReq)
    api.Trigger_TerminateAllCommands(server_resp)

    return api.types.status.SUCCESS

def trigger_iperf(tc, num_queues):
    num_flows = 1
    test_packet_count = 10000
    queue = []
    tc.server_cmds = []
    tc.client_cmds = []
    tc.cmd_descr = []
    tc.failed_queues = []

    # get the client and server port numbers, based on hash and ip addr.
    for queue, server_port, client_port in generate_flows(tc, num_flows, num_queues):
        api.Logger.info(f"------- RSS for: queue ---> {queue} -------")

        # collect stats before the test
        stats_before_test = get_rx_queue_stats(tc)
        if len(stats_before_test) == 0:
            return api.types.status.FAILURE

        # send iperf, generate traffic on a specific queues                
        send_iperf_cmd(tc, client_port, server_port, test_packet_count)

        if verify_iperf(tc) is api.types.status.SUCCESS:
            stats_after_test = get_rx_queue_stats(tc)
            # test if the packets landed in the right queue
            # if RSS is disabled, expect all traffic to land on queue [0]
            if (tc.iterators.rss == 'disabled'): current_stats = stats_after_test[0] - stats_before_test[0]
            else: current_stats = stats_after_test[queue] - stats_before_test[queue]

            if  current_stats < test_packet_count: 
                    api.Logger.error (f"RSS Failed for queue [{queue}]")
                    tc.failed_queues.append (queue)
        else: return api.types.status.FAILURE

    return api.types.status.SUCCESS

def verify_iperf(tc):
    """
    Verifies iperf command output
    Handles exceptions
    Returns Success/Failure
    """

    conn_timedout = 0
    control_socker_err = 0
    for idx, cmd in enumerate(tc.iperf_client_resp.commands):
        api.Logger.info(tc.cmd_descr[idx])
        api.Logger.info("Server cmd  %s" % (tc.server_cmds[idx]))
        api.Logger.info("Client cmd %s" % (tc.client_cmds[idx]))
        if cmd.exit_code != 0:
            api.Logger.error("Iperf client exited with error")
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
                return api.types.status.FAILURE
            if not iperf.Success(cmd.stdout):
                api.Logger.error("Iperf failed", iperf.Error(cmd.stdout))
                return api.types.status.FAILURE

    if conn_timedout > 0:
        api.Logger.info("Number of connection timeouts : {}".format(conn_timedout))
    if control_socker_err > 0: 
        api.Logger.info("Number of control socket errors : {}".format(control_socker_err))

    api.Logger.info("Iperf test successfull")

    return api.types.status.SUCCESS

def get_rx_queue_stats(tc):
    """
    get stats for all RX queues from the interface under test
    """
    queue_stats = []

    get_rx_queue_stats_cmd= f"ethtool -S  {tc.server.parent_interface} | awk '/rx_._pkts|rx_.._pkts/ {{ print $2 }}'"
    req = api.Trigger_CreateExecuteCommandsRequest()

    api.Trigger_AddHostCommand(req, tc.server.node_name, get_rx_queue_stats_cmd)
    resp = api.Trigger(req)

    if resp is None:
        api.Logger.error(f"Failed to get que stats info from interface {tc.server.parent_interface}" )
        return queue_stats

    cmd = resp.commands.pop()

    if cmd.exit_code != 0:
        api.Logger.error(f"Error in {get_rx_queue_stats_cmd}")
        return queue_stats

    if cmd.stdout == "": 
        api.Logger.error(f"Error in {get_rx_queue_stats_cmd} ")
        api.Logger.error(f"Queue Stats: {queue_stats}")
        return queue_stats

    # split stdout by lines to get the stat values
    # make sure all values are a min of zero
    queue_stats = cmd.stdout.splitlines()
    for i in range(len(queue_stats)):
        if queue_stats[i] == '': queue_stats[i] = '0'

    # convert to a list of ints
    queue_stats = list(map(int,queue_stats))
    api.Logger.info (f"Queue Stats for {tc.server.parent_interface}: {queue_stats}")

    return queue_stats

def Setup(tc):

    #map iterators from IOTA infra to the enum for the toeplitz utilitys
    iterators = {'tcp4' : RSS.IPV4_TCP,  'udp4':RSS.IPV4_UDP , 'tcp6' : RSS.IPV6_TCP, \
                 'udp6':RSS.IPV6_UDP, 'ip6':RSS.IPV6,  'ip4tcp':RSS.IPV4, 'ip4udp':RSS.IPV4, 'ip6tcp':RSS.IPV6, 'ip6udp':RSS.IPV6}
    tc.rss_enum = iterators.get(tc.iterators.rxflowhash, "none")

    # iperf options for iterators
    # IPv4 vs IPv6 iterator
    ip_proto_iterators = {'tcp4' : 'v4',  'udp4':'v4' , 'tcp6' : 'v6', 'udp6':'v6', 'ip4tcp':'v4', \
                         'ip4udp':'v4', 'ip6tcp':'v6', 'ip6udp':'v6'}
    tc.tc_ip_proto = ip_proto_iterators.get(tc.iterators.rxflowhash, "none")
  
    # UDP vs TCP iterator
    proto_iterators = {'tcp4' : 'tcp',  'udp4':'udp' , 'tcp6':'tcp', 'udp6':'udp', 'ip4tcp':'tcp', 
                       'ip4udp':'udp', 'ip6tcp':'tcp', 'ip6udp':'udp'}
    tc.proto = proto_iterators.get(tc.iterators.rxflowhash, "none")
  
    if tc.proto  == "none" or tc.tc_ip_proto == "none " or tc.rss_enum == "none": 
        api.Logger.error ( f"Not able to map the iterators. Iterator:{tc.iterators.rxflowhash} tc_ip_proto: {tc.tc_ip_proto}, proto: {tc.proto}, tc.rss_enum: {tc.rss_enum}")
        return api.types.status.FAILURE

    # number of sessions iterator
    tc.num_sessions = int(getattr(tc.args, "num_sessions", 1))

    # log which iterration is in progress:
    api.Logger.info (f"=============== %s ===============" % tc.rss_enum)
    api.Logger.info(f"ip_proto:{tc.tc_ip_proto}, proto: {tc.proto}, rss: {tc.iterators.rss}, iperf_sessions: {tc.num_sessions}")

    #TODO: Is this different ?
    #tc.nodes = api.GetNaplesHostnames()
    tc.nodes = api.GetWorkloadNodeHostnames()
    tc.os = api.GetNodeOs(tc.nodes[0])
    if tc.os == 'freebsd':
        return api.types.status.SUCCESS

    # Identify the receiving node (testing RSS)
    # This will be the client node for iPerf
    # All configuration, and testing and verification will be done on this node
    for n in tc.nodes:
        if api.IsNaplesNode(n):
            api.Logger.info(f"Found Naples Node: [{n}]")
            ReceiveNode = n
            break
    else: 
        api.Logger.error(f"Failed to find a Naples Node!")
        return api.types.status.FAILURE

    
    # Get workload pars for iperf sessions
    workload_pairs = api.GetRemoteWorkloadPairs()
    if len(workload_pairs) == 0:
        api.Logger.info("Skipping Testcase due to no workload pairs.")
        tc.skip = True

    # assign client/server node based on selected Receiving Node
    for pair in workload_pairs:
        if ReceiveNode == pair[0].node_name:
            tc.client = pair[1]
            tc.server = pair[0]
        else:
            tc.client = pair[0]
            tc.server = pair[1]
        break

    # unload driver, to clear stats (server node only)
    # Re-add workloads
    if host.UnloadDriver(tc.os, tc.server.node_name, "all") is api.types.status.FAILURE:
        return api.types.status.FAILURE
    if host.LoadDriver(tc.os, tc.server.node_name) is api.types.status.FAILURE:
        return api.types.status.FAILURE
    hw_config.ReAddWorkloads(tc.server.node_name)

    if tc.tc_ip_proto is 'v6': 
        tc.server_ip = ipaddress.ip_address(tc.server.ipv6_address)
        tc.server_ip = str(tc.server_ip.exploded)

        tc.client_ip = ipaddress.ip_address(tc.client.ipv6_address)
        tc.client_ip = str(tc.client_ip.exploded)
    else:
        tc.server_ip = tc.server.ip_address
        tc.client_ip = tc.client.ip_address

    return api.types.status.SUCCESS

def Trigger(tc):
    get_queues = "grep -c processor /proc/cpuinfo"
    #get_queues = f"ethtool -l {tc.client.parent_interface} | grep -m 1 Combined | awk '/Combined:/' | awk '{{print $2}}'"
    api.Logger.info(get_queues)

    # TODO: FreeBSD logic is to be implemented
    if tc.os == 'freebsd':
        return api.types.status.SUCCESS

    # discover a number of cores on the server node
    req = api.Trigger_CreateExecuteCommandsRequest()
    api.Trigger_AddHostCommand(req, tc.server.node_name, get_queues)
    resp = api.Trigger(req)

    if resp is None:
        api.Logger.error(f"Failed to get queue count from {tc.server.node_name} \
                                                  -{tc.server.parent_interface}")
        return api.types.status.FAILURE
    
    cmd = resp.commands.pop()
    
    if cmd.exit_code != 0:
        api.Logger.error(f"Error in {get_queues}")
        api.PrintCommandResults(cmd)
        return api.types.status.FAILURE

    # number of queues is min of Naples Max or Test Case Max
    num_queues = tc.args.maxqueues
    num_queues = min(tc.args.maxqueues, int(cmd.stdout.strip()))

    req = api.Trigger_CreateExecuteCommandsRequest()

    if tc.iterators.rss == 'disabled':
        # disable RSS hashing
        init_rss_cmd = f"ethtool -K {tc.server.parent_interface} rxhash off"

    else:
        # enable RSS hashing
        enable_rss_cmd = f"ethtool -K {tc.server.parent_interface} rxhash on"
        api.Trigger_AddHostCommand(req, tc.server.node_name, enable_rss_cmd)
        # init hash key and num of queues
        key = ':'.join([hex(x)[2:] for x in toeplitz_symmetric_key])
        init_rss_cmd =  f"ethtool -X {tc.server.parent_interface} hfunc toeplitz hkey {key} equal {num_queues}"

    api.Logger.info(f"{init_rss_cmd}")    
    api.Trigger_AddHostCommand(req, tc.server.node_name, init_rss_cmd)

    resp = api.Trigger(req)

    if resp is None:
        api.Logger.error(f"Failed to initialize RSS on host {n}, interface{tc.server.parent_interface} ")
        return api.types.status.FAILURE

    for cmd in resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            api.Logger.error(f"Error in {enable_rss_cmd}")
            api.PrintCommandResults(cmd)
            return api.types.status.FAILURE

    api.Logger.info(f"Total Queues: {num_queues}")

    return trigger_iperf(tc, num_queues)

def Verify(tc):
    if tc.failed_queues:
        api.Logger.error ("The following queues failed in this iteration: %s" % ', '.join(map(str, tc.failed_queues)))
        return api.types.status.FAILURE

    return api.types.status.SUCCESS
