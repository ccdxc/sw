#!/usr/bin/python3

import iota.harness.api as api
import json
import os
import time
from iota.test.athena.config.netagent.hw_push_config import AddWorkloads

FLOWS = [ {'sip' : '10.0.0.1', 'dip' : '10.0.0.65', 'proto' : 'UDP', 
            'src_port' : 100, 'dst_port' : 200},
            {'sip' : '192.168.0.1', 'dip' : '192.168.0.100', 'proto' : 'UDP', 
            'src_port' : 5000, 'dst_port' : 6000},
            {'sip' : '100.0.0.1', 'dip' : '100.0.10.1', 'proto' : 'UDP', 
            'src_port' : 60, 'dst_port' : 80} ]

PROTO_NUM = {'UDP' : 17, 'TCP': 6, 'ICMP': 1}
SNIFF_TIMEOUT = 5
ATHENA_SEC_APP_RSTRT_SLEEP = 140 # secs

class FlowInfo():

    def __init__(self):
        self.smac = None
        self.dmac = None
        self.sip = None
        self.dip = None
        self.proto = None
        self.src_port = None
        self.dst_port = None
        self.icmp_type = None
        self.icmp_code = None 

    # Setters
    def set_smac(self, smac):
        self.smac = smac

    def set_dmac(self, dmac):
        self.dmac = dmac

    def set_sip(self, sip):
        self.sip = sip

    def set_dip(self, dip):
        self.dip = dip

    def set_proto(self, proto):
        self.proto = proto

    def set_src_port(self, src_port):
        self.src_port = src_port

    def set_dst_port(self, dst_port):
        self.dst_port = dst_port

    def set_icmp_type(self, icmp_type):
        self.icmp_type = icmp_type

    def set_icmp_code(self, icmp_code):
        self.icmp_code = icmp_code

    # Getters
    def get_smac(self):
        return self.smac

    def get_dmac(self):
        return self.dmac

    def get_sip(self):
        return self.sip

    def get_dip(self):
        return self.dip

    def get_proto(self):
        return self.proto

    def get_src_port(self):
        return self.src_port

    def get_dst_port(self):
        return self.dst_port

    def get_icmp_type(self):
        return self.icmp_type

    def get_icmp_code(self):
        return self.icmp_code
    
    def display(self):
        return ('{{SIP: {}, DIP: {}, Prot: {}, SrcPort: {}, '
                'DstPort: {}}}'.format(self.sip, self.dip, self.proto,
                self.src_port, self.dst_port))


def get_intf_smac(node, host_intf):

    smac = None
    cmd = 'ifconfig ' + host_intf
    out = node.RunSshCmd(cmd) 

    op = out.splitlines()
    for line in op:
        if line.strip().startswith('ether'):
            smac_str = line.strip()
            smac = smac_str.split()[1]    
            break

    return smac


