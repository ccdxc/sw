---
id: ldap
---
## LDAP

LDAP stands for Lightweight Directory Access Protocol. As the name suggests, it is a lightweight client-server protocol for accessing directory services, specifically X.500-based directory services. LDAP runs over TCP/IP or other connection oriented transfer services.

![LDAP image](/images/ldap.png)

The toggle allows for enabling/disabling LDAP authentication.

##### LDAP Configuration
<load-table group:auth obj:LDAP 
            omit:enable, attribute-mapping, servers>
<load-table group:auth obj:LdapAttributeMapping
            omitHeader:true>

##### LDAP Server Configuration
<load-table group:auth obj:LdapServer
            omit:tls-options>
<load-table group:auth obj:TLSOptions omitHeader:true>

Footer info about LDAP