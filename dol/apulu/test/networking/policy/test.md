# Testcases for packet flow

Initial setup:
    a) Configuration:
            On the DUT VPCs, subnets, VNICs, local mappings, remote mappings,
            route tables, tunnels, nexthops, security groups and policies are
            configured.

## L2L Inter-subnet:
    Trigger:
    IPv4/6 packet

    Expectation:
        a) untagged packet with some modifications is sent back to host.
        b) Subnet's VR MAC is used as source address in the eth header.
        c) Source address in the packet from host is used as destination address.

## L2R Intra-subnet case:
    Trigger:
        IPv4/6 or ICMPv4/6 packet destined to remote VNIC in the
        same subnet is sent to host interface.

    Expectation:
        a) A VXLAN header is added to the incoming packet and sent out on one of
           the uplink ports.
        b) The VNI info is taken from subnet.
        c) Source and destination for the outer ipv4 header is taken from device
           and tunnel respectively.
        d) Source and destination MAC for the outer eth header is taken from the
           uplink port and tunnel respectively.
        e) If source guard is enabled and either source IP or source MAC is
           known, then the packet is dropped. No response is sent.
        f) If source guard is enabled and either source IP or source MAC is
           known, the response packet is still generated.

## L2R Inter-subnet case:
    Trigger:
    IPv4/6 or ICMPv4/6 packet destined to remote VNIC in a
    different subnet is sent to host interface.

    Expectation:
        a) Source address of the incoming packet is swapped with public IP from
           local mapping.
        b) A VXLAN header is added to the incoming packet and sent out on one of
           the uplink ports.
        c) The VNI info is taken from VPC.
        d) Source and destination for the outer ipv4 header is taken from device
           and tunnel respectively.
        e) Source and destination MAC for the outer eth header is taken from the
           uplink port and tunnel respectively.

## R2L Intra-Subnet case:
    Trigger:
        IPv4/6 packet with VxLAN encap destined to one of the local vnic in the same
        subnet is sent to one of the uplink ports.

    Expectation:
        a) VxLAN header is decapsulated
        b) Subnet's VR MAC is used for the source mac of the inner ethernet header.
        c) VNIC's MAC is used for the destination mac of the inner ethernet header.

## R2L Inter-Subnet case:
    Trigger:
        IPv4/6 packet with VxLAN encap destined to one of the local vnic in different
        subnet is sent to one of the uplink ports.

    Expectation:
        a) VxLAN header is decapsulated
        b) Subnet's VR MAC is used for the source mac of the inner ethernet header.
        c) VNIC's MAC is used for the destination mac of the inner ethernet header.

## L2N IPv4/6 packet:
    Trigger:
        IPv4/6 packet  is sent to NAPLES SIM via host interface.

    Expectation:
        a) A VXLAN header is added to the incoming packet and sent out on one
           of the uplink ports.
        b) The VNI info is taken from VPC.
        c) Source and destination for the outer ipv4 header is taken from device
           and tunnel respectively.
        d) Source and destination MAC for the outer eth header is taken from the
           uplink port and tunnel respectively.

## N2L IPv4/6 VXLAN packet:
    Trigger:
        IPv4/6 packet with VxLAN encap is sent to one of the uplink ports.

    Expectation:
        a) VxLAN header is decapsulated
        b) Subnet's VR MAC is used for the source mac of the inner ethernet header.
        c) VNIC's MAC is used for the destination mac of the inner ethernet header.

