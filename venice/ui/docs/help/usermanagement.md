---
id: localuser
---
# User Management

You can create local users, which will authenticate from the local-user database.  That is done under the "Admin" --> "User Management" section. Click "User" - "Add User" to create a new user. 


The user management section allows the creation of new users to the local database.  You can add a new user and provide the following information:


<load-table group:auth obj:AuthUserSpec
            include:fullname>
| Login Name | Unique login name |
<load-table group:auth obj:AuthUserSpec
            omit:fullname omitHeader:true>
| Bind to Role | Choose from the drop-down box for the available roles |

User can update his/her information and change his/her own password.

Administrator can update other users inforamtion and change pasword for other users.

# Role Binding

The role binding associates the user or user groups to a particular role.  The options for this configuration is the following:

- Name: Unique name for the role binding
- Role: Select from the drop down box the list of roles created
- User Groups:  Type in the name of the user group to bind to this role
- Users:  A list of local users are shown and to select the user, move it from the available list to Selected list

<load-table group:auth obj:ApiObjectMeta include:name>
<load-table group:auth obj:AuthRoleBindingSpec  omitHeader:true>

## Example:

**Name:** HRAdmins-binding

**Role:** HRAdminRole

**User Groups:** HRAdmin-Group

**Users:** johndoe


# Role Management

You can create role, which authorize user access.  That is done under the "Admin" --> "User Management" section. Click "Role" - "Add Role" to create a new user. 

#### Add And Update Role

With the roles configuration, it is possible to create various types of roles that provide various capabilities.  Here are the following options with the grouping option that corelates to the "kind" of capabilities with the grouping:

<load-table group:auth obj:ApiObjectMeta include:name>
<load-table group:auth obj:AuthPermission  omit:resource-tenant,resource-namespace,resource-names  omitHeader:true>

# Role configuration

UI pages include mulitple objects. PSM administrator should configure roles.

#### Cluster

To access cluster and cluster-node page (https://server:port/#/cluster/cluster and https://server:port/#/cluster/cluster/node1), you can set up a role and permissions as 

![CLUSTER_READ image](/images/roleconfig/cluster_read.png)


![NODE_READ image](/images/roleconfig/node_read.png)


To access cluster DSCs and DSC detail pages (https://server:port/#/cluster/dscs and https://server:port/#/cluster/dscs/xxx), you can set up a role and permissions as:

To view DSCs

![DSCS_READ image](/images/roleconfig/dscs_read.png)

To create, update or delete DSCs

![DSCS_CUD image](/images/roleconfig/dscs_cud.png)

To access cluster node page (https://server:port/#/cluster/hosts) you can set up a role and permissions as 

To view host

![hosts_read image](/images/roleconfig/hosts_read.png)

To create, update or delete host

![hosts_cud image](/images/roleconfig/hosts_cud.png)

#### Security

To access security policis and security-detail pages (https://server:port/#/security/sgpolicies, https://server:port/#security/sgpolicies/sg-1)  you can set up a role and permissions as 

To view policies or policy detail

![policies_read image](/images/roleconfig/policies_read.png)

To create, update or delete policies or policy detail

![policies_cud image](/images/roleconfig/policies_read.png)


To access security apps page (https://server:port/#/security/securityapps) you can set up a role and permissions as 

To view apps 

![policies_read image](/images/roleconfig/app_read.png)

To create, update or delete app

![policies_cud image](/images/roleconfig/app_cud.png)

#### Tech Support

To access tech support page  (https://server:port/#/admin/techsupport) you can set up a role and permissions as 

To view tech-supports 

![techsupport_read image](/images/roleconfig/techsupport_read.png)

To create, update or delete tech-support

![techsupport_cud image](/images/roleconfig/techsupport_cud.png)

#### Pages only accessible by admin role user

Only admin role user can access auth policy, system upgrade, snapshot and server certificate pages.  

![admin role image](/images/roleconfig/admin_role.png)

#### Manage User, Role and Rolebinding

To view user, role and rolebinding

![user_management_read image](/images/roleconfig/user_management_read.png)

To create, update or delete user, role and rolebinding

![user_management_cud image](/images/roleconfig/user_management_cud.png)


#### Alerts & Events

To access alerts and events pages,  (https://server:port/#/monitoring/alertsevents), you can setup role as

![alertsevents_crud image](/images/roleconfig/alertsevents_crud.png)

#### Event alert policy and alert destination

To access alert desination page (https://server:port/#/monitoring/alertsevents/alertdestinations), you can setup role as

![alertsdestination_crud image](/images/roleconfig/alertdestination_crud.png)

To access event alert policies page (https://server:port/#/monitoring/alertsevents/alertpolicies), you can setut role as

![event_alert_policy_crud image](/images/roleconfig/eventalert_policy_crud.png)

#### Metrics and Audit Logs

System requires user to have 'read all' permission to access metrics and audit logs

![read only image](/images/roleconfig/readonly.png)

#### Firewall Logs Policies

To manage firewall logs plicies, you can setup role as

![firewall log policies image](/images/roleconfig/firewalllog_policies_crud.png)

#### Flow Export

To manage flow export plicies, you can setup role as

![flow export image](/images/roleconfig/flowexport_policies_crud.png)