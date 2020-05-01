 #! /usr/bin/python3

import sys
import grpc
import bgp_pb2
import types_pb2
import socket
import struct
import defines

stub = None
channel = None

def process_response(req_msg, resp_msg):
    if (resp_msg.ApiStatus != types_pb2.API_STATUS_OK):
        print ("Op failed: %d" % resp_msg.ApiStatus)
        return
    else:
        print ("Op Success")
    return

def clear():
    req = bgp_pb2.BGPClearRouteRequest()
    req.Option = opt
    if key == 1:
        req.Peer.Key.PeerAddr.Af = 1
        req.Peer.Key.PeerAddr.V4Addr = paddr
        req.Peer.Key.LocalAddr.Af = 1
        req.Peer.Key.LocalAddr.V4Addr = laddr
    elif key == 2:
        req.PeerAf.Key.PeerAddr.Af = 1
        req.PeerAf.Key.PeerAddr.V4Addr = paddr
        req.PeerAf.Key.LocalAddr.Af = 1
        req.PeerAf.Key.LocalAddr.V4Addr = laddr
        req.PeerAf.Key.Afi = afi
        req.PeerAf.Key.Safi = safi
    resp = stub.BGPClearRoute(req)
    process_response(req, resp)
    return

def init():
    global channel
    global stub
    server = 'localhost:' + str(defines.AGENT_GRPC_PORT)
    channel = grpc.insecure_channel(server)
    stub = bgp_pb2.BGPSvcStub(channel)
    return

def print_help():
    print ("Usage: %s <opt> <key> local_addr peer_addr [afi] [safi]>" % sys.argv[0])
    print ("eg   : %s 1 1 10.1.1.1 10.1.1.2 1" %sys.argv[0])
    print ("eg   : %s 2 2 10.1.1.1 10.1.1.2 1 1 1" %sys.argv[0])
    print ("opt  : 0: none\t1: hard-reset\t2: rr in \t3: rr out\t4: rr both")
    print ("key  : 0: none\t1: peer\t2: peer-af")
    return

def read_args():
    global opt
    global key
    global laddr
    global paddr
    global afi
    global safi

    laddr=0
    paddr=0
    afi=1
    safi=1
    opt = int (sys.argv[1])
    if args > 1:
        key = int(sys.argv[2])
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
        clear()
