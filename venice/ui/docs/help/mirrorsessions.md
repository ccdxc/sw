---
id: mirrorsessions
---
# Mirror Sessions

Venice allows to send specific remote mirrored traffic to an external destination collector through ERSPAN format. Users can send the mirrored traffic to multiple collectors. To configure the mirror sessions, the following are the configuration parameters.


<load-table group:monitoring obj:ApiObjectMeta
            include:name >
| Schedule | Allows user configuration on specific date and time |
<load-table group:monitoring obj:MonitoringMirrorSessionSpec
            include:packet-size omitHeader:true >
| Packet Filters | Default is set to "All Packets"  |

##### Collectors

<load-table group:monitoring obj:MonitoringMirrorCollector
            include:type>
<load-table group:monitoring obj:MonitoringMirrorExportConfig
            omitHeader:true>

###### Interface Selector

<load-table group:monitoring obj:MonitoringMirrorSessionSpec
            include:interface-selector>

##### Rules

This is the filters to apply before sending packets. Packets will only be exported if they match these rules.

<load-table group:monitoring obj:MonitoringMatchRule
            >

## Example:

**Name:** Trouleshoot_Session1

**Schedule:** select date and time

**Packet Size:** 1024

**Packet Filters:** All Packets

**Collectors:**

	- Type: ERSPAN

	- Destination: 10.29.30.21

**Interface Selectors:**

	- location

	- equals

	- HQ

**Rules:**

	- Source:
		- IP Address: 10.29.30.0/24
		- Mac Address:

	- Target:
		- IP Address: 10.29.31.0/24
		- Mac Address:
		
	- Protocol or App
		- Proto-Ports: tcp/80
		- Apps:
