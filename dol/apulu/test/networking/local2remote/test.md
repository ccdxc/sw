# Testcases for host to remote host packet path

Initial setup:
    a) Configuration:
            On the DUT VPCs, subnets, VNICs, local mappings, remote mappings,
            route tables, tunnels, nexthops, security groups and policies are
            configured.

## Intra-subnet case:
    Trigger:
        IPv4/6 or ICMPv4/6 packet with VLAN tag destined to remote VNIC in the
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

## Intra-subnet case with source guard enabled:
    Setup:
        a) A VNIC with source guard flag enabled is selected.

    Trigger:
        IPv4/6 or ICMPv4/6 packet with VLAN tag destined to remote VNIC in the
        same subnet is sent to host interface.

    Expectation:
        a) If any of source IP and source MAC are unknown, then the packet
           is dropped. No response is sent.
        b) If neither of source IP and source MAC is unknown, the response
           packet is sent to host interface.
        c) A VXLAN header is added to the incoming packet and sent out on one
           of the uplink ports.
        d) The VNI info is taken from subnet.
        e) Source and destination for the outer ipv4 header is taken from device
           and tunnel respectively.
        f) Source and destination MAC for the outer eth header is taken from the
           uplink port and tunnel respectively.
        g) If source guard flag is disabled and either source IP or source MAC
           is unknown, the response packet is still generated.

## Intra-subnet case with source guard disabled:
    Setup:
        a) A VNIC with source guard flag disabled is selected.

    Trigger:
    IPv4/6 or ICMPv4/6 packet with VLAN tag destined to remote VNIC in the same
    subnet is sent to host interface.

    Expectation:
        a) A response packet is sent to host interface even if any of source IP
           and source MAC addresses are unknown.
        b) A VXLAN header is added to the incoming packet and sent out on one of
           the uplink ports.
        c) The VNI info is taken from subnet.
        d) Source and destination for the outer ipv4 header is taken from device
           and tunnel respectively.
        e) Source and destination MAC for the outer eth header is taken from the
           uplink port and tunnel respectively.
        f) If source guard flag is disabled and either source IP or source MAC
           is unknown, the response packet is still generated.

## Intra-subnet case:
    Setup:
    If source guard is enabled, then packet is chosen with an unknown source
    or unknown source IP or both.
    Trigger:
    IPv4/6 or ICMPv4/6 packet with VLAN tag destined to remote VNIC in the same
    subnet is sent to host interface.

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

## Inter-subnet case:
    Trigger:
    IPv4/6 or ICMPv4/6 packet with VLAN tag destined to remote VNIC in a
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
