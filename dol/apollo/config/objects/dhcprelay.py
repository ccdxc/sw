#! /usr/bin/python3
import pdb
import ipaddress

from infra.common.logging import logger

import apollo.config.resmgr as resmgr
import apollo.config.agent.api as api
import apollo.config.utils as utils
import apollo.config.objects.base as base

class DhcpRelayObject(base.ConfigObjectBase):
    def __init__(self, node, vpc, serverip, agentip):
        super().__init__(api.ObjectTypes.DHCP_RELAY, node)
        self.Id = next(resmgr.DhcpIdAllocator)
        self.GID("Dhcp%d"%self.Id)

        ########## PUBLIC ATTRIBUTES OF DHCPRELAY CONFIG OBJECT ##############
        self.Vpc = vpc
        self.ServerIp = serverip
        self.AgentIp = agentip
        ########## PRIVATE ATTRIBUTES OF DHCPRELAY CONFIG OBJECT #############
        self.Show()
        return

    def __repr__(self):
        return "DHCPRelayId%d|Vpc:%d|ServerIp:%s|AgentIp:%s" %\
               (self.Id, self.Vpc, self.ServerIp, self.AgentIp)

    def Show(self):
        logger.info("Dhcp Relay config Object: %s" % self)
        logger.info("- %s" % repr(self))
        return

    def PopulateKey(self, grpcmsg):
        grpcmsg.Id.append(str.encode(str(self.Id)))
        return

    def PopulateSpec(self, grpcmsg):
        spec = grpcmsg.Request.add()
        spec.Id = str.encode(str(self.Id))
        spec.VPCId = str.encode(str([self.Vpc]))
        utils.GetRpcIPAddr(self.ServerIp, spec.ServerIP)
        utils.GetRpcIPAddr(self.AgentIp, spec.AgentIP)
        return

    def ValidateSpec(self, spec):
        if int(spec.Id) != self.Id:
            return False
        if int(spec.VPCId) != self.Vpc:
            return False
        if spec.ServerIP != self.ServerIp:
            return False
        if spec.AgentIP != self.AgentIp:
            return False
        return True

    def ValidateYamlSpec(self, spec):
        if int(spec['id']) != self.Id:
            return False
        return True

class DhcpRelayObjectClient(base.ConfigClientBase):
    def __init__(self):
        super().__init__(api.ObjectTypes.DHCP_RELAY, resmgr.MAX_DHCP_RELAY)
        return

    def GetKeyfromSpec(self, spec, yaml=False):
        if yaml: return int(spec['id'])
        return int(spec.Id)

    def GetDhcpRelayObject(self, node):
        return self.GetObjectByKey(node, 1)

    def GenerateObjects(self, node, dhcpspec):
        def __add_dhcp_relay_config(dhcpspec):
            vpcid = dhcpspec.vpcid
            serverip = ipaddress.ip_address(dhcpspec.serverip)
            agentip = ipaddress.ip_address(dhcpspec.agentip)
            obj = DhcpRelayObject(node, vpcid, serverip, agentip)
            self.Objs[node].update({obj.Id: obj})

        if not hasattr(dhcpspec, 'dhcprelay'):
            return

        for dhcp_relay_spec_obj in dhcpspec.dhcprelay:
            __add_dhcp_relay_config(dhcp_relay_spec_obj)
        return

client = DhcpRelayObjectClient()
