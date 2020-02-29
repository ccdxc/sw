#! /usr/bin/python3
import pdb
import ipaddress
import json

from infra.common.logging import logger

from apollo.config.resmgr import client as ResmgrClient
from apollo.config.resmgr import Resmgr

import apollo.config.agent.api as api
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
        self.LocalASN = getattr(spec, "localasn", 0)
        self.RouterId = int(ipaddress.ip_address(getattr(spec, "routerid", 0)))
        self.ClusterId = getattr(spec, "clusterid", 0)
        self.Show()
        return

    def __repr__(self):
        return "BGP: %s |Id:%d|LocalASN:%d|RouterId:%s|ClusterId:%d" %\
               (self.UUID, self.Id, self.LocalASN, self.RouterId, self.ClusterId)

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
                    "ip-address": obj.PeerAddr.exploded,
                    "remote-as": obj.RemoteASN,
                    "enable-address-families": [f"{obj.PeerAf.Afi}-{obj.PeerAf.Safi}"]
                }
            peers.append(peerjson)

        spec = {
              "kind": "RoutingConfig",
              "meta": {
                "name": self.GID(),
                "tenant": "default",
                "namespace": "default",
                "uuid": self.UUID.UuidStr,
                "labels": {
                    "CreatedBy": "Venice"
                },
              },
              "spec": {
                  "bgp-config": {
                      "router-id": ipaddress.ip_address(0).exploded,
                      "as-number": self.LocalASN,
                      "neighbors" :  peers,
                  }
              }
            }
        return json.dumps(spec)

class BgpObjectClient(base.ConfigClientBase):
    def __init__(self):
        super().__init__(api.ObjectTypes.BGP, Resmgr.MAX_BGP_SESSIONS)
        return

    def GetBgpObject(self, node):
        return self.GetObjectByKey(node, 1)

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

client = BgpObjectClient()
