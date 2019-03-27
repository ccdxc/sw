meta:
    description: Memory Calculator Parameter File

# VPC Parameters
vpc:
    # Number of VPCs required
    count: 1K
    
# Policy Tables parameters per VPC
policy_tables:
    # Number of Policy Tables, usually same as VPC
    count: 1K

    ipv4:
        # Number of Ingress IPv4 ACLs per Policy Table
        ingress: 1K

        # Number of Egress IPv4 ACLs per Policy Table
        egress : 0
    ipv6:
        # Number of Ingress IPv6 ACLs per Policy Table
        ingress: 1K

        # Number of Egress IPv6 ACLs per Policy Table
        egress : 0

# Routing Table Parameters
route_tables:
    # Number of Route Tables
    count: 1K

    # Number of routes per route-table
    routes:
        # Number of IPv4 Routes per Route Table
        ipv4: 1K
        
        # Number of IPv6 Routes per Route Table
        ipv6: 1K

    # Number of nexthops per Route Table
    nexthops: 1K

# Flow Table Parameters
flows:
    # Number of Flows
    count : 2M
    

# Mapping Table Parameters
mappings:
    locals: 
        # Number of IPv4 Local Mapping Entries for all VPCs
        ipv4: 32K

        # Number of IPv6 Local Mapping Entries for all VPCs
        ipv6: 32K
    remotes:
        # Number of instances of Remote Mapping Tables
        instances: 2

        # Number of IPv4 Remote Mapping Entries for all VPCs
        ipv4: 512K

        # Number of IPv6 Remote Mapping Entries for all VPCs
        ipv6: 512K

# TEP (Tunnel Endpoint) Table Parameters
teps:
    # Number of TEPs for all VPCs
    count: 1K

# NAT Table Parameters
nat:
    # Number of NAT entries for all VPCs
    count: 64K
