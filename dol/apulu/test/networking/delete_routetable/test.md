# Testcases for delete of route table object

Initial setup:
    a) Configuration:
        On the DUT VPCs, subnets, VNICs, local mappings, remote mappings, route
        tables, tunnels, nexthops, security groups and policies are configured.

#1 Route table delete
    Trigger:
    a) A VPC is deleted.
    b) Packets originating from VNICs in the deleted VPC, destined to
       remote VNICs are sent to the host interface.

    Expectation:
    a) Read of deleted VPC, it's subnets, VNICs and mappings will fail.
    b) No packet egresses on the network uplink ports.

    Trigger:
    a) Deleted VPC along with it's subnets, VNICs, mappings is restored.
    b) Packets originating from VNICs in the restored VPC, destined to
       remote VNICs are sent to the host interface.

    Expectation:
    a) Read of restored VPC, it's subnets, VNICs and mappings will succeed.
    b) Packet egresses on the network uplink ports.
