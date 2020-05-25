#!/usr/bin/python3

'''
Utils script to receive a packet and validate it using 
scapy based on given args and policy.json config
'''
import argparse
import logging
import subprocess 
import json
import sys
import time

from ipaddress import ip_address
from scapy.all import *
from scapy.contrib.mpls import MPLS

logging.basicConfig(level=logging.INFO, handlers=[logging.StreamHandler(sys.stdout)])

DEFAULT_NUM_RECV_PKTS = 10
DEFAULT_PAYLOAD = 'abcdefghijklmnopqrstuvwzxyabcdefghijklmnopqrstuvwzxy'
DEFAULT_POLICY_JSON_FILENAME = './policy.json'

DEFAULT_H2S_RECV_PKT_FILENAME = './h2s_recv_pkt.pcap'
DEFAULT_S2H_RECV_PKT_FILENAME = './s2h_recv_pkt.pcap'
DEFAULT_H2S_UNMATCHED_RECV_PKTS_FILENAME = './unmatched_h2s_recv_pkts.pcap'
DEFAULT_S2H_UNMATCHED_RECV_PKTS_FILENAME = './unmatched_s2h_recv_pkts.pcap'


class Interface():

    def __init__(self):
        self.intf_name = None
        self.vlan = None
        self.mac = None

    # Setters
    def set_name(self, intf_name):
        self.intf_name = intf_name

    def set_vlan(self, vlan):
        if (int(vlan) < 0) or (int(vlan) > 4095):
            raise Exception('Invalid vlan for interface %s' % vlan)
        
        self.vlan = vlan 

    def set_mac(self, mac):
        self.mac = mac

    # Getters
    def get_name(self):
        return self.intf_name

    def get_vlan(self):
        return self.vlan

    def get_mac(self):
        return self.mac


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


def get_curr_time():
    return time.strftime("%Y-%m-%d %H:%M:%S", time.localtime()) 


def config_intf_vlan(intf):

    intf_name = intf.get_name()
    vlan = intf.get_vlan()

    vlan_intf = intf_name + '.' + vlan
    vlan_intf_cmd0 = 'ip link delete ' + vlan_intf  # cleanup
    vlan_intf_cmd1 = 'ip link add link ' + intf_name + ' name ' + vlan_intf +\
                                                    ' type vlan id ' + vlan
    vlan_intf_cmd2 = 'ip link set dev ' + vlan_intf + ' up' 
    subprocess.run(vlan_intf_cmd0.split())
    subprocess.run(vlan_intf_cmd1.split(), check=True)
    subprocess.run(vlan_intf_cmd2.split(), check=True)