def Athena_sec_app_restart(tc):
    
    athena_sec_app_pid = None
    req = api.Trigger_CreateExecuteCommandsRequest()
    
    cmd = "mv /policy.json /data/policy.json"
    api.Trigger_AddNaplesCommand(req, tc.bitw_node.Name(), cmd)
    
    cmd = "ps -aef | grep athena_app | grep soft-init | grep -v grep"
    api.Trigger_AddNaplesCommand(req, tc.bitw_node.Name(), cmd)

    resp = api.Trigger(req)
    for cmd in resp.commands:
        api.PrintCommandResults(cmd)
        if "mv" in cmd.command and cmd.exit_code != 0:
            return api.types.status.FAILURE
    
        if "ps" in cmd.command and "athena_app" in cmd.stdout:
            athena_sec_app_pid = cmd.stdout.strip().split()[1]
    
    if athena_sec_app_pid:
        api.Logger.info("Athena sec app already running with pid %s. Kill and "
                "restart." % athena_sec_app_pid)
        
        req = api.Trigger_CreateExecuteCommandsRequest()
        api.Trigger_AddNaplesCommand(req, tc.bitw_node.Name(), "kill -SIGTERM "\
                                    + athena_sec_app_pid)
        resp = api.Trigger(req)
        for cmd in resp.commands:
            api.PrintCommandResults(cmd)
            if cmd.exit_code != 0:
                return api.types.status.FAILURE

    req = api.Trigger_CreateExecuteCommandsRequest()
    cmd = "/nic/tools/start-sec-agent-iota.sh"
    api.Trigger_AddNaplesCommand(req, tc.bitw_node.Name(), cmd, background = True)
    resp = api.Trigger(req)
    for cmd in resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            return api.types.status.FAILURE
    
    # sleep for init to complete
    time.sleep(ATHENA_SEC_APP_RSTRT_SLEEP)

    athena_sec_app_pid = None
    req = api.Trigger_CreateExecuteCommandsRequest()
    cmd = "ps -aef | grep athena_app | grep soft-init | grep -v grep"
    api.Trigger_AddNaplesCommand(req, tc.bitw_node.Name(), cmd)

    resp = api.Trigger(req)
    for cmd in resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            return api.types.status.FAILURE
    
        if "ps" in cmd.command and "athena_app" in cmd.stdout:
            athena_sec_app_pid = cmd.stdout.strip().split()[1]
    
    if athena_sec_app_pid:
        api.Logger.info('Athena sec app restarted and has pid %s' % \
                        athena_sec_app_pid)
    else:
        api.Logger.info('Athena sec app failed to restart')
        return api.types.status.FAILURE

    return api.types.status.SUCCESS


