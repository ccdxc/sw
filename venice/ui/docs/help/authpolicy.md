---
id: authpolicy
---
# Authentication Policy

An authentication policy provides authentication to verify the identity of an entity (person or device) accessing Venice devices. The authentication is based on the user ID and password combination provided by the entity trying to gain access. Authentication can be performed locally, using the local database, or remotely, using one or more LDAP or RADIUS servers.

### Local
You can create local users which authentication will check with local-user database.

### LDAP
You can create LDAP providers and LDAP provider groups and configure the default LDAP authentication settings.
[Learn more](%ldap)

### Radius
RADIUS is a distributed client/server system that secures networks against unauthorized access. A RADIUS provider is a remote server supporting the RADIUS protocol that will be used for authentication.
[Learn more](%radius)