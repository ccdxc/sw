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
    attempts = int(getattr(tc.args, "attempts", 3))
    sessions = int(getattr(tc.args, "num_sessions", 1))
    session_time = str(getattr(tc.args, "session_time", "10")) + "s"
    static_arp = int(getattr(tc.args, "static_arp", False))

    if static_arp:
        #install arp entries before running
        arpReqs = store["arp_ctx"]
        api.Trigger(arpReqs)

    #Spread CPS evenly
    clientReq = api.Trigger_CreateAllParallelCommandsRequest()

    cps_per_node = int(cps / len(store["client_ctxts"]))
    for index, (_, client) in enumerate(store["client_ctxts"].items()):
        jsonInput = {"connections" : []}
        for serverIPPort in client.GetServers():
            jsonInput["connections"].append({"ServerIPPort" : serverIPPort, "proto" : "tcp"})
        outfile = api.GetTestDataDirectory() + "/" + client.workload_name + "_fuz.json"
        with open(outfile, 'w') as fp:
            json.dump(jsonInput, fp)
        api.CopyToWorkload(client.node_name, client.workload_name, [outfile], "")
        clientCmd = fuz_init.FUZ_EXEC[client.workload_name]  + " -attempts " + str(attempts) + " -duration " + session_time + " -conns " + str(sessions) + " -cps " + str(cps_per_node) + " -talk  --jsonOut --jsonInput " + os.path.basename(outfile)
        api.Trigger_AddCommand(clientReq, client.node_name, client.workload_name,
                               clientCmd,  background = True)


    #Initiate connections
    store["client_req"] = api.Trigger(clientReq)

    return api.types.status.SUCCESS


def Verify(tc):

    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS
