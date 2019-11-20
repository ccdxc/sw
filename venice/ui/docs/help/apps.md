---
id: apps
---

# Apps
Creating Apps allows to define specific protocol and ports that makes up the particular application.  Support for application-level gateway (ALG) is provided with the following predefined ALGs

- icmp (ping)
- dns (domain name system)
- ftp (file transfer protocol)
- sunrpc (Sun Remote Procedure Call)
- msrpc (Microsoft Remote Procedure Call)

Building "apps" can be based upon these two (2) options and security rules can be applied towards this app.  The "App" is based upon the following attributes.

<load-table group:security obj:ApiObjectMeta
            include:name >
<load-table group:security obj:SecurityAppSpec
            include:filter omitHeader:true >
Name: Unique name for the App

There are three (3) options to choose from to base the App on through the radio button

	- ALG Only
	- Protocols and Port Only
	- Both 

### ALG Only
	| ALG Type | Fields |
	| ------ | ------ |
	| icmp | Type: <br> Code: |
	| dns | Drop multi-question packets: (off/on) <br> Drop large domain name packets: (off/on) <br> Drop long label packets: (off/on) <br>  Max Message Length: Integer value <br> Max question response timeout: |
	| ftp | Allow Mismatched IP Address: (off/on) |
	| sunrpc |  Additional Program ID and Timeout values can be added by clicking on "+Add <br> Program ID: <br> Timeout: Integer value |
	| msrpc |  Additional Program UUID and Timeout values can be added by clicking on "+Add <br> Program UUID: <br> Timeout:  Integer value |
			
### Protocols and Ports Only
Additional Protocols and Ports can be added by clicking on "+Add"

<load-table group:security obj:SecurityProtoPort
            >
- Protocols and Ports Only
	- Protocols: valid entries is tcp, upd, icmp
	- Ports: valid port numbers are 0-65535

	
### Both
	Combination of ALG and Protocols/Ports is allowed
