#! /usr/bin/python3

import iota.harness.api as api
import yaml

def get_conntrackinfo(cmd):
    yaml_out = yaml.load_all(cmd.stdout)
    print(type(yaml_out))
    for data in yaml_out:
        if data is not None:
            spec = data['spec']
            init_flow = spec['initiatorflow']
            iseq_num = init_flow['flowdata']['conntrackinfo']['tcpseqnum']
            iack_num = init_flow['flowdata']['conntrackinfo']['tcpacknum']
            iwindosz = init_flow['flowdata']['conntrackinfo']['tcpwinsz']
            iwinscale = init_flow['flowdata']['conntrackinfo']['tcpwinscale']
            print('init flow: seq_num {} ack_num {} iwindosz {} iwinscale {}'.format(iseq_num, iack_num, iwindosz, iwinscale))
            
            resp_flow = spec['responderflow']
            rseq_num = resp_flow['flowdata']['conntrackinfo']['tcpseqnum']
            rack_num = resp_flow['flowdata']['conntrackinfo']['tcpacknum']
            rwindosz = resp_flow['flowdata']['conntrackinfo']['tcpwinsz']
            rwinscale = resp_flow['flowdata']['conntrackinfo']['tcpwinscale']
            print('resp flow: seq_num {} ack_num {} iwindosz {} iwinscale {}'.format(rseq_num, rack_num, rwindosz, rwinscale))
            return iseq_num, iack_num, iwindosz, iwinscale, rseq_num, rack_num, rwindosz, rwinscale

def get_yaml(cmd):
    yaml_out = yaml.load_all(cmd.stdout)
    for data in yaml_out:
        if data is not None:
            return data

def get_respflow(data):
    return data['spec']['responderflow']

def get_initflow(data):
    return data['spec']['initiatorflow']

def get_conntrack_info(flow):
    return flow['flowdata']['conntrackinfo']
def get_exceptions(conn_info):
    return conn_info['exceptions']

def Setup(tc):
    api.Logger.info("Setup.")
    return api.types.status.SUCCESS

def Trigger(tc):
    api.Logger.info("Trigger.")
    pairs = api.GetLocalWorkloadPairs()
    tc.cmd_cookies = {}
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    #for w1,w2 in pairs:
    server,client  = pairs[0]
    cmd_cookie = "nc -l 1234"
    api.Trigger_AddCommand(req, server.node_name, server.workload_name, cmd_cookie, background=True)
    tc.cmd_cookies['server'] = cmd_cookie

    cmd_cookie = "nc {} 1234 -p 52252".format(server.ip_address)
    api.Trigger_AddCommand(req, client.node_name, client.workload_name,cmd_cookie, background=True)
    tc.cmd_cookies['client'] = cmd_cookie
       
    cmd_cookie = "/nic/bin/halctl show session --dstport 1234 --dstip {} --yaml".format(server.ip_address)
    api.Trigger_AddNaplesCommand(req, client.node_name, cmd_cookie)
    trig_resp1 = api.Trigger(req)
    cmd = trig_resp1.commands[-1] 
    print(trig_resp1)
    print(cmd)
    iseq_num, iack_num, iwindosz, iwinscale, rseq_num, rack_num, rwindo_sz, rwinscale = get_conntrackinfo(cmd)
    new_seq_num = iseq_num + rwindo_sz * (2 ** rwinscale)

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    #left out of window - retransmit
    cmd_cookie = "hping3 -c 1 -s 52252 -p 1234 -M {}  -L {} --ack --tcp-timestamp {} -d 10".format(iseq_num - 200, iack_num, server.ip_address)    
    api.Trigger_AddCommand(req, client.node_name, client.workload_name, cmd_cookie)
    tc.cmd_cookies['fail ping'] = cmd_cookie

    cmd_cookie = "sleep 3 && /nic/bin/halctl show session --dstport 1234 --dstip {} --yaml".format(server.ip_address)
    api.Trigger_AddNaplesCommand(req, client.node_name, cmd_cookie)
    tc.cmd_cookies['show after'] = cmd_cookie
    
    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    term_resp1 = api.Trigger_TerminateAllCommands(trig_resp1)
    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)
    return api.types.status.SUCCESS    
        
def Verify(tc):
    api.Logger.info("Verify.")
    for cmd in tc.resp.commands:
        #api.PrintCommandResults(cmd)
        with open("out_cmd", "w+") as f:
            f.write(cmd.stdout)
    
        if tc.cmd_cookies['show after'] == cmd.command:     
            print(cmd.stdout)
            yaml_out = get_yaml(cmd)
            init_flow = get_initflow(yaml_out)
            conn_info = get_conntrack_info(init_flow)
            excep =  get_exceptions(conn_info)
            if (excep['tcpfullretransmit'] == 'false'):
                return api.types.status.FAILURE 
        
    #print(tc.resp)
    return api.types.status.SUCCESS

def Teardown(tc):
    api.Logger.info("Teardown.")
    return api.types.status.SUCCESS
