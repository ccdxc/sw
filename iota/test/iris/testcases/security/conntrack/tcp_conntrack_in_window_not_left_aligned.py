#! /usr/bin/python3

import iota.harness.api as api
from iota.test.iris.testcases.security.conntrack.session_info import *
from iota.test.iris.testcases.security.conntrack.conntrack_utils import *
from iota.test.iris.utils import vmotion_utils

def Setup(tc):
    api.Logger.info("Setup.")
    if tc.iterators.kind == "remote":
        pairs = api.GetRemoteWorkloadPairs()
        if not pairs:
            api.Logger.info("no remtote eps")
            return api.types.status.SUCCESS
    else:
        pairs = api.GetLocalWorkloadPairs()
    tc.cmd_cookies = {}
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    #for w1,w2 in pairs:
    if pairs[0][0].IsNaples():
        tc.client,tc.server = pairs[0]
    else:
        tc.server,tc.client = pairs[0]
    cmd_cookie = "nc -l 1234"
    api.Trigger_AddCommand(req, tc.server.node_name, tc.server.workload_name, cmd_cookie, background=True)
    tc.cmd_cookies['server'] = cmd_cookie

    cmd_cookie = "nc {} 1234 -p 52252".format(tc.server.ip_address)
    api.Trigger_AddCommand(req, tc.client.node_name, tc.client.workload_name,cmd_cookie, background=True)
    tc.cmd_cookies['client'] = cmd_cookie
       
    cmd_cookie = "/nic/bin/halctl show session --dstport 1234 --dstip {} --yaml".format(tc.server.ip_address)
    api.Trigger_AddNaplesCommand(req, tc.client.node_name, cmd_cookie)
    tc.setup_cmd_resp = api.Trigger(req)

    cmd = tc.setup_cmd_resp.commands[-1] 
    api.PrintCommandResults(cmd)
    tc.pre_ctrckinf = get_conntrackinfo(cmd)
    if getattr(tc.args, 'vmotion_enable', False):
        vmotion_utils.PrepareWorkloadVMotion(tc, [tc.server])

    return api.types.status.SUCCESS

def Trigger(tc):
    api.Logger.info("Trigger.")
    new_seq_num = tc.pre_ctrckinf.i_tcpseqnum + tc.pre_ctrckinf.r_tcpwinsz * (2 ** tc.pre_ctrckinf.r_tcpwinscale)

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    #within the window - outoforder
    cmd_cookie = "sleep 2 && hping3 -c 1 -s 52252 -p 1234 -M {}  -L {} --ack --tcp-timestamp {} -d 10".format(tc.pre_ctrckinf.i_tcpseqnum + 100, tc.pre_ctrckinf.i_tcpacknum, tc.server.ip_address)    
    api.Trigger_AddCommand(req, tc.client.node_name, tc.client.workload_name, cmd_cookie)
    tc.cmd_cookies['fail ping'] = cmd_cookie

    cmd_cookie = "sleep 3 && /nic/bin/halctl show session --dstport 1234 --dstip {} --yaml".format(tc.server.ip_address)
    api.Trigger_AddNaplesCommand(req, tc.client.node_name, cmd_cookie)
    tc.cmd_cookies['show after'] = cmd_cookie
    
    cmd_cookie = "/nic/bin/halctl clear session"
    add_command(req, tc, 'clear', tc.client, cmd_cookie, naples=True)

    if tc.server.IsNaples():
        cmd_cookie = "/nic/bin/halctl clear session"
        add_command(req, tc, 'clear', tc.server, cmd_cookie, naples=True)

    
    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    term_resp1 = api.Trigger_TerminateAllCommands(tc.setup_cmd_resp)
    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)
    
    if getattr(tc.args, 'vmotion_enable', False):
        vmotion_utils.PrepareWorkloadRestore(tc)

    return api.types.status.SUCCESS    
        
def Verify(tc):
    api.Logger.info("Verify.")
    if tc.resp == None:
        return api.types.status.SUCCESS
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if tc.cmd_cookies['show after'] == cmd.command:    
            if not cmd.stdout:
                print("none stdout")
                return api.types.status.SUCCESS
            print(cmd.stdout)
            
            yaml_out = get_yaml(cmd)
            init_flow = get_initflow(yaml_out)
            conn_info = get_conntrack_info(init_flow)
            excep =  get_exceptions(conn_info)
            if (excep['tcppacketreorder'] == 'false'):
                return api.types.status.FAILURE 
        
    #print(tc.resp)
    return api.types.status.SUCCESS

def Teardown(tc):
    api.Logger.info("Teardown.")
    return api.types.status.SUCCESS
