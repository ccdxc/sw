#! /usr/bin/python3
import os
import iota.harness.api as api

TFTP_NONSTD_PORT = 2069

dir_path = os.path.dirname(os.path.realpath(__file__))

def SetupTFTPServer(server, non_standard_port=False):
    node = server.node_name
    workload = server.workload_name
    fullpath = dir_path + '/' + "tftp_server.txt"
    api.Logger.info("fullpath %s" % (fullpath))
    resp = api.CopyToWorkload(node, workload, [fullpath], 'tftpdir')
    if resp is None:
       return None

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    api.Trigger_AddCommand(req, node, workload,
                           "cp tftpdir/tftp_server.txt /var/lib/tftpboot")
    api.Trigger_AddCommand(req, node, workload,
                         "touch /var/lib/tftpboot/tftp_client.txt")
    api.Trigger_AddCommand(req, node, workload,
                           "chmod 666 /var/lib/tftpboot/tftp_client.txt")
    if non_standard_port == True:
        api.Trigger_AddCommand(req, node, workload,
                           "systemctl disable tftp")
        api.Trigger_AddCommand(req, node, workload,
                           "systemctl stop tftp")
        api.Trigger_AddCommand(req, node, workload,
                           "/usr/sbin/in.tftpd -s /var/lib/tftpboot -l --address %s:%s"%(server.ip_address, TFTP_NONSTD_PORT))
    else:
        api.Trigger_AddCommand(req, node, workload,
                           "systemctl start tftp")
        api.Trigger_AddCommand(req, node, workload,
                           "systemctl enable tftp")

    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    for cmd in trig_resp.commands:
        api.PrintCommandResults(cmd)
    return api.types.status.SUCCESS

def SetupTFTPClient(client):
    node = client.node_name
    workload = client.workload_name
    fullpath = dir_path + '/' + "tftp_client.txt"
    api.Logger.info("fullpath %s" % (fullpath))
    resp = api.CopyToWorkload(node, workload, [fullpath], 'tftpdir')
    if resp is None:
       return None
    return api.types.status.SUCCESS

def Cleanup(server, client):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    api.Trigger_AddCommand(req, server.node_name, server.workload_name,
                           "rm -rf /var/lib/tftpboot/*")
    api.Trigger_AddCommand(req, server.node_name, server.workload_name,
                           "rm -rf tftpdir")
    api.Trigger_AddCommand(req, client.node_name, client.workload_name,
                           "rm -rf tftpdir")
    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    return api.types.status.SUCCESS

