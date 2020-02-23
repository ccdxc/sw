# Testcases for Learn functionality

Initial setup:
    a) Configuration:
            On the DUT VPCs, subnets, VNICs, local mappings, remote mappings,
            route tables, tunnels, nexthops, security groups and policies are
            configured.
    b) Learn topology does not create VNICs and lcoal mappings as they are
       marked "DISCOVERED"

## ARP request for known remote IP
    Trigger:
        ARP request for a destination from any remote mapping is sent to host
        interface.

    Expectation:
        a) ARP reply with destination mac from the remote mapping is sent back
           on host interface.
        Note: Seeing ARP reply verifies that:
            1) ARP request is sent to learn thread
            2) Learn thread creates VNIC and local IP mapping based on ARP
            3) Packet is injected back to P4
            4) Packet is delivered to VPP this time as local mapping is created

##  DHCP Discover
    Trigger:
        DHCP discover packet sent to interface.

    Expectation:
        a) DHCP request is relayed on uplink port
        Note: This verifies that:
            1) DHCP discover packet is sent to learn thread
            2) Learn thread creates VNIC from source MAC in the pkt
            3) Learn thread ignores IP address in the pkt
            4) Packet is sent to VPP cpu mnic
