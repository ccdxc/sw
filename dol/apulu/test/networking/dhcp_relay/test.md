# Test cases for DHCP relay feature in vpp

Initial setup:
    Configuration:
        On the DUT VPCs, subnets, VNICs, local mappings, remote mappings, route
        tables, tunnels, nexthops, security groups, DHCP relay and policies 
        are configured.

##  DHCP Discover and Request
    Trigger:
        DHCP discover/request packet sent to interface.

    Expectation
        a) Option 82 with suboptions 1, 5, 11, 151 and 152 and giaddr
           are added in DHCP header.
        b) Device IP is used as source IP and DHCP server IP is used as
           destination IP in IP header.
        c) Uplink port MAC is used as source MAC and DHCP server MAC is used as
           destination MAC in ethernet header.

## DHCP Offer and ACK
    Trigger:
        DHCP offer or ACK packet originating from a server is sent to uplink
        port

    Expectation:
        a) Subnet VR MAC is used as source MAC.
        b) If bootflags is set to broadcast, boradcast MAC is used as destination
           MAC, else VNIC MAC is used as destination MAC.
        c) Broadcast IP is used as dest IP , device IP is used as source IP in
           IP header.
