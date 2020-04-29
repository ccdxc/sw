# Import all the config objects

import api
import vpc
import device
import batch
import tunnel
import route
import vnic
import mapping
import dhcp
import subnet
import policy
import mirror
import interface
import nh
import node
import nat
import bgp
import service_mapping

import netaddr
import argparse
import os
import time
import sys
import pdb

# Import types and ipaddress

import types_pb2
import interface_pb2
import vpc_pb2
import tunnel_pb2
import ipaddress

# Parse argument
parser = argparse.ArgumentParser()
parser.add_argument("naples_ip", help="naples ip address")
parser.add_argument("--grpc_port", help="naples grpc port (default=50054)", default=50054, type=str)
args = parser.parse_args()
naplesip = args.naples_ip
naplesport = args.grpc_port
os.environ['AGENT_GRPC_IP'] = naplesip
os.environ['AGENT_GRPC_PORT'] = naplesport

#Variables

# node uuid
node_uuid=node.NodeObject().GetNodeMac()

# Device object inputs
local_tep_ip='13.13.13.13'

# VPC object inputs
vpc1_id=1
vpc1_vxlan_encap=2002
vpc2_id=100
vpc2_vxlan_encap=1002

# L3-interface (underlay) objects
loopback_prefix=local_tep_ip + '/32'
intf1_prefix='13.1.0.1/24'
intf2_prefix='13.2.0.1/24'

# BGP config
local_asn = 100
router_id = ipaddress.IPv4Address(local_tep_ip)

local_addr1 = ipaddress.IPv4Address('13.1.0.1')
peer_addr1 = ipaddress.IPv4Address('13.1.0.2')
admin_state = 1
send_comm = True
send_ext_comm = True
connect_retry = 5
rrclient = False
remote_asn = 200
holdtime = 180
keepalive = 60

local_addr2 = ipaddress.IPv4Address('13.2.0.1')
peer_addr2 = ipaddress.IPv4Address('13.2.0.2')

# Tunnel object inputs (Underlay)
tunnel_id=1
tunnel_local_ip=local_tep_ip
tunnel_remote_ip='14.14.14.14'
tunnel_vnid=0

# VCN objects
vcn_vnic_id = 100
vcn_subnet_id = 100
vcn_subnet_pfx='11.0.0.0/8'
vcn_host_if_idx='0x80000046'
vcn_intf_prefix='11.1.1.2/8'
vcn_intf_ip=ipaddress.IPv4Address('11.1.1.2')
vcn_v4_router_ip=ipaddress.IPv4Address('11.1.1.1')
vcn_vpc_encap=11
vcn_subnet_encap=12
vcn_vpc_id=11
vcn_virt_router_mac='00:66:01:00:00:01'
vcn_vnic_mac='fe:ff:0b:01:01:02'
vcn_route_prefix1='11.1.2.0/24'
vcn_route_table_id=100

# Subnet object inputs
ipv4_subnet1='2.1.0.0/24'
# The host_if_idx is an encoding for PF1
subnet1_host_if_idx='0x80000047'
subnet1_fabric_encap=202
subnet1_v4_router_ip=ipaddress.IPv4Address('2.1.0.1')
subnet1_virt_router_mac='00:55:01:00:00:01'
subnet1_route_prefix1='64.0.0.0/24'
subnet1_route_table_id=1
subnet1_gw_ip_addr=ipaddress.IPv4Address('2.1.0.1')

# VNIC and mapping (local and remote) table objects 
subnet1_local_vnic_mac='00:ae:cd:00:08:11'
subnet1_remote_vnic_mac='00:ae:cd:00:00:0a'
subnet1_local_host_ip=ipaddress.IPv4Address('2.1.0.2')
subnet1_remote_host_ip=ipaddress.IPv4Address('2.1.0.3')

# Subnet object inputs
ipv4_subnet2='3.1.0.0/24'
# The host_if_idx is an encoding for PF1
subnet2_host_if_idx='0x80000048'
subnet2_fabric_encap=203
subnet2_v4_router_ip=ipaddress.IPv4Address('3.1.0.1')
subnet2_virt_router_mac='00:55:02:00:00:01'
subnet2_route_prefix1='64.0.0.0/24'
subnet2_route_table_id=2

