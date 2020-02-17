#! /usr/bin/python3
import pdb
import ipaddress

from infra.common.logging import logger

from apollo.config.resmgr import client as ResmgrClient
from apollo.config.resmgr import Resmgr
from apollo.config.objects.metaswitch.bgp_peeraf import BgpPeerAfObject
from apollo.config.objects.metaswitch.bgp_peeraf import client as BgpPeerAfObjectClient

import apollo.config.agent.api as api
import apollo.config.utils as utils
import apollo.config.objects.base as base
import bgp_pb2 as bgp_pb2

class BgpPeerObject(base.ConfigObjectBase):
    def __init__(self, node, spec, peeraf_obj):
        super().__init__(api.ObjectTypes.BGP_PEER, node)
        self.BatchUnaware = True
        self.Id = next(ResmgrClient[node].BgpPeerIdAllocator)
        self.UUID = utils.PdsUuid(self.Id)
        self.GID("BGPPeer%d"%self.Id)
        self.State = getattr(spec, "adminstate", None)
        self.LocalAddr = ipaddress.ip_address(getattr(spec, "localaddr", None))
        self.PeerAddr = ipaddress.ip_address(getattr(spec, "peeraddr", None))
        self.RemoteASN = getattr(spec, "remoteasn", 0)
        self.SendComm = getattr(spec, "sendcomm", False)
        self.SendExtComm = getattr(spec, "sendextcomm", False)
        self.RRClient = getattr(spec, "rrclient", None)
        self.ConnectRetry = getattr(spec, "connectretry", 0)
        self.HoldTime = getattr(spec, "holdtime", 0)
        self.KeepAlive = getattr(spec, "keepalive", 0)
        self.Password = ""
        #self.Password = getattr(spec, "password", "")
        self.PeerAf = peeraf_obj
        self.Show()
        return

    def __repr__(self):
        return "BGPPeer: %s |Id:%d|Localaddr:%s|PeerAddr:%s|RemoteASN:%d|" \
               "SendComm:%s|SendExtComm:%s|RRClient:%s|ConnectRetry:%d|"   \
               "HoldTime:%d|KeepAlive:%d|Password:%s" % \
               (self.UUID, self.Id, self.LocalAddr, self.PeerAddr, self.RemoteASN, \
                self.SendComm, self.SendExtComm, self.RRClient, self.ConnectRetry, \
                self.HoldTime, self.KeepAlive, self.Password)

    def Show(self):
        logger.info("BGP Peer Object: %s" % self)
        logger.info("- %s" % repr(self))
        return

    def PopulateKey(self, grpcmsg):
        grpcmsg.Id.append(self.GetKey())
        return

    def GetPeerRRClient(self):
        if self.RRClient == "client":
            return bgp_pb2.BGP_PEER_RR_CLIENT
        elif self.RRClient == "mesh":
            return bgp_pb2.BGP_PEER_RR_MESHED_CLIENT
        else:
            return bgp_pb2.BGP_PEER_RR_NONE

    def PopulateSpec(self, grpcmsg):
        spec = grpcmsg.Request.add()
        utils.GetRpcIPAddr(self.LocalAddr, spec.LocalAddr)
        utils.GetRpcIPAddr(self.PeerAddr, spec.PeerAddr)
        spec.RemoteASN = self.RemoteASN
        spec.SendComm = self.SendComm
        spec.SendExtComm = self.SendExtComm
        spec.RRClient = self.GetPeerRRClient()
        spec.ConnectRetry = self.ConnectRetry
        spec.HoldTime = self.HoldTime
        spec.KeepAlive = self.KeepAlive
        #spec.Password = self.Password
        spec.Id = self.GetKey()
        return

    def ValidateSpec(self, spec):
        if spec.Id != self.GetKey():
            return False
        if spec.LocalAddr != self.LocalAddr:
            return False
        if spec.PeerAddr != self.PeerAddr:
            return False
        if spec.RemoteASN != self.RemoteASN:
            return False
        if spec.SendComm != self.SendComm:
            return False
        if spec.SendExtComm != self.SendExtComm:
            return False
        if spec.RRClient != self.RRClient:
            return False
        if spec.ConnectRetry != self.ConnectRetry:
            return False
        if spec.HoldTime != self.HoldTime:
            return False
        if spec.KeepAlive != self.KeepAlive:
            return False
        if spec.Password != self.Password:
            return False
        return True

    def ValidateYamlSpec(self, spec):
        if spec['id'] != self.GetKey():
            return False
        return True

class BgpPeerObjectClient(base.ConfigClientBase):
    def __init__(self):
        super().__init__(api.ObjectTypes.BGP_PEER, Resmgr.MAX_BGP_PEERS)
        return

    def GetBgpPeerObject(self, node):
        return self.GetObjectByKey(node, 1)

    def GenerateObjects(self, node, parent, vpcspec):
        def __add_bgp_peer(peerspec):
            peerafspec = getattr(peerspec, "bgppeeraf", None)
            peeraf_obj = BgpPeerAfObject(node, peerafspec)
            BgpPeerAfObjectClient.Objs[node].update({peeraf_obj.Id: peeraf_obj})
            obj = BgpPeerObject(node, peerspec, peeraf_obj)
            self.Objs[node].update({obj.Id: obj})

        bgpPeer = getattr(vpcspec, 'bgppeer', None)
        if not bgpPeer:
            logger.info("No BGP peer config in topology")
            return

        for peerspec in bgpPeer:
            __add_bgp_peer(peerspec)
        return

client = BgpPeerObjectClient()
