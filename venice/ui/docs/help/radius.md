---
id: radius 
---
## Radius

Remote Authentication Dial-In User Service is a networking protocol, operating on port 1812 that provides centralized Authentication, Authorization, and Accounting management for users who connect and use a network service. 

The toggle allows for enabling/disabling Radius authentication.

<load-table group:auth obj:AuthRadiusDomain
            omit:servers, tag>
| Server:Port   | &lt;IP address&gt;:&lt;Port&gt; of the RADIUS server  |
| Server secret   | Secret is the shared secret between PSM and RADIUS server  |
<load-table group:auth obj:RadiusServer include:auth-method
            omitHeader:true>
