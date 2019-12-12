#!/usr/bin/python3

import sys
import argparse
from  scapy.all import *
import logging
from ipaddress import *

logging.basicConfig(level=logging.DEBUG, handlers=[logging.StreamHandler(sys.stdout)] )

PORT_MIN = 0
PORT_MAX = 65535
ICMP_DEST_UNRECH = 3

def validatePort(value):
    try:
        value = int(value)
        if PORT_MIN <= value <= PORT_MAX:
            return value
        else:
            raise argparse.ArgumentTypeError("Invalid L3 Port %s"%value)
    except:
        raise argparse.ArgumentTypeError("Invalid L3 Port %s"%value)
    
def filter_icmp_unreach(pkt):
    i,o = pkt[0], pkt[1]
    logging.debug("Finding ICMP in pkt .. ")
    #o.show2()
    return ICMP in o and o[ICMP].type == ICMP_DEST_UNRECH

def validate_icmp_unreach(pkt):
    icmp_pkt = pkt.filter(filter_icmp_unreach)
    for i,o in icmp_pkt:
        if UDP in i and UDPerror in o :
            if i[UDP].sport == o[UDPerror].sport and \
               i[UDP].dport == o[UDPerror].dport:
                return True
    return False

def filter_tcp_rst(pkt):
    i,o = pkt[0], pkt[1]
    logging.debug("Finding TCP reset flag in pkt .. ")
    #o.show2()
    return TCP in o and o[TCP].flags == 'R'

def validate_tcp_rst(pkt):
    tcp_pkt = pkt.filter(filter_tcp_rst)
    for i,o in tcp_pkt:
        if TCP in i and TCP in o :
            if i[TCP].sport == o[TCP].dport and \
               i[TCP].dport == o[TCP].sport:
                return True
    return False

def test_reject(srcIp, dstIp, srcPort, dstPort, proto):
    if proto == "udp":
        pkt = IP(src=str(srcIp), dst=str(dstIp))/UDP(sport=srcPort, dport=dstPort)
    elif proto == "tcp":
        pkt = IP(src=str(srcIp), dst=str(dstIp))/TCP(sport=srcPort, dport=dstPort)
    else:
        logging.error("Unsupported proto: %s"%proto)
        exit(1)
    
    logging.info("Sending packet ..")
    #pkt.show2()
    ans, uans = sr(pkt)
    if len(uans):
        logging.info("unanswered packet summary")
        uans.summary()
    
    if len(ans):
        logging.info("Answered packet summary")
        ans.summary()

        if proto == "udp" and validate_icmp_unreach(ans):
            logging.info("Matched ICMP unreachable packet")
            return 0
        elif proto == "tcp" and validate_tcp_rst(ans):
            logging.info("Matched TCP Reset packet")
            return 0
        else:
            logging.error("Could not find fitting ans to protocol")
    return 1

def main():
    parser = argparse.ArgumentParser(description='Utiliy REJECT firewall policy for TCP and UDP protocol.')

    parser.add_argument('--sp', type=validatePort, default=80,
                        help="Source port")
    parser.add_argument('--dp', type=validatePort, default=80,
                        help="Destination port")
    parser.add_argument('--proto', type=str,
                        help='TCP or UDP protocol', default="TCP")
    parser.add_argument('--src_ip', type=str,
                        help='Source IP address', default="128.128.128.128")
    parser.add_argument('dst_ip', metavar='IP', type=str,
                        help='Destination ip address to hit')

    args = parser.parse_args()

    srcIp = ip_address(args.src_ip)
    logging.info("Source IP : %s"%(args.src_ip))

    dstIp = ip_address(args.dst_ip)
    logging.info("Destination IP : %s"%(args.dst_ip))

    proto = args.proto.lower()
    if proto not in ["tcp", "udp"]:
        logging.error("Invalid protocol : %s"%(proto))
        exit(1)
    logging.info("Protocol : %s"%proto)

    srcPort = args.sp
    logging.info("Source port : %s"%srcPort)

    dstPort = args.dp
    logging.info("Destination port : %s"%dstPort)

    return test_reject(srcIp, dstIp, srcPort, dstPort, proto)
    
if __name__ == "__main__":
    main()
