# Testcases for delete of VNIC object

Initial setup:
    a) Configuration:
        On the DUT VPCs, subnets, VNICs, local mappings, remote mappings, route
        tables, tunnels, nexthops, security groups and policies are configured.

#1 VNIC delete
    Trigger:
    a) A few VNICs are deleted.
    b) Packets originating from deleted VNICs, destined to remote VNICs are
       sent to the host interface.

    Expectation:
    a) Read of deleted VNICs and corresponding mappings will fail.
    b) No packet egresses on the network uplink ports.

    Trigger:
    a) Deleted VNICs, with their corresponding mappings are restored.
    b) Packets originating from restored VNICs, destined to remote VNICs are
       sent to the host interface.

    Expectation:
    a) Read of deleted VNICs and corresponding mappings will succeed.
    b) Packet egresses on the network uplink ports.
