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

# Role Management

You can create role, which authorize user access.  That is done under the "Admin" --> "User Management" section. Click "Role" - "Add Role" to create a new user. 

#### Add And Update Role

With the roles configuration, it is possible to create various types of roles that provide various capabilities.  Here are the following options with the grouping option that corelates to the "kind" of capabilities with the grouping:

<load-table group:auth obj:ApiObjectMeta include:name>
<load-table group:auth obj:AuthPermission  omit:resource-tenant,resource-namespace,resource-names  omitHeader:true>


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



