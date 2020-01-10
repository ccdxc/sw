# Testcases for internet-gateway to host packet path

Initial setup:
    a) Configuration:
            On the DUT VPCs, subnets, VNICs, local mappings, remote mappings,
            route tables, tunnels, nexthops, security groups and policies are
            configured.

## IPv4/6 VXLAN packet:
    Trigger:
        IPv4/6 packet with VxLAN encap is sent to one of the uplink ports.

    Expectation:
        a) VxLAN header is decapsulated
        b) Subnet's VR MAC is used for the source mac of the inner ethernet header.
        c) VNIC's MAC is used for the destination mac of the inner ethernet header.
        d) VLAN tag with VLAN id taken from VNIC is added.

## IPv4/6 VXLAN packet with DNAT enabled:
    Setup:
        A route table and tunnel object with NAT enabled are selected.

    Trigger:
        IPv4/6 packet with VxLAN encap is sent to one of the uplink ports.

    Expectation:
        a) DNAT is done on the inner IPv4/6 packet.
        b) VxLAN header is decapsulated
        c) Subnet's VR MAC is used for the source mac of the inner ethernet header.
        d) VNIC's MAC is used for the destination mac of the inner ethernet header.
        e) VLAN tag with VLAN id taken from VNIC is added.