def Setup(tc):

    # set up workloads
    AddWorkloads()
    
    workloads = api.GetWorkloads()
    if len(workloads) == 0:
        api.Logger.error('No workloads available')
        return api.types.status.FAILURE

    # All workloads created will be on same node currently
    tc.wl_node_name = workloads[0].node_name
    
    tc.wl_node = None
    tc.nodes = api.GetNodes() 
    for node in tc.nodes:
        if node.Name() == tc.wl_node_name:
            tc.wl_node = node
            break

    if not tc.wl_node:
        api.Logger.error('Failed to get workload node')
        return api.types.status.FAILURE

        
    host_intfs = api.GetNaplesHostInterfaces(tc.wl_node_name)
    
    # Assuming single nic per host 
    if len(host_intfs) != 2:
        api.Logger.error('Failed to get host interfaces')
        return api.types.status.FAILURE

    tc.up0_intf = host_intfs[0]
    tc.up1_intf = host_intfs[1] 
    tc.up0_vlan, tc.up1_vlan = None, None
    tc.up0_mac, tc.up1_mac = None, None

    # Assuming only two workloads today. One associated 
    # with up0 and vlanA, and other with up1 and vlanB
    for wl in workloads:
        if wl.parent_interface == tc.up0_intf:
            tc.up0_vlan = wl.uplink_vlan
            tc.up0_mac = wl.mac_address  
    
        if wl.parent_interface == tc.up1_intf:
            tc.up1_vlan = wl.uplink_vlan
            tc.up1_mac = wl.mac_address

    if not tc.up0_vlan or not tc.up1_vlan:
        api.Logger.error('Failed to get workload sub-intf VLANs')
        return api.types.status.FAILURE

    if not tc.up0_mac or not tc.up1_mac:
        api.Logger.error('Failed to get workload sub-intf mac addresses')
        return api.types.status.FAILURE

    api.Logger.info('Workload0: up0_intf %s up0_vlan %s up0_mac %s' % (
                    tc.up0_intf, tc.up0_vlan, tc.up0_mac))
    api.Logger.info('Workload1: up1_intf %s up1_vlan %s up1_mac %s' % (
                    tc.up1_intf, tc.up1_vlan, tc.up1_mac))


    # create flows for testing 
    tc.flows = []

    if not FLOWS:
        api.Logger.error('Flow list is empty')
        return api.types.status.FAILURE

    for flow in FLOWS:
        flow_info = FlowInfo()
        flow_info.set_sip(flow['sip'])
        flow_info.set_dip(flow['dip'])
        flow_info.set_proto(flow['proto'])

        if flow['proto'] == 'UDP':
            flow_info.set_src_port(flow['src_port'])
            flow_info.set_dst_port(flow['dst_port'])
        
        else:
            api.Logger.error('flow protocol %s not supported' % flow['proto'])
            return api.types.status.FAILURE

        tc.flows.append(flow_info)
   

    # setup policy.json file
    curr_dir = os.path.dirname(os.path.realpath(__file__))
    plcy_obj = None
    vnic_id = '1'
    
    # read from template file 
    with open(curr_dir + '/config/template_policy.json') as fd:
        plcy_obj = json.load(fd)

    vnics = plcy_obj['vnic']
        
    # Assuming one vnic only
    vnic_id = vnics[0]['vnic_id']

    vnics[0]['vlan_id'] = str(tc.up1_vlan)
    vnics[0]['session']['to_switch']['host_mac'] = str(tc.up1_mac)
    vnics[0]['rewrite_underlay']['vlan_id'] = str(tc.up0_vlan)
    vnics[0]['rewrite_underlay']['dmac'] = str(tc.up0_mac)
    vnics[0]['rewrite_host']['dmac'] = str(tc.up1_mac)

    for flow in tc.flows:
        v4_flow = {'vnic_lo': vnic_id, 'vnic_hi': vnic_id, 'sip_lo': flow.get_sip(), 
                    'sip_hi': flow.get_sip(), 'dip_lo': flow.get_dip(), 
                    'dip_hi': flow.get_dip(), 'proto':
                    str(PROTO_NUM[flow.get_proto()]), 'sport_lo':
                    str(flow.get_src_port()), 'sport_hi':
                    str(flow.get_src_port()), 'dport_lo':
                    str(flow.get_dst_port()), 'dport_hi':
                    str(flow.get_dst_port())}
    
        plcy_obj['v4_flows'].append(v4_flow)

    # write vlan/mac addr and flow info to actual file 
    with open(curr_dir + '/config/policy.json', 'w+') as fd:
        json.dump(plcy_obj, fd, indent=4)


    # copy policy.json file and send/recv scripts to node
    tc.nodes = api.GetNodes()
    tc.bitw_node = None

    for node in tc.nodes:
        if node.IsNaplesHwWithBumpInTheWire():
            policy_json_fname = curr_dir + '/config/policy.json'
            api.CopyToNaples(node.Name(), [policy_json_fname], "") 
            tc.bitw_node = node
            continue
         

        if node is tc.wl_node:
            send_pkt_script_fname = curr_dir + '/scripts/send_pkt.py' 
            recv_pkt_script_fname = curr_dir + '/scripts/recv_pkt.py' 
            policy_json_fname = curr_dir + '/config/policy.json'

            api.CopyToHost(node.Name(), [send_pkt_script_fname], "")
            api.CopyToHost(node.Name(), [recv_pkt_script_fname], "")
            api.CopyToHost(node.Name(), [policy_json_fname], "")

    return api.types.status.SUCCESS


