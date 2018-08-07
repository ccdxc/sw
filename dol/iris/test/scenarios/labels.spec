# This file contains the list of scenarios.
meta:
    id: NETWORKING

labels:
    sources:
        - enic_useg
        - enic_pvlan
        - enic_direct
        - uplink
        - uplinkpc

    destinations:
        - enic_useg
        - enic_pvlan
        - enic_direct
        - uplink
        - uplinkpc

    ip_family:
        - ipv4
        - ipv6

    ip_protos:
        - tcp
        - udp
        - icmp
        - unknown

    mac_family:
        - arp
        - garp
        - rarp
        - unknown

    encaps:
        - untag
        - qtag
        - ptag
        - vxlan
        - vxlan_transit


