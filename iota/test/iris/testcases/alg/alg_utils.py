#! /usr/bin/python3
import iota.harness.api as api

def GetThreeWorkloads():
    triplet = []
    for w1 in api.GetWorkloads():
        for w2 in api.GetWorkloads():
            for w3 in api.GetWorkloads():
                if id(w1) == id(w2) or \
                   id(w2) == id(w3) or id(w1) == id(w3): continue
                if w1.uplink_vlan != w2.uplink_vlan or \
                   w2.uplink_vlan != w3.uplink_vlan or \
                   w1.uplink_vlan != w3.uplink_vlan: continue
                triplet.append((w1, w2, w3))
    return triplet

def ForceReleasePort(port, node):
   req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
   api.Trigger_AddCommand(req, node.node_name, node.workload_name, "fuser -k %s"%(port))
   trig_resp = api.Trigger(req)
   term_resp = api.Trigger_TerminateAllCommands(trig_resp)
   resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp) 
   for cmd in trig_resp.commands:
        api.PrintCommandResults(cmd)

   return api.types.status.SUCCESS
