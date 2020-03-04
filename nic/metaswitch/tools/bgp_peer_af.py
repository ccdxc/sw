 #! /usr/bin/python3

import sys
import grpc
import bgp_pb2
import types_pb2
import socket
import struct

stub = None
channel = None

def process_response(req_msg, resp_msg):
    if (resp_msg.ApiStatus != types_pb2.API_STATUS_OK):
        print ("Op failed: %d" % resp_msg.ApiStatus)
        return
    if resp_msg.DESCRIPTOR.name == "BGPPeerAfGetResponse":
        print ("Number of entries retrieved: %d" % len(resp_msg.Response))
        for idx in range(len(resp_msg.Response)):
            resp = resp_msg.Response[idx]
            spec = resp.Spec
            print ("Entry %d" %idx)
            print ("-----------------------");
            print ("Spec: LocalAddr   : %s" % long2ip(socket.ntohl(spec.LocalAddr.V4Addr)))
            print ("Spec: PeerAddr    : %s" % long2ip(socket.ntohl(spec.PeerAddr.V4Addr)))
            print ("Spec: Afi         : %s" % spec.Afi)
            print ("Spec: Safi        : %s" % spec.Safi)
            print ("Spec: NexthopSelf : %s" % spec.NexthopSelf)
            print ("Spec: DefaultOrig : %s" % spec.DefaultOrig)
    else:
        print ("Op Success")
    return

def create_peer_af():
    req = bgp_pb2.BGPPeerAfRequest()
    req_msg = req.Request.add()
    req_msg.Id = uuid
    req_msg.PeerAddr.Af = 1
    req_msg.PeerAddr.V4Addr = paddr
    req_msg.LocalAddr.Af = 1
    req_msg.LocalAddr.V4Addr = laddr
    req_msg.Afi = afi
    req_msg.Safi= safi
    req_msg.NexthopSelf = 0 
    req_msg.DefaultOrig = 0 
    resp = stub.BGPPeerAfCreate(req)     
    process_response(req, resp)
    return

def update_peer_af():
    req = bgp_pb2.BGPPeerAfRequest()
    req_msg = req.Request.add()
    req_msg.Id = uuid
    req_msg.PeerAddr.Af = 1
    req_msg.PeerAddr.V4Addr = paddr
    req_msg.LocalAddr.Af = 1
    req_msg.LocalAddr.V4Addr = laddr
    req_msg.Afi = afi
    req_msg.Safi= safi
    req_msg.NexthopSelf = 0
    req_msg.DefaultOrig = 0
    resp = stub.BGPPeerAfUpdate(req)
    process_response(req, resp)
    return

def get_peer_af():
    req = bgp_pb2.BGPPeerAfGetRequest()
    req_msg = req.Request.add()
    #req_msg.Id = uuid
    req_msg.Key.PeerAddr.Af = 1
    req_msg.Key.PeerAddr.V4Addr = paddr
    req_msg.Key.LocalAddr.Af = 1
    req_msg.Key.LocalAddr.V4Addr = laddr
    req_msg.Key.Afi = afi
    req_msg.Key.Safi= safi
    resp =  stub.BGPPeerAfGet(req)     
    process_response(req, resp)
    return

def get_all_peer_af():
    req = bgp_pb2.BGPPeerAfGetRequest()
    resp =  stub.BGPPeerAfGet(req)     
    process_response(req, resp)
    return

def delete_peer_af():
    req = bgp_pb2.BGPPeerAfDeleteRequest()
    req_msg = req.Request.add()
    #req_msg.Id = uuid
    req_msg.Key.PeerAddr.Af = 1
    req_msg.Key.PeerAddr.V4Addr = paddr
    req_msg.Key.LocalAddr.Af = 1
    req_msg.Key.LocalAddr.V4Addr = laddr
    req_msg.Key.Afi = afi
    req_msg.Key.Safi= safi
    resp = stub.BGPPeerAfDelete(req)     
    process_response(req, resp)
    return

def init():
    global channel
    global stub
    server = 'localhost:50054'
    channel = grpc.insecure_channel(server)
    stub = bgp_pb2.BGPSvcStub(channel)
    return

def print_help():
    print ("Usage: %s <opt> UUID local_addr peer_addr afi<1/25> safi<1/70> " % sys.argv[0])
    print ("eg   : %s 1 100 10.1.1.1 10.1.1.2 1 1 " %sys.argv[0])
    print ("opt  : 1: create\t2: update\t3: delete\t4: get")
    print ("empty get does get-all")
    return

def read_args():
    global opt
    global uuid
    global laddr
    global paddr
    global afi 
    global safi 
    laddr = 0
    paddr = 0
    afi=1
    safi=1

    opt = int (sys.argv[1])
    if args > 1:
        uuid = sys.argv[2]
    if args > 2:
        laddr = socket.htonl(ip2long (sys.argv[3]))
    if args > 3:
        paddr = socket.htonl(ip2long (sys.argv[4]))
    if args > 4:
        afi = int (sys.argv[5])
    if args > 5:
        safi = int (sys.argv[6])
    return

def ip2long(ip):
    packedIP = socket.inet_aton(ip)
    return struct.unpack("!L", packedIP)[0] 

def long2ip(addr):
    return socket.inet_ntoa(struct.pack('!L', addr))

if __name__ == '__main__':
    global args
    args = len (sys.argv) - 1
    if not args:
        print_help()
    else:
        init()
        read_args()
        if opt == 1:
            create_peer_af()
        elif opt == 2:
            update_peer_af()
        elif opt == 3:
            delete_peer_af()
        elif opt == 4:
            if args == 1:
                get_all_peer_af()
            else:
                get_peer_af()
        else:
            print_help()
