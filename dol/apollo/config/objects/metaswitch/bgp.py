#! /usr/bin/python3
import pdb
import ipaddress
import json

from infra.common.logging import logger

from apollo.config.resmgr import client as ResmgrClient
from apollo.config.resmgr import Resmgr

import apollo.config.agent.api as api
import apollo.config.topo as topo
import apollo.config.utils as utils
import apollo.config.objects.base as base
from apollo.config.objects.metaswitch.bgp_peer import client as BGPPeerClient

class BgpObject(base.ConfigObjectBase):
    def __init__(self, node, spec):
        super().__init__(api.ObjectTypes.BGP, node)
        self.BatchUnaware = True
        self.Id = next(ResmgrClient[node].BgpIdAllocator)
        self.GID("BGP%d"%self.Id)
        self.UUID = utils.PdsUuid(self.Id, api.ObjectTypes.BGP)
        self.Mutable = utils.IsUpdateSupported()
        self.LocalASN = getattr(spec, "localasn", 0)
        # If loopback ip exists in testbed json, use that,
        # else use from cfgyaml
        self.RouterId = utils.GetNodeLoopbackIp(node)
        if not self.RouterId:
            self.RouterId = ipaddress.ip_address(getattr(spec, "routerid", 0))
        self.RouterId = int(self.RouterId)
        self.ClusterId = getattr(spec, "clusterid", 0)
        self.KeepAliveInterval = topo.KEEPALIVE_INTERVAL
        self.HoldTime = topo.HOLD_TIME
        self.Show()
        return

    def __repr__(self):
        return "BGP: %s |Id:%d|LocalASN:%d|RouterId:%s|ClusterId:%d|KeepAliveInterval:%d|HoldTime:%d" %\
               (self.UUID, self.Id, self.LocalASN, self.RouterId, \
                self.ClusterId, self.KeepAliveInterval, self.HoldTime)

    def Show(self):
        logger.info("Bgp config Object: %s" % self)
        logger.info("- %s" % repr(self))
        return

    def PopulateKey(self, grpcmsg):
        grpcmsg.Id.append(self.GetKey())
        return

    def PopulateSpec(self, grpcmsg):
        spec = grpcmsg.Request
        spec.LocalASN = self.LocalASN
        spec.RouterId = self.RouterId
        spec.ClusterId = self.ClusterId
        spec.Id = self.GetKey()
        return

    def ValidateSpec(self, spec):
        if spec.Id != self.GetKey():
            return False
        if spec.LocalASN != self.LocalASN:
            return False
        if spec.RouterId != self.RouterId:
            return False
        if spec.ClusterId != self.ClusterId:
            return False
        return True

    def ValidateYamlSpec(self, spec):
        if spec['id'] != self.GetKey():
            return False
        return True

    def PopulateAgentJson(self):
        peers = []
        for obj in BGPPeerClient.Objects(self.Node):
            peerjson = {
                    "dsc-auto-config" : True,
                    "remote-as": obj.RemoteASN,
                    "enable-address-families": [f"{obj.PeerAf.Afi}-{obj.PeerAf.Safi}"]
                }
            peers.append(peerjson)

        spec = {
              "kind": "RoutingConfig",
              "meta": {
                "name": self.GID(),
                "tenant": self.Tenant,
                "namespace": self.Namespace,
                "uuid": self.UUID.UuidStr,
                "labels": {
                    "CreatedBy": "Venice"
                },
              },
              "spec": {
                  "bgp-config": {
                      "dsc-auto-config": True,
                      "as-number": self.LocalASN,
                      "neighbors" :  peers,
                      "keepalive-interval" : self.KeepAliveInterval,
                      "holdtime" : self.HoldTime,
                  }
              }
            }
        return json.dumps(spec)

    def CheckPeerMatch(self, cfg, operpeers):
        for obj in operpeers:
            if cfg.RemoteASN == obj['remote-as'] and \
               cfg.PeerAddr == ipaddress.ip_address(obj['ip-address']) and \
               cfg.PeerAf.AfiStr == obj['enable-address-families'][0]:
                return True
        return False

    def ValidateJSONSpec(self, spec):
        if spec['kind'] != 'RoutingConfig': return False
        if spec['meta']['name'] != self.GID(): return False
        if spec['spec']['bgp-config']['as-number'] != self.LocalASN: return False
        if int(ipaddress.ip_address(spec['spec']['bgp-config']['router-id'])) != self.RouterId:
            return False
        cfgpeers = BGPPeerClient.Objects(self.Node)
        operpeers = spec['spec']['bgp-config']['neighbors']
        if (len(cfgpeers) != len(operpeers)):
            logger.error(f"Mismatch in number of peers. cfg {len(cfgpeers)} oper {len(operpeers)}")
            return False
        for peer1 in cfgpeers:
            if not self.CheckPeerMatch(peer1, operpeers):
                return False
        return True

    def UpdateAttributes(self):
        self.Show()
        self.KeepAliveInterval = self.KeepAliveInterval + 10
        self.HoldTime = self.HoldTime + 10
        self.Show()

    def RollbackAttributes(self):
        attrlist = ["KeepAliveInterval", "HoldTime"]
        self.RollbackMany(attrlist)

class BgpObjectClient(base.ConfigClientBase):
    def __init__(self):
        super().__init__(api.ObjectTypes.BGP, Resmgr.MAX_BGP_SESSIONS)
        return

    def GetBgpObject(self, node):
        return self.GetObjectByKey(node, 1)

    def PdsctlRead(self, node):
        # TODO: Add json support
        logger.error("pdsctl read not supported for BGP object")
        return True

    def GenerateObjects(self, node, vpc, vpcspec):
        def __add_bgp_config(bgpspec):
            obj = BgpObject(node, bgpspec)
            self.Objs[node].update({obj.Id: obj})
        bgpSpec = getattr(vpcspec, 'bgpglobal', None)
        if not bgpSpec:
            logger.info(f"No BGP config in VPC {vpc.VPCId}")
            return

        for bgp_spec_obj in bgpSpec:
            __add_bgp_config(bgp_spec_obj)
        return

    def ValidateGrpcRead(self, node, getResp):
        logger.error("GRPC read not supported for BGP object")
        return True

client = BgpObjectClient()
