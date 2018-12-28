#! /usr/bin/python3
import os
import iota.harness.api as api
from iota.test.iris.testcases.alg.alg_utils import *

dir_path = os.path.dirname(os.path.realpath(__file__))
FTP_STANDARD_PORT = 21

def SetupFTPServer(node, workload, promiscous=False, 
                   bind_fail=False, listen_port=FTP_STANDARD_PORT, restart=True):
    ftpdata = dir_path + '/' + "ftp_server.txt"
    api.Logger.info("fullpath %s" % (ftpdata))
    resp = api.CopyToWorkload(node, workload, [ftpdata], 'ftpdir')
    if resp is None:
       return None

    if promiscous == True:
        if bind_fail == True:
            vsftpd_file = "lftp_vsftpd_bind_fail.conf"
            vsftpd_conf = dir_path + '/' + "lftp_vsftpd_bind_fail.conf"
        else:
            vsftpd_file = "lftp_vsftpd.conf"
            vsftpd_conf = dir_path + '/' + "lftp_vsftpd.conf"
    else:
        vsftpd_file = "vsftpd.conf"
        vsftpd_conf = dir_path + '/' + "vsftpd.conf"
    resp = api.CopyToWorkload(node, workload, [vsftpd_conf], 'ftpdir')
    if resp is None:
       return None

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    api.Trigger_AddCommand(req, node, workload,
                           "cp ftpdir/%s /etc/vsftpd/vsftpd.conf"%(vsftpd_file))
    if listen_port != FTP_STANDARD_PORT:
        api.Trigger_AddCommand(req, node, workload,
                               "echo \"listen_port=%d\" | tee -a /etc/vsftpd/vsftpd.conf"%(listen_port))   
    api.Trigger_AddCommand(req, node, workload,
                           "useradd -m -c \"admin\" -s /bin/bash admin")
    api.Trigger_AddCommand(req, node, workload,
                           "useradd -m -c \"admin1\" -s /bin/bash admin1")
    api.Trigger_AddCommand(req, node, workload,
                           "echo \"admin\" | tee -a /etc/vsftpd.userlist")
    api.Trigger_AddCommand(req, node, workload,
                           "echo \"admin1\" | tee -a /etc/vsftpd.userlist")
    api.Trigger_AddCommand(req, node, workload,
                           "echo \"linuxpassword\" | passwd --stdin admin")
    api.Trigger_AddCommand(req, node, workload,
                           "echo \"docker\" | passwd --stdin admin1")
    api.Trigger_AddCommand(req, node, workload,
                           "echo \"docker\" | passwd --stdin admin1")
    api.Trigger_AddCommand(req, node, workload,
                           "mkdir /home/admin/ftp && mv ftpdir/ftp_server.txt /home/admin/ftp")
    api.Trigger_AddCommand(req, node, workload,
                           "touch /home/admin/ftp/ftp_client.txt && chmod 666 /home/admin/ftp/ftp_client.txt")
    if restart == True:
        api.Trigger_AddCommand(req, node, workload,
                           "systemctl stop vsftpd")
 
    api.Trigger_AddCommand(req, node, workload,
                           "systemctl start vsftpd")
    api.Trigger_AddCommand(req, node, workload,
                           "systemctl enable vsftpd")
    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    for cmd in trig_resp.commands:
        api.PrintCommandResults(cmd)
    return api.types.status.SUCCESS

def SetupFTPClient(node, workload):
    ftpdata = dir_path + '/' + "ftp_client.txt"
    api.Logger.info("fullpath %s" % (ftpdata))
    resp = api.CopyToWorkload(node, workload, [ftpdata], 'ftpdir')
    if resp is None:
       return None

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    api.Trigger_AddCommand(req, node, workload, "mkdir /home/admin/ftp")
    api.Trigger_AddCommand(req, node, workload, "cp ftpdir/ftp_client.txt /home/admin/ftp/")

    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    for cmd in trig_resp.commands:
        api.PrintCommandResults(cmd)
    return api.types.status.SUCCESS

def SetupFTPMediator(node, workload):

    ftpcommand = dir_path + '/' + ".lftprc"
    f = open(ftpcommand, "w")
    f.write("set ftp:use-fxp true\n")
    f.close()

    resp = api.CopyToWorkload(node, workload, [ftpcommand], 'ftpdir')
    if resp is None:
       return None

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    api.Trigger_AddCommand(req, node, workload, "cp ftpdir/.lftprc ~/")

    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    for cmd in trig_resp.commands:
        api.PrintCommandResults(cmd)
    return api.types.status.SUCCESS

def Cleanup(server, client):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    api.Trigger_AddCommand(req, server.node_name, server.workload_name,
                           "rm -rf ftp_*")
    api.Trigger_AddCommand(req, client.node_name, client.workload_name,
                           "rm -rf ftp_*")
    api.Trigger_AddCommand(req, server.node_name, server.workload_name,
                           "rm -rf /home/admin/ftp")
    api.Trigger_AddCommand(req, client.node_name, client.workload_name,
                           "rm -rf /home/admin/ftp")
    ftpfile = os.path.dirname(os.path.realpath(__file__)) + '/' + ".lftprc"
    os.remove(ftpfile)
    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    return api.types.status.SUCCESS
