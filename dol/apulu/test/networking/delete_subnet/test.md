# Testcases for delete of subnet object

Initial setup:
    a) Configuration:
        On the DUT VPCs, subnets, VNICs, local mappings, remote mappings, route
        tables, tunnels, nexthops, security groups and policies are configured.

#1 Subnet delete
    Trigger:
    a) A few subnets are deleted.
    b) Packets originating from VNICs in the deleted subnet, destined to
       remote VNICs are sent to the host interface.

    Expectation:
    a) Read of deleted subnets, their VNICs and mappings will fail.
    b) No packet egresses on the network uplink ports.

    Trigger:
    a) Deleted subnets along with their VNICs, mappings are restored.
    b) Packets originating from VNICs in the restored subnets, destined to
       remote VNICs are sent to the host interface.

    Expectation:
    a) Read of deleted subnets, their VNICs and mappings will succeed.
    b) Packet egresses on the network uplink ports.
