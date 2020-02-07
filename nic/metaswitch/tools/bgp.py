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
    if "GetResponse" in resp_msg.DESCRIPTOR.name:
        resp = resp_msg.Response
        spec = resp.Spec
        print ("-----------------------");
        print ("Spec: LocalASN : %d" % spec.LocalASN)
        print ("Spec: RouterID: 0x%X" % spec.RouterId)
        print ("Spec: ClusterID: 0x%X" % spec.ClusterId)
    else:
        print ("Op Success")
    return

def create_bgp():
    req = bgp_pb2.BGPRequest()
    req_msg = req.Request
    req_msg.Id = uuid
    req_msg.LocalASN = asn
    req_msg.RouterId = routerid;
    req_msg.ClusterId = clusterid;
    resp = stub.BGPCreate(req)     
    process_response(req, resp)
    return

def update_bgp():
    req = bgp_pb2.BGPRequest()
    req_msg = req.Request
    req_msg.Id = uuid
    req_msg.LocalASN = asn
    req_msg.RouterId = routerid;
    req_msg.ClusterId = clusterid;
    resp = stub.BGPUpdate(req)
    process_response(req, resp)
    return

def get_bgp():
    req = bgp_pb2.BGPRequest()
    req_msg = req.Request
    req_msg.Id = uuid
    resp =  stub.BGPGet(req)     
    process_response(req, resp)
    return

def get_bgp_all():
    req = bgp_pb2.BGPRequest()
    resp =  stub.BGPGet(req)     
    process_response(req, resp)
    req

def delete_bgp():
    req = bgp_pb2.BGPRequest()
    req_msg = req.Request
    req_msg.Id = uuid
    resp = stub.BGPDelete(req)     
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
    print ("Usage: %s <opt> UUID LocalASN RouterID (hex) ClusterID (hex)" % sys.argv[0])
    print ("eg   : %s 1 100  1  0 0" %sys.argv[0])
    print ("opt  : 1: create\t2: update\t3: delete\t4: get")
    print ("empty get does get-all")
    return
   
def read_args():
    global uuid 
    global asn
    global routerid
    global clusterid
    asn=0
    routerid=0
    clusterid=0
    if args > 1:
        uuid = sys.argv[2]
    if args > 2:
        asn = int (sys.argv[3])
    if args > 3:
        routerid = int (sys.argv[4],16)
    if args > 4:
        clusterid = int (sys.argv[5],16)
    return

if __name__ == '__main__':
    args = len (sys.argv) - 1
    if not args:
        print_help()
    else:
        init()
        read_args()
        opt = int (sys.argv[1])
        if opt == 1:
            create_bgp()
        elif opt == 2:
            update_bgp()
        elif opt == 3:
            delete_bgp()
        elif opt == 4:
            if  args == 1:
                get_bgp_all()
            else:    
                get_bgp()
        else:
            print_help()
