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
    tc.resp_flow = getattr(tc.args, "resp_flow", 0)
    tc.cmd_cookies = {}
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    if pairs[0][0].IsNaples():
        tc.client,tc.server = pairs[0]
    else:
        tc.server,tc.client = pairs[0]
    cmd_cookie = start_nc_server(tc.server, "1237")
    add_command(req, tc, 'server', tc.server, cmd_cookie, True) 


    cmd_cookie = start_nc_client(tc.server, "52255", "1237")
    add_command(req, tc, 'client', tc.client, cmd_cookie, True)

       
    cmd_cookie = "/nic/bin/halctl show session --dstport 1237 --dstip {} --yaml".format(tc.server.ip_address)
    add_command(req, tc, 'show before', tc.client, cmd_cookie, naples=True)

    
    tc.setup_cmd_resp = api.Trigger(req)
    cmd = tc.setup_cmd_resp.commands[-1] 
    api.PrintCommandResults(cmd)
    tc.pre_ctrckinf = get_conntrackinfo(cmd)
    if getattr(tc.args, 'vmotion_enable', False):
        vmotion_utils.PrepareWorkloadVMotion(tc, [tc.server])

    return api.types.status.SUCCESS

def Trigger(tc):
    api.Logger.info("Trigger.")

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    if tc.resp_flow:
        new_seq_num = tc.pre_ctrckinf.r_tcpseqnum + tc.pre_ctrckinf.i_tcpwinsz * (2 ** tc.pre_ctrckinf.i_tcpwinscale)
        req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
        #left out of window - retransmit
        cmd_cookie = "hping3 -c 1 -s 1237 -p 52255 -M {}  -L {} --ack --fin --tcp-timestamp {} ".format(wrap_around(tc.pre_ctrckinf.r_tcpseqnum, 0), tc.pre_ctrckinf.r_tcpacknum, tc.client.ip_address)    
        add_command(req, tc, 'fail ping', tc.server, cmd_cookie)
    else:
        new_seq_num = tc.pre_ctrckinf.i_tcpseqnum + tc.pre_ctrckinf.r_tcpwinsz * (2 ** tc.pre_ctrckinf.r_tcpwinscale)
        #left out of window - retransmit
        cmd_cookie = "hping3 -c 1 -s 52255 -p 1237 -M {}  -L {} --ack --tcp-timestamp {} --fin".format(wrap_around(tc.pre_ctrckinf.i_tcpseqnum, 0), tc.pre_ctrckinf.i_tcpacknum, tc.server.ip_address)   
        add_command(req, tc,"fail ping", tc.client, cmd_cookie)

    cmd_cookie = "sleep 3 && /nic/bin/halctl show session --dstport 1237 --dstip {} --yaml".format(tc.server.ip_address)
    add_command(req, tc, 'show after', tc.client, cmd_cookie, naples=True)

    
    cmd_cookie = "/nic/bin/halctl clear session"
    add_command(req, tc, 'clear', tc.client, cmd_cookie, naples=True)
    if tc.server.IsNaples():
        cmd_cookie = "/nic/bin/halctl clear session"
        add_command(req, tc, 'clear', tc.server, cmd_cookie, naples=True)

    trig_resp = api.Trigger(req)
    term_resp1 = api.Trigger_TerminateAllCommands(tc.setup_cmd_resp)
    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp1)
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
                #until we can manipulated close timeout to smaller value
                return api.types.status.SUCCESS
            print(cmd.stdout)
            yaml_out = get_yaml(cmd)
            if tc.resp_flow:
                flow = get_respflow(yaml_out)
            else:   
                flow = get_initflow(yaml_out)   
            state = get_tcpstate(flow)
            print(state)
            if (int(state) != TcpState.FIN_RCVD) and (int(state) != TcpState.BIDIR_FIN_RCVD):
                return api.types.status.FAILURE
            else:
                return api.types.status.SUCCESS
    #print(tc.resp)
    return api.types.status.SUCCESS

def Teardown(tc):
    api.Logger.info("Teardown.")
    return api.types.status.SUCCESS