def validate_recv_pkt_h2s(intf, flow, recv_pkts):
    
    # craft recv pkt for h2s dir
    encap_vlan = intf.get_vlan()
    with open(DEFAULT_POLICY_JSON_FILENAME) as json_fd:
        plcy_obj = json.load(json_fd) 

    vnics = plcy_obj['vnic']
    trg_vnic = None

    for vnic in vnics:
        encap_info = vnic['rewrite_underlay']
        if encap_info['vlan_id'] == encap_vlan:
            trg_vnic = vnic 
            break

    if trg_vnic is None:
        raise Exception('vnic config not found encap vlan %s' % encap_vlan) 

    outer_smac, outer_dmac = None, None
    outer_sip, outer_dip = None, None
    outer_dport = None
    inner_sip, inner_dip = None, None
    inner_sport, inner_dport = None, None
    gen_pkt = None
    payload = DEFAULT_PAYLOAD

    encap_info = trg_vnic['rewrite_underlay']
    if encap_info['type'] == 'mplsoudp':
        
        outer_smac = encap_info['smac']
        outer_dmac = encap_info['dmac']
        outer_sip = encap_info['ipv4_sip']
        outer_dip = encap_info['ipv4_dip']
        outer_dport = 6635
        mpls_lbl1 = encap_info['mpls_label1']
        mpls_lbl2 = encap_info['mpls_label2']

        if flow.get_proto() == 'UDP':
            inner_sip = str(flow.get_sip())
            inner_dip = str(flow.get_dip())
            inner_sport = flow.get_src_port()
            inner_dport = flow.get_dst_port()

            gen_pkt = Ether(src=outer_smac, dst=outer_dmac)/\
                    Dot1Q(vlan=int(encap_vlan))/\
                    IP(src=outer_sip, dst=outer_dip, id=0)/\
                    UDP(sport=0, dport=int(outer_dport))/\
                    MPLS(label=int(mpls_lbl1), s=0)/\
                    MPLS(label=int(mpls_lbl2), s=1)/\
                    IP(src=inner_sip, dst=inner_dip, id=0)/\
                    UDP(sport=int(inner_sport), dport=int(inner_dport))/\
                    payload

            # zero out outer udp csum since we ignore it for validation
            gen_pkt['UDP'].chksum = 0
        else:
            raise Exception('protocol %s not supported currently' %
                    flow.get_proto())

    else:
        raise Exception('encap type %s not supported currently' %
        encap_info['type'])

    with open(DEFAULT_H2S_RECV_PKT_FILENAME, 'w+') as fd:
        logging.info('Writing crafted recv pkt for h2s dir validation '
                    'to file %s' % fd.name)
        wrpcap(fd.name, gen_pkt)

    # validate received packets
    count = 0
    unmatched_pkts = []
    # read from pcap file so that csum is generated
    gen_pkt = rdpcap(DEFAULT_H2S_RECV_PKT_FILENAME) 

    for r_pkt in recv_pkts:

        if encap_info['type'] == 'mplsoudp':
            if MPLS not in r_pkt or UDP not in r_pkt['MPLS'].underlayer:
                continue

            # ignore randomly generated sport when comparing mpls-in-udp pkts
            r_pkt['UDP'].sport = 0  
            r_pkt['UDP'].chksum = 0
            # TODO: check if it's a bug
            r_pkt.getlayer(MPLS, nb=1).ttl = 0
            r_pkt.getlayer(MPLS, nb=2).ttl = 0

            if r_pkt == gen_pkt[0]:
                count += 1    
            
            else:
                unmatched_pkts.append(r_pkt)  

    if count != DEFAULT_NUM_RECV_PKTS:
        logging.error('FAIL !! - Received matching pkt count (%d pkts) '
                'doesn\'t equal sent pkt count in h2s dir for interface %s. '
                'Writing unmatched pkts to %s.' % (count, intf.get_name(), 
                DEFAULT_H2S_UNMATCHED_RECV_PKTS_FILENAME))
            
        with open(DEFAULT_H2S_UNMATCHED_RECV_PKTS_FILENAME, 'w+') as fd:
            wrpcap(fd.name, unmatched_pkts)
        
        return 1

    else:
        logging.info('PASS !! - Received matching pkt count (%d pkts) equals '
                'sent pkt count in h2s dir for interface %s' % (count, 
                intf.get_name()))

        return 0


def validate_recv_pkt_s2h(intf, flow, recv_pkts):

    # craft recv pkt for s2h dir
    vlan = intf.get_vlan()
    with open(DEFAULT_POLICY_JSON_FILENAME) as json_fd:
        plcy_obj = json.load(json_fd) 

    vnics = plcy_obj['vnic']
    trg_vnic = None

    for vnic in vnics:
        if vnic['vlan_id'] == vlan:
            trg_vnic = vnic 
            break

    if trg_vnic is None:
        raise Exception('vnic config not found vlan %s' % vlan) 

    smac, dmac = None, None
    sip = str(flow.get_sip())
    dip = str(flow.get_dip())
    proto = flow.get_proto()
    src_port = flow.get_src_port()
    dst_port = flow.get_dst_port()
    payload = DEFAULT_PAYLOAD
    gen_pkt = None

    rewrite_host_info = trg_vnic['rewrite_host']
    if proto == 'UDP':
        smac = rewrite_host_info['smac'] 
        dmac = rewrite_host_info['dmac']

        gen_pkt = Ether(src=smac, dst=dmac)/\
                Dot1Q(vlan=int(vlan))/\
                IP(src=sip, dst=dip, id=0)/\
                UDP(sport=int(src_port), dport=int(dst_port))/\
                payload

    else:
        raise Exception('flow protocol %s not supported currently' % proto)

    with open(DEFAULT_S2H_RECV_PKT_FILENAME, 'w+') as fd:
        logging.info('Writing crafted recv pkt for s2h dir validation ' 
                        'to file %s' % fd.name)
        wrpcap(fd.name, gen_pkt)

    # validate received packets
    count = 0
    unmatched_pkts = []
    # read from pcap file so that csum is generated
    gen_pkt = rdpcap(DEFAULT_S2H_RECV_PKT_FILENAME) 
    
    for r_pkt in recv_pkts:

        if proto == 'UDP':
            if UDP not in r_pkt:
                continue

            if r_pkt == gen_pkt[0]:
                count += 1    
            
            else:
                unmatched_pkts.append(r_pkt)  

    if count != DEFAULT_NUM_RECV_PKTS:
        logging.error('FAIL !! - Received matching pkt count (%d pkts) '
                'doesn\'t equal sent pkt count in s2h dir for interface %s. '
                'Writing unmatched pkts to %s.' % (count, intf.get_name(), 
                DEFAULT_S2H_UNMATCHED_RECV_PKTS_FILENAME))
            
        with open(DEFAULT_S2H_UNMATCHED_RECV_PKTS_FILENAME, 'w+') as fd:
            wrpcap(fd.name, unmatched_pkts)
        
        return 1

    else:
        logging.info('PASS !! - Received matching pkt count (%d pkts) equals '
                'sent pkt count in s2h dir for interface %s' % (count, 
                intf.get_name()))

        return 0


