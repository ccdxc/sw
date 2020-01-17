# Testcases for IPv4 security policy functionality in host to internet gateway packet path

## Initial setup:
    Configuration:
        On the DUT VPCs, subnets, VNICs, local mappings, remote mappings, route
        table with default route, tunnels, nexthops, different types of
        security groups and policies are configured.

        Following are the different types of security policies configured in the
        egress direction for subnets
        #1 Overlap
            At least one of the attributes of policy rule (SIP/DIP prefix,
            sport/dport range) is overlapping (partial/complete) with the
            attributes of other rules in the policy
            a) Policy rules with left overlapping attributes
            b) Policy rules with left overlapping attributes and wildcard rule
            c) Policy rules with multiple left overlapping attributes
            d) Policy rules with multiple left overlapping attributes
               and wildcard rule
            e) Policy rules with right overlapping attributes
            f) Policy rules with right overlapping attributes and wildcard rule
            g) Policy rules with multiple right overlapping attributes
            h) Policy rules with multiple right overlapping attributes
               and wildcard rule

        #2 Adjacent
            At least one of the attributes of policy rule (SIP/DIP prefix,
            sport/dport range) is consecutive to the attributes of the other
            rules in the policy
            a) Policy rules with consecutive attributes
            b) Policy rules with consecutive attributes and wildcard rule
            c) Policy rules with maximum number of consecutive attributes
            d) Policy rules with maximum number of consecutive attributes
               and wildcard rule

        #3 Disjoint
            At least one of the attributes of policy rule (SIP/DIP prefix,
            sport/dport range) is disjoint to the attributes of the other rules
            in the policy
            a) Policy rules with completely disjoint attributes
            b) Policy rules with completely disjoint attributes
               and wildcard rule

        #4 Policy with wildcard rules for all proto

        #5 Policy with rules having only host prefixes

        #6 Policy with duplicate rules

## Common test procedure for all test cases:
    Trigger:
        Based on the testcase, following differs
            - policy object selection
            - packet attributes like protocol, DIP, sport, dport
        From the subnet where the selected policy is attached to,
        An IPv4 packet is sent to DSC SIM via host interface.

    Expectation:
        All the IPv4 egress policies attached to the selected policy's subnet and
        the VNIC of the selected localmapping are evaluated sequentially.
        If the highest priority rule across all policies says 'deny',
        packet should be dropped and no packets should egress the uplink ports.
        Otherwise,
            - A VXLAN header with VNId from VPC of the route table is added to
              the packet ingressing from host interface
            - An IPv4 header is added with SIP and DIP from device and
              tunnel respectively.
            - An ethernet header is added with SMAC and DMAC from l3 interface
              of egressing uplink port (based on default route's nexthop) and
              tunnel respectively.
            - Based on default route's nexthop, now this encapped packet is
              expected to egress one of the uplink port


## Testcases based on the type of rules in the policy

### No rules
    Trigger:
        - Policy without any rules is selected.
        - An IPv4 UDP packet with random DIP, sport, dport is sent to DSC SIM
          via host interface.

    Expectation:
        Packet gets dropped & no packets should egress the uplink ports.

### All rules with DENY action
    Trigger:
        - Policy with all rules having 'deny' action is selected.
        - An IPv4 packet with random IP (from random rule's DIP pfx) as DIP
          is sent to DSC SIM from selected policy's subnet's host interface.

      Expectation:
        Packet gets dropped & no packets should egress the uplink ports.

### Duplicate rules with different priority
    Trigger:
        - Policy with rules having same attributes except priority is selected.
        - A random rule is chosen from the selected policy for selecting
          packet's protocol, DIP, sport & dport.
        - An IPv4 packet is framed based of above chosen random rule
          and sent to DSC SIM from selected policy's subnet's host interface.

    Expectation:
        Packet gets dropped / egresses the uplink port based on rule of
        highest priority as explained in "Common Testcase scenarios"

### Duplicate rules but with different priority & action
    Trigger:
        - Policy with rules having same attributes except priority & 
          action is selected.
        - A random rule is chosen from the selected policy for selecting
          packet's protocol, DIP, sport & dport.
        - An IPv4 packet is framed based of above chosen random rule
          and sent to DSC SIM from selected policy's subnet's host interface.

    Expectation:
        Packet gets dropped / egresses the uplink port based on rule of
        highest priority as explained in "Common Testcase scenarios"

### Duplicate rules
    Trigger:
        - Policy with duplicate rules is selected.
        - A random rule is chosen from the selected policy for selecting
          packet's protocol, DIP, sport & dport.
        - An IPv4 packet is framed based of above chosen random rule
          and sent to DSC SIM from selected policy's subnet's host interface.

    Expectation:
        Packet gets dropped / egresses the uplink port based on rule of
        highest priority as explained in "Common Testcase scenarios"

### Duplicate rules with different action
    Trigger:
        - Policy with rules having same attributes except action is selected.
        - A random rule is chosen from the selected policy for selecting
          packet's protocol, DIP, sport & dport.
        - An IPv4 packet is framed based of above chosen random rule
          and sent to DSC SIM from selected policy's subnet's host interface.

    Expectation:
        As all rules are having same priority, based on the action of first rule
        in the selected policy, packet gets either dropped / egresses out as
        explained in 'Common Testcase scenarios'

The following testcases are run for all the policy types defined in
**'Initial setup'** section.

### Random values
    Trigger:
        Host to internet gateway packet with random destination IP, sport, dport
        from the rule's attributes is sent to host interface.

    Expectation:
        Packet gets dropped / egresses the uplink port based on rule of
        highest priority as explained in 'Common Testcase scenarios'

### First values
    Trigger:
        Host to internet gateway packet with DIP as first host address of DIP
        prefix, and sport & dport being the first value from rule's attributes
        is sent to host interface.

    Expectation:
        Packet gets dropped / egresses the uplink port based on rule of
        highest priority as explained in 'Common test procedure for all test cases'

### Last values
    Trigger:
        Host to internet gateway packet with DIP as last host address of DIP
        prefix, and sport & dport being the last value from rule's attributes
        is sent to host interface.

    Expectation:
        Packet gets dropped / egresses the uplink port based on rule of
        highest priority as explained in 'Common Testcase scenarios'
