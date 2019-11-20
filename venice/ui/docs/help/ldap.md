---
id: ldap
---
## LDAP

LDAP stands for Lightweight Directory Access Protocol. As the name suggests, it is a lightweight client-server protocol for accessing directory services, specifically X.500-based directory services. LDAP runs over TCP/IP or other connection oriented transfer services.

![LDAP image](/images/ldap.png)

The toggle allows for enabling/disabling LDAP authentication.

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
