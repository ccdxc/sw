# Testcases for apulu

## runorder
    Following is the order in which the sub features and the testcases are run
    Idea is to fail fast & keep it simple for debugging and therefore all the
    basic cases come first.
    
    local2remote
    remote2local
    local2igw
    igw2local
    local2local
    negative
    arp_proxy
    dhcp_relay
    update
        vnic
        interface
        nexthop
        nhgroup
        tunnel
        subnet
        device
    delete
        lmapping
        rmapping
        vnic
        interface
        nexthop
        nhgroup
        tunnel
        routetable
        subnet
        vpc
        device
