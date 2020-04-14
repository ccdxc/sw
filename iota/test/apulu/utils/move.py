#! /usr/bin/python3
import iota.harness.api as api
import iota.harness.infra.store as store

import iota.test.apulu.config.api as config_api
import iota.test.apulu.config.bringup_workloads as wl_api
import iota.test.utils.arping as arp
import iota.test.apollo.config.utils as utils

import iota.harness.infra.utils.parser as parser

import infra.common.objects as objects

lmap_client   = config_api.GetObjClient('lmapping')
rmap_client   = config_api.GetObjClient('rmapping')
vnic_client   = config_api.GetObjClient('vnic')
subnet_client = config_api.GetObjClient('subnet')
intf_client   = config_api.GetObjClient('interface')

def __ip_from_prefix(prefix):
    return prefix.split("/")[0]

def __add_routes(workload):

    req = api.Trigger_CreateExecuteCommandsRequest(serial=False)
    vnic = workload.vnic
    api.Logger.info(f"VNIC remote routes {vnic.RemoteRoutes}")

    for route in vnic.RemoteRoutes:
        cmd = f"ip route add {route} via {vnic.SUBNET.VirtualRouterIPAddr[1]}"
        api.Trigger_AddCommand(req, workload.node_name, workload.workload_name, cmd)

    resp = api.Trigger(req)
    for cmd in resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            api.Logger.error(f"Failed to add route {cmd.command}")
            return api.types.status.FAILURE
    return api.types.status.SUCCESS

def __dump_client_dol_db(clients=[], node=None):
    nodes = [node] if node else api.GetNaplesHostnames()
    for client in clients:
        for node in nodes:
            print(f"\n\n\n##################### {client.GetObjectType().name} DUMP {node} #####################")
            client.ShowObjects(node)
            print("\n")

def GetDestHomeAlternatives(wl):
    """
    Gets a list of alternative homes for workload.

    """
    return [node for node in api.GetNaplesHostnames() if node != wl.node_name] if wl else []

def __mapping_entry_handler(vnic, old_subnet, ep_ip_prefixes):
    """
    Generate/Destroy the lmap and rmap entries associated with vnic
    based on input EP IP entries.

    """
    api.Logger.info(f"Fixing local and remote mapping for {vnic}, ip prefixes {ep_ip_prefixes} ")
    ep_ips = set()
    for prefix in ep_ip_prefixes:
        ep_ips.add(__ip_from_prefix(prefix))
    api.Logger.info(f"IP address set: {ep_ips}")

    # Delete lmap entries
    for lmap in vnic.Children.copy():
        api.Logger.info(f"Handling lmap {lmap} {lmap.IP}")
        if lmap.IP in ep_ips:
            ep_ips.remove(lmap.IP)
        else:
            lmap.Delete()
            lmap.Destroy()
            # Delete rmap entries on other nodes
            for node in api.GetNaplesHostnames():
                if node != vnic.Node:
                    node_subnet = subnet_client.GetSubnetObject(node, vnic.SUBNET.SubnetId)
                    rmap = node_subnet.GetRemoteMappingObjectByIp(lmap.IP)
                    assert(rmap)
                    rmap.Delete()
                    rmap.Destroy()

    # Add new lmap entries
    for ep_ip in ep_ips:
        lmap_spec = dict()
        lmap_spec['origin'] = 'discovered'
        lmap_spec['lipaddr'] = ep_ip
        vnic_spec = parser.Dict2Object({'lmap': [lmap_spec]})
        lmap_client.GenerateObjects(vnic.Node, vnic, vnic_spec)

        # Add rmap entries on other nodes
        for node in api.GetNaplesHostnames():
            if node != vnic.Node:
                mac = "macaddr/%s"%vnic.MACAddr.get()
                rmap_spec = dict()
                rmap_spec['rmacaddr'] = objects.TemplateFieldObject(mac)
                rmap_spec['ripaddr'] = ep_ip
                ipversion = utils.IP_VERSION_6 if lmap.AddrFamily == 'IPV6' else utils.IP_VERSION_4
                node_subnet = subnet_client.GetSubnetObject(node, vnic.SUBNET.SubnetId)
                rmap_client.GenerateObj(node, node_subnet, rmap_spec, ipversion)

    # Dump all local and remote mappings
    #__dump_client_dol_db([lmap_client, rmap_client])

def MoveEpMACEntry(workload, target_subnet, ep_mac_addr, ep_ip_prefixes):
    """
    Moves workload to target subnet.

    """
    vnic = workload.vnic
    old_subnet = vnic.SUBNET

    # Update vnic information from target subnet
    vnic.MACAddr = objects.TemplateFieldObject(f"macaddr/{ep_mac_addr}")
    vnic.Vnid = target_subnet.Vnid
    assert(vnic and vnic.SUBNET)

    # Move vnic to target subnet
    vnic_client.ChangeSubnet(vnic, target_subnet)

    # Move around mapping entries
    __mapping_entry_handler(vnic, old_subnet, ep_ip_prefixes)

    # Delete the workload.
    wl_api.DeleteWorkload(workload)

    # Update workload object with new information
    workload.node_name = target_subnet.Node
    workload.ip_prefix = ep_ip_prefixes[0]
    workload.ip_address = __ip_from_prefix(ep_ip_prefixes[0])
    workload.sec_ip_prefixes = ep_ip_prefixes[1:]
    workload.sec_ip_addresses = [ __ip_from_prefix(prefix) for prefix in ep_ip_prefixes[1:] ]
    workload.parent_interface = intf_client.GetHostInterfaceName(target_subnet.Node, target_subnet.HostIfIdx)
    workload.interface = workload.parent_interface
    workload.mac_address = vnic.MACAddr.get()
    workload.vlan = vnic.VlanId

    # Set workload in running state, otherwise command trigger on workload will fail.
    store.SetWorkloadRunning(workload.workload_name)

    # Re add workload
    wl_api.ReAddWorkload(workload)

    # Add routes
    __add_routes(workload)

    # Send Grat ARP
    arp.SendGratArp(workload)

    return api.types.status.SUCCESS
