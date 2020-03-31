#! /usr/bin/python3
import pdb
import utils
import re
import socket

import bgp_pb2 as bgp_pb2
import types_pb2 as types_pb2

import api

class BgpObject():
    def __init__(self, id, local_asn, router_id):
        super().__init__()
        self.id = id
        self.uuid = utils.PdsUuid(self.id, objtype=api.ObjectTypes.BGP)
        self.local_asn = local_asn
        self.router_id = router_id
        return

    def GetGrpcCreateMessage(self):
        grpcmsg = bgp_pb2.BGPRequest()
        spec = grpcmsg.Request
        spec.Id = self.uuid.GetUuid()
        spec.LocalASN = self.local_asn
        spec.RouterId = int(self.router_id)
        return grpcmsg

class BgpPeerObject():
    def __init__(self, id, local_addr, peer_addr, admin_state, send_comm, send_ext_comm, connect_retry, rrclient, remote_asn, holdtime, keepalive):
        super().__init__()
        self.id = id
        self.uuid = utils.PdsUuid(self.id, objtype=api.ObjectTypes.BGP_PEER)
        self.local_addr = local_addr
        self.peer_addr = peer_addr
        self.admin_state = admin_state
        self.send_comm = send_comm
        self.send_ext_comm = send_ext_comm
        self.connect_retry = connect_retry
        if rrclient == "client":
            self.rrclient = bgp_pb2.BGP_PEER_RR_CLIENT
        elif rrclient == "mesh":
            self.rrclient = bgp_pb2.BGP_PEER_RR_MESHED_CLIENT
        else:
            self.rrclient = bgp_pb2.BGP_PEER_RR_NONE
        self.remote_asn = remote_asn
        self.holdtime = holdtime
        self.keepalive = keepalive
        return

    def GetGrpcCreateMessage(self):
        grpcmsg = bgp_pb2.BGPPeerRequest()
        spec = grpcmsg.Request.add()
        spec.Id = self.uuid.GetUuid()
        spec.LocalAddr.Af = types_pb2.IP_AF_INET
        spec.LocalAddr.V4Addr = int(self.local_addr)
        spec.PeerAddr.Af = types_pb2.IP_AF_INET
        spec.PeerAddr.V4Addr = int(self.peer_addr)
        spec.RemoteASN = self.remote_asn
        spec.SendComm = self.send_comm
        spec.SendExtComm = self.send_ext_comm
        spec.RRClient = self.rrclient
        spec.ConnectRetry = self.connect_retry
        spec.HoldTime = self.holdtime
        spec.KeepAlive = self.keepalive
        spec.State = self.admin_state
        return grpcmsg

class BgpPeerAfObject():
    def __init__(self, id, local_addr, peer_addr):
        super().__init__()
        self.id = id
        self.uuid = utils.PdsUuid(self.id, objtype=api.ObjectTypes.BGP_PEER_AF)
        self.local_addr = local_addr
        self.peer_addr = peer_addr
        return

    def GetGrpcCreateMessage(self):
        grpcmsg = bgp_pb2.BGPPeerAfRequest()
        spec = grpcmsg.Request.add()
        spec.Id = self.uuid.GetUuid()
        spec.LocalAddr.Af = types_pb2.IP_AF_INET
        spec.LocalAddr.V4Addr = int(self.local_addr)
        spec.PeerAddr.Af = types_pb2.IP_AF_INET
        spec.PeerAddr.V4Addr = int(self.peer_addr)
        spec.Afi = bgp_pb2.BGP_AFI_IPV4
        spec.Safi = bgp_pb2.BGP_SAFI_UNICAST
        return grpcmsg

