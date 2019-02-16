#! /usr/bin/python3
import pdb
import time
import ipaddress
import iota.harness.api as api
import iota.test.iris.testcases.penctl.common as common
import iota.harness.infra.utils.parser as parser

class MplsOverUdpTunnel:
    def __init__(self, spec, ltep, rtep):
        self.ltep = ltep
        self.rtep = rtep
        self.label_in = self.__get_label_from_ip(ltep.ip_address)
        self.label_out = self.__get_label_from_ip(rtep.ip_address)
        self.encap = spec.encap
        self.policer = spec.policer
        self.ltep.substrate_ip = str(next(spec.ip_address_pool))
        self.rtep.substrate_ip = str(next(spec.ip_address_pool))
        self.gateway_prefix = spec.substrate.ipam_base
        return

    def __get_label_from_ip(self, ipaddr):
        ipint = int(ipaddress.IPv4Address(ipaddr))
        return ipint & ((1 << 20) - 1)

    def __add_static_arp(self, req):
        api.Trigger_AddHostCommand(req, self.ltep.node_name,
                    "arp -s %s %s" % (self.rtep.ip_address, self.rtep.mac_address))
        return

    def Create(self, req):
        cmd = "update interface --name %s " % self.ltep.interface
        cmd += "--gw-mac %s " % self.rtep.mac_address
        cmd += "--substrate-ip %s " % self.ltep.ip_address
        cmd += "--overlay-ip %s " % self.ltep.ip_address
        cmd += "--mpls-in %d " % self.label_in
        cmd += "--mpls-out %d " % self.label_out
        cmd += "--tunnel-dest-ip %s " % self.rtep.ip_address
        cmd += "--source-gw %s " % self.gateway_prefix
        cmd += "--ingress-bw %d " % self.policer.ingress
        cmd += "--egress-bw %d " % self.policer.egress
        cmd += "--encap %s " % self.encap

        api.Logger.info("Creating Tunnel %s ==> %s" % (self.ltep.workload_name, self.rtep.workload_name))
        api.Logger.info("- command: %s" % cmd)
        common.AddPenctlCommand(req, self.ltep.node_name, cmd)
        self.__add_static_arp(req)
        return api.types.status.SUCCESS

    def Delete(self, req):
        cmd = "delete interface --name %s " % self.ltep.interface
        cmd += "--encap %s " % self.encap
        api.Logger.info("Deleting Tunnel on interface %s" % (self.ltep.interface))
        api.Logger.info("- command: %s" % cmd)
        common.AddPenctlCommand(req, self.ltep.node_name, cmd)
        self.__add_static_arp(req)
        return api.types.status.SUCCESS

class MplsOverUdpTunnelManager:
    def __init__(self):
        self.__spec = self.__parse_tunnel_config_yml()
        self.__tunnels = None
        return

    def __parse_tunnel_config_yml(self):
        config_yml = "{}/config.yml".format(api.GetTopologyDirectory())
        spec = parser.YmlParse(config_yml)
        tunnel_spec = spec.tunnels
        tunnel_spec.ip_address_pool = ipaddress.IPv4Network(tunnel_spec.substrate.ipam_base).hosts()
        return tunnel_spec

    def __process_req(self, req):
        #Now Send all the commands
        resp = api.Trigger(req)
        if resp == None:
            return api.types.status.FAILURE

        result = api.types.status.SUCCESS
        for cmd in resp.commands:
            api.PrintCommandResults(cmd)
            if cmd.exit_code != 0:
                result = api.types.status.FAILURE

        return result

    def CreateTunnels(self):
        nodes = api.GetNaplesHostnames()
        # Atleast 2 Naples nodes should be present
        assert(len(nodes) >= 2)
        wlpool1 = api.GetWorkloads(nodes[0])
        wlpool2 = api.GetWorkloads(nodes[1])
        assert(len(wlpool1) == len(wlpool2))

        req = api.Trigger_CreateAllParallelCommandsRequest()
        tunnels = []
        for wid in range(len(wlpool1)):
            # Create tunnel in local node
            tunnel = MplsOverUdpTunnel(self.__spec, wlpool1[wid], wlpool2[wid])
            tunnel.Create(req)
            tunnels.append(tunnel)
            # Create tunnel in remote node
            tunnel = MplsOverUdpTunnel(self.__spec, wlpool2[wid], wlpool1[wid])
            tunnel.Create(req)
            tunnels.append(tunnel)

        self.__tunnels = tunnels
        return self.__process_req(req)

    def DeleteTunnels(self):
        req = api.Trigger_CreateAllParallelCommandsRequest()
        for tunnel in self.__tunnels:
            tunnel.Delete(req)
        return self.__process_req(req)

    def GetTunnels(self):
        return self.__tunnels

def Init():
    tmgr = MplsOverUdpTunnelManager()
    api.SetTestsuiteAttr("MPLSUDP_TUNNEL_MANAGER", tmgr)
    return api.types.status.SUCCESS

def GetTunnelManager():
    return api.GetTestsuiteAttr("MPLSUDP_TUNNEL_MANAGER")

def GetTunnels():
    return GetTunnelManager().GetTunnels()
