#! /usr/bin/python3
import os
import time
import iota.harness.api as api

def Setup(tc):
    return api.types.status.SUCCESS

def SetupFTPServer(node, workload):
    dir_path = os.path.dirname(os.path.realpath(__file__))
    ftpdata = dir_path + '/' + "ftp_server.txt"
    api.Logger.info("fullpath %s" % (ftpdata))
    resp = api.CopyToWorkload(node, workload, [ftpdata], 'ftpdir')
    if resp is None:
       return None

    vsftpd_conf = dir_path + '/' + "vsftpd.conf"
    resp = api.CopyToWorkload(node, workload, [vsftpd_conf], 'ftpdir')
    if resp is None:
       return None
    
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    
    api.Trigger_AddCommand(req, node, workload,
                           "cp ftpdir/vsftpd.conf /etc/vsftpd/")
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
                           "mkdir /home/admin/ftp && mv ftpdir/ftp_server.txt /home/admin/ftp")
    api.Trigger_AddCommand(req, node, workload,
                           "touch /home/admin/ftp/ftp_client.txt && chmod 666 /home/admin/ftp/ftp_client.txt")
    api.Trigger_AddCommand(req, node, workload,
                           "systemctl start vsftpd")
    api.Trigger_AddCommand(req, node, workload,
                           "systemctl enable vsftpd")
    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    for cmd in trig_resp.commands:
        api.PrintCommandResults(cmd)
    return api.types.status.SUCCESS

def SetupFTPClient(node, workload, server, mode):
    dir_path = os.path.dirname(os.path.realpath(__file__))
    ftpdata = dir_path + '/' + "ftp_client.txt"
    api.Logger.info("fullpath %s" % (ftpdata))
    resp = api.CopyToWorkload(node, workload, [ftpdata], 'ftpdir')
    if resp is None:
       return None

    ftpcommand = dir_path + '/' + "ftp.sh" 
    f = open(ftpcommand, "w")
    f.write("#!/bin/bash\n")
    f.write("ftp -inv %s <<EOF\n"%(server.ip_address))
    f.write("user admin linuxpassword\n")
    f.write("cd /home/admin/ftp\n")
    f.write("pwd\n")
    f.write("dir\n")
    if (mode == 'active'):
        f.write("pass\n")
    f.write("mget ftp_server.txt\n")
    f.write("mput ftp_client.txt\n")
    f.close()

    resp = api.CopyToWorkload(node, workload, [ftpcommand], 'ftpdir')
    if resp is None:
       return None

    return api.types.status.SUCCESS 

def Cleanup(server, client):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    api.Trigger_AddCommand(req, server.node_name, server.workload_name,
                           "rm -rf ftp_*")
    api.Trigger_AddCommand(req, client.node_name, client.workload_name,
                           "rm -rf ftp_*")
    ftpfile = os.path.dirname(os.path.realpath(__file__)) + '/' + "ftp.sh"
    os.remove(ftpfile)
    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    return api.types.status.SUCCESS

def Trigger(tc):
    pairs = api.GetLocalWorkloadPairs()
    w1 = pairs[0][0]
    w2 = pairs[0][1]
    tc.cmd_cookies = []

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    tc.cmd_descr = "Server: %s(%s) <--> Client: %s(%s)" %\
                   (w1.workload_name, w1.ip_address, w2.workload_name, w2.ip_address)
    api.Logger.info("Starting FTP test from %s" % (tc.cmd_descr))

    SetupFTPServer(w1.node_name, w1.workload_name)
    SetupFTPClient(w2.node_name, w2.workload_name, w1, tc.iterators.mode)

    api.Trigger_AddCommand(req, w2.node_name, w2.workload_name,
                           "cat ftpdir/ftp_server.txt | grep \"I am FTP server\"")
    tc.cmd_cookies.append("Before get on client")

    api.Trigger_AddCommand(req, w1.node_name, w1.workload_name,
                           "cat /home/admin/ftp/ftp_client.txt | grep \"I am FTP client\"")
    tc.cmd_cookies.append("Before put on server")

    api.Trigger_AddCommand(req, w2.node_name, w2.workload_name,
                           "cd ftpdir && chmod +x ftp.sh && ./ftp.sh")
    tc.cmd_cookies.append("Run FTP")

    ## Add Naples command validation
 
    api.Trigger_AddCommand(req, w2.node_name, w2.workload_name,
                           "cat ftpdir/ftp_server.txt | grep \"I am FTP server\"")
    tc.cmd_cookies.append("After get on client")
    
    api.Trigger_AddCommand(req, w1.node_name, w1.workload_name,
                           "cat /home/admin/ftp/ftp_client.txt | grep \"I am FTP client\"")
    tc.cmd_cookies.append("After put on server")

    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)

    Cleanup(w1, w2)
    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS
    api.Logger.info("Results for %s" % (tc.cmd_descr))
    cookie_idx = 0
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            if tc.cmd_cookies[cookie_idx].find("Before") != -1:
                result = api.types.status.SUCCESS
            else:
                result = api.types.status.FAILURE
        cookie_idx += 1

    return result

def Teardown(tc):
    return api.types.status.SUCCESS