def Trigger(tc):
    
    # Copy policy.json to /data and restart Athena sec app on Athena Node
    ret = Athena_sec_app_restart(tc) 
    if ret != api.types.status.SUCCESS:
        return (ret)

    for node in tc.nodes:
        if node is not tc.wl_node:
            continue
    
        # convention: regular pkts sent on up1 and encap pkts sent on up0
        for flow in tc.flows:
        
            # send pkts and validate in h2s dir
            req = api.Trigger_CreateExecuteCommandsRequest(serial=False)
            
            recv_cmd = "./recv_pkt.py --intf_name %s --intf_vlan %s \
                        --intf_mac %s --skip_vlan_cfg --timeout %s \
                        --proto %s --src_ip %s --dst_ip %s --src_port %s \
                        --dst_port %s --dir h2s" % (tc.up0_intf, tc.up0_vlan, 
                        tc.up0_mac, str(SNIFF_TIMEOUT), flow.proto, flow.sip, 
                        flow.dip, str(flow.src_port), str(flow.dst_port))

            api.Trigger_AddHostCommand(req, node.Name(), recv_cmd,
                                                background=True)

            send_cmd = './send_pkt.py --intf_name %s --intf_vlan %s \
                        --intf_mac %s --skip_vlan_cfg --proto %s --src_ip %s \
                        --dst_ip %s --src_port %s --dst_port %s --dir h2s' % (
                        tc.up1_intf, tc.up1_vlan, tc.up1_mac, flow.proto, 
                        flow.sip, flow.dip, flow.src_port, flow.dst_port)
 
            api.Trigger_AddHostCommand(req, node.Name(), 'sleep 2')
            api.Trigger_AddHostCommand(req, node.Name(), send_cmd)

            trig_resp = api.Trigger(req)
            time.sleep(SNIFF_TIMEOUT) 
            term_resp = api.Trigger_TerminateAllCommands(trig_resp)

            resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)

            for cmd in resp.commands:
                api.PrintCommandResults(cmd)
                if cmd.exit_code != 0:
                    if 'send_pkt' in cmd.command:
                        api.Logger.error("send pkts script failed in h2s dir")
                    if 'recv_pkt' in cmd.command:
                        api.Logger.error("recv pkts script failed in h2s dir")
                    return api.types.status.FAILURE
    
                if 'recv_pkt' in cmd.command and 'FAIL' in cmd.stdout:
                    api.Logger.error("Datapath test failed in h2s dir for "
                    "flow %s" % flow.display())
                    return api.types.status.FAILURE


            # send pkts and validate in s2h dir
            req = api.Trigger_CreateExecuteCommandsRequest(serial=False)
            
            recv_cmd = './recv_pkt.py --intf_name %s --intf_vlan %s \
                        --intf_mac %s --skip_vlan_cfg --timeout %s --proto %s \
                        --src_ip %s --dst_ip %s --src_port %s --dst_port %s \
                        --dir s2h' % (tc.up1_intf, tc.up1_vlan, tc.up1_mac, 
                        str(SNIFF_TIMEOUT), flow.proto, flow.sip, flow.dip, 
                        str(flow.src_port), str(flow.dst_port))

            api.Trigger_AddHostCommand(req, node.Name(), recv_cmd,
                                                background=True)
   
            send_cmd = './send_pkt.py --intf_name %s --intf_vlan %s \
                        --intf_mac %s --skip_vlan_cfg --proto %s --src_ip %s \
                        --dst_ip %s --src_port %s --dst_port %s --dir s2h' % (
                        tc.up0_intf, tc.up0_vlan, tc.up1_mac, flow.proto, 
                        flow.sip, flow.dip, flow.src_port, flow.dst_port)
 
            api.Trigger_AddHostCommand(req, node.Name(), 'sleep 2')
            api.Trigger_AddHostCommand(req, node.Name(), send_cmd)

            trig_resp = api.Trigger(req)
            time.sleep(SNIFF_TIMEOUT) 
            term_resp = api.Trigger_TerminateAllCommands(trig_resp)

            resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)

            for cmd in resp.commands:
                api.PrintCommandResults(cmd)
                if cmd.exit_code != 0:
                    if 'send_pkt' in cmd.command:
                        api.Logger.error("send pkts script failed in s2h dir")
                    if 'recv_pkt' in cmd.command:
                        api.Logger.error("recv pkts script failed in s2h dir")
                    return api.types.status.FAILURE
    
                if 'recv_pkt' in cmd.command and 'FAIL' in cmd.stdout:
                    api.Logger.error("Datapath test failed in s2h dir for "
                    "flow %s" % flow.display())
                    return api.types.status.FAILURE


            api.Logger.info('Datapath test passed for flow %s' % flow.display())

    return api.types.status.SUCCESS

def Verify(tc):
    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS

