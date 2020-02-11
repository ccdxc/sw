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
        print ("Number of entries retrieved: %d" % len(resp_msg.Response))
        for idx in range(len(resp_msg.Response)):
            resp = resp_msg.Response[idx]
            st = resp.Status
            print ("\nEntry %d" %idx)
            print ("-----------------------");
            print ("Afi: %d" % st.Afi)
            print ("Safi: %d" % st.Safi)
            print ("Prefix-Len: %d" % len(st.Prefix))
            print ("Prefix: "),
            for i in st.Prefix:
                mm=int(i.encode('hex'),16)
                print(hex(mm) ),
            print
            print ("PrefixLen: %d" % st.PrefixLen)
            print ("RouteSource: %d" % st.RouteSource)
            print ("RouteSourceIndex: %d" % st.RouteSourceIndex)
            print ("PathID: %d" % st.PathID)
            print ("BestRoute: %d "% st.BestRoute)
            print ("ASPathStr: "),
            for i in st.ASPathStr:
                mm=int(i.encode('hex'),16)
                print(hex(mm) ),
            print
            print ("PathOrigId: %s"% st.PathOrigId)
            print ("NexthopAddr: "),
            for i in st.NextHopAddr:
                mm=int(i.encode('hex'), 16)
                print((mm) ),
            print
    else:
        print ("Op Success")
    return

def get_nlri():
    req = bgp_pb2.BGPNLRIPrefixRequest()
    resp =  stub.BGPNLRIPrefixGet(req)     
    process_response(req, resp)
    return

def init():
    global channel
    global stub
    server = 'localhost:50054'
    channel = grpc.insecure_channel(server)
    stub = bgp_pb2.BGPSvcStub(channel)
    return

def ip2long(ip):
    packedIP = socket.inet_aton(ip)
    return struct.unpack("!L", packedIP)[0] 

def long2ip(addr):
    return socket.inet_ntoa(struct.pack('!L', addr))

if __name__ == '__main__':
    args = len (sys.argv) - 1
    if args:
        print_help()
    else:
        init()
        get_nlri()
