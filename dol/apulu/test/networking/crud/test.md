# Testcases for delete and update of PDS networking objects:

 - NAPLES SIM is the DUT (Device Under Test) and all objects are created on DUT.
 - An object (config object/interface/port) when mentioned refers to an object
   on the DUT unless remote is specified, in which case it refers to an object
   on a remote node. Remote mapping is an exception which refers to an object
   on DUT.

Initial setup:
    a) Configuration:
        On the DUT VPCs, subnets, VNICs, local mappings, remote mappings, route
        tables, tunnels, nexthops, security groups and policies are configured.

# List of delete testcases -
#1 VPC delete
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

#2 Subnet delete
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

#3 VNIC delete
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

#4 Route table delete
    Trigger:
    a) A few route tables are deleted.
    b) Packets originating from host destined to an internet gateway with
       destination address picked from deleted route tables, are sent to
       the host interface.

    Expectation:
    a) Read of route tables will fail.
    b) No packet egresses on the network uplink ports.

    Trigger:
    a) Deleted route tables are restored.
    b) Packets originating from host destined to an internet gateway with
       destination address picked from restored route tables are sent to
       the host interface.

    Expectation:
    a) Read of route tables will succeed.
    b) Packet egresses on the network uplink ports.

#3 Tunnel delete
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

# List of update testcases -
#TODO - Rathina to add.
