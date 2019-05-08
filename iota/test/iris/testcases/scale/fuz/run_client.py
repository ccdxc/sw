#! /usr/bin/python3
import time
import json
import os
import pdb
import iota.harness.api as api
import iota.test.iris.config.netagent.api as netagent_api
import iota.test.iris.testcases.scale.fuz.init as fuz_init



def Setup(tc):

    tc.skip = False
    tc.workload_pairs = tc.selected
    if len(tc.workload_pairs) == 0:
        api.Logger.info("Skipping Testcase due to no workload pairs.")
        tc.skip = True

    tc.action = str(getattr(tc.args, "action", "PERMIT"))
    return api.types.status.SUCCESS

def Trigger(tc):
    if tc.skip: return api.types.status.SUCCESS


    store = tc.GetBundleStore()
    cps = int(getattr(tc.args, "cps", 500))
    pcap_capture = int(getattr(tc.args, "pcap", False))
    sessions = int(getattr(tc.args, "num_sessions", 1))
    session_time = str(getattr(tc.args, "session_time", "10")) + "s"
    static_arp = int(getattr(tc.args, "static_arp", False))

    if static_arp:
        #install arp entries before running
        arpReqs = store["arp_ctx"]
        api.Trigger(arpReqs)

    #Spread CPS evenly
    clientReq = api.Trigger_CreateAllParallelCommandsRequest()

    clientPcapReq = api.Trigger_CreateAllParallelCommandsRequest()
    cps_per_node = int(cps / len(store["client_ctxts"]))
    for index, (_, client) in enumerate(store["client_ctxts"].items()):
        jsonInput = {"connections" : []}
        for serverIPPort in client.GetServers():
            jsonInput["connections"].append({"ServerIPPort" : serverIPPort, "proto" : "tcp"})
        outfile = api.GetTestDataDirectory() + "/" + client.workload_name + "_fuz.json"
        with open(outfile, 'w') as fp:
            json.dump(jsonInput, fp)
        api.CopyToWorkload(client.node_name, client.workload_name, [outfile], "")
        api.Trigger_AddCommand(clientPcapReq, client.node_name, client.workload_name,
                               "tcpdump -i eth1 -w %s.pcap" % (client.workload_name),  background=True)
        clientCmd = fuz_init.FUZ_EXEC[client.workload_name]  + " -duration " + session_time + " -conns " + str(sessions) + " -cps " + str(cps_per_node) + " -talk  --jsonOut --jsonInput " + os.path.basename(outfile)
        api.Trigger_AddCommand(clientReq, client.node_name, client.workload_name,
                               clientCmd, timeout = 0)


    if pcap_capture:
        #Start Server Pcap
        serverPcapReq = api.Trigger_CreateAllParallelCommandsRequest()
        for pcap_info in store["server_pcap_info"]:
            node_name, intf, pcap_file = pcap_info[0], pcap_info[1], pcap_info[2]
            api.Trigger_AddHostCommand(serverPcapReq, node_name,
                               "tcpdump -i %s -w %s" % (intf, pcap_file),  background=True)

        tc.server_pcap_resp = api.Trigger(serverPcapReq)
        tc.fuz_client_pcap_resp = api.Trigger(clientPcapReq)

    #Initiate connections
    tc.fuz_client_resp = api.Trigger(clientReq)

    if pcap_capture:
        #Stop tcpdump
        api.Trigger_TerminateAllCommands(tc.fuz_client_pcap_resp)
        api.Trigger_TerminateAllCommands(tc.server_pcap_resp)

        for _, client in store["client_ctxts"].items():
            api.CopyFromWorkload(client.node_name, client.workload_name, [client.workload_name + ".pcap"], tc.GetLogsDir())

        #Stop server Pcaps too
        #for pcap_info in store["server_pcap_info"]:
        #    node_name, intf, pcap_file = pcap_info[0], pcap_info[1], pcap_info[2]
        #    api.CopyFromHost(node_name, [pcap_file], tc.GetLogsDir())



    return api.types.status.SUCCESS

def __get_json(fuz_out):
    try:
        return json.loads(fuz_out)
    except:
        api.Logger.error("Failed to parse fuz json output :", fuz_out)
        pdb.set_trace()
        assert(0)

def verify_fuz(tc):
    failed_connections = 0
    success_connections = 0
    for idx, cmd in enumerate(tc.fuz_client_resp.commands):
        jsonOut = __get_json(cmd.stdout)
        success_connections = success_connections + jsonOut["successConnections"]
        if cmd.exit_code != 0:
            failed_connections = failed_connections + jsonOut["failedConnections"]
            for conn in jsonOut["connections"]:
                if  conn["error"] != "":
                    api.Logger.error("Failed Connection {}:{}, err : {}, sent/received : {}/{}".format(conn["ServerIPPort"],
                     conn["ClientIPPort"], conn["error"], conn["dataSent"], conn["dataReceived"]))

    api.Logger.info("Total Success connections {}".format(success_connections))

    if tc.action == "PERMIT" and failed_connections:
        api.Logger.info("Total Failed connections (Policy : ALLOW) {}".format(failed_connections))
        api.Logger.error("fuz test had errors")
        return api.types.status.FAILURE
    elif tc.action == "DENY" and success_connections:
        api.Logger.info("Total Success connections  (Policy : DENY)  {}".format(success_connections))
        api.Logger.error("fuz test had errors")
        return api.types.status.FAILURE
    else:
        api.Logger.info("Fuz test successfull")
    return api.types.status.SUCCESS

def Verify(tc):

    if tc.skip: return api.types.status.SUCCESS

    if verify_fuz(tc) != api.types.status.SUCCESS:
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS
