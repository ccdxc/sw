#! /usr/bin/python3
import time
import re
import os
import subprocess
import iota.harness.api as api
import iota.test.iris.config.netagent.api as netagent_api
import iota.test.iris.utils.iperf as iperf
import iota.test.apollo.config.api as config_api
import iota.harness.infra.utils.utils as utils

def Setup(tc):

    tc.skip = False

    # TODO Get programmatically or from Global options
    naples_node     = 'node1' 
    trex_node  = "node2"
    # TODO get location from GlobalOptions
    trex_bindir = "/opt/trex/v2.53"
    trex_libdir = "/root/trex_iota"
    local_file_path = "test/apollo/config/trex_iota/"

    naples_console = utils.NaplesConsoleManagement( \
        api.GetNicConsoleIP(naples_node), \
        api.GetNicConsolePort(naples_node))
    naples_console.Connect()

    # TODO what if agent is started by default
    # output = naples_console.RunCommandOnConsoleWithOutput('start-agent.sh &')
    output = naples_console.RunCommandOnConsoleWithOutput('grep "Finished activate config stage" /var/log/pensando/pds-agent.log')
    api.Logger.info(output)
    if not re.search('[0-9]+\-[0-9]+\-[0-9]+',str(output)):
        output = naples_console.RunCommandOnConsoleWithOutput('time testapp -i %s'%tc.args.cfg_json)
        api.Logger.info(output)

        if re.search('fail', str(output)):
            tc.skip = True
            return api.types.status.FAILURE
    serverReq1 = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    api.Trigger_AddHostCommand(serverReq1, trex_node, 'rm -Rf %s'%trex_libdir)
    api.Trigger_AddHostCommand(serverReq1, trex_node, 'mkdir -p %s'%trex_libdir)
    server_resp = api.Trigger(serverReq1)

    for lfile in ['apollo_sanity.yml','exp_lib.py','traffic_lib.py','yamlordereddictloader.py']:
        cmd="sshpass -p %s scp %s/%s %s@%s:%s"%(api.GetTestbedPassword(),local_file_path,lfile,api.GetTestbedUsername(),api.GetMgmtIPAddress(trex_node),trex_libdir)
        #api.Logger.info(cmd)
        os.system(cmd)

    serverReq = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    api.Trigger_AddHostCommand(serverReq, trex_node, 'ln -s %s trex_bin'%trex_bindir)
    api.Trigger_AddHostCommand(serverReq, trex_node, 'ln -s %s trex_lib'%trex_libdir)
    server_resp = api.Trigger(serverReq)
    return api.types.status.SUCCESS

def Trigger(tc):
    if tc.skip: return api.types.status.SUCCESS

    tc.serverCmds = []
    tc.cmd_descr = []
    trex_node  = "node2"
    # TODO get cores from testbed definition
    cmd = "./t-rex-64 -i -c 6"
    trex_bindir = "trex_bin"
    trex_libdir = "trex_lib"

    # Start trex gen
    serverReq = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    api.Trigger_AddHostCommand(serverReq, trex_node, cmd, background = True, rundir = trex_bindir)
    server_resp = api.Trigger(serverReq)
    api.Logger.info(server_resp)

    # Run the test
    time.sleep(10)

    # If stateful, run at a lower rate first
    if re.search('stateful', tc.args.cfg_json):
        serverReq2 = api.Trigger_CreateExecuteCommandsRequest(serial = True)
        test_params = tc.args.cps_params.split(",")
        cmdparams = " ".join(test_params)
        cmd = "python traffic_lib.py apollo_sanity.yml %s"%cmdparams
        api.Trigger_AddHostCommand(serverReq2, trex_node, cmd, rundir = trex_libdir, timeout = int(test_params[-2])+10)
        server_resp2 = api.Trigger(serverReq2)
        api.Logger.info(server_resp2)


    serverReq1 = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    test_params = tc.args.test_params.split(",")
    cmdparams = " ".join(test_params)
    cmd = "python traffic_lib.py apollo_sanity.yml %s"%cmdparams
    api.Trigger_AddHostCommand(serverReq1, trex_node, cmd, rundir = trex_libdir, timeout = int(test_params[-2])+10)
    server_resp1 = api.Trigger(serverReq1)
    api.Logger.info(server_resp1)
        

    # Terminate Trexgen
    api.Trigger_TerminateAllCommands(server_resp)
    scapy_server_pid = getScapyServerPid(tc)

    # TODO scapy server doesnt get killed with TerminateAllCommands
    if scapy_server_pid:
        serverReq = api.Trigger_CreateExecuteCommandsRequest(serial = True)
        api.Trigger_AddHostCommand(serverReq, trex_node, 'kill -9 %s'%scapy_server_pid)
        server_resp = api.Trigger(serverReq)
        api.Logger.info(server_resp)
    time.sleep(10)

    if re.search('pass', str(server_resp1)):
        return api.types.status.SUCCESS
    else:
        return api.types.status.FAILURE


def getScapyServerPid(tc):

    trex_node  = "node2"
    serverReq = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    api.Trigger_AddHostCommand(serverReq, trex_node, 'ps aef | grep -i scapy')
    server_resp = api.Trigger(serverReq)
    search_rslt = re.search('([0-9]+)',str(server_resp))
    if search_rslt:
        return search_rslt.group(1)
    else:
        return None
    

def Verify(tc):

    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS
