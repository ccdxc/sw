# Testcases for operational objects

Initial setup:
    a) Configuration:
        On the DUT VPCs, subnets, VNICs, local mappings, remote mappings, route
        tables, tunnels, nexthops, security groups and policies are configured.

## Link Events
    Trigger:
        a) Port is brought admin down

    Expectation:
        a) Read port status and verify it indeed came down
        b) Check if Link down alert is generated for that port

    Trigger:
        a) Port is brought admin up

    Expectation:
        a) Read port status and verify it indeed came back up
        b) Check if Link up alert is generated for that port
