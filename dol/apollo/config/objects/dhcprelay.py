#! /usr/bin/python3
import pdb
import ipaddress

from infra.common.logging import logger

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

    def PopulateSpec(self, grpcmsg):
        spec = grpcmsg.Request.add()
        spec.Id = self.GetKey()
        relaySpec = spec.RelaySpec
        relaySpec.VPCId = utils.PdsUuid.GetUUIDfromId(self.Vpc, api.ObjectTypes.VPC)
        utils.GetRpcIPAddr(self.ServerIp, relaySpec.ServerIP)
        utils.GetRpcIPAddr(self.AgentIp, relaySpec.AgentIP)
        return

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
        if spec['id'] != self.GetKey():
            return False
        return True

class DhcpRelayObjectClient(base.ConfigClientBase):
    def __init__(self):
        super().__init__(api.ObjectTypes.DHCP_RELAY, Resmgr.MAX_DHCP_RELAY)
        return

    def GetDhcpRelayObject(self, node):
        return self.GetObjectByKey(node, 1)

    def GenerateObjects(self, node, dhcpspec):
        def __add_dhcp_relay_config(dhcpspec):
            vpcid = dhcpspec.vpcid
            serverip = ipaddress.ip_address(dhcpspec.serverip)
            agentip = ipaddress.ip_address(dhcpspec.agentip)
            obj = DhcpRelayObject(node, vpcid, serverip, agentip)
            self.Objs[node].update({obj.Id: obj})

        dhcprelaySpec = getattr(dhcpspec, 'dhcprelay', None)
        if not dhcprelaySpec:
            return

        for dhcp_relay_spec_obj in dhcprelaySpec:
            __add_dhcp_relay_config(dhcp_relay_spec_obj)
        return

client = DhcpRelayObjectClient()
