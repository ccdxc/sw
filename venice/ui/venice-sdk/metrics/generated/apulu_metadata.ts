import { MetricMeasurement } from './metadata';

export const MetricsMetadataApulu: { [key: string]: MetricMeasurement } = {
  FlowStatsSummary: {
  "name": "FlowStatsSummary",
  "description": "Session Summary",
  "displayName": "Flow Stats Summary",
  "tags": [
    "Level4"
  ],
  "scope": "PerASIC",
  "fields": [
    {
      "name": "TCP sessions over IPv4",
      "displayName": "TCP sessions over IPv4",
      "description": "TCP sessions over IPv4",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "UDP sessions over IPv4",
      "displayName": "UDP sessions over IPv4",
      "description": "UDP sessions over IPv4",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "ICMP sessions over IPv4",
      "displayName": "ICMP sessions over IPv4",
      "description": "ICMP sessions over IPv4",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Other sessions over IPv4",
      "displayName": "Other sessions over IPv4",
      "description": "Other sessions over IPv4",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "TCP sessions over IPv6",
      "displayName": "TCP sessions over IPv6",
      "description": "TCP sessions over IPv6",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "UDP sessions over IPv6",
      "displayName": "UDP sessions over IPv6",
      "description": "UDP sessions over IPv6",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "ICMP sessions over IPv6",
      "displayName": "ICMP sessions over IPv6",
      "description": "ICMP sessions over IPv6",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Other sessions over IPv6",
      "displayName": "Other sessions over IPv6",
      "description": "Other sessions over IPv6",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "L2 sessions",
      "displayName": "L2 sessions",
      "description": "L2 sessions",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Session create errors",
      "displayName": "Session create errors",
      "description": "Session create errors",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "reporterID",
      "description": "Name of reporting object",
      "baseType": "string",
      "jsType": "string",
      "isTag": true,
      "displayName": "reporterID",
      "tags": [
        "Level4"
      ]
    }
  ],
  "objectKind": "DistributedServiceCard"
},
  MacMetrics: {
  "name": "MacMetrics",
  "description": "Key index - PORT ID",
  "displayName": "Uplink Interface Packet Statistics",
  "tags": [
    "Level4"
  ],
  "scope": "PerPort",
  "fields": [
    {
      "name": "Frames Rx ok",
      "displayName": "Frames Rx ok",
      "description": "Frames received OK",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx all",
      "displayName": "Frames Rx all",
      "description": "Frames Received All (Good and Bad Frames)",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx bad fcs",
      "displayName": "Frames Rx bad fcs",
      "description": "Frames Received with Bad FCS",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx bad all",
      "displayName": "Frames Rx bad all",
      "description": "Frames with any bad (CRC, Length, Align)",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Octets Rx ok",
      "displayName": "Octets Rx ok",
      "description": "Octets Received in Good Frames",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Octets Rx all",
      "displayName": "Octets Rx all",
      "description": "Octets Received (Good/Bad Frames)",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx unicast",
      "displayName": "Frames Rx unicast",
      "description": "Frames Received with Unicast Address",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx multicast",
      "displayName": "Frames Rx multicast",
      "description": "Frames Received with Multicast Address",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx broadcast",
      "displayName": "Frames Rx broadcast",
      "description": "Frames Received with Broadcast Address",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx pause",
      "displayName": "Frames Rx pause",
      "description": "Frames Received of type PAUSE",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx bad length",
      "displayName": "Frames Rx bad length",
      "description": "Frames Received with Bad Length",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx undersized",
      "displayName": "Frames Rx undersized",
      "description": "Frames Received Undersized",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx oversized",
      "displayName": "Frames Rx oversized",
      "description": "Frames Received Oversized",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx fragments",
      "displayName": "Frames Rx fragments",
      "description": "Fragments Received",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx jabber",
      "displayName": "Frames Rx jabber",
      "description": "Jabber Received",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx priority pause",
      "displayName": "Frames Rx priority pause",
      "description": "Priority Pause Frames Received",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx stomped crc",
      "displayName": "Frames Rx stomped crc",
      "description": "Stomped CRC Frames Received",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx too long",
      "displayName": "Frames Rx too long",
      "description": "Received Frames Too Long",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx vlan good",
      "displayName": "Frames Rx vlan good",
      "description": "Received VLAN Frames (Good)",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx drop",
      "displayName": "Frames Rx drop",
      "description": "Received Frames Dropped (Buffer Full)",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx less than 64B",
      "displayName": "Frames Rx less than 64B",
      "description": "Frames Received Length less than 64",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx 64B",
      "displayName": "Frames Rx 64B",
      "description": "Frames Received Length=64",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx 65B-127B",
      "displayName": "Frames Rx 65B-127B",
      "description": "Frames Received Length=65~127",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx 128B-255B",
      "displayName": "Frames Rx 128B-255B",
      "description": "Frames Received Length=128~255",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx 256B-511B",
      "displayName": "Frames Rx 256B-511B",
      "description": "Frames Received Length=256~511",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx 512B-1023B",
      "displayName": "Frames Rx 512B-1023B",
      "description": "Frames Received Length=512~1023",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx 1024B-1518B",
      "displayName": "Frames Rx 1024B-1518B",
      "description": "Frames Received Length=1024~1518",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx 1519B-2047B",
      "displayName": "Frames Rx 1519B-2047B",
      "description": "Frames Received Length=1519~2047",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx 2048B-4095B",
      "displayName": "Frames Rx 2048B-4095B",
      "description": "Frames Received Length=2048~4095",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx 4096B-8191B",
      "displayName": "Frames Rx 4096B-8191B",
      "description": "Frames Received Length=4096~8191",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx 8192B-9215B",
      "displayName": "Frames Rx 8192B-9215B",
      "description": "Frames Received Length=8192~9215",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx other",
      "displayName": "Frames Rx other",
      "description": "Frames Received Length greater than 9215",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Tx ok",
      "displayName": "Frames Tx ok",
      "description": "Frames Transmitted OK",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Tx all",
      "displayName": "Frames Tx all",
      "description": "Frames Transmitted All (Good/Bad Frames)",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Tx bad",
      "displayName": "Frames Tx bad",
      "description": "Frames Transmitted Bad",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Octets Tx ok",
      "displayName": "Octets Tx ok",
      "description": "Octets Transmitted Good",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Octets Tx total",
      "displayName": "Octets Tx total",
      "description": "Octets Transmitted Total (Good/Bad)",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Tx unicast",
      "displayName": "Frames Tx unicast",
      "description": "Frames Transmitted with Unicast Address",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Tx multicast",
      "displayName": "Frames Tx multicast",
      "description": "Frames Transmitted with Multicast Address",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Tx broadcast",
      "displayName": "Frames Tx broadcast",
      "description": "Frames Transmitted with Broadcast Address",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Tx pause",
      "displayName": "Frames Tx pause",
      "description": "Frames Transmitted of type PAUSE",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Tx priority pause",
      "displayName": "Frames Tx priority pause",
      "description": "Frames Transmitted of type PriPAUSE",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Tx vlan",
      "displayName": "Frames Tx vlan",
      "description": "Frames Transmitted VLAN",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Tx less than 64B",
      "displayName": "Frames Tx less than 64B",
      "description": "Frames Transmitted Length<64",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Tx 64B",
      "displayName": "Frames Tx 64B",
      "description": "Frames Transmitted Length=64",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Tx 65B-127B",
      "displayName": "Frames Tx 65B-127B",
      "description": "Frames Transmitted Length=65~127",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Tx 128B-255B",
      "displayName": "Frames Tx 128B-255B",
      "description": "Frames Transmitted Length=128~255",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Tx 256B-511B",
      "displayName": "Frames Tx 256B-511B",
      "description": "Frames Transmitted Length=256~511",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Tx 512B-1023B",
      "displayName": "Frames Tx 512B-1023B",
      "description": "Frames Transmitted Length=512~1023",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Tx 1024B-1518B",
      "displayName": "Frames Tx 1024B-1518B",
      "description": "Frames Transmitted Length=1024~1518",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Tx 1519B-2047B",
      "displayName": "Frames Tx 1519B-2047B",
      "description": "Frames Transmitted Length=1519~2047",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Tx 2048B-4095B",
      "displayName": "Frames Tx 2048B-4095B",
      "description": "Frames Transmitted Length=2048~4095",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Tx 4096B-8191B",
      "displayName": "Frames Tx 4096B-8191B",
      "description": "Frames Transmitted Length=4096~8191",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Tx 8192B-9215B",
      "displayName": "Frames Tx 8192B-9215B",
      "description": "Frames Transmitted Length=8192~9215",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Tx other",
      "displayName": "Frames Tx other",
      "description": "Frames Transmitted Length greater than 9215",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Tx priority 0",
      "displayName": "Frames Tx priority 0",
      "description": "Pri#0 Frames Transmitted",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Tx priority 1",
      "displayName": "Frames Tx priority 1",
      "description": "Pri#1 Frames Transmitted",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Tx priority 2",
      "displayName": "Frames Tx priority 2",
      "description": "Pri#2 Frames Transmitted",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Tx priority 3",
      "displayName": "Frames Tx priority 3",
      "description": "Pri#3 Frames Transmitted",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Tx priority 4",
      "displayName": "Frames Tx priority 4",
      "description": "Pri#4 Frames Transmitted",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Tx priority 5",
      "displayName": "Frames Tx priority 5",
      "description": "Pri#5 Frames Transmitted",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Tx priority 6",
      "displayName": "Frames Tx priority 6",
      "description": "Pri#6 Frames Transmitted",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Tx priority 7",
      "displayName": "Frames Tx priority 7",
      "description": "Pri#7 Frames Transmitted",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx priority 0",
      "displayName": "Frames Rx priority 0",
      "description": "Pri#0 Frames Received",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx priority 1",
      "displayName": "Frames Rx priority 1",
      "description": "Pri#1 Frames Received",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx priority 2",
      "displayName": "Frames Rx priority 2",
      "description": "Pri#2 Frames Received",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx priority 3",
      "displayName": "Frames Rx priority 3",
      "description": "Pri#3 Frames Received",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx priority 4",
      "displayName": "Frames Rx priority 4",
      "description": "Pri#4 Frames Received",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx priority 5",
      "displayName": "Frames Rx priority 5",
      "description": "Pri#5 Frames Received",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx priority 6",
      "displayName": "Frames Rx priority 6",
      "description": "Pri#6 Frames Received",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx priority 7",
      "displayName": "Frames Rx priority 7",
      "description": "Pri#7 Frames Received",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Tx priority pause 0 1us count",
      "displayName": "Tx priority pause 0 1us count",
      "description": "Transmit Pri#0 Pause 1US Count",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Tx priority pause 1 1us count",
      "displayName": "Tx priority pause 1 1us count",
      "description": "Transmit Pri#1 Pause 1US Count",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Tx priority pause 2 1us count",
      "displayName": "Tx priority pause 2 1us count",
      "description": "Transmit Pri#2 Pause 1US Count",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Tx priority pause 3 1us count",
      "displayName": "Tx priority pause 3 1us count",
      "description": "Transmit Pri#3 Pause 1US Count",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Tx priority pause 4 1us count",
      "displayName": "Tx priority pause 4 1us count",
      "description": "Transmit Pri#4 Pause 1US Count",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Tx priority pause 5 1us count",
      "displayName": "Tx priority pause 5 1us count",
      "description": "Transmit Pri#5 Pause 1US Count",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Tx priority pause 6 1us count",
      "displayName": "Tx priority pause 6 1us count",
      "description": "Transmit Pri#6 Pause 1US Count",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Tx priority pause 7 1us count",
      "displayName": "Tx priority pause 7 1us count",
      "description": "Transmit Pri#7 Pause 1US Count",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Rx priority pause 0 1us count",
      "displayName": "Rx priority pause 0 1us count",
      "description": "Receive Pri#0 Pause 1US Count",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Rx priority pause 1 1us count",
      "displayName": "Rx priority pause 1 1us count",
      "description": "Receive Pri#1 Pause 1US Count",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Rx priority pause 2 1us count",
      "displayName": "Rx priority pause 2 1us count",
      "description": "Receive Pri#2 Pause 1US Count",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Rx priority pause 3 1us count",
      "displayName": "Rx priority pause 3 1us count",
      "description": "Receive Pri#3 Pause 1US Count",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Rx priority pause 4 1us count",
      "displayName": "Rx priority pause 4 1us count",
      "description": "Receive Pri#4 Pause 1US Count",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Rx priority pause 5 1us count",
      "displayName": "Rx priority pause 5 1us count",
      "description": "Receive Pri#5 Pause 1US Count",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Rx priority pause 6 1us count",
      "displayName": "Rx priority pause 6 1us count",
      "description": "Receive Pri#6 Pause 1US Count",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Rx priority pause 7 1us count",
      "displayName": "Rx priority pause 7 1us count",
      "description": "Receive Pri#7 Pause 1US Count",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Rx pause 1us count",
      "displayName": "Rx pause 1us count",
      "description": "Receive Standard Pause 1US Count",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Tx truncated",
      "displayName": "Frames Tx truncated",
      "description": "Frames Truncated",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "reporterID",
      "description": "Name of reporting object",
      "baseType": "string",
      "jsType": "string",
      "isTag": true,
      "displayName": "reporterID",
      "tags": [
        "Level4"
      ]
    }
  ],
  "objectKind": "DistributedServiceCard"
},
  MgmtMacMetrics: {
  "name": "MgmtMacMetrics",
  "description": "Key index - PORT ID",
  "displayName": "Management Interface Packet Statistics",
  "tags": [
    "Level4"
  ],
  "scope": "PerPort",
  "fields": [
    {
      "name": "Frames Rx ok",
      "displayName": "Frames Rx ok",
      "description": "Frames received OK",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx all",
      "displayName": "Frames Rx all",
      "description": "Frames Received All (Good and Bad Frames)",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx bad fcs",
      "displayName": "Frames Rx bad fcs",
      "description": "Frames Received with Bad FCS",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx bad all",
      "displayName": "Frames Rx bad all",
      "description": "Frames with any bad (CRC, Length, Align)",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Octets Rx ok",
      "displayName": "Octets Rx ok",
      "description": "Octets Received in Good Frames",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Octets Rx all",
      "displayName": "Octets Rx all",
      "description": "Octets Received (Good/Bad Frames)",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx unicast",
      "displayName": "Frames Rx unicast",
      "description": "Frames Received with Unicast Address",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx multicast",
      "displayName": "Frames Rx multicast",
      "description": "Frames Received with Multicast Address",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx broadcast",
      "displayName": "Frames Rx broadcast",
      "description": "Frames Received with Broadcast Address",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx pause",
      "displayName": "Frames Rx pause",
      "description": "Frames Received of type PAUSE",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx bad length",
      "displayName": "Frames Rx bad length",
      "description": "Frames Received with Bad Length",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx undersized",
      "displayName": "Frames Rx undersized",
      "description": "Frames Received Undersized",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx oversized",
      "displayName": "Frames Rx oversized",
      "description": "Frames Received Oversized",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx fragments",
      "displayName": "Frames Rx fragments",
      "description": "Fragments Received",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx jabber",
      "displayName": "Frames Rx jabber",
      "description": "Jabber Received",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx 64B",
      "displayName": "Frames Rx 64B",
      "description": "Frames Received Length=64",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx 65B-127B",
      "displayName": "Frames Rx 65B-127B",
      "description": "Frames Received Length=65~127",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx 128B-255B",
      "displayName": "Frames Rx 128B-255B",
      "description": "Frames Received Length=128~255",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx 256B-511B",
      "displayName": "Frames Rx 256B-511B",
      "description": "Frames Received Length=256~511",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx 512B-1023B",
      "displayName": "Frames Rx 512B-1023B",
      "description": "Frames Received Length=512~1023",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx 1024B-1518B",
      "displayName": "Frames Rx 1024B-1518B",
      "description": "Frames Received Length=1024~1518",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx > 1518B",
      "displayName": "Frames Rx > 1518B",
      "description": "Frames Received Length greater than 1518",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx fifo full",
      "displayName": "Frames Rx fifo full",
      "description": "Frames Received FIFO Full",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Tx ok",
      "displayName": "Frames Tx ok",
      "description": "Frames Transmitted OK",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Tx all",
      "displayName": "Frames Tx all",
      "description": "Frames Transmitted All (Good/Bad Frames)",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Tx bad",
      "displayName": "Frames Tx bad",
      "description": "Frames Transmitted Bad",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Octets Tx ok",
      "displayName": "Octets Tx ok",
      "description": "Octets Transmitted Good",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Octets Tx total",
      "displayName": "Octets Tx total",
      "description": "Octets Transmitted Total (Good/Bad)",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Tx unicast",
      "displayName": "Frames Tx unicast",
      "description": "Frames Transmitted with Unicast Address",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Tx multicast",
      "displayName": "Frames Tx multicast",
      "description": "Frames Transmitted with Multicast Address",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Tx broadcast",
      "displayName": "Frames Tx broadcast",
      "description": "Frames Transmitted with Broadcast Address",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Tx pause",
      "displayName": "Frames Tx pause",
      "description": "Frames Transmitted of type PAUSE",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "reporterID",
      "description": "Name of reporting object",
      "baseType": "string",
      "jsType": "string",
      "isTag": true,
      "displayName": "reporterID",
      "tags": [
        "Level4"
      ]
    }
  ],
  "objectKind": "DistributedServiceCard"
},
  LifMetrics: {
  "name": "LifMetrics",
  "description": "Key index - LIF IfIndex",
  "displayName": "Logical Interface Statistics",
  "tags": [
    "Level4"
  ],
  "scope": "PerLIF",
  "fields": [
    {
      "name": "Octets Rx unicast",
      "displayName": "Octets Rx unicast",
      "description": "Octets Rx unicast",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx unicast",
      "displayName": "Frames Rx unicast",
      "description": "Frames Rx unicast",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Octets Rx multicast",
      "displayName": "Octets Rx multicast",
      "description": "Octets Rx multicast",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx multicast",
      "displayName": "Frames Rx multicast",
      "description": "Frames Rx multicast",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Octets Rx broadcast",
      "displayName": "Octets Rx broadcast",
      "description": "Octets Rx broadcast",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx broadcast",
      "displayName": "Frames Rx broadcast",
      "description": "Frames Rx broadcast",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Octets Rx unicast drop",
      "displayName": "Octets Rx unicast drop",
      "description": "Octets Rx unicast drop",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx unicast drop",
      "displayName": "Frames Rx unicast drop",
      "description": "Frames Rx unicast drop",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Octets Rx multicast drop",
      "displayName": "Octets Rx multicast drop",
      "description": "Octets Rx multicast drop",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx multicast drop",
      "displayName": "Frames Rx multicast drop",
      "description": "Frames Rx multicast drop",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Octets Rx broadcast drop",
      "displayName": "Octets Rx broadcast drop",
      "description": "Octets Rx broadcast drop",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx broadcast drop",
      "displayName": "Frames Rx broadcast drop",
      "description": "Frames Rx broadcast drop",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Rx DMA error",
      "displayName": "Rx DMA error",
      "description": "Rx DMA error",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Octets Tx unicast",
      "displayName": "Octets Tx unicast",
      "description": "Octets Tx unicast",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Tx unicast",
      "displayName": "Frames Tx unicast",
      "description": "Frames Tx unicast",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Octets Tx multicast",
      "displayName": "Octets Tx multicast",
      "description": "Octets Tx multicast",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Tx multicast",
      "displayName": "Frames Tx multicast",
      "description": "Frames Tx multicast",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Octets Tx broadcast",
      "displayName": "Octets Tx broadcast",
      "description": "Octets Tx broadcast",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Tx broadcast",
      "displayName": "Frames Tx broadcast",
      "description": "Frames Tx broadcast",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Octets Tx unicast drop",
      "displayName": "Octets Tx unicast drop",
      "description": "Octets Tx unicast drop",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Tx unicast drop",
      "displayName": "Frames Tx unicast drop",
      "description": "Frames Tx unicast drop",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Octets Tx multicast drop",
      "displayName": "Octets Tx multicast drop",
      "description": "Octets Tx multicast drop",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Tx multicast drop",
      "displayName": "Frames Tx multicast drop",
      "description": "Frames Tx multicast drop",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Octets Tx broadcast drop",
      "displayName": "Octets Tx broadcast drop",
      "description": "Octets Tx broadcast drop",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Tx broadcast drop",
      "displayName": "Frames Tx broadcast drop",
      "description": "Frames Tx broadcast drop",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Tx DMA error",
      "displayName": "Tx DMA error",
      "description": "Tx DMA error",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx queue disable drops",
      "displayName": "Frames Rx queue disable drops",
      "description": "Frames Rx queue disable drops",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx queue empty drops",
      "displayName": "Frames Rx queue empty drops",
      "description": "Frames Rx queue empty drops",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Rx queue scheduling errors",
      "displayName": "Rx queue scheduling errors",
      "description": "Rx queue scheduling errors",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Rx descriptor fetch errors",
      "displayName": "Rx descriptor fetch errors",
      "description": "Rx descriptor fetch errors",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Rx descriptor data fetch errors",
      "displayName": "Rx descriptor data fetch errors",
      "description": "Rx descriptor data fetch errors",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Tx queue disabled drops",
      "displayName": "Frames Tx queue disabled drops",
      "description": "Frames Tx queue disabled drops",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Tx queue scheduling errors",
      "displayName": "Tx queue scheduling errors",
      "description": "Tx queue scheduling errors",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Tx descriptor fetch errors",
      "displayName": "Tx descriptor fetch errors",
      "description": "Tx descriptor fetch errors",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Tx descriptor data fetch errors",
      "displayName": "Tx descriptor data fetch errors",
      "description": "Tx descriptor data fetch errors",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Octets Tx RDMA unicast",
      "displayName": "Octets Tx RDMA unicast",
      "description": "Octets Tx RDMA unicast",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Tx RDMA unicast",
      "displayName": "Frames Tx RDMA unicast",
      "description": "Frames Tx RDMA unicast",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Octets Tx RDMA multicast",
      "displayName": "Octets Tx RDMA multicast",
      "description": "Octets Tx RDMA multicast",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Tx RDMA multicast",
      "displayName": "Frames Tx RDMA multicast",
      "description": "Frames Tx RDMA multicast",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Tx RDMA CNP",
      "displayName": "Frames Tx RDMA CNP",
      "description": "Frames Tx RDMA CNP",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Octets Rx RDMA unicast",
      "displayName": "Octets Rx RDMA unicast",
      "description": "Octets Rx RDMA unicast",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx RDMA unicast",
      "displayName": "Frames Rx RDMA unicast",
      "description": "Frames Rx RDMA unicast",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Octets Rx RDMA multicast",
      "displayName": "Octets Rx RDMA multicast",
      "description": "Octets Rx RDMA multicast",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx RDMA multicast",
      "displayName": "Frames Rx RDMA multicast",
      "description": "Frames Rx RDMA multicast",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx RDMA CNP",
      "displayName": "Frames Rx RDMA CNP",
      "description": "Frames Rx RDMA CNP",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx RDMA ECN",
      "displayName": "Frames Rx RDMA ECN",
      "description": "Frames Rx RDMA ECN",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx RDMA request sequence errors",
      "displayName": "Frames Rx RDMA request sequence errors",
      "description": "Frames Rx RDMA request sequence errors",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Rx RDMA request RNR retry errors",
      "displayName": "Rx RDMA request RNR retry errors",
      "description": "Rx RDMA request RNR retry errors",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Rx RDMA request remote access errors",
      "displayName": "Rx RDMA request remote access errors",
      "description": "Rx RDMA request remote access errors",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Rx RDMA request remote invalid request errors",
      "displayName": "Rx RDMA request remote invalid request errors",
      "description": "Rx RDMA request remote invalid request errors",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Rx RDMA request remote operation errors",
      "displayName": "Rx RDMA request remote operation errors",
      "description": "Rx RDMA request remote operation errors",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Rx RDMA request implied NACK sequence errors",
      "displayName": "Rx RDMA request implied NACK sequence errors",
      "description": "Rx RDMA request implied NACK sequence errors",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Rx RDMA request CQE errors",
      "displayName": "Rx RDMA request CQE errors",
      "description": "Rx RDMA request CQE errors",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Rx RDMA request CQE flush errors",
      "displayName": "Rx RDMA request CQE flush errors",
      "description": "Rx RDMA request CQE flush errors",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Rx RDMA request duplicate responses",
      "displayName": "Rx RDMA request duplicate responses",
      "description": "Rx RDMA request duplicate responses",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Frames Rx RDMA request invalid",
      "displayName": "Frames Rx RDMA request invalid",
      "description": "Frames Rx RDMA request invalid",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Tx RDMA request local access errors",
      "displayName": "Tx RDMA request local access errors",
      "description": "Tx RDMA request local access errors",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Tx RDMA request local operation errors",
      "displayName": "Tx RDMA request local operation errors",
      "description": "Tx RDMA request local operation errors",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Tx RDMA request memory management errors",
      "displayName": "Tx RDMA request memory management errors",
      "description": "Tx RDMA request memory management errors",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Rx RDMA response duplicate request errors",
      "displayName": "Rx RDMA response duplicate request errors",
      "description": "Rx RDMA response duplicate request errors",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Rx RDMA response out of buffer errors",
      "displayName": "Rx RDMA response out of buffer errors",
      "description": "Rx RDMA response out of buffer errors",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Rx RDMA response out of sequence errors",
      "displayName": "Rx RDMA response out of sequence errors",
      "description": "Rx RDMA response out of sequence errors",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Rx RDMA response CQE errors",
      "displayName": "Rx RDMA response CQE errors",
      "description": "Rx RDMA response CQE errors",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Rx RDMA response CQE flush errors",
      "displayName": "Rx RDMA response CQE flush errors",
      "description": "Rx RDMA response CQE flush errors",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Rx RDMA response local length errors",
      "displayName": "Rx RDMA response local length errors",
      "description": "Rx RDMA response local length errors",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Rx RDMA response invalid request errors",
      "displayName": "Rx RDMA response invalid request errors",
      "description": "Rx RDMA response invalid request errors",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Rx RDMA response local QP operation errors",
      "displayName": "Rx RDMA response local QP operation errors",
      "description": "Rx RDMA response local QP operation errors",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Rx RDMA response out of atomic resources errors",
      "displayName": "Rx RDMA response out of atomic resources errors",
      "description": "Rx RDMA response out of atomic resources errors",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Tx RDMA response packet sequence errors",
      "displayName": "Tx RDMA response packet sequence errors",
      "description": "Tx RDMA response packet sequence errors",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Tx RDMA response remote invalid request errors",
      "displayName": "Tx RDMA response remote invalid request errors",
      "description": "Tx RDMA response remote invalid request errors",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Tx RDMA response remote access errors",
      "displayName": "Tx RDMA response remote access errors",
      "description": "Tx RDMA response remote access errors",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Tx RDMA response remote operation errors",
      "displayName": "Tx RDMA response remote operation errors",
      "description": "Tx RDMA response remote operation errors",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Tx RDMA response RNR retry errors",
      "displayName": "Tx RDMA response RNR retry errors",
      "description": "Tx RDMA response RNR retry errors",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "reporterID",
      "description": "Name of reporting object",
      "baseType": "string",
      "jsType": "string",
      "isTag": true,
      "displayName": "reporterID",
      "tags": [
        "Level4"
      ]
    }
  ],
  "objectKind": "NetworkInterface",
  "interfaceType": "host-pf"
},
  MemoryMetrics: {
  "name": "MemoryMetrics",
  "description": "System Memory",
  "displayName": "System Memory",
  "tags": [
    "Level7"
  ],
  "scope": "PerASIC",
  "fields": [
    {
      "name": "Totalmemory",
      "displayName": "Total memory",
      "description": "Total memory of the system",
      "units": "KB",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Availablememory",
      "displayName": "Available memory",
      "description": "Available memory of the system",
      "units": "KB",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Freememory",
      "displayName": "Free memory",
      "description": "Free memory of the system",
      "units": "KB",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "reporterID",
      "description": "Name of reporting object",
      "baseType": "string",
      "jsType": "string",
      "isTag": true,
      "displayName": "reporterID",
      "tags": [
        "Level4"
      ]
    }
  ],
  "objectKind": "DistributedServiceCard"
},
  PowerMetrics: {
  "name": "PowerMetrics",
  "description": "Asic Power",
  "displayName": "Asic Power",
  "tags": [
    "Level7"
  ],
  "scope": "PerASIC",
  "fields": [
    {
      "name": "Pin",
      "displayName": "Input Power",
      "description": "Input power to the system",
      "units": "MilliWatts",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Pout1",
      "displayName": "Core Output Power",
      "description": "Core output power",
      "units": "MilliWatts",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Pout2",
      "displayName": "ARM Output Power",
      "description": "ARM output power",
      "units": "MilliWatts",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "reporterID",
      "description": "Name of reporting object",
      "baseType": "string",
      "jsType": "string",
      "isTag": true,
      "displayName": "reporterID",
      "tags": [
        "Level4"
      ]
    }
  ],
  "objectKind": "DistributedServiceCard"
},
  AsicTemperatureMetrics: {
  "name": "AsicTemperatureMetrics",
  "description": "Asic Temperature",
  "displayName": "Asic Temperature",
  "tags": [
    "Level7"
  ],
  "scope": "PerASIC",
  "fields": [
    {
      "name": "LocalTemperature",
      "displayName": "Local Temperature",
      "description": "Temperature of the board in celsius",
      "units": "Celsius",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "DieTemperature",
      "displayName": "Die Temperature",
      "description": "Temperature of the die in celsius",
      "units": "Celsius",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "HbmTemperature",
      "displayName": "HBM Temperature",
      "description": "Temperature of the HBM in celsius",
      "units": "Celsius",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "reporterID",
      "description": "Name of reporting object",
      "baseType": "string",
      "jsType": "string",
      "isTag": true,
      "displayName": "reporterID",
      "tags": [
        "Level4"
      ]
    }
  ],
  "objectKind": "DistributedServiceCard"
},
  PortTemperatureMetrics: {
  "name": "PortTemperatureMetrics",
  "description": "Port Transceiver Temperature",
  "displayName": "Port Transceiver Temperature",
  "tags": [
    "Level7"
  ],
  "scope": "PerPort",
  "fields": [
    {
      "name": "Temperature",
      "displayName": "Transceiver temperature",
      "description": "Transceiver temperature in celsius",
      "units": "Celsius",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "WarningTemperature",
      "displayName": "Transceiver warning temperature",
      "description": "Transceiver warning temperature in celsius",
      "units": "Celsius",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "AlarmTemperature",
      "displayName": "Transceiver alarm temperature",
      "description": "Transceiver alarm temperature in celsius",
      "units": "Celsius",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "reporterID",
      "description": "Name of reporting object",
      "baseType": "string",
      "jsType": "string",
      "isTag": true,
      "displayName": "reporterID",
      "tags": [
        "Level4"
      ]
    }
  ],
  "objectKind": "DistributedServiceCard"
},
  Node: {
  "name": "Node",
  "objectKind": "Node",
  "displayName": "Node",
  "description": "Contains metrics reported from the Venice Nodes",
  "tags": [
    "Level7"
  ],
  "fields": [
    {
      "name": "CPUUsedPercent",
      "displayName": "Percent CPU Used",
      "description": "CPU usage (percent) ",
      "units": "Percent",
      "baseType": "gauge",
      "jsType": "number",
      "scaleMin": 0,
      "scaleMax": 100
    },
    {
      "name": "DiskFree",
      "displayName": "Disk Free",
      "description": "Disk Free in bytes",
      "units": "Bytes",
      "baseType": "gauge",
      "jsType": "number",
      "scaleMin": 0
    },
    {
      "name": "DiskTotal",
      "displayName": "Total Disk Space",
      "description": "Total Disk Space in bytes",
      "units": "Bytes",
      "baseType": "gauge",
      "jsType": "number",
      "scaleMin": 0
    },
    {
      "name": "DiskUsed",
      "displayName": "Disk Used",
      "description": "Disk Used in bytes",
      "units": "Bytes",
      "baseType": "gauge",
      "jsType": "number",
      "scaleMin": 0
    },
    {
      "name": "DiskUsedPercent",
      "displayName": "Percent disk Used",
      "description": "Disk usage (percent) ",
      "units": "Percent",
      "baseType": "gauge",
      "jsType": "number",
      "scaleMin": 0,
      "scaleMax": 100
    },
    {
      "name": "InterfaceRxBytes",
      "displayName": "Interface Rx",
      "description": "Interface Rx in bytes",
      "units": "Bytes",
      "baseType": "counter",
      "jsType": "number",
      "scaleMin": 0
    },
    {
      "name": "InterfaceTxBytes",
      "displayName": "Interface Tx",
      "description": "Interface Tx in bytes",
      "units": "Bytes",
      "baseType": "counter",
      "jsType": "number",
      "scaleMin": 0
    },
    {
      "name": "MemFree",
      "displayName": "Memory Free",
      "description": "Memory Free in bytes",
      "units": "Bytes",
      "baseType": "gauge",
      "jsType": "number",
      "scaleMin": 0
    },
    {
      "name": "MemTotal",
      "displayName": "Total Memory Space",
      "description": "Total Memory Space in bytes",
      "units": "Bytes",
      "baseType": "gauge",
      "jsType": "number",
      "scaleMin": 0
    },
    {
      "name": "MemUsed",
      "displayName": "Memory Used",
      "description": "Memory Used in bytes",
      "units": "Bytes",
      "baseType": "gauge",
      "jsType": "number",
      "scaleMin": 0
    },
    {
      "name": "MemUsedPercent",
      "displayName": "Percent Memory Used",
      "description": "Memory usage (percent) ",
      "units": "Percent",
      "baseType": "gauge",
      "jsType": "number",
      "scaleMin": 0,
      "scaleMax": 100
    },
    {
      "name": "reporterID",
      "displayName": "Node",
      "description": "Name of reporting node",
      "baseType": "string",
      "jsType": "string",
      "isTag": true
    }
  ]
},
  DistributedServiceCard: {
  "name": "DistributedServiceCard",
  "objectKind": "DistributedServiceCard",
  "displayName": "DSC",
  "description": "Contains metrics reported from the DSC",
  "tags": [
    "Level7"
  ],
  "fields": [
    {
      "name": "CPUUsedPercent",
      "displayName": "Percent CPU Used",
      "description": "CPU usage (percent) ",
      "units": "Percent",
      "baseType": "gauge",
      "jsType": "number",
      "scaleMin": 0,
      "scaleMax": 100
    },
    {
      "name": "DiskFree",
      "displayName": "Disk Free",
      "description": "Disk Free in bytes",
      "units": "Bytes",
      "baseType": "gauge",
      "jsType": "number",
      "scaleMin": 0
    },
    {
      "name": "DiskTotal",
      "displayName": "Total Disk Space",
      "description": "Total Disk Space in bytes",
      "units": "Bytes",
      "baseType": "gauge",
      "jsType": "number",
      "scaleMin": 0
    },
    {
      "name": "DiskUsed",
      "displayName": "Disk Used",
      "description": "Disk Used in bytes",
      "units": "Bytes",
      "baseType": "gauge",
      "jsType": "number",
      "scaleMin": 0
    },
    {
      "name": "DiskUsedPercent",
      "displayName": "Percent Disk Used",
      "description": "Disk usage (percent) ",
      "units": "Percent",
      "baseType": "gauge",
      "jsType": "number",
      "scaleMin": 0,
      "scaleMax": 100
    },
    {
      "name": "InterfaceRxBytes",
      "displayName": "Interface Rx",
      "description": "Interface Rx in bytes",
      "units": "Bytes",
      "baseType": "counter",
      "jsType": "number",
      "scaleMin": 0
    },
    {
      "name": "InterfaceTxBytes",
      "displayName": "Interface Tx",
      "description": "Interface Tx in bytes",
      "units": "Bytes",
      "baseType": "counter",
      "jsType": "number",
      "scaleMin": 0
    },
    {
      "name": "MemFree",
      "displayName": "Memory Free",
      "description": "Memory Free in bytes",
      "units": "Bytes",
      "baseType": "gauge",
      "jsType": "number",
      "scaleMin": 0
    },
    {
      "name": "MemTotal",
      "displayName": "Total Memory Space",
      "description": "Total Memory Space in bytes",
      "units": "Bytes",
      "baseType": "gauge",
      "jsType": "number",
      "scaleMin": 0
    },
    {
      "name": "MemUsed",
      "displayName": "Memory Used",
      "description": "Memory Used in bytes",
      "units": "Bytes",
      "baseType": "gauge",
      "jsType": "number",
      "scaleMin": 0
    },
    {
      "name": "MemUsedPercent",
      "displayName": "Percent Memory Used",
      "units": "Percent",
      "description": "Memory usage (percent) ",
      "baseType": "gauge",
      "jsType": "number",
      "scaleMin": 0,
      "scaleMax": 100
    },
    {
      "name": "reporterID",
      "displayName": "DSC",
      "description": "Name of reporting DSC",
      "baseType": "string",
      "jsType": "string",
      "isTag": true
    }
  ]
},
  Cluster: {
  "name": "Cluster",
  "objectKind": "Cluster",
  "displayName": "Cluster",
  "description": "Contains metrics related to the cluster",
  "tags": [
    "Level6",
    "SingleReporter"
  ],
  "fields": [
    {
      "name": "AdmittedNICs",
      "displayName": "Admitted DSCs",
      "description": "Number of admitted DSCs",
      "units": "count",
      "baseType": "gauge",
      "jsType": "number",
      "scaleMin": 0,
      "tags": [
        "Level6"
      ]
    },
    {
      "name": "DecommissionedNICs",
      "displayName": "Decomissioned DSCs",
      "description": "Number of decommissioned DSCs",
      "units": "count",
      "baseType": "gauge",
      "jsType": "number",
      "scaleMin": 0,
      "tags": [
        "Level6"
      ]
    },
    {
      "name": "DisconnectedNICs",
      "displayName": "Disconnected DSCs",
      "description": "Number of disconnected DSCs",
      "units": "count",
      "baseType": "gauge",
      "jsType": "number",
      "scaleMin": 0,
      "tags": [
        "Level6"
      ]
    },
    {
      "name": "HealthyNICs",
      "displayName": "Healthy DSCs",
      "description": "Number of healthy DSCs",
      "units": "count",
      "baseType": "gauge",
      "jsType": "number",
      "scaleMin": 0,
      "tags": [
        "Level6"
      ]
    },
    {
      "name": "PendingNICs",
      "displayName": "Pending DSCs",
      "description": "Number of pending DSCs",
      "units": "count",
      "baseType": "gauge",
      "jsType": "number",
      "scaleMin": 0,
      "tags": [
        "Level6"
      ]
    },
    {
      "name": "RejectedNICs",
      "displayName": "Rejected DSCs",
      "description": "Number of rejected DSCs",
      "units": "count",
      "baseType": "gauge",
      "jsType": "number",
      "scaleMin": 0,
      "tags": [
        "Level6"
      ]
    },
    {
      "name": "UnhealthyNICs",
      "displayName": "Unhealthy DSCs",
      "description": "Number of unhealthy DSCs",
      "units": "count",
      "baseType": "gauge",
      "jsType": "number",
      "scaleMin": 0,
      "tags": [
        "Level6"
      ]
    }
  ]
},
}