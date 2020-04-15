List of non-templatized testcases for policy

Update cases -
Add
1. Add rules to an empty policy
    - l3 and l4
2. Add rules to a non-empty policy
    - l3 and l4
3. Add l3 rules to a policy with only l4 rules
4. Add l4 rules to a policy with only l3 rules
5. Add wildcard rules to an empty policy
    - l3 and l4
5. Add wildcard rules to a non-empty policy
    - l3 and l4

Delete
1. Delete a rule from a policy with more than one rule
2. Delete a rule from a policy with one rule

Rule updates
1. Modify stateful
2. Modify priority
3. Modify l3 match attributes
4. Modify l4 match atrributes
5. Modify both
6. Modify default action - allow/deny
7. Modify l3 rule to wildcard
8. Modify l4 rule to wildcard
    - icmp type wildcard
    - icmp type and code wildcard
