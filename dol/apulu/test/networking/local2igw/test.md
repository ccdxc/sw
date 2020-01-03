# Testcases for host to internet-gateway packet path

Initial setup:
    a) Configuration:
        On the DUT VPCs, subnets, VNICs, local mappings, remote mappings, route
        tables, tunnels, nexthops, security groups and policies are configured.

#1 IPv4 UDP VLAN tagged packet:
    Trigger:
    IPV4 UDP packet with VLAN tag is sent to NAPLES SIM via host interface.

    Expectation:
    a) A VXLAN header is added to the incoming packet and sent out on one of the
       uplink ports.
    b) The VNI info is taken from VPC.
    c) Source and destination for the outer ipv4 header is taken from device and
       tunnel respectively.
    d) Source and destination MAC for the outer eth header is taken from the
       uplink port and tunnel respectively.

#2 IPv4 UDP VLAN tagged packet with SNAT enabled:
    Trigger:
    IPV4 UDP packet with VLAN tag is sent to NAPLES SIM via host interface.

    Expectation:
    a) Source address of the incoming packet is swapped with public IP from
       local mapping.
    b) A VXLAN header is added to the incoming packet and sent out on one of the
       uplink ports.
    c) The VNI info is taken from VPC.
    d) Source and destination for the outer ipv4 header is taken from device and
       tunnel respectively.
    e) Source and destination MAC for the outer eth header is taken from the
       uplink port and tunnel respectively.