# VNIC and mapping (local and remote) table objects 
subnet2_local_vnic_mac='00:ae:cd:00:08:12'
subnet2_remote_vnic_mac='00:ae:cd:00:00:0b'
subnet2_local_host_ip=ipaddress.IPv4Address('3.1.0.2')
subnet2_local_vnic_public_ip=ipaddress.IPv4Address('50.0.0.100')
subnet2_backend_ip=subnet2_local_vnic_public_ip
subnet2_backend_port=9999
subnet2_vip=ipaddress.IPv4Address('100.0.0.100')
subnet2_service_port=10000

# NAT Prefix
nat_prefix=ipaddress.IPv4Network('50.0.0.1/32')
nat_port_lo=10000
nat_port_hi=20000

# Initialize objects.

api.Init()

batch1=batch.BatchObject()

# Create device object
# local_tep_ip, gatewayip, local_tep_mac
device1=device.DeviceObject(ipaddress.IPv4Address(local_tep_ip), None, None)

security_profile1=policy.SecurityProfileObject(1, 60, 60, 30)

# Create VPC object 
# id, v4prefix, type = vpc_pb2.VPC_TYPE_TENANT, encaptype=types_pb2.ENCAP_TYPE_VXLAN, encapvalue
vpc1=vpc.VpcObject(vpc1_id, type=vpc_pb2.VPC_TYPE_TENANT, encaptype=types_pb2.ENCAP_TYPE_VXLAN, encapvalue=vpc1_vxlan_encap )

# Create VPC for Infra
vpc100=vpc.VpcObject(vpc2_id, type=vpc_pb2.VPC_TYPE_UNDERLAY, encaptype=types_pb2.ENCAP_TYPE_VXLAN, encapvalue=vpc2_vxlan_encap )

# Create VPC for VCN
vcn_route1=route.RouteObject(ipaddress.IPv4Network(vcn_route_prefix1), "tunnel", tunnel_id)
vcn_route_table=route.RouteTableObject(vcn_route_table_id, types_pb2.IP_AF_INET, [vcn_route1])
vcn_vpc=vpc.VpcObject(vcn_vpc_id, type=vpc_pb2.VPC_TYPE_CONTROL, encaptype=types_pb2.ENCAP_TYPE_VXLAN, encapvalue=vcn_vpc_encap, v4routetableid=vcn_route_table_id )

# Create L3 Interfaces ..
# id, iftype, ifadminstatus, vpcid, prefix, portid, encap, macaddr
intf1=interface.InterfaceObject( 1, interface_pb2.IF_TYPE_L3, interface_pb2.IF_STATUS_UP, vpc2_id, intf1_prefix, 1, types_pb2.ENCAP_TYPE_NONE, node_uuid=node_uuid )
intf2=interface.InterfaceObject( 2, interface_pb2.IF_TYPE_L3, interface_pb2.IF_STATUS_UP, vpc2_id, intf2_prefix, 2, types_pb2.ENCAP_TYPE_NONE, node_uuid=node_uuid )
lo1=interface.InterfaceObject( 3, interface_pb2.IF_TYPE_LOOPBACK, interface_pb2.IF_STATUS_UP, prefix=loopback_prefix )

# BGP config
bgp1=bgp.BgpObject(1, local_asn, router_id)

bgp_peer1=bgp.BgpPeerObject(1, local_addr1, peer_addr1, admin_state, send_comm, send_ext_comm, connect_retry, rrclient, remote_asn, holdtime, keepalive)
bgp_peer_af1=bgp.BgpPeerAfObject(1, local_addr1, peer_addr1)

bgp_peer2=bgp.BgpPeerObject(2, local_addr2, peer_addr2, admin_state, send_comm, send_ext_comm, connect_retry, rrclient, remote_asn, holdtime, keepalive)
bgp_peer_af2=bgp.BgpPeerAfObject(2, local_addr2, peer_addr2)

# Create VCN interface
vcn0=interface.InterfaceObject( 100, interface_pb2.IF_TYPE_CONTROL, interface_pb2.IF_STATUS_UP, prefix=vcn_intf_prefix, macaddr=vcn_vnic_mac, gateway=vcn_v4_router_ip )


# Create Tunnel Objects ..
# id, vpcid, localip, remoteip, macaddr, encaptype, vnid, nhid
tunnel1 = tunnel.TunnelObject( tunnel_id,vpc1_id, tunnel_local_ip, tunnel_remote_ip, None, tunnel_pb2.TUNNEL_TYPE_NONE, types_pb2.ENCAP_TYPE_VXLAN, tunnel_vnid) 

