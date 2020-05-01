 #! /usr/bin/python3

import sys
import grpc
import evpn_pb2
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
    if resp_msg.DESCRIPTOR.name == "EvpnEviRtGetResponse":
        print ("Number of entries retrieved: %d" % len(resp_msg.Response))
        for idx in range(len(resp_msg.Response)):
            resp = resp_msg.Response[idx]
            spec = resp.Spec
            print ("\nEntry %d" %idx)
            print ("-----------------------");
            print ("RT: "),
            for i in spec.RT:
                mm=int(i.encode('hex'),16)
                print(hex(mm) ),
            print
            print ("Spec: RTType : %d" % spec.RTType)
    else:
        print ("Op Success")
    return

def create_req():
    req = evpn_pb2.EvpnEviRtRequest()
    req_msg = req.Request.add()
    req_msg.Id = uuid+chr(0)+chr(0)+chr(0)+chr(0)+chr(0)+chr(0)+chr(64)
    req_msg.SubnetId = subnetid+chr(0)+chr(0)+chr(0)+chr(0)+chr(0)+chr(0)+chr(64)
    req_msg.RT = rt_str
    req_msg.RTType = rttype
    resp = stub.EvpnEviRtCreate(req)
    process_response(req, resp)
    return

def update_req():
    req = evpn_pb2.EvpnEviRtRequest()
    req_msg = req.Request.add()
    req_msg.Id = uuid+chr(0)+chr(0)+chr(0)+chr(0)+chr(0)+chr(0)+chr(64)
    req_msg.SubnetId = subnetid+chr(0)+chr(0)+chr(0)+chr(0)+chr(0)+chr(0)+chr(64)
    req_msg.RT = rt_str
    req_msg.RTType = rttype
    resp = stub.EvpnEviRtUpdate(req)
    process_response(req, resp)
    return

def get_req():
    req = evpn_pb2.EvpnEviRtGetRequest()
    req_msg = req.Request.add()
    req_msg.Key.SubnetId = subnetid+chr(0)+chr(0)+chr(0)+chr(0)+chr(0)+chr(0)+chr(64)
    req_msg.Key.RT = rt_str
    resp =  stub.EvpnEviRtGet(req)
    process_response(req, resp)
    return

def get_all_req():
    req = evpn_pb2.EvpnEviRtGetRequest()
    resp =  stub.EvpnEviRtGet(req)
    process_response(req, resp)
    return

def delete_req():
    req = evpn_pb2.EvpnEviRtDeleteRequest()
    req_msg = req.Request.add()
    req_msg.Key.SubnetId = subnetid+chr(0)+chr(0)+chr(0)+chr(0)+chr(0)+chr(0)+chr(64)
    req_msg.Key.RT = rt_str
    resp = stub.EvpnEviRtDelete(req)
    process_response(req, resp)
    return

def init():
    global channel
    global stub
    server = 'localhost:' + str(defines.AGENT_GRPC_PORT)
    channel = grpc.insecure_channel(server)
    stub = evpn_pb2.EvpnSvcStub(channel)
    return

def print_help():
    print ("Usage: %s <opt> UUID SubnetId RT rttype" % sys.argv[0])
    print ("eg   : %s 1 500 500 0:2:0:0:0:0:1:100 3" %sys.argv[0])
    print ("opt  : 1: create_req\t2: update_req\t3: delete_req\t4: get_req")
    print ("empty get does get-all")
    return

def read_args():
    global opt
    global uuid
    global subnetid
    global rt_str
    global rttype
    rt_str = ""
    uuid = "0"
    subnetid = "0"
    rttype=evpn_pb2.EVPN_RT_NONE
    opt = int (sys.argv[1])
    if args > 1:
        uuid = sys.argv[2]
    if args > 2:
        subnetid = sys.argv[3]
    if args > 3:
        rt = sys.argv[4]
        for x in rt.split(':'):
            rt_str = rt_str+chr(int(x))
    if args > 4:
        rttype = int(sys.argv[5])
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
