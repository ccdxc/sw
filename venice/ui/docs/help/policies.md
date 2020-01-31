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

To add another rule, click on "+RULE".  

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

 