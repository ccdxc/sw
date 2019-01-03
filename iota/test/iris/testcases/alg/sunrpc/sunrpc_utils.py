#! /usr/bin/python3
import os
import iota.harness.api as api

def SetupNFSServer(server, client):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    node = server.node_name
    workload = server.workload_name
    api.Trigger_AddCommand(req, node, workload, "chkconfig nfs on")
    api.Trigger_AddCommand(req, node, workload, "service rpcbind start")
    api.Trigger_AddCommand(req, node, workload, "service nfs start")
    api.Trigger_AddCommand(req, node, workload, "mkdir /home/sunrpcmntdir")
    api.Trigger_AddCommand(req, node, workload,
        "echo \"/home/sunrpcmntdir %s(rw,sync,no_subtree_check,no_root_squash)\" | tee -a /etc/exports"%(client.ip_address))
    api.Trigger_AddCommand(req, node, workload, "exportfs -ra")
    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    for cmd in trig_resp.commands:
        api.PrintCommandResults(cmd)
    return api.types.status.SUCCESS

def CleanupNFSServer(server, client):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    node = server.node_name
    workload = server.workload_name
    api.Trigger_AddCommand(req, client.node_name, client.workload_name, "umount sunrpcmntdir")
    api.Trigger_AddCommand(req, node, workload, "chkconfig nfs off")
    api.Trigger_AddCommand(req, node, workload, "service rpcbind stop")
    api.Trigger_AddCommand(req, node, workload, "service nfs stop")
    api.Trigger_AddCommand(req, node, workload, "rm -rf /home/sunrpcmntdir")
    api.Trigger_AddCommand(req, node, workload, "rm -f /etc/exports")
    api.Trigger_AddCommand(req, node, workload,
        "echo \" \" | tee -a /etc/exports")
    api.Trigger_AddCommand(req, node, workload, "exportfs -ra")

    api.Trigger_AddCommand(req, client.node_name, client.workload_name, "rm -rf sunrpcmntdir") 
    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    for cmd in trig_resp.commands:
        api.PrintCommandResults(cmd)
    return api.types.status.SUCCESS
