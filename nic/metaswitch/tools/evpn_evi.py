 #! /usr/bin/python3

import sys
import grpc
import evpn_pb2
import types_pb2
import socket
import struct

stub = None
channel = None

def process_response(req_msg, resp_msg):
    if (resp_msg.ApiStatus != types_pb2.API_STATUS_OK):
        print ("Op failed: %d" % resp_msg.ApiStatus)
        return
    if resp_msg.DESCRIPTOR.name == "EvpnEviGetResponse":
        print ("Number of entries retrieved: %d" % len(resp_msg.Response))
        for idx in range(len(resp_msg.Response)):
            resp = resp_msg.Response[idx]
            spec = resp.Spec
            print ("Entry %d" %idx)
            print ("-----------------------");
            print ("Spec: AutoRd: %s" % ("Auto" if spec.AutoRD==evpn_pb2.EVPN_CFG_AUTO else "Manual"))
            print ("Spec: RD: %s"% spec.RD)
            print ("Spec: AutoRt: %s" % ("Auto" if spec.AutoRT==evpn_pb2.EVPN_CFG_AUTO else "Manual"))
            print ("Spec: RTType: %d" % spec.RTType)
            status = resp.Status
            print ("Oper RD: "),
            for i in status.RD:
                mm=int(i.encode('hex'),16)
                print(hex(mm) ),
            print
    else:
        print ("Op Success")
    return

def create_req():
    req = evpn_pb2.EvpnEviRequest()
    req_msg = req.Request.add()
    req_msg.Id = uuid+chr(0)+chr(0)+chr(0)+chr(0)+chr(0)+chr(0)+chr(64)
    req_msg.SubnetId = subnetid+chr(0)+chr(0)+chr(0)+chr(0)+chr(0)+chr(0)+chr(64)
    req_msg.AutoRD = autord
    req_msg.AutoRT = autort
    req_msg.RD = rd_str
    req_msg.RTType = rttype
    resp = stub.EvpnEviCreate(req)     
    process_response(req, resp)
    return

def update_req():
    req = evpn_pb2.EvpnEviRequest()
    req_msg = req.Request.add()
    req_msg.Id = uuid+chr(0)+chr(0)+chr(0)+chr(0)+chr(0)+chr(0)+chr(64)
    req_msg.SubnetId = subnetid+chr(0)+chr(0)+chr(0)+chr(0)+chr(0)+chr(0)+chr(64)
    req_msg.AutoRD = autord
    req_msg.AutoRT = autort
    req_msg.RD = rd_str
    req_msg.RTType = rttype
    resp = stub.EvpnEviUpdate(req)     
    process_response(req, resp)
    return

def get_req():
    req = evpn_pb2.EvpnEviGetRequest()
    req_msg = req.Request.add()
    req_msg.Key.SubnetId = subnetid+chr(0)+chr(0)+chr(0)+chr(0)+chr(0)+chr(0)+chr(64)
    resp =  stub.EvpnEviGet(req)     
    process_response(req, resp)
    return

def get_all_req():
    req = evpn_pb2.EvpnEviGetRequest()
    resp =  stub.EvpnEviGet(req)     
    process_response(req, resp)
    return

def delete_req():
    req = evpn_pb2.EvpnEviDeleteRequest()
    req_msg = req.Request.add()
    req_msg.Key.SubnetId = subnetid+chr(0)+chr(0)+chr(0)+chr(0)+chr(0)+chr(0)+chr(64)
    resp = stub.EvpnEviDelete(req)     
    process_response(req, resp)
    return

def init():
    global channel
    global stub
    server = 'localhost:50054'
    channel = grpc.insecure_channel(server)
    stub = evpn_pb2.EvpnSvcStub(channel)
    return

def print_help():
    print ("Usage: %s <opt> UUID SubnetId autord rd autort rttype" % sys.argv[0])
    print ("eg   : %s 1 500 500 1 0 1 4" %sys.argv[0])
    print ("eg   : %s 1 500 500 0 0:2:1:2:3:4:100:200 1 3" %sys.argv[0])
    print ("opt  : 1: create_req\t2: update_req\t3: delete_req\t4: get_req")
    print ("empty get does get-all")
    return

def read_args():
    global opt
    global uuid
    global subnetid
    global rd_str
    global autord
    global autort
    global rttype
    rd_str = ""
    uuid = "0"
    subnetid = "0"
    rttype=evpn_pb2.EVPN_RT_NONE
    opt = int (sys.argv[1])
    if args > 1:
        uuid = sys.argv[2]
    if args > 2:
        subnetid = sys.argv[3]
    if args > 3:
        autord = evpn_pb2.EVPN_CFG_AUTO if int(sys.argv[4]) else evpn_pb2.EVPN_CFG_MANUAL
    if args > 4:
        rd = sys.argv[5]
        for x in rd.split(':'):
            rd_str = rd_str+chr(int(x))
    if args > 5:
        autort = evpn_pb2.EVPN_CFG_AUTO if int(sys.argv[6]) else evpn_pb2.EVPN_CFG_MANUAL
    if args > 6:
        rttype = int(sys.argv[7])
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
        else:
            print_help()
