#! /usr/bin/python3
import os
import time
import json
import iota.harness.api as api
import iota.test.iris.config.netagent.api as netagent_api
import iota.test.iris.testcases.scale.fuz.init as fuz_init


def Setup(tc):

    tc.skip = False
    tc.workload_pairs = tc.selected
    if len(tc.workload_pairs) == 0:
        api.Logger.info("Skipping Testcase due to no workload pairs.")
        tc.skip = True

    return api.types.status.SUCCESS

def Trigger(tc):
    if tc.skip: return api.types.status.SUCCESS

    store = tc.GetBundleStore()
    serverReq = store["server_req"]


    if serverReq:
        store["server_resp"] = api.Trigger(serverReq)

    #If we want to start server combined
    server_pcap_info = []
    if store.get("server_ctxts", None):
        serverReq = api.Trigger_CreateAllParallelCommandsRequest()
        for _, server in store["server_ctxts"].items():
            jsonInput = {"connections" : []}
            for serverIPPort in server.GetServers():
                jsonInput["connections"].append({"ServerIPPort" : serverIPPort, "proto" : "tcp"})
            outfile = api.GetTestDataDirectory() + "/" + server.node_name + "_fuz.json"
            with open(outfile, 'w') as fp:
                json.dump(jsonInput, fp)
            api.CopyToHost(server.node_name, [outfile], "")
            serverCmd = fuz_init.FUZ_EXEC[server.workload_name]  +  " --jsonInput " + os.path.basename(outfile) + " --jsonOut"
            api.Trigger_AddHostCommand(serverReq, server.node_name,
                                   serverCmd, background = True)

            for intf in server.interfaces:
                server_pcap_info.append((server.node_name, intf, intf + ".pcap"))

        #Start server
        store["combined_server_resp"] = api.Trigger(serverReq)
        #Start Pcap just before starting client
        store["server_pcap_info"] = server_pcap_info


    #Sleep for some time as bg may not have been started.
    time.sleep(30)

    return api.types.status.SUCCESS

def Verify(tc):

    store = tc.GetBundleStore()
    if store.get("combined_server_resp", None):
        for idx, cmd in enumerate(store["combined_server_resp"].commands):
            if cmd.exit_code != 0:
                api.Logger.error("Error starting fuz")
                return api.types.status.FAILURE

    if store.get("server_resp", None):
        for idx, cmd in enumerate(store["server_resp"].commands):
            if cmd.exit_code != 0:
                api.Logger.error("Error starting fuz")
                return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS
