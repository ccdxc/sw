 #! /usr/bin/python3

import sys
import grpc
import cp_route_pb2
import types_pb2
import common_pb2
import socket
import struct

stub = None
channel = None

def process_response(req_msg, resp_msg):
    if (resp_msg.ApiStatus != types_pb2.API_STATUS_OK):
        print ("Op failed: %d" % resp_msg.ApiStatus)
        return
    if "CPStaticRouteGetResponse" in resp_msg.DESCRIPTOR.name:
        print ("Number of static routes retrieved: %d" % len(resp_msg.Response))
        for idx in range(len(resp_msg.Response)):
            resp = resp_msg.Response[idx]
            spec = resp.Spec
            print ("\nEntry %d" %idx)
            print ("-----------------------");
            print ("Dest Addr: %s"  % long2ip(socket.ntohl(spec.DestAddr.V4Addr)))
            print ("Prefix Len: %d"% spec.PrefixLen)
            print ("Nexthop Addr: %s"  % long2ip(socket.ntohl(spec.NextHopAddr.V4Addr)))
            print ("State : %d"%spec.State) 
            print ("Override: %d"%spec.Override)
            print ("AdminDist: %d"%spec.AdminDist)
    elif "CPRouteGetResponse" in resp_msg.DESCRIPTOR.name:
        print ("Number of routes retrieved: %d" % len(resp_msg.Response))
        for idx in range(len(resp_msg.Response)):
            resp = resp_msg.Response[idx]
            status = resp.Status
            print ("\nEntry %d" %idx)
            print ("-----------------------");
            print ("Dest Addr: %s"  % long2ip(socket.ntohl(status.DestAddr.V4Addr)))
            print ("Prefix Len: %d"% status.DestPrefixLen)
            print ("Nexthop Addr: %s"  % long2ip(socket.ntohl(status.NHAddr.V4Addr)))
            print ("IfIndex: 0x%X" % status.IfIndex)
            print ("Type: %d" % status.Type)
            print ("Proto: %d" % status.Proto)
    else:
        print ("Op Success")
    return

def ip2long(ip):
    packedIP = socket.inet_aton(ip)
    return struct.unpack("!L", packedIP)[0] 
def long2ip(addr):
    return socket.inet_ntoa(struct.pack('!L', addr))

def create_req():
    req = cp_route_pb2.CPStaticRouteRequest()
    req_msg = req.Request.add()
    req_msg.RouteTableId = RouteTableId
    req_msg.DestAddr.Af = 1
    req_msg.DestAddr.V4Addr = destaddr
    req_msg.NextHopAddr.Af = 1
    req_msg.NextHopAddr.V4Addr = nhaddr
    req_msg.PrefixLen = prefixlen
    req_msg.State = state
    req_msg.Override = override
    req_msg.AdminDist = admindist
    resp = stub.CPStaticRouteCreate(req)     
    process_response(req, resp)
    return

def update_req():
    req = cp_route_pb2.CPStaticRouteRequest()
    req_msg = req.Request.add()
    req_msg.RouteTableId = RouteTableId
    req_msg.DestAddr.Af = 1
    req_msg.DestAddr.V4Addr = destaddr
    req_msg.NextHopAddr.Af = 1
    req_msg.NextHopAddr.V4Addr = nhaddr
    req_msg.PrefixLen = prefixlen
    req_msg.State = state
    req_msg.Override = override
    req_msg.AdminDist = admindist
    resp = stub.CPStaticRouteUpdate(req)     
    process_response(req, resp)
    return

def get_req():
    req = cp_route_pb2.CPStaticRouteRequest()
    req_msg = req.Request.add()
    req_msg.DestAddr.Af = 1
    req_msg.DestAddr.V4Addr = destaddr
    req_msg.NextHopAddr.Af = 1
    req_msg.NextHopAddr.V4Addr = nhaddr
    req_msg.PrefixLen = prefixlen
    resp =  stub.CPStaticRouteGet(req)     
    process_response(req, resp)
    return

def get_all_req():
    req = cp_route_pb2.CPStaticRouteRequest()
    resp =  stub.CPStaticRouteGet(req)     
    process_response(req, resp)
    return

def get_route():
    req = cp_route_pb2.CPRouteGetRequest()
    resp = stub.CPRouteGet(req)
    process_response(req, resp)
    return

def delete_req():
    req = cp_route_pb2.CPStaticRouteRequest()
    req_msg = req.Request.add()
    req_msg.DestAddr.Af = 1
    req_msg.DestAddr.V4Addr = destaddr
    req_msg.NextHopAddr.Af = 1
    req_msg.NextHopAddr.V4Addr = nhaddr
    req_msg.PrefixLen = prefixlen
    resp = stub.CPStaticRouteDelete(req)     
    process_response(req, resp)
    return

def init():
    global channel
    global stub
    server = 'localhost:50054'
    channel = grpc.insecure_channel(server)
    stub = cp_route_pb2.CPRouteSvcStub(channel)
    return

def print_help():
    print ("Usage: %s <opt> RouteTableId dest-addr prefix-len nh-addr" % sys.argv[0])
    print ("eg   : %s 1 192.168.0.1 24 10.1.1.254" %sys.argv[0])
    print ("opt  : 1: create_st_route\t2: update_st_route\t3: delete_st_route\t4: get_st_route\t5: get_route")
    print ("empty get does get-all")
    return

def read_args():
    global opt
    global RouteTableId
    global override
    global admindist
    global state
    global destaddr
    global prefixlen
    global nhaddr
    RouteTableId = 1
    override = 1
    admindist = 1
    state=common_pb2.ADMIN_STATE_ENABLE
    opt = int (sys.argv[1])
    if args > 1:
        RouteTableId = sys.argv[2]
    if args > 2:
        destaddr = socket.htonl(ip2long(sys.argv[3]))
    if args > 3:
        prefixlen = int(sys.argv[4])
    if args > 4:
        nhaddr = socket.htonl(ip2long(sys.argv[5]))
    return

if __name__ == '__main__':
    global args
    args = len (sys.argv) - 1
    if not args:
        print_help()
    else:
        init()
        read_args()
        if opt == 1:
            create_req()
        elif opt == 2:
            update_req()
        elif opt == 3:
            delete_req()
        elif opt == 4:
            if args == 1:
                get_all_req()
            else:
                get_req()
        elif opt == 5:
            get_route()
        else:
            print_help()