def recv_packet(intf, skip_vlan_cfg, flow, traffic_dir, timeout):

    intf_name = intf.get_name()

    # vlan interface config
    if not skip_vlan_cfg:
        config_intf_vlan(intf)

    # receive packets 
    logging.info('[%s] Started sniffing packets in %s dir on interface %s' %
                (get_curr_time(), traffic_dir, intf_name))    
    
    recv_pkts = sniff(iface=intf_name, timeout=timeout)
    
    logging.info('[%s] Stopped sniffing packets in %s dir on interface %s' %
            (get_curr_time(), traffic_dir, intf_name))    
    
    # validate received packets 
    if traffic_dir == 'h2s':
        ret = validate_recv_pkt_h2s(intf, flow, recv_pkts)
        
    else:
        ret = validate_recv_pkt_s2h(intf, flow, recv_pkts)
    
    if ret != 0:
        sys.exit('Failed')


def main():
    parser = argparse.ArgumentParser(description='script to receive pkts and '
                                                'validate them with scapy')

    parser.add_argument('--intf_name', type=str, default=None, 
                        help='Rx Interface')
    parser.add_argument('--intf_vlan', type=str, default=None,
                        help='Uplink interface vlan')
    parser.add_argument('--intf_mac', type=str, default=None,
                        help='Interface mac address')
    parser.add_argument('--skip_vlan_cfg', action='store_true',
                        help='Skip vlan interface cfg')
    parser.add_argument('--timeout', type=int, default=None,
                        help='Timeout (secs)')
    parser.add_argument('--proto', type=str, default='UDP',
                        help='UDP/TCP/ICMP protocol')
    parser.add_argument('--src_ip', type=str, default=None,
                        help='Source IP address')
    parser.add_argument('--dst_ip', type=str, default=None,
                        help='Destination IP address')
    parser.add_argument('--src_port', type=str, default=None,
                        help='Source L4 port')
    parser.add_argument('--dst_port', type=str, default=None,
                        help='Destination L4 port') 
    parser.add_argument('--dir', type=str, default=None,
                        help='Traffic dir H2S or S2H')

    args = parser.parse_args()

    intf = Interface()
    flow = FlowInfo()
    skip_vlan_cfg = False
    timeout = None

    logging.info("Rx interface: %s" % (args.intf_name))
    intf.set_name(args.intf_name)
    
    logging.info("Uplink interface vlan: %s" % (args.intf_vlan))
    intf.set_vlan(args.intf_vlan)
   
    logging.info("Rx interface mac: %s" % (args.intf_mac))
    intf.set_mac(args.intf_mac)
    
    if args.skip_vlan_cfg:
        logging.info("Skipping vlan interface cfg")
        skip_vlan_cfg = True
    
    logging.info("Timeout: %d secs" % (args.timeout))
    timeout = args.timeout
    
    logging.info("protocol: %s" % (args.proto))
    flow.set_proto(args.proto)

    logging.info("Src IP: %s" % (args.src_ip))
    flow.set_sip(ip_address(args.src_ip))

    logging.info("Dst IP: %s" % (args.dst_ip))
    flow.set_dip(ip_address(args.dst_ip))

    logging.info("L4 Src port: %s" % (args.src_port))
    flow.set_src_port(args.src_port)

    logging.info("L4 Dst port: %s" % (args.dst_port))
    flow.set_dst_port(args.dst_port)

    logging.info("Traffic dir: %s" % (args.dir))
    traffic_dir = args.dir

    return recv_packet(intf, skip_vlan_cfg, flow, traffic_dir, timeout)
    

if __name__ == '__main__':
    main()
