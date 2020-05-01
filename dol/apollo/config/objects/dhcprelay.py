#! /usr/bin/python3
import ipaddress
import json
import copy

from infra.common.logging import logger

from apollo.config.store import client as EzAccessStoreClient
from apollo.config.store import EzAccessStore
from apollo.config.resmgr import client as ResmgrClient
from apollo.config.resmgr import Resmgr

import apollo.config.agent.api as api
import apollo.config.utils as utils
import apollo.config.objects.base as base

class DhcpRelayObject(base.ConfigObjectBase):
    def __init__(self, node, vpc, serverip, agentip):
        super().__init__(api.ObjectTypes.DHCP_RELAY, node)
        self.Id = next(ResmgrClient[node].DhcpIdAllocator)
        self.GID("Dhcp%d"%self.Id)
        self.UUID = utils.PdsUuid(self.Id, self.ObjType)
        ########## PUBLIC ATTRIBUTES OF DHCPRELAY CONFIG OBJECT ##############
        self.Vpc = vpc
        self.ServerIp = serverip
        self.AgentIp = agentip
        ########## PRIVATE ATTRIBUTES OF DHCPRELAY CONFIG OBJECT #############
        self.Show()
        return

    def __repr__(self):
        return "DHCPRelay: %s |Vpc:%d|ServerIp:%s|AgentIp:%s" %\
               (self.UUID, self.Vpc, self.ServerIp, self.AgentIp)

    def Show(self):
        logger.info("Dhcp Relay config Object: %s" % self)
        logger.info("- %s" % repr(self))
        return

    def PopulateKey(self, grpcmsg):
        grpcmsg.Id.append(self.GetKey())
        return

    def UpdateAttributes(self):
        self.ServerIp = self.ServerIp + 1

    def RollbackAttributes(self):
        attrlist = ["ServerIp"]
        self.RollbackMany(attrlist)
        return

    def PopulateSpec(self, grpcmsg):
        spec = grpcmsg.Request.add()
        spec.Id = self.GetKey()
        relaySpec = spec.RelaySpec
        relaySpec.VPCId = utils.PdsUuid.GetUUIDfromId(self.Vpc, api.ObjectTypes.VPC)
        utils.GetRpcIPAddr(self.ServerIp, relaySpec.ServerIP)
        utils.GetRpcIPAddr(self.AgentIp, relaySpec.AgentIP)
        return

    def PopulateAgentJson(self):
        #TODO revisit in case of multiple dhcp servers
        servers = []
        serverjson = {
            "ip-address": self.ServerIp.exploded,
            "virtual-router": str(self.Vpc)
        }
        servers.append(serverjson)
        spec = {
                "kind": "IPAMPolicy",
                "meta": {
                    "name": self.GID(),
                    "namespace": self.Namespace,
                    "tenant": self.Tenant,
                    "uuid" : self.UUID.UuidStr,
                    "labels": {
                        "CreatedBy": "Venice"
                    },
                },
                "spec": {
                    "type": "0",
                    "dhcp-relay": {
                        "relay-servers": servers
                    }
                }
            }
        return json.dumps(spec)

    def CheckServerMatch(self, cfg, operservers):
        for obj in operservers:
            if cfg['ip-address'] == obj['ip-address'] and \
               cfg['virtual-router'] == obj['virtual-router']:
                return True
        return False

    def ValidateJSONSpec(self, spec):
        if spec['kind'] != 'IPAMPolicy': return False
        if spec['meta']['name'] != self.GID(): return False
        if spec['spec']['spec']['type'] !=  0: return False
        operservers =  spec['spec']['spec']['relay-servers']
        cfgservers = []
        serverjson = {
            "ip-address": self.ServerIp.exploded,
            "virtual-router": str(self.Vpc),
        }
        cfgservers.append(serverjson)
        if (len(cfgservers) != len(operservers)):
            logger.error(f"Mismatch in number of servers. cfg {len(cfgservers)}\
                 oper {len(operservers)}")
            return False
        for server in cfgservers:
            if not self.CheckServerMatch(server, operservers):
                return False
        return True

    def ValidateSpec(self, spec):
        if spec.Id != self.GetKey():
            return False
        relaySpec = spec.RelaySpec
        if relaySpec.VPCId != utils.PdsUuid.GetUUIDfromId(self.Vpc, api.ObjectTypes.VPC):
            return False
        if relaySpec.ServerIP != self.ServerIp:
            return False
        if relaySpec.AgentIP != self.AgentIp:
            return False
        return True

    def ValidateYamlSpec(self, spec):
        if utils.GetYamlSpecAttr(spec) != self.GetKey():
            return False
        return True

class DhcpRelayObjectClient(base.ConfigClientBase):
    def __init__(self):
        super().__init__(api.ObjectTypes.DHCP_RELAY, Resmgr.MAX_DHCP_RELAY)
        return

    def GetDhcpRelayObject(self, node, dhcprelayid=1):
        #TODO: Fix flow.py for sending dhcprelayid
        return self.GetObjectByKey(node, dhcprelayid)

    def PdsctlRead(self, node):
        # pdsctl show not supported for dhcp policy yet
        return True

    def GrpcRead(self, node):
        # grpc read not supported for dhcp policy yet
        return True

    def IsReadSupported(self):
        if utils.IsNetAgentMode():
            return True
        return False

    def GenerateObjects(self, node, vpcid, topospec=None):
        def __add_dhcp_relay_config(node, dhcpobj):
            serverip = ipaddress.ip_address(dhcpobj.serverip)
            agentip = ipaddress.ip_address(dhcpobj.agentip)
            obj = DhcpRelayObject(node, vpcid, serverip, agentip)
            self.Objs[node].update({obj.Id: obj})

        dhcprelaySpec = None
        if topospec:
            dhcprelaySpec = getattr(topospec, 'dhcprelay', None)

        if not dhcprelaySpec:
            tbSpec = EzAccessStore.GetTestbedSpec()
            dhcprelaySpec = getattr(tbSpec, 'DHCPRelay', None)
            if dhcprelaySpec:
                for obj in dhcprelaySpec:
                    attrb = copy.copy(vars(obj))
                    for key, val in attrb.items():
                        setattr(obj, key.lower(), val)
            else:
                return

        for dhcp_relay_spec_obj in dhcprelaySpec:
            __add_dhcp_relay_config(node, dhcp_relay_spec_obj)
        EzAccessStoreClient[node].SetDhcpRelayObjects(self.Objects(node))
        ResmgrClient[node].CreateDHCPRelayAllocator()
        return

client = DhcpRelayObjectClient()
