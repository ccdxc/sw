#! /usr/bin/python3
import os
import iota.harness.api as api

def SetupNFSServer(server, client):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    node = server.node_name
    workload = server.workload_name
    api.Trigger_AddCommand(req, node, workload, "sudo chkconfig nfs on")
    api.Trigger_AddCommand(req, node, workload, "sudo service rpcbind start")
    api.Trigger_AddCommand(req, node, workload, "sudo service nfs start")
    api.Trigger_AddCommand(req, node, workload, "mkdir /home/sunrpcmntdir")
    api.Trigger_AddCommand(req, node, workload,
        "sudo echo \'/home/sunrpcmntdir %s(rw,sync,no_subtree_check,no_root_squash)\' | sudo tee -a /etc/exports"%(client.ip_address))
    api.Trigger_AddCommand(req, node, workload, "sudo exportfs -ra")
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
    api.Trigger_AddCommand(req, node, workload, "sudo chkconfig nfs off")
    api.Trigger_AddCommand(req, node, workload, "sudo service rpcbind stop")
    api.Trigger_AddCommand(req, node, workload, "sudo service nfs stop")
    api.Trigger_AddCommand(req, node, workload, "rm -rf /home/sunrpcmntdir")
    api.Trigger_AddCommand(req, node, workload, "sudo rm -f /etc/exports")
    api.Trigger_AddCommand(req, node, workload,
        "sudo touch /etc/exports")
    api.Trigger_AddCommand(req, node, workload, "sudo exportfs -ra")

    api.Trigger_AddCommand(req, client.node_name, client.workload_name, "rm -rf sunrpcmntdir") 
    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    for cmd in trig_resp.commands:
        api.PrintCommandResults(cmd)
    return api.types.status.SUCCESS
