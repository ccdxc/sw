#!/usr/bin/python3

'''
Utils script to craft a packet in scapy based on 
given args and policy.json and send it on wire
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
from random import randint

logging.basicConfig(level=logging.INFO, handlers=[logging.StreamHandler(sys.stdout)])

DEFAULT_NUM_SEND_PKTS = 10
DEFAULT_PAYLOAD = 'abcdefghijklmnopqrstuvwzxyabcdefghijklmnopqrstuvwzxy'
DEFAULT_POLICY_JSON_FILENAME = './policy.json'

DEFAULT_PKT_DMAC = '00:aa:bb:cc:dd:ee'
DEFAULT_H2S_GEN_PKT_FILENAME = './h2s_pkt.pcap'
DEFAULT_S2H_GEN_PKT_FILENAME = './s2h_pkt.pcap'


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


def get_intf_smac(intf):

    smac = None
    intf_name = intf.get_name()
    
    sub_p = subprocess.Popen(['ifconfig', intf_name], stdout=subprocess.PIPE,
            stderr=subprocess.PIPE, universal_newlines=True)
    out, err = sub_p.communicate()
    
    if err:
        raise Exception('ifconfig %s command failed with error %s' %
                (intf_name, err))
    
    op = out.splitlines()
    for line in op:
        if line.strip().startswith('ether'):
            smac_str = line.strip()
            smac = smac_str.split()[1]    
            break

    return smac


def craft_h2s_pkt(intf, flow):

    smac = intf.get_mac()
    dmac = DEFAULT_PKT_DMAC
    vlan = intf.get_vlan()
    
    sip = str(flow.get_sip())
    dip = str(flow.get_dip())
    proto = flow.get_proto()
    src_port = flow.get_src_port()
    dst_port = flow.get_dst_port()
    payload = DEFAULT_PAYLOAD

    if proto == 'UDP':
        pkt = Ether(src=smac, dst=dmac)/\
                Dot1Q(vlan=int(vlan))/\
                IP(src=sip, dst=dip, id=0)/\
                UDP(sport=int(src_port), dport=int(dst_port))/\
                payload
    else:
        raise Exception('protocol %s not supported currently' % proto)

    return pkt


def craft_s2h_pkt(intf, flow):

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

    outer_smac, outer_dmac, outer_vlan = None, None, None
    outer_sip, outer_dip = None, None
    outer_sport, outer_dport = None, None
    inner_sip, inner_dip = None, None
    inner_sport, inner_dport = None, None
    pkt = None
    payload = DEFAULT_PAYLOAD

    encap_info = trg_vnic['rewrite_underlay']
    if encap_info['type'] == 'mplsoudp':
        outer_sport = randint(49152, 65535) 
        outer_dport = 6635
        
        outer_smac = intf.get_mac()
        outer_dmac = DEFAULT_PKT_DMAC
        outer_vlan = encap_vlan
        outer_sip = encap_info['ipv4_sip']
        outer_dip = encap_info['ipv4_dip']
        mpls_lbl1 = encap_info['mpls_label1']
        mpls_lbl2 = encap_info['mpls_label2']

        if flow.get_proto() == 'UDP':
            inner_sip = str(flow.get_sip())
            inner_dip = str(flow.get_dip())
            inner_sport = flow.get_src_port()
            inner_dport = flow.get_dst_port()

            pkt = Ether(src=outer_smac, dst=outer_dmac)/\
                    Dot1Q(vlan=int(outer_vlan))/\
                    IP(src=outer_sip, dst=outer_dip, id=0)/\
                    UDP(sport=int(outer_sport), dport=int(outer_dport))/\
                    MPLS(label=int(mpls_lbl1), s=0)/\
                    MPLS(label=int(mpls_lbl2), s=1)/\
                    IP(src=inner_sip, dst=inner_dip, id=0)/\
                    UDP(sport=int(inner_sport), dport=int(inner_dport))/\
                    payload

        else:
            raise Exception('protocol %s not supported currently' %
                    flow.get_proto())

    else:
        raise Exception('encap type %s not supported currently' %
        encap_info['type'])

    return pkt


def send_packet(intf, skip_vlan_cfg, flow, traffic_dir):

    intf_name = intf.get_name()

    # vlan interface config
    if not skip_vlan_cfg:
        config_intf_vlan(intf)

    # craft packet 
    if traffic_dir == 'h2s':
        pkt = craft_h2s_pkt(intf, flow)
        
        with open(DEFAULT_H2S_GEN_PKT_FILENAME, 'w+') as fd:
            logging.info('Writing crafted h2s pkt to file %s' % fd.name)
            wrpcap(fd.name, pkt)

    else:
        pkt = craft_s2h_pkt(intf, flow)

        with open(DEFAULT_S2H_GEN_PKT_FILENAME, 'w+') as fd:
            logging.info('Writing crafted s2h pkt to file %s' % fd.name)
            wrpcap(fd.name, pkt)

    # send packet on wire
    logging.info('[%s] Sending %d packets in %s dir on interface %s' %
            (get_curr_time(), int(DEFAULT_NUM_SEND_PKTS), traffic_dir, intf_name))    
    
    sendp(pkt, iface=intf_name, count=DEFAULT_NUM_SEND_PKTS)    


def main():
    parser = argparse.ArgumentParser(description='script to craft and send ' 
                                                'pkts on wire with scapy')

    parser.add_argument('--intf_name', type=str, default=None, 
                        help='Tx Interface')
    parser.add_argument('--intf_vlan', type=str, default=None,
                        help='Uplink interface vlan')
    parser.add_argument('--intf_mac', type=str, default=None,
                        help='Interface mac address')
    parser.add_argument('--skip_vlan_cfg', action='store_true',
                        help='Skip vlan interface cfg')
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

    logging.info("Tx interface: %s" % (args.intf_name))
    intf.set_name(args.intf_name)
    
    logging.info("Uplink interface vlan: %s" % (args.intf_vlan))
    intf.set_vlan(args.intf_vlan)
    
    logging.info("Tx interface mac: %s" % (args.intf_mac))
    intf.set_mac(args.intf_mac)
    
    if args.skip_vlan_cfg:
        logging.info("Skipping vlan interface cfg")
        skip_vlan_cfg = True
    
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

    return send_packet(intf, skip_vlan_cfg, flow, traffic_dir)
    

if __name__ == '__main__':
    main()
