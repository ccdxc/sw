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
    if resp_msg.DESCRIPTOR.name == "BGPPeerGetResponse":
        print ("Number of entries retrieved: %d" % len(resp_msg.Response))
        for idx in range(len(resp_msg.Response)):
            resp = resp_msg.Response[idx]
            spec = resp.Spec
            print ("Entry %d" %idx)
            print ("-----------------------");
            print ("Spec: LocalAddr: %s" % long2ip(socket.ntohl(spec.LocalAddr.V4Addr)))
            print ("Spec: PeerAddr : %s" % long2ip(socket.ntohl(spec.PeerAddr.V4Addr)))
            print ("Spec: RemoteASN : %d" % spec.RemoteASN)
            print ("Spec: Password : %s" % spec.Password)
            print ("Spec: AdminStatus : %d" % spec.State)
            print ("Spec: RetryInterval : %d" % spec.ConnectRetry)
            print ("Spec: HoldTime : %d" % spec.HoldTime)
            print ("Spec: KeepAlive: %d" % spec.KeepAlive)
            status = resp.Status
            print ("Status: LocalAddr: %s" % long2ip(socket.ntohl(spec.LocalAddr.V4Addr)))
            print ("Status: PeerAddr : %s" % long2ip(socket.ntohl(spec.PeerAddr.V4Addr)))
            print ("Status: Status   : %d\n" % status.Status)
    else:
        print ("Op Success")
    return

def create_peer():
    req = bgp_pb2.BGPPeerRequest()
    req_msg = req.Request.add()
    req_msg.Id = uuid
    req_msg.State = admin
    req_msg.PeerAddr.Af = 1
    req_msg.PeerAddr.V4Addr = paddr
    req_msg.LocalAddr.Af = 1
    req_msg.LocalAddr.V4Addr = laddr
    req_msg.RemoteASN = 1
    req_msg.ConnectRetry = 10
    req_msg.SendComm = 2;
    req_msg.SendExtComm = 2;
    req_msg.Password = "test"
    resp = stub.BGPPeerCreate(req)
    process_response(req, resp)
    return

def update_peer():
    req = bgp_pb2.BGPPeerRequest()
    req_msg = req.Request.add()
    req_msg.Id = uuid
    req_msg.State = admin
    req_msg.PeerAddr.Af = 1
    req_msg.PeerAddr.V4Addr = paddr
    req_msg.LocalAddr.Af = 1
    req_msg.LocalAddr.V4Addr = laddr
    req_msg.RemoteASN = int(input("enter remote ASN: "))
    req_msg.ConnectRetry = int (input ("enter retry interval: "))
    req_msg.HoldTime = int(input("enter Holdtime: "))
    req_msg.KeepAlive = int(input("enter KeepAliveTimer: "))
    req_msg.SendComm = 2;
    req_msg.SendExtComm = 2;
    resp = stub.BGPPeerUpdate(req)
    process_response(req, resp)
    return

def get_peer():
    req = bgp_pb2.BGPPeerGetRequest()
    req_msg = req.Request.add()
    #req_msg.Id = uuid
    req_msg.Key.PeerAddr.Af = 1
    req_msg.Key.PeerAddr.V4Addr = paddr
    req_msg.Key.LocalAddr.Af = 1
    req_msg.Key.LocalAddr.V4Addr = laddr
    resp =  stub.BGPPeerGet(req)
    process_response(req, resp)
    return

def get_all_peer():
    req = bgp_pb2.BGPPeerGetRequest()
    resp =  stub.BGPPeerGet(req)
    process_response(req, resp)
    return

def delete_peer():
    req = bgp_pb2.BGPPeerDeleteRequest()
    req_msg = req.Request.add()
    #req_msg.Id = uuid
    req_msg.Key.PeerAddr.Af = 1
    req_msg.Key.PeerAddr.V4Addr = paddr
    req_msg.Key.LocalAddr.Af = 1
    req_msg.Key.LocalAddr.V4Addr = laddr
    resp = stub.BGPPeerDelete(req)
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
    print ("Usage: %s <opt> UUID local_addr peer_addr <admin-status(1/2)>" % sys.argv[0])
    print ("eg   : %s 1 100 10.1.1.1 10.1.1.2 1" %sys.argv[0])
    print ("opt  : 1: create_peer\t2: update_peer\t3: delete_peer\t4: get_peer")
    print ("empty get does get-all")
    return

def read_args():
    global opt
    global uuid
    global laddr
    global paddr
    global admin

    laddr=0;
    paddr=0;
    opt = int (sys.argv[1])
    if args > 1:
        uuid = sys.argv[2]
    if args > 2:
        laddr = socket.htonl(ip2long (sys.argv[3]))
    if args > 3:
        paddr = socket.htonl(ip2long (sys.argv[4]))
    if args > 4:
        admin = int (sys.argv[5])
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
            create_peer()
        elif opt == 2:
            update_peer()
        elif opt == 3:
            delete_peer()
        elif opt == 4:
            if args == 1:
                get_all_peer()
            else:
                get_peer()
        else:
            print_help()
