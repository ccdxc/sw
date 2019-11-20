---
id: localuser
---
# User Management

You can create local users, which will authenticate from the local-user database.  That is done under the "Admin" --> "User Management" section. Click "Add User" to create a new user. 


#### Add User

The user management section allows the creation of new users to the local database.  You can add a new user and provide the following information:


<load-table group:auth obj:AuthUserSpec
            include:fullname>
| Login Name | Unique login name |
<load-table group:auth obj:AuthUserSpec
            omit:fullname omitHeader:true>
| Bind to Role | Choose from the drop-down box for the available roles |