---
id: policies
---
  
# Network Security Policies
In this Network Security Policy section, network security rules are created.  The security policy rules can be associated to either a Security Group or a Tenant. To create the various rules to the security policy, they will have the following attributes.

<load-table group:security obj:ApiObjectMeta
            include:name >
| Rule Number | The order of this rule. Must be an integer. |
| Action | The rule action, etierh Permit, Deny, or Reject. <br> Deny - Packet will be dropped and no response is sent back to the source. <br> Reject - Packet will be dropped but a packet "port unreachable" will be sent to the source. <br> Permit - Allows the packet to be forwarded |
<load-table group:security obj:SecuritySGRule
            omitHeader: true >

If a policy already exist, you are able to modify the rules of the policy.  To do this, hover over to the right side of the policy and a pencil icon will appear to edit the policy.  To remove the policy, click on the trash can icon to delete the policy.

When modifying an existing policy, you can do the following:

	Change the order of the rule.  To change the order, click on the check box of the rule you want to change.  Then enter in the number where you want the rule to be moved to.  Then click on box "Change Order" to commit the change.

	Insert a new rule.  Hover over the entry and to the right side of the entry a "+" sign will appear.  Click on it to insert a new rule.
	
	Editing or deleting one or more existing rules.  To modify or delete one or more existing rules, select the rules on the check-box and right above rules list, the pencil and trash bin icon will appear.  To modify the selected rules, click on the pencil icon.  To remove the selected rules, click on the trash bin icon.



## Example:

**Name:** DC1-Security

**Attach:** Tenant

**Rules:**

	1 	
		Action: Permit
		From: 0.0.0.0/0
		To: DHCP-Servers
		Proto-Ports:
			Protocol: udp
			Ports: 67-68
	2
		Action: Permit
		From: 10.29.100.0/24
		To: 10.29.50.101 10.29.50.102
		Proto-Ports:
			Protocol: tcp
			Ports: 80, 443

 