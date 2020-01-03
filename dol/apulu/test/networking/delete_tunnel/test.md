# Testcases for delete of tunnel object

Initial setup:
    a) Configuration:
        On the DUT VPCs, subnets, VNICs, local mappings, remote mappings, route
        tables, tunnels, nexthops, security groups and policies are configured.

#1 Tunnel delete
    Trigger:
    a) A few tunnels are deleted.
    b) Packets originating from host, destined to deleted tunnels are sent to
       the host interface.

    Expectation:
    a) Read of deleted tunnels will fail.
    b) No packet egresses on the network uplink ports.

    Trigger:
    a) Deleted tunnels are restored.
    b) Packets originating from host, destined to restored tunnels are sent to
       the host interface.

    Expectation:
    a) Read of deleted tunnels will succeed.
    b) Packet egresses on the network uplink ports.