# Create DHCP Policy
dhcp_policy1 = dhcp.DhcpPolicyObject(1, server_ip=subnet1_v4_router_ip, mtu=9000, gateway_ip=subnet1_v4_router_ip, dns_server=subnet1_v4_router_ip, ntp_server=subnet1_v4_router_ip, domain_name="test.com", boot_filename="test1")
dhcp_policy2 = dhcp.DhcpPolicyObject(2, server_ip=subnet2_v4_router_ip, mtu=9000, gateway_ip=subnet2_v4_router_ip, dns_server=subnet2_v4_router_ip, ntp_server=subnet2_v4_router_ip, domain_name="test.com", boot_filename="test2")

# Create NAT Port Block
nat_pb1 = nat.NatPbObject(1, vpc1_id, nat_prefix, nat_port_lo, nat_port_hi, "udp")
nat_pb2 = nat.NatPbObject(2, vpc1_id, nat_prefix, nat_port_lo, nat_port_hi, "tcp")
nat_pb3 = nat.NatPbObject(3, vpc1_id, nat_prefix, 0, 0, "icmp")

# Create Subnets
# id, vpcid, v4prefix, v6prefix, hostifindex, v4virtualrouterip, v6virtualrouterip, virtualroutermac, v4routetableid, v6routetableid, ingv4securitypolicyid, egrv4securitypolicyid, ingv6securitypolicyid, egrv6securitypolicyid, fabricencap='VXLAN', fabricencapid=1
subnet1_route1=route.RouteObject(ipaddress.IPv4Network(subnet1_route_prefix1), "tunnel", tunnel_id, "napt")
subnet1_route_table=route.RouteTableObject(subnet1_route_table_id, types_pb2.IP_AF_INET, [subnet1_route1])
subnet1 = subnet.SubnetObject( 1, vpc1_id, ipaddress.IPv4Network(ipv4_subnet1), subnet1_host_if_idx, subnet1_v4_router_ip, subnet1_virt_router_mac, subnet1_route_table_id, 'VXLAN', subnet1_fabric_encap, node_uuid=node_uuid, dhcp_policy_id=1 )
subnet2 = subnet.SubnetObject( 2, vpc1_id, ipaddress.IPv4Network(ipv4_subnet2), subnet2_host_if_idx, subnet2_v4_router_ip, subnet2_virt_router_mac, subnet1_route_table_id, 'VXLAN', subnet2_fabric_encap, node_uuid=node_uuid, dhcp_policy_id=2 )

# VCN subnet
vcn_subnet = subnet.SubnetObject( vcn_subnet_id, vcn_vpc_id, ipaddress.IPv4Network(vcn_subnet_pfx), vcn_host_if_idx, vcn_v4_router_ip, vcn_virt_router_mac, 0, 'VXLAN', vcn_subnet_encap, node_uuid=node_uuid )


# Create VNIC object
# id, subnetid, vpcid, macaddr, hostifindex, sourceguard=False, fabricencap='NONE', fabricencapid=1, rxmirrorid = [], txmirrorid = []
vnic1 = vnic.VnicObject(1, 1, subnet1_local_vnic_mac, subnet1_host_if_idx, False, 'VXLAN', subnet1_fabric_encap, node_uuid=node_uuid )
vnic2 = vnic.VnicObject(2, 2, subnet2_local_vnic_mac, subnet2_host_if_idx, False, 'VXLAN', subnet2_fabric_encap, node_uuid=node_uuid )

# Create VCN VNIC object
vcn_vnic = vnic.VnicObject(vcn_vnic_id, vcn_subnet_id, vcn_vnic_mac, vcn_host_if_idx, False, 'VXLAN', vcn_subnet_encap, node_uuid=node_uuid )


# Create Mapping Objects 1 for local vnic and another for remote IP
# self, key_type, macaddr, ip, vpcid, subnetid, tunnelid, encaptype, encapslotid, nexthopgroupid, publicip, providerip, vnicid = 0

map1 = mapping.MappingObject( 1, 'l3', vcn_vnic_mac, vcn_intf_ip, vcn_vpc_id, subnetid=vcn_subnet_id, vnicid=vcn_vnic_id )
map2 = mapping.MappingObject( 2, 'l3', subnet1_local_vnic_mac, subnet1_local_host_ip, vpc1_id, subnetid=1, vnicid=1 )
map3 = mapping.MappingObject( 3, 'l3', subnet1_remote_vnic_mac, subnet1_remote_host_ip, vpc1_id, subnetid=1, tunnelid=1 )
map4 = mapping.MappingObject( 4, 'l3', subnet2_local_vnic_mac, subnet2_local_host_ip, vpc1_id, subnetid=2, vnicid=2, public_ip=subnet2_local_vnic_public_ip )

