#! /usr/bin/python3
import pdb
import time
import ipaddress
import iota.harness.api as api
import iota.test.iris.testcases.penctl.common as common
import iota.harness.infra.utils.parser as parser

class TunnelMplsOverUdp:
    def __init__(self):
        return

def __parse_tunnel_config_yml():
    config_yml = "{}/config.yml".format(api.GetTopologyDirectory())
    spec = parser.YmlParse(config_yml)
    tunnel_spec = spec.tunnels
    tunnel_spec.ip_address_pool = ipaddress.IPv4Network(tunnel_spec.substrate.ipam_base).hosts()
    return tunnel_spec

def __get_create_tunnel_command(tunnel, src, dst, label_in, label_out):
    cmd = "update interface --name %s " % src.interface
    cmd += "--gw-mac %s " % dst.mac_address
    cmd += "--substrate-ip %s " % src.ip_address
    cmd += "--overlay-ip %s " % src.ip_address
    cmd += "--mpls-in %d " % label_in
    cmd += "--mpls-out %d " % label_out
    cmd += "--tunnel-dest-ip %s " % dst.ip_address
    cmd += "--source-gw %s " % tunnel.gateway_prefix
    cmd += "--ingress-bw %d " % tunnel.policer.ingress
    cmd += "--egress-bw %d " % tunnel.policer.egress
    cmd += "--encap %s " % tunnel.encap
    return cmd

def Main(tc):
    nodes = api.GetNaplesHostnames()
    spec = __parse_tunnel_config_yml()

    # Select two nodes
    node1 = nodes[0]
    node2 = nodes[1]

    node1_workloads = api.GetWorkloads(node1)
    node2_workloads = api.GetWorkloads(node2)
    assert(len(node1_workloads) == len(node2_workloads))

    req = api.Trigger_CreateExecuteCommandsRequest(serial = False)
    tunnels = []
    for wid in range(len(node1_workloads)):
        tunnel = TunnelMplsOverUdp()
        tunnel.w1 = node1_workloads[wid]
        tunnel.w2 = node2_workloads[wid]
        tunnel.encap = spec.encap
        tunnel.policer = spec.policer
        tunnel.labels = spec.labels
        tunnel.w1.substrate_ip = str(next(spec.ip_address_pool))
        tunnel.w2.substrate_ip = str(next(spec.ip_address_pool))
        tunnel.gateway_prefix = spec.substrate.ipam_base

        command = __get_create_tunnel_command(tunnel, tunnel.w1, tunnel.w2,
                                              spec.labels.ingress, spec.labels.egress)
        api.Logger.info("Creating Tunnel %s ==> %s" % (tunnel.w1.workload_name, tunnel.w2.workload_name))
        api.Logger.info("- command: %s" % command)
        common.AddPenctlCommand(req, tunnel.w1.node_name, command)
        api.Trigger_AddHostCommand(req, tunnel.w1.node_name,
                                   "arp -s %s %s" % (tunnel.w2.ip_address, tunnel.w2.mac_address))
                               
        
        command = __get_create_tunnel_command(tunnel, tunnel.w2, tunnel.w1,
                                              spec.labels.egress, spec.labels.ingress)
        api.Logger.info("Creating Tunnel %s ==> %s" % (tunnel.w2.workload_name, tunnel.w1.workload_name))
        api.Logger.info("- command: %s" % command)
        common.AddPenctlCommand(req, tunnel.w2.node_name, command)
        api.Trigger_AddHostCommand(req, tunnel.w2.node_name,
                                   "arp -s %s %s" % (tunnel.w1.ip_address, tunnel.w1.mac_address))
        tunnels.append(tunnel)

    api.SetTestsuiteAttr("TUNNELS_MPLSUDP", tunnels)

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
