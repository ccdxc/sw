---
id: securitygroups
---

# Security Groups
Creating a security group allows the grouping of workloads based upon an ip prefix or based upon labels.  This provides efficiency when it comes to building security rules based upon these security groups versus individual workloads.  To create this security group, this object has the following attributes.

<load-table group:security obj:ApiObjectMeta
            include:name >
<load-table group:security obj:SecuritySecurityGroupSpec
            omitHeader: true >


- Name: Unique name for the security group
- Match Prefixes: The prefix is anything that matches within a particular subnet
- Workload labels: Further filtering based upon workload labels is allowed. 
	- The first box is the label or category that you want filter by
	- The equation allows for "equal" or "not equals"
	- The second box is the attribute for the label/category
- Service labels: ? (is this containers?)
	- The first box is the label or category that you want filter by
	- The equation allows for "equal" or "not equals"
	- The second box is the attribute for the label/category

Click on "Create Security Group" to complete the task

## Example:

**Name:** Web-Servers

**Match-Prefixes:** 192.168.10.0/24

**Workload Labels:** Operating System  equals Centos7
