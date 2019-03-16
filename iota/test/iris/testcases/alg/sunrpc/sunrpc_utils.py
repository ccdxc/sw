#! /usr/bin/python3
import os
import iota.harness.api as api

dir_path = os.path.dirname(os.path.realpath(__file__))

def GetTcpdumpData(node):
    resp = api.CopyFromWorkload(node.node_name, node.workload_name, ['rpc.pcap'], dir_path)
    if resp is None:
       return None

def SetupNFSServer(server, client, nfs=True):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    node = server.node_name
    workload = server.workload_name
    api.Trigger_AddCommand(req, node, workload, "sudo chkconfig nfs on")
    api.Trigger_AddCommand(req, node, workload, "sudo service rpcbind start")
    api.Trigger_AddCommand(req, node, workload, "sudo service nfs start")
    if (nfs == True): 
        api.Trigger_AddCommand(req, client.node_name, client.workload_name,
                           "rpcinfo -s %s"%(server.ip_address))
        api.Trigger_AddCommand(req, node, workload, "mkdir /home/sunrpcmntdir")
        api.Trigger_AddCommand(req, node, workload, "sudo chmod 777 /home/sunrpcmntdir")
        api.Trigger_AddCommand(req, node, workload,
             "sudo echo \'/home/sunrpcmntdir %s(rw,sync,no_subtree_check,no_root_squash)\' | sudo tee -a /etc/exports"%(client.ip_address))
        api.Trigger_AddCommand(req, node, workload, "sudo exportfs -ra")
    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    for cmd in trig_resp.commands:
        api.PrintCommandResults(cmd)
    rpcfile = dir_path + '/' + "sunrpc_file.txt"
    f = open(rpcfile, "w")
    f.write("Hello World!!!\n")
    f.close()

    resp = api.CopyToWorkload(client.node_name, client.workload_name, [rpcfile], 'sunrpcdir')
    os.remove(rpcfile)
    return api.types.status.SUCCESS

def CleanupNFSServer(server, client):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    node = server.node_name
    workload = server.workload_name
    api.Trigger_AddCommand(req, client.node_name, client.workload_name, "umount /home/sunrpcdir")
    api.Trigger_AddCommand(req, node, workload, "sudo chkconfig nfs off")
    api.Trigger_AddCommand(req, node, workload, "sudo service rpcbind stop")
    api.Trigger_AddCommand(req, node, workload, "sudo service nfs stop")
    api.Trigger_AddCommand(req, node, workload, "rm -rf /home/sunrpcmntdir")
    api.Trigger_AddCommand(req, node, workload, "sudo rm -f /etc/exports")
    api.Trigger_AddCommand(req, node, workload,
        "sudo touch /etc/exports")
    api.Trigger_AddCommand(req, node, workload, "sudo exportfs -ra")

    api.Trigger_AddCommand(req, client.node_name, client.workload_name, "rm -rf /home/sunrpcdir") 
    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    for cmd in trig_resp.commands:
        api.PrintCommandResults(cmd)
    return api.types.status.SUCCESS
