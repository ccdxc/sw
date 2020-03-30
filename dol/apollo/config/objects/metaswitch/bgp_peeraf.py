#! /usr/bin/python3
import pdb
import ipaddress

from infra.common.logging import logger

from apollo.config.resmgr import client as ResmgrClient
from apollo.config.resmgr import Resmgr

import apollo.config.agent.api as api
import apollo.config.utils as utils
import apollo.config.objects.base as base
import bgp_pb2 as bgp_pb2

class BgpPeerAfObject(base.ConfigObjectBase):
    def __init__(self, node, spec):
        super().__init__(api.ObjectTypes.BGP_PEER_AF, node)
        self.BatchUnaware = True
        self.Id = next(ResmgrClient[node].BgpPeerAfIdAllocator)
        self.UUID = utils.PdsUuid(self.Id, api.ObjectTypes.BGP_PEER_AF)
        self.GID("BGPPeerAf%d"%self.Id)
        self.PeerAddr = None
        if hasattr(spec, 'interface'):
            # override IPs from testbed json
            self.LocalAddr = utils.GetNodeUnderlayIp(node, spec.interface)
            self.PeerAddr = utils.GetNodeUnderlayNexthop(node, spec.interface)
        else:
            self.LocalAddr = ipaddress.ip_address(getattr(spec, "localaddr", "0.0.0.0"))

        if self.PeerAddr == None:
            self.PeerAddr = ipaddress.ip_address(getattr(spec, "peeraddr", "0.0.0.0"))
        self.Afi = getattr(spec, "afi", "ipv4")
        self.Safi = getattr(spec, "safi", "unicast")
        self.AfiStr = f"{self.Afi}-{self.Safi}"
        self.NexthopSelf = getattr(spec, "nexthopself", False)
        self.DefaultOrig = getattr(spec, "defaultorig", False)
        self.Show()
        return

    def __repr__(self):
        return "BGPPeerAf: %s |Id:%d|Localaddr:%s|PeerAddr:%s|Afi:%s|Safi:%s|"\
               "%s|Nexthopself:%d|DefaultOrig:%s" %\
               (self.UUID, self.Id, self.LocalAddr, self.PeerAddr, self.Afi, \
                self.Safi, self.AfiStr, self.NexthopSelf, \
                self.DefaultOrig)

    def Show(self):
        logger.info("BGP Peer Af Object: %s" % self)
        logger.info("- %s" % repr(self))
        return

    def PopulateKey(self, grpcmsg):
        grpcmsg.Id.append(self.GetKey())
        return

    def GetAfi(self):
        if self.Afi == "ipv4":
            return bgp_pb2.BGP_AFI_IPV4
        elif self.Afi == "ipv6":
            return bgp_pb2.BGP_AFI_IPV6
        elif self.Afi == "l2vpn":
            return bgp_pb2.BGP_AFI_L2VPN
        else:
            return bgp_pb2.BGP_AFI_NONE

    def GetSafi(self):
        if self.Safi == "unicast":
            return bgp_pb2.BGP_SAFI_UNICAST
        elif self.Safi == "multicast":
            return bgp_pb2.BGP_SAFI_MULTICAST
        elif self.Safi == "both":
            return bgp_pb2.BGP_SAFI_BOTH
        elif self.Safi == "label":
            return bgp_pb2.BGP_SAFI_LABEL
        elif self.Safi == "vpls":
            return bgp_pb2.BGP_SAFI_VPLS
        elif self.Safi == "evpn":
            return bgp_pb2.BGP_SAFI_EVPN
        elif self.Safi == "mpls":
            return bgp_pb2.BGP_SAFI_MPLS_BGP_VPN
        elif self.Safi == "private":
            return bgp_pb2.BGP_SAFI_PRIVATE
        else:
            return bgp_pb2.BGP_SAFI_NONE

    def PopulateSpec(self, grpcmsg):
        spec = grpcmsg.Request.add()
        utils.GetRpcIPAddr(self.LocalAddr, spec.LocalAddr)
        utils.GetRpcIPAddr(self.PeerAddr, spec.PeerAddr)

        spec.Afi = self.GetAfi()
        spec.Safi = self.GetSafi()
        spec.NexthopSelf = self.NexthopSelf
        spec.DefaultOrig = self.DefaultOrig
        spec.Id = self.GetKey()
        return

    def ValidateSpec(self, spec):
        if spec.Id != self.GetKey():
            return False
        if spec.LocalAddr != self.LocalAddr:
            return False
        if spec.PeerAddr != self.PeerAddr:
            return False
        if spec.Afi != self.Afi:
            return False
        if spec.Safi != self.Safi:
            return False
        if spec.Nexthopself != self.Nexthopself:
            return False
        if spec.DefaultOrig != self.DefaultOrig:
            return False
        return True

    def ValidateYamlSpec(self, spec):
        if spec['id'] != self.GetKey():
            return False
        return True

class BgpPeerAfObjectClient(base.ConfigClientBase):
    def __init__(self):
        super().__init__(api.ObjectTypes.BGP_PEER_AF, Resmgr.MAX_BGP_PEER_AF_SESSIONS)
        return

    def GetBgpPeerAfObject(self, node, peerafid):
        return self.GetObjectByKey(node, peerafid)

    def IsReadSupported(self):
        return False

    def PdsctlRead(self, node):
        return True

    """
    def GenerateObjects(self, node, parent, vpcspec):
        def __add_bgp_peer(peerspec):
            obj = BgpPeerAfObject(node, peer)
            self.Objs[node].update({obj.Id: obj})

        bgpPeer = getattr(vpcspec, 'bgppeer', None)
        if not bgpPeer:
            logger.info("No BGP peer config in topology")
            return

        for peerspec in bgpPeer:
            __add_bgp_peer(peerspec)
        return
    """

client = BgpPeerAfObjectClient()
