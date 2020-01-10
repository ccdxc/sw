# Testcases for packet flow between endpoints on the same host

Initial setup:
    a) Configuration:
            On the DUT VPCs, subnets, VNICs, local mappings, remote mappings,
            route tables, tunnels, nexthops, security groups and policies are
            configured.

## Intra-VNIC:
    Trigger:
        IPv4/6 packet with VLAN tag is sent to host interface.

    Expectation:
        a) VLAN tagged packet with some modifications is sent back to host.
        b) Subnet's VR MAC is used as source address in the eth header.
        c) Source address in the packet from host is used as destination address.

## Inter-VNIC:
    Trigger:
        IPv4/6 packet with VLAN tag is sent to host interface.

    Expectation:
        a) VLAN tagged packet with some modifications is sent back to host.
        b) Subnet's VR MAC is used as source address in the eth header.
        c) Source address in the packet from host is used as destination address.

## Inter-VNIC with source guard enabled:
    Setup:
        a) A VNIC with source guard flag enabled is selected.

    Trigger:
        IPv4/6 packet with VLAN tag is sent to host interface either with an
        unknown source IP or source MAC or both.

    Expectation:
        a) If any of source IP and source MAC are unknown, then the packet
           is dropped. No response is sent.
        b) If neither of source IP and source MAC is unknown, the response
           packet is sent to host interface.

## Inter-VNIC with source guard disabled:
    Setup:
        a) A VNIC with source guard flag disabled is selected.

    Trigger:
        IPv4/6 packet with VLAN tag is sent to host interface either with an
        unknown source IP or source MAC or both.

    Expectation:
        a) A response packet is sent to host interface even if any of source IP
           and source MAC addresses are unknown.
        b) VLAN tagged packet with some modifications is sent back to host.
        c) Subnet's VR MAC is used as source address in the eth header.
        d) Source address in the packet from host is used as destination address.

## Inter-subnet:
    Trigger:
    IPv4/6 packet with VLAN tag is sent to host interface.

    Expectation:
        a) VLAN tagged packet with some modifications is sent back to host.
        b) Subnet's VR MAC is used as source address in the eth header.
        c) Source address in the packet from host is used as destination address.
