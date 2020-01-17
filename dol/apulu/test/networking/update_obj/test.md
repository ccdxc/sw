# Testcases for update of objects

Initial setup:
    a) Configuration:
        On the DUT VPCs, subnets, VNICs, local mappings, remote mappings, route
        tables, tunnels, nexthops, security groups and policies are configured.

## Device Update
    Trigger:
        a) Device IP address is updated.
        b) Packets destined to remote hosts are injected on the host interface.

    Expectation:
        a) Read of device object returns updated attribute values.
        b) Packet that egresses on the network uplink ports has new device IP
           as source address in outer IPv4 header.

    Trigger:
        a) Packets from internet gateway are injected on one of the uplink ports
           with old device IP as destination IP address in outer IP header.

    Expectation:
        a) Packets are dropped since the destination IP does not match
           the device IP configured.

    Trigger:
        a) Device IP is rolled back to its old value.
        b) Packets from internet gateway are injected on one of the uplink ports.

    Expectation:
        a) Read of device object returns the current attribute values.
        b) Packet egresses on the host interface.

## Subnet Update
    Trigger:
        a) Virtual Router MAC and Host interface index are updated on few subnets.
        b) Packets from internet gateway are injected on one of the uplink ports.

    Expectation:
        a) Read of subnet objects returns updated attribute values.
        b) Packets egress on the correct host interface with new VR MAC as 
           source MAC in the inner ethernet header.

    Trigger:
        a) Updated attributes in subnet objects are rolled back to old values.
        b) Packets from internet gateway are injected on one of the uplink ports.

    Expectation:
        a) Read of subnet objects return current attribute values.
        b) Packets egress on correct host interface with new VR MAC as 
           source MAC in the inner ethernet header.

## VNIC Update
    Trigger:
        a) VNIC Encap is updated on few VNICs.
        b) Packets destined to remote hosts are injected on host interface.

    Expectation:
        a) Read of VNIC objects return updated attribute values.
        b) Tagged packets that egress on network uplink ports are based on
           new vnic encap.

    Trigger:
        a) Update attributes in VNIC objects are rolled back to old values.
        b) Packets destined to remote hosts are injected on host interface.

    Expectation:
        a) Read of VNIC objects return current attribute values.
        b) Tagged packets that egress on network uplink ports are based on
           current vnic encap.

## Tunnel Update
    Trigger:
        a) Remote IP Address and Nexthop used by few tunnels are updated.
        b) Packets destined to remote hosts are injected on host interface.

    Expectation:
        a) Read of tunnel objects return updated attribute values.
        b) Packets egress on the network uplink ports according to
           current nexthops with updated remote IP as destination IP
           in the outer IP header.

    Trigger:
        a) Updated attributes in tunnel objects are rolled back to old values.
        b) Packets destined to remote hosts are injected on host interface.

    Expectation:
        a) Read of tunnel objects return current attribute values.
        b) Packets egress on the network uplink ports according to current
           nexthops with the current remote IP as destination IP
           in the outer IP header.

## Interface Update
    Trigger:
        a) Interface MAC address is updated.
        b) Packets destined to remote hosts are injected on the host interface.

    Expectation:
        a) Read of interface objects return updated attribute values.
        b) Packets egress on the network uplink ports with 
           the new interface MAC as source MAC in outer eth header.

    Trigger:
        a) Packets from internet gateway are injected on one of the uplink ports
           with old interface MAC as destination MAC address in outer eth header.

    Expectation:
        a) Packets are expected to be dropped since the destination MAC
           does not match the MAC configured.

    Trigger:
        a) Updated attributes in interface object is rolled back to old values.
        b) Packets from internet gateway are injected on one of the uplink ports.

    Expectation:
        a) Read of interface objects return current attribute values.
        b) Packet egresses on the host interface.

## Nexthop Update
    Trigger:
        a) L3 interface associated and MAC address are updated for underlay
           nexthops.
        b) Tunnel id used is updated for overlay nexthops.
        c) Packets destined to remote hosts are injected on the host interface.

    Expectation:
        a) Read of nexthop objects return updated attribute values.
        b) For underlay nexthops, packets egress on the current network uplink
           ports based on new L3 interface associated, with new
           underlay MAC address as destination MAC in outer eth packet.
        c) For overlay nexthops, packets use the attributes associated with the
           updated tunnel id.

    Trigger:
        a) Updated attributes in nexthop objects are rolled back to old values.
        b) Packets destined to remote hosts are injected on the host interface.

    Expectation:
        a) Read of nexthop objects return current attribute values.
        b) For underlay nexthops, packets egress on the current network uplink
           ports based on current L3 interface associated, with current
           underlay MAC address as destination MAC in outer eth packet.
        c) For overlay nexthops, packets use the attributes associated with the
           current tunnel id.
