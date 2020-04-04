---
id: authpolicy
---
# Authentication Policy

An authentication policy provides authentication to verify the identity of an entity (person or device) accessing PSM platform. The authentication is based on the user ID and password combination provided by the entity trying to gain access. 

Authentication can be performed by

- [Local Users](%localuser)
- LDAP 
- RADIUS 

### LDAP
You can enable LDAP providers to the authentication access method.  To do so, enable the "Create LDAP Authenticator".

Once created, the toggle allows for enabling/disabling LDAP authentication.

### LDAP Configuration
##### Credentials
<load-table group:auth obj:AuthLdapDomain 
            include:bind-dn, bind-password>

##### Scope
<load-table group:auth obj:AuthLdapDomain 
            omit:bind-dn, bind-password, attribute-mapping, servers, tag>
<load-table group:auth obj:LdapAttributeMapping include:user-object-class,group-object-class omitHeader:true>

##### Attribute Mapping
<load-table group:auth obj:LdapAttributeMapping omit:user-object-class, group-object-class>

##### LDAP Server Configuration
<load-table group:auth obj:LdapServer
            omit:tls-options>
<load-table group:auth obj:TLSOptions omitHeader:true>

### RADIUS
RADIUS is a distributed client/server system that secures networks against unauthorized access. A RADIUS provider is a remote server supporting the RADIUS protocol that will be used for authentication. To use the RADIUS authentication method, enable the "Create RADIUS Authenticator". 

Once created, the toggle allows for enabling/disabling Radius authentication.

<load-table group:auth obj:AuthRadiusDomain
            omit:servers, tag>
| Server:Port   | &lt;IP address&gt;:&lt;Port&gt; of the RADIUS server  |
| Server secret   | Secret is the shared secret between PSM and RADIUS server  |
<load-table group:auth obj:RadiusServer include:auth-method
            omitHeader:true>

