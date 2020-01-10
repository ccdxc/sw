# Testcases for delete of objects

Initial setup:
    a) Configuration:
        On the DUT VPCs, subnets, VNICs, local mappings, remote mappings, route
        tables, tunnels, nexthops, security groups and policies are configured.

## VPC delete
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

## Subnet delete
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
        a) Read of restored subnets, their VNICs and mappings will succeed.
        b) Packet egresses on the network uplink ports.

## VNIC delete
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
        a) Read of restored VNICs and corresponding mappings will succeed.
        b) Packet egresses on the network uplink ports.

## Tunnel delete
    Trigger:
        a) A few tunnels are deleted.
        b) Packets originating from host, destined to deleted tunnels are sent
           to the host interface.

    Expectation:
        a) Read of deleted tunnels will fail.
        b) No packet egresses on the network uplink ports.

    Trigger:
        a) Deleted tunnels are restored.
        b) Packets originating from host, destined to restored tunnels are sent
           to the host interface.

    Expectation:
        a) Read of restored tunnels will succeed.
        b) Packet egresses on the network uplink ports.

## Route table delete
    Trigger:
        a) A route table is deleted.
        b) Packets with destination address chosen from deleted route table
           are sent to the host interface.

    Expectation:
        a) Read of deleted route table will fail.
        b) No packet egresses on the network uplink ports.

    Trigger:
        a) Deleted route table is restored.
        b) Packets with destination address chosen from deleted route table
           are sent to the host interface.

    Expectation:
        a) Read of restored route table will succeed.
        b) Packet egresses on the network uplink ports.

## Device delete
    Trigger:
        a) Device object is deleted.
        b) Any valid packet is sent to the host interface.

    Expectation:
        a) Read of all objects will fail.
        b) No packet egresses on the network uplink ports.

    Trigger:
        a) Device object is restored.
        b) Any valid packet is sent to the host interface.

    Expectation:
        a) Read of all restored objects will succeed.
        b) Packet egresses on the network uplink ports.

## Interface delete
    Trigger:
        a) An interface is deleted.
        b) Packets expected on the deleted interface destination is sent
           to the host interface.

    Expectation:
        a) Read of deleted interface will fail.
        b) No packet egresses on the deleted interface.

    Trigger:
        a) Deleted interface is restored.
        b) Packets expected on the deleted interface destination is sent
           to the host interface.

    Expectation:
        a) Read of restored interface will succeed.
        b) Packet egresses on the restored network uplink port.
