#! /usr/bin/python3
import time
from collections import defaultdict
import iota.harness.api as api
import iota.test.iris.config.netagent.api as netagent_api

FUZ_EXEC =  {
}

class FuzContext:
    def __init__(self, workload_name, workload_intf, node_name):
        self.node_name = node_name
        self.workload_name = workload_name
        self.workload_intf = workload_intf
        self.pcap_file = workload_intf + ".pcap"
        self.servers = []
        self.interfaces = set()

    def AddServer(self, server_ip, server_port):
        self.servers.append(server_ip + ":" + str(server_port))

    def AddInterface(self, interface):
        self.interfaces.add(interface)

    def GetServers(self):
        return self.servers

def Setup(tc):

    tc.skip = False
    tc.workload_pairs = tc.selected
    if len(tc.workload_pairs) == 0:
        api.Logger.info("Skipping Testcase due to no workload pairs.")
        tc.skip = True

    wloads = set()
    for w1, w2 in tc.workload_pairs:
        wloads.add(w1)
        wloads.add(w2)

    fullpath = api.GetTopDir() + '/iota/bin/fuz'
    for w in wloads:
        resp = api.CopyToWorkload(w.node_name, w.workload_name, [fullpath], '')
        #Create a symlink at top level
        realPath = "realpath fuz"
        req = api.Trigger_CreateExecuteCommandsRequest()
        api.Trigger_AddCommand(req, w.node_name, w.workload_name, realPath, background = False)
        resp = api.Trigger(req)
        for cmd in resp.commands:
            api.PrintCommandResults(cmd)
            if cmd.exit_code != 0:
                return api.types.status.FAILURE
            FUZ_EXEC[cmd.entity_name] = cmd.stdout.split("\n")[0]

    return api.types.status.SUCCESS

def Trigger(tc):
    if tc.skip: return api.types.status.SUCCESS

    tc.serverCmds = []
    tc.clientCmds = []
    tc.cmd_descr = []


    fuzClients = {}
    fuzServers = {}
    clientReq = api.Trigger_CreateAllParallelCommandsRequest()
    serverReq = api.Trigger_CreateAllParallelCommandsRequest()
    clientArp = defaultdict(lambda : {})

    for idx, pairs in enumerate(tc.workload_pairs):
        client = pairs[0]
        server = pairs[1]

        cmd_descr = "Server: %s(%s) <--> Client: %s(%s)" %\
                       (server.workload_name, server.ip_address, client.workload_name, client.ip_address)
        tc.cmd_descr.append(cmd_descr)
        num_sessions = int(getattr(tc.args, "num_sessions", 1))
        api.Logger.info("Starting Fuz test from %s num-sessions %d" % (cmd_descr, num_sessions))

        serverCmd = None


        port = api.AllocateTcpPort()
        tc.serverCmds.append(serverCmd)

        fuzClient = fuzClients.get(client.workload_name, None)
        if not fuzClient:
            fuzClient = FuzContext(client.workload_name, client.interface, client.node_name)
            fuzClients[client.workload_name] = fuzClient

        fuzClient.AddServer(server.ip_address, port)

        #Combine baremetal workloads
        if api.IsBareMetalWorkloadType(server.node_name):
            fuzServer = fuzServers.get(server.node_name, None)
            if not fuzServer:
                fuzServer = FuzContext(server.workload_name, server.interface, server.node_name)
                fuzServers[server.node_name] = fuzServer
            else:
                fuzServer.AddInterface(server.interface)
            fuzServer.AddServer(server.ip_address, port)
        else:
            serverCmd = FUZ_EXEC[server.workload_name]  + " -port " + str(port)
            api.Trigger_AddCommand(serverReq, server.node_name, server.workload_name,
                                   serverCmd, background = True,
                                   stdout_on_err = True, stderr_on_err = True)

        #For now add static arp
        if api.IsBareMetalWorkloadType(client.node_name):
            if not clientArp[client.node_name].get(server.ip_address, None):
                clientArp[client.node_name][server.ip_address] = server.mac_address
                arp_cmd = "arp -s " + server.ip_address + " " + server.mac_address
                api.Trigger_AddCommand(clientReq, client.node_name, client.workload_name,
                                       arp_cmd)
        else:
            if not clientArp[client.workload_name].get(server.ip_address, None):
                clientArp[client.workload_name][server.ip_address] = server.mac_address
                arp_cmd = "arp -s " + server.ip_address + " " + server.mac_address
                api.Trigger_AddCommand(clientReq, client.node_name, client.workload_name,
                                       arp_cmd)


    store = tc.GetBundleStore()
    store["server_req"] = serverReq
    store["client_ctxts"] = fuzClients
    store["server_ctxts"] = fuzServers
    store["arp_ctx"] = clientReq

    return api.types.status.SUCCESS

def Verify(tc):
    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS
