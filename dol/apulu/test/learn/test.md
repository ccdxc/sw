# Testcases for learn functionality

Intial setup:
    a) Configuration:
            On the DUT VPCs, subnets, VNICs, local mappings, remote mappings,
            route tables, tunnels, nexthops, security groups and policies are
            configured. On device spec, learning is enabled and age is set to
            lowest value of 30s.

## Learn MAC and then IP from DHCP requests and exercise clear commands
    Trigger     : From host, send DHCP discover pkt.
    Expectation : MAC is learnt.

    Trigger     : From host, send DHCP request pkt.
    Expectation : IP is learnt.

    Trigger     : Clear specific IP, then specific MAC.
    Expectation : IP is deleted first, then MAC is deleted.

    Trigger     : From host, send DHCP discover pkt.
    Expectation : MAC is learnt again.

    Trigger     : From host, send DHCP request pkt.
    Expectation : IP is learnt again.

    Trigger     : Clear all IPs.
    Expectation : IP is deleted.

    Trigger     : From host, send DHCP request pkt.
    Expectation : IP is learnt again.

    Trigger     : Clear all MACs.
    Expectation : IP and MAC both are clered.

## Learn MAC and IP from ARP requests and test aging
    Trigger     : From host, send ARP request to a remote host.
    Expectation : MAC and IP are learnt, ARP reply is received. Learn stats are
                  incremented correctly.

    Trigger     : Wait for aging timeout.
    Expectation : ARP probe is received from DUT.

    Trigger     : Send ARP reply from host to DUT
    Expectation : IP TTL is refreshed.

    Trigger     : Wait for aging timeout x 3.
    Expectation : 3 ARP probes are received and IP is deleted.

    Trigger     : Wait for aging timeout.
    Expectation : MAC is deleted.

    Trigger     : From host, send ARP request to a remote host.
    Expectation : MAC and IP are learnt again, ARP reply is received.
                  Learn stats are incremented correctly.