# Create service mapping
svc_map1 = service_mapping.SvcMappingObject(1, vpc1_id, subnet2_backend_ip, subnet2_backend_port, subnet2_vip, subnet2_service_port)

# Push the configuration
api.client.Start(api.ObjectTypes.BATCH, batch1.GetGrpcMessage())

# Create configs on the Naples
api.client.Create(api.ObjectTypes.SWITCH, [device1.GetGrpcCreateMessage()])

api.client.Create(api.ObjectTypes.SECURITY_PROFILE, [security_profile1.GetGrpcCreateMessage()])

api.client.Create(api.ObjectTypes.VPC, [vpc1.GetGrpcCreateMessage()])
api.client.Create(api.ObjectTypes.VPC, [vpc100.GetGrpcCreateMessage()])

api.client.Create(api.ObjectTypes.INTERFACE, [intf1.GetGrpcCreateMessage()])
api.client.Create(api.ObjectTypes.INTERFACE, [intf2.GetGrpcCreateMessage()])
api.client.Create(api.ObjectTypes.INTERFACE, [lo1.GetGrpcCreateMessage()])

api.client.Create(api.ObjectTypes.BGP, [bgp1.GetGrpcCreateMessage()])
api.client.Create(api.ObjectTypes.BGP_PEER, [bgp_peer1.GetGrpcCreateMessage()])
api.client.Create(api.ObjectTypes.BGP_PEER_AF, [bgp_peer_af1.GetGrpcCreateMessage()])
api.client.Create(api.ObjectTypes.BGP_PEER, [bgp_peer2.GetGrpcCreateMessage()])
api.client.Create(api.ObjectTypes.BGP_PEER_AF, [bgp_peer_af2.GetGrpcCreateMessage()])

api.client.Create(api.ObjectTypes.INTERFACE, [vcn0.GetGrpcCreateMessage()])

api.client.Create(api.ObjectTypes.TUNNEL, [tunnel1.GetGrpcCreateMessage()])
api.client.Create(api.ObjectTypes.ROUTE, [vcn_route_table.GetGrpcCreateMessage()])
api.client.Create(api.ObjectTypes.VPC, [vcn_vpc.GetGrpcCreateMessage()])

api.client.Create(api.ObjectTypes.NAT, [nat_pb1.GetGrpcCreateMessage()])
api.client.Create(api.ObjectTypes.NAT, [nat_pb2.GetGrpcCreateMessage()])
api.client.Create(api.ObjectTypes.NAT, [nat_pb3.GetGrpcCreateMessage()])
api.client.Create(api.ObjectTypes.DHCP_POLICY, [dhcp_policy1.GetGrpcCreateMessage()])
api.client.Create(api.ObjectTypes.DHCP_POLICY, [dhcp_policy2.GetGrpcCreateMessage()])
api.client.Create(api.ObjectTypes.SUBNET, [vcn_subnet.GetGrpcCreateMessage()])
api.client.Create(api.ObjectTypes.ROUTE, [subnet1_route_table.GetGrpcCreateMessage()])
api.client.Create(api.ObjectTypes.SUBNET, [subnet1.GetGrpcCreateMessage()])
api.client.Create(api.ObjectTypes.SUBNET, [subnet2.GetGrpcCreateMessage()])

api.client.Create(api.ObjectTypes.VNIC, [vcn_vnic.GetGrpcCreateMessage()])
api.client.Create(api.ObjectTypes.VNIC, [vnic1.GetGrpcCreateMessage()])
api.client.Create(api.ObjectTypes.VNIC, [vnic2.GetGrpcCreateMessage()])
api.client.Create(api.ObjectTypes.MAPPING, [map1.GetGrpcCreateMessage()])
api.client.Create(api.ObjectTypes.MAPPING, [map2.GetGrpcCreateMessage()])
api.client.Create(api.ObjectTypes.MAPPING, [map3.GetGrpcCreateMessage()])
api.client.Create(api.ObjectTypes.MAPPING, [map4.GetGrpcCreateMessage()])

api.client.Create(api.ObjectTypes.SVC_MAPPING, [svc_map1.GetGrpcCreateMessage()])

sys.exit(1)
