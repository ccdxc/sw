#! /usr/bin/python3
import pdb
import time
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.testcases.penctl.common as common
import iota.test.iris.testcases.telemetry.utils as utils

def Setup(tc):

    tc.workload_pairs = api.GetRemoteWorkloadPairs()
    tc.Nodes = api.GetNaplesHostnames()
    tc.Workloads = api.GetWorkloads()

    if len(tc.workload_pairs) == 0:
        api.Logger.info("Skipping Testcase due to no workload pairs.")
        tc.skip = True

    return api.types.status.SUCCESS

def GetIPOptionsStr(src_ip, dst_ip):
    sd = src_ip + "." + dst_ip
    sd_list = sd.split('.')
    string = r"--ip-options '\x07\x13\x20\x00\x00\x00\x00\xde\xad\xbe\xef\x{:02x}\x{:02x}\x{:02x}\x{:02x}\x{:02x}\x{:02x}\x{:02x}\x{:02x}\x00'".\
                format(*map(int, sd_list))
    return string
 
def Trigger(tc):
    #Run all commands in serial
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    tc.cmd_cookies = []

#    for n in tc.Nodes:
#        common.AddPenctlCommand(req, n, "update mode -f eth-dev-scale")
#        tc.cmd_cookies.append("Update mode to eth-dev-scale")
#        common.RebootHost(n)
#        tc.cmd_cookies.append("Sysreset naples and reboot host")

    for pair in tc.workload_pairs:
        w1 = pair[0]
        w2 = pair[1]

        if w1.IsNaples():
            master = w1
            peer = w2
        elif w2.IsNaples():
            master = w2
            peer = w1

        api.Logger.info("Master,Peer pair %s, %s" %(master.ip_address, peer.ip_address))

        gw_prefix = master.ip_address.split(".")
        gw_prefix[1] = "0"
        gw_prefix[2] = "0"
        gw_prefix[3] = "0"
        gw_prefix_str = ".".join(gw_prefix)

        #Configure MPLSoUDP interface
        api.Logger.info("Configure MPLSoUDP intf using Penctl")
        penctl_cmd = "update interface --name %s --gw-mac %s --substrate-ip %s --overlay-ip %s --mpls-in 100 --tunnel-dest-ip %s --mpls-out 200 --source-gw %s/%d --ingress-bw 1000 --egress-bw 1000 --encap MPLSoUDP" % (master.interface, peer.mac_address, master.ip_address, master.ip_address, peer.ip_address, gw_prefix_str, 8)
        common.AddPenctlCommand(req, master.node_name, penctl_cmd)
        tc.cmd_cookies.append("penctl create tunnel")

        api.Trigger_AddNaplesCommand(req, master.node_name, "/nic/bin/halctl show interface | grep Tunnel")
        tc.cmd_cookies.append("halctl show interface")

        #Add static arp entry for peer
        api.Logger.info("Add static ARP entry for %s" %(peer.ip_address))
        api.Trigger_AddCommand(req, master.node_name, master.workload_name,
                                "arp -s %s %s" % (peer.ip_address, peer.mac_address))
        tc.cmd_cookies.append("Add static arp entry for %s" % peer.ip_address)

        tc.cmd_descr = "Client: %s(%s) <--> Server: %s(%s)" %\
                    (master.workload_name, master.ip_address, peer.workload_name, peer.ip_address)

        #Start ping traffic
        api.Logger.info("Starting ping test from %s" % (tc.cmd_descr))
        cmd = utils.GetTcpDumpCmd(peer.interface, "udp", 6635)
        api.Trigger_AddCommand(req, peer.node_name, peer.workload_name,
                           cmd, background = True)
        tc.cmd_cookies.append("Start tcpdump")
        api.Trigger_AddCommand(req, master.node_name, master.workload_name,
                           "ping -c 1 %s" % peer.ip_address, background = False)
        tc.cmd_cookies.append("Send ping packet to regular dest ip")
        #Start tcp traffic
        api.Logger.info("Starting tcp test from %s" % (tc.cmd_descr))
        cmd = utils.GetTcpDumpCmd(peer.interface, "udp", 6635)
        api.Trigger_AddCommand(req, peer.node_name, peer.workload_name,
                           cmd, background = True)
        tc.cmd_cookies.append("Start tcpdump")
        cmd = utils.GetHping3Cmd("tcp", peer.ip_address, 8080)
        api.Trigger_AddCommand(req, master.node_name, master.workload_name,
                           cmd, background = False)
        tc.cmd_cookies.append("Send tcp packet to regular dest ip")
        #Start udp traffic
        api.Logger.info("Starting udp test from %s" % (tc.cmd_descr))
        cmd = utils.GetTcpDumpCmd(peer.interface, "udp", 6635)
        api.Trigger_AddCommand(req, peer.node_name, peer.workload_name,
                           cmd, background = True)
        tc.cmd_cookies.append("Start tcpdump")
        cmd = utils.GetHping3Cmd("udp", peer.ip_address, 8080)
        api.Trigger_AddCommand(req, master.node_name, master.workload_name,
                           cmd, background = False)
        tc.cmd_cookies.append("Send udp packet to regular dest ip")

        #Add IP route for class-E address
        api.Logger.info("Add IP route for 254.254.0.1")
        api.Trigger_AddCommand(req, master.node_name, master.workload_name,
                                "ip route add 254.254.0.1 dev %s" % master.interface)
        tc.cmd_cookies.append("Add IP route for 254.254.0.1")

        #Add static arp entry for class-E address
        api.Logger.info("Add static ARP entry for 254.254.0.1")
        api.Trigger_AddCommand(req, master.node_name, master.workload_name,
                                "arp -s 254.254.0.1 %s" % peer.mac_address)
        tc.cmd_cookies.append("Add static arp entry for 254.254.0.1")

        tc.cmd_descr = "Client: %s(%s) <--> Server: %s(%s)" %\
                    (master.workload_name, master.ip_address, peer.workload_name, "254.254.0.1")

        #Start udp traffic without IP options
        api.Logger.info("Starting udp test to class-E address without ip options from %s" % (tc.cmd_descr))
        cmd = utils.GetTcpDumpCmd(peer.interface, "udp", 6635)
        api.Trigger_AddCommand(req, peer.node_name, peer.workload_name,
                           cmd, background = True)
        tc.cmd_cookies.append("Start tcpdump")
        cmd = utils.GetHping3Cmd("udp", "254.254.0.1", 8080)
        api.Trigger_AddCommand(req, master.node_name, master.workload_name,
                           cmd, background = False)
        tc.cmd_cookies.append("Send udp packet for class-e with no ip options")

        #Start udp traffic with valid IP options
        api.Logger.info("Starting udp test to class-E address with valid ip options from %s" % (tc.cmd_descr))
        cmd = utils.GetTcpDumpCmd(peer.interface, "udp", 6635)
        api.Trigger_AddCommand(req, peer.node_name, peer.workload_name,
                           cmd, background = True)
        tc.cmd_cookies.append("Start tcpdump")
        cmd = utils.GetNpingCmd("udp", "254.254.0.1", 8080)
        #Create new dest ip
        ip_opt_dest_ip = list(map(int, peer.ip_address.split(".")))
        ip_opt_dest_ip[3] = (ip_opt_dest_ip[3] + 10) % 255
        dest_ip = list(map(str, ip_opt_dest_ip))
        nping_cmd = cmd + " " + GetIPOptionsStr(master.ip_address, ".".join(dest_ip))
        api.Trigger_AddCommand(req, master.node_name, master.workload_name,
                           nping_cmd, background = False)
        tc.cmd_cookies.append("Send udp packet for class-e with valid ip options")

        #Start udp traffic with invalid IP options
        api.Logger.info("Starting udp test to class-E address with invalid ip options from %s" % (tc.cmd_descr))
        cmd = utils.GetTcpDumpCmd(peer.interface, "udp", 6635)
        api.Trigger_AddCommand(req, peer.node_name, peer.workload_name,
                           cmd, background = True)
        tc.cmd_cookies.append("Start tcpdump")
        cmd = utils.GetNpingCmd("udp", "254.254.0.1", 8080)
        nping_cmd = cmd + " " + GetIPOptionsStr(master.ip_address, "0.0.0.0")
        api.Trigger_AddCommand(req, master.node_name, master.workload_name,
                           nping_cmd, background = False)
        tc.cmd_cookies.append("Send udp packet for class-e with invalid ip options")

        break

    #Now Send all the commands
    trig_resp = api.Trigger(req)
    time.sleep(10)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)

    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS
    cookie_idx = 0

    for cmd in tc.resp.commands:
        api.Logger.info("Results for %s" % (tc.cmd_cookies[cookie_idx]))
        api.PrintCommandResults(cmd)
        cookie_idx += 1

    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS
