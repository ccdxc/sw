export interface MetricField {
  name: string,
  description?: string,
  displayName: string,
  units?: string,
  baseType: string,
  jsType: string,
  allowedValues?: string[],
  scaleMin?: number,
  scaleMax?: number,
  tags?: string[],
  scope?: string,
  isTag?: boolean,
}

export interface MetricMeasurement {
  name: string,
  description: string,
  displayName: string,
  objectKind?: string,
  scope?: string,
  fields: MetricField[],
  tags?: string[],
}

export const MetricsMetadata: { [key: string]: MetricMeasurement } = {
  DropMetrics: {
  "name": "DropMetrics",
  "description": "Key index - Global ID",
  "displayName": "Global Drop Statistics",
  "fields": [
    {
      "name": "DropMalformedPkt",
      "displayName": "Malformed Packet Drops",
      "description": " malformed pkt drop",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "DropParserIcrcError",
      "displayName": "RDMA ICRC Errors",
      "description": "number of packets dropped due to RDMA ICRC Errors",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "DropParseLenError",
      "displayName": "Packet Length Errors",
      "description": "number of packets dropped due to parse length errors",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "DropHardwareError",
      "displayName": "Hardware Errors",
      "description": "number of packets dropped due to hardware errors seen",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "DropInputMapping",
      "displayName": "Input Mapping Table Drops",
      "description": "number of packets dropped due to missing lookup in input mapping table",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "DropInputMappingDejavu",
      "displayName": "Input Mapping Deja-vu Drops",
      "description": "number of packets dropped due to dejavu in input mapping table",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "DropMultiDestNotPinnedUplink",
      "displayName": "Multi-dest-not-pinned-uplink Drops",
      "description": "number of multi-destination (multicast) packets dropped because they were not seen on right pinned uplink",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "DropFlowHit",
      "displayName": "Drop-flow-hit Drops",
      "description": "number of packets dropped due to hitting drop flows",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "DropFlowMiss",
      "displayName": "Flow-miss Drops",
      "description": "number of packets dropped due to missing a flow-hit",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "DropNacl",
      "displayName": "Drop-NACL-hit Drops",
      "description": "number of packets dropped due to drop-nacl hit",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "DropIpsg",
      "displayName": "Drop-IPSG Drops",
      "description": "number of packets dropped due to drop-ipsg hit",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "DropIpNormalization",
      "displayName": "IP-Normalization Drops",
      "description": "number of packets dropped due to IP packet normalization",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "DropTcpNormalization",
      "displayName": "TCP-Normalization Drops",
      "description": "number of TCP packets dropped due to TCP normalization",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "DropTcpRstWithInvalidAckNum",
      "displayName": "TCP-RST-Invalid-ACK Drops",
      "description": "number of TCP RST packets dropped due to invalid ACK number",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "DropTcpNonSynFirstPkt",
      "displayName": "TCP-RST-Invalid-ACK Drops",
      "description": "number of TCP non-SYN first packets dropped",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "DropIcmpNormalization",
      "displayName": "ICMP-Normalization Drops",
      "description": "number of packets dropped due to ICMP packet normalization",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "DropInputPropertiesMiss",
      "displayName": "Input-properties-miss Drops",
      "description": "number of packets dropped due to input properties miss",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "DropTcpOutOfWindow",
      "displayName": "TCP-out-of-window Drops",
      "description": "number of TCP packets dropped due to out-of-window",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "DropTcpSplitHandshake",
      "displayName": "TCP-split-handshake Drops",
      "description": "number of TCP packets dropped due to split handshake",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "DropTcpWinZeroDrop",
      "displayName": "TCP-zero-window Drops",
      "description": "number of TCP packets dropped due to window size being zero",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "DropTcpDataAfterFin",
      "displayName": "TCP-data-after-FIN Drops",
      "description": "number of TCP packets dropped due to data received after FIN was seen",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "DropTcpNonRstPktAfterRst",
      "displayName": "TCP-non-RST-after-RST Drops",
      "description": "number of TCP packets dropped due to non-RST seen after RST",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "DropTcpInvalidResponderFirstPkt",
      "displayName": "TCP-responder-first-packet Drops",
      "description": "number of TCP packets dropped due to invalid first packet seen from responder",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "DropTcpUnexpectedPkt",
      "displayName": "TCP-unexpected-packet Drops",
      "description": "number of TCP packets dropped due to unexpected packet seen",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "DropSrcLifMismatch",
      "displayName": "Source-LIF-mismatch Drops",
      "description": "number of packets dropped due to packets received on unexpected source LIF",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "DropVfIpLabelMismatch",
      "displayName": "VF-IP-Label-mismatch Drops",
      "description": "number of packets dropped due to packets VF IP Label mismatch",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "DropVfBadRrDstIp",
      "displayName": "VF-Bad-RR-Destination-IP Drops",
      "description": "number of packets dropped due to packets VF bad RR destination IP",
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
  "scope": "PerASIC",
  "objectKind": "SmartNIC"
},
  EgressDropMetrics: {
  "name": "EgressDropMetrics",
  "description": "Key index - Global ID",
  "displayName": "Global Egress Drop Statistics",
  "fields": [
    {
      "name": "DropOutputMapping",
      "displayName": "Output-mapping-miss Drops",
      "description": "number of packets dropped due to output mapping miss",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "DropPruneSrcPort",
      "displayName": "Source-port-prune Drops",
      "description": "number of packets dropped due to source port pruning",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "DropMirror",
      "displayName": "Mirror-hit Drops",
      "description": "number of packets dropped due to mirror hit",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "DropPolicer",
      "displayName": "Policer Drops",
      "description": "number of packets dropped due to policer",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "DropCopp",
      "displayName": "COPP Drops",
      "description": "number of packets dropped due to COPP",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "DropChecksumErr",
      "displayName": "Checksum-error Drops",
      "description": "number of packets dropped due to checksum errors",
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
  "scope": "PerASIC",
  "objectKind": "SmartNIC"
},
  FteCPSMetrics: {
  "name": "FteCPSMetrics",
  "description": "Key index - FTE ID",
  "displayName": "per-FTE CPS Statistics",
  "fields": [
    {
      "name": "ConnectionsPerSecond",
      "displayName": "CPS",
      "description": "Connections per second",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "MaxConnectionsPerSecond",
      "displayName": "Max-CPS",
      "description": "Max Connections per second",
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
  "scope": "PerFTE",
  "objectKind": "SmartNIC"
},
  FteLifQMetrics: {
  "name": "FteLifQMetrics",
  "description": "Key index - FTE ID",
  "displayName": "per-FTE Queue Statistics",
  "fields": [
    {
      "name": "FlowMissPackets",
      "displayName": "Flow-miss Packets",
      "description": "Number of flow miss packets processed by this FTE",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "FlowRetransmitPackets",
      "displayName": "Flow-retransmit Packets",
      "description": "Number of flow retransmits seen",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "L4RedirectPackets",
      "displayName": "L4-redirect Packets",
      "description": "Number of packets that hit the L4 redirect queue",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "AlgControlFlowPackets",
      "displayName": "ALG-control-flow Packets",
      "description": "Number of packets that hit the ALG control flow queue",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "TcpClosePackets",
      "displayName": "TCP-Close Packets",
      "description": "Number of packets that hit the TCP close queue",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "TlsProxyPackets",
      "displayName": "TLS-proxy Packets",
      "description": "Number of packets that hit the TLS Proxy queue",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "FteSpanPackets",
      "displayName": "FTE-Span Packets",
      "description": "Number of packets that hit the FTE SPAN queue",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "SoftwareQueuePackets",
      "displayName": "Software-config-Q Requests",
      "description": "Number of packets that hit the FTE config path",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "QueuedTxPackets",
      "displayName": "Queued-Tx Packets",
      "description": "Number of packets enqueue in the FTE TX queue",
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
  "scope": "PerFTE",
  "objectKind": "SmartNIC"
},
  SessionSummaryMetrics: {
  "name": "SessionSummaryMetrics",
  "description": "Key index - FTE ID",
  "displayName": "Global Session Summary Statistics",
  "fields": [
    {
      "name": "TotalActiveSessions",
      "displayName": "Total Active Sessions",
      "description": "Total Number of active sessions",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "NumL2Sessions",
      "displayName": "L2 Sessions",
      "description": "Total Number of L2 Sessions",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "NumTcpSessions",
      "displayName": "TCP Sessions",
      "description": "Total Number of TCP sessions",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "NumUdpSessions",
      "displayName": "UDP Sessions",
      "description": "Total Number of UDP sessions",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "NumIcmpSessions",
      "displayName": "ICMP Sessions",
      "description": "Total Number of ICMP sessions",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "NumDropSessions",
      "displayName": "Drop Sessions",
      "description": "Total Number of Drop sessions",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "NumAgedSessions",
      "displayName": "Aged Sessions",
      "description": "Total Number of Aged sessions",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "NumTcpResets",
      "displayName": "TCP RST Sent",
      "description": "Total Number of TCP Resets sent as a result of SFW Reject",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "NumIcmpErrors",
      "displayName": "ICMP Error Sent",
      "description": "Total Number of ICMP Errors sent as a result of SFW Reject",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "NumTcpCxnsetupTimeouts",
      "displayName": "Connection-timeout Sessions",
      "description": "Total Number of sessions that timed out at connection setup",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "NumSessionCreateErrors",
      "displayName": "Session Create Errors",
      "description": "Total Number of sessions that errored out during creation",
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
  "scope": "PerFTE",
  "objectKind": "SmartNIC"
},
  AccelHwRingMetrics: {
  "name": "AccelHwRingMetrics",
  "description": "Key indices - RId: ring ID, SubRId: sub-ring ID",
  "displayName": "Metrics for hardware rings",
  "fields": [
    {
      "name": "PIndex",
      "displayName": "P Index",
      "description": "ring producer index",
      "units": "ID",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "CIndex",
      "displayName": "C Index",
      "description": "ring consumer index",
      "units": "ID",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "InputBytes",
      "displayName": "Input Bytes",
      "description": "total input bytes (not available for cp_hot, dc_hot, xts_enc/dec, gcm_enc/dec)",
      "units": "Bytes",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "OutputBytes",
      "displayName": "Output Bytes",
      "description": "total output bytes (not available for cp_hot, dc_hot, xts_enc/dec, gcm_enc/dec)",
      "units": "Bytes",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "SoftResets",
      "displayName": "Soft Resets",
      "description": "number of soft resets executed",
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
  "scope": "PerRingPerSubRing",
  "objectKind": "SmartNIC"
},
  AccelPfInfo: {
  "name": "AccelPfInfo",
  "description": "Key index - logical interface ID",
  "displayName": "Device information",
  "fields": [
    {
      "name": "HwLifId",
      "displayName": "Hw Lif Id",
      "description": "hardware logical interface ID",
      "units": "ID",
      "baseType": "uint64",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "NumSeqQueues",
      "displayName": "Num Seq Queues",
      "description": "number of sequencer queues available",
      "units": "Count",
      "baseType": "uint32",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "CryptoKeyIdxBase",
      "displayName": "Crypto Key Idx Base",
      "description": "crypto key base index",
      "units": "Count",
      "baseType": "uint32",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "NumCryptoKeysMax",
      "displayName": "Num Crypto Keys Max",
      "description": "maximum number of crypto keys allowed",
      "units": "Count",
      "baseType": "uint32",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "IntrBase",
      "displayName": "Intr Base",
      "description": "CPU interrupt base",
      "units": "Count",
      "baseType": "uint32",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "IntrCount",
      "displayName": "Intr Count",
      "description": "CPU interrupt vectors available",
      "units": "Count",
      "baseType": "uint32",
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
  "scope": "PerLIF",
  "objectKind": "SmartNIC"
},
  AccelSeqQueueInfoMetrics: {
  "name": "AccelSeqQueueInfoMetrics",
  "description": "Key indices are - LifID and QId",
  "displayName": "Sequencer queues information",
  "fields": [
    {
      "name": "QStateAddr",
      "displayName": "Q State Add",
      "description": "queue state memory address",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "QGroup",
      "displayName": "Q Group",
      "description": "queue group\\n           : 0 - compress/decompress\\n           : 1 - compress/decompress status\\n           : 2 - crypto\\n           : 3 - crypto status\\n",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "CoreId",
      "displayName": "Core Id",
      "description": "CPU core ID (not available currently",
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
  "scope": "PerLIFPerQ",
  "objectKind": "SmartNIC"
},
  AccelSeqQueueMetrics: {
  "name": "AccelSeqQueueMetrics",
  "description": "Key indices are - LifID and QId",
  "displayName": "Metrics for sequencer queues",
  "fields": [
    {
      "name": "InterruptsRaised",
      "displayName": "Interrupts Raised",
      "description": "CPU interrupts raised",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "NextDBsRung",
      "displayName": "Next DBs Rung",
      "description": "chaining doorbells rung",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "SeqDescsProcessed",
      "displayName": "Seq Descs Processed",
      "description": "sequencer descriptors processed",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "SeqDescsAborted",
      "displayName": "Seq Descs Aborted",
      "description": "sequencer descriptors aborted (due to reset)",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "StatusPdmaXfers",
      "displayName": "Status Pdma Xfers",
      "description": "status descriptors copied",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "HwDescXfers",
      "displayName": "Hw Desc Xfers",
      "description": "descriptors transferred to hardware",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "HwBatchErrors",
      "displayName": "Hw Batch Errors",
      "description": "hardware batch (length) errors",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "HwOpErrors",
      "displayName": "Hw Op Errors",
      "description": "hardware operation errors",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "AolUpdateReqs",
      "displayName": "Aol Update Reqs",
      "description": "AOL list updates requested",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "SglUpdateReqs",
      "displayName": "Sgl Update Reqs",
      "description": "scatter/gather list updates requested",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "SglPdmaXfers",
      "displayName": "Sgl Pdma Xfers",
      "description": "payload DMA transfers executed",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "SglPdmaErrors",
      "displayName": "Sgl Pdma Errors",
      "description": "payload DMA errors encountered",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "SglPadOnlyXfers",
      "displayName": "Sgl Pad Only Xfers",
      "description": "pad-data-only DMA transfers executed",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "SglPadOnlyErrors",
      "displayName": "Sgl Pad Only Errors",
      "description": "pad-data-only DMA errors encountered",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "AltDescsTaken",
      "displayName": "Alt Descs Taken",
      "description": "alternate (bypass-onfail) descriptors executed",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "AltBufsTaken",
      "displayName": "Alt Bufs Taken",
      "description": "alternate buffers taken",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "LenUpdateReqs",
      "displayName": "Len Update Reqs",
      "description": "length updates requested",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "CpHeaderUpdates",
      "displayName": "Cp Header Updates",
      "description": "compression header updates requested",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "SeqHwBytes",
      "displayName": "Seq Hw Bytes",
      "description": "bytes processed",
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
  "scope": "PerLIFPerQ",
  "objectKind": "SmartNIC"
},
  RuleMetrics: {
  "name": "RuleMetrics",
  "description": "Key index - Rule ID",
  "displayName": "per-Rule Statistics",
  "fields": [
    {
      "name": "TcpHits",
      "displayName": "TCP Hits",
      "description": "Number of TCP packets that hit the rule",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "UdpHits",
      "displayName": "UDP Hits",
      "description": "Number of UDP packets that hit the rule",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "IcmpHits",
      "displayName": "ICMP Hits",
      "description": "Number of ICMP packets that hit the rule",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "EspHits",
      "displayName": "ESP Hits",
      "description": "Number of ESP packets that hit the rule",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "OtherHits",
      "displayName": "Other Hits",
      "description": "Number of non-TCP/UDP/ICMP/ESP packets that hit the rule",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "TotalHits",
      "displayName": "Total Hits",
      "description": "Number of total packets that hit the rule",
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
  "scope": "PerFwRule",
  "objectKind": "SmartNIC"
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
      "baseType": "number",
      "jsType": "number",
      "scaleMin": 0,
      "scaleMax": 100
    },
    {
      "name": "DiskFree",
      "displayName": "Disk Free",
      "description": "Disk Free in bytes",
      "units": "Bytes",
      "baseType": "number",
      "jsType": "number",
      "scaleMin": 0
    },
    {
      "name": "DiskTotal",
      "displayName": "Total Disk Space",
      "description": "Total Disk Space in bytes",
      "units": "Bytes",
      "baseType": "number",
      "jsType": "number",
      "scaleMin": 0
    },
    {
      "name": "DiskUsed",
      "displayName": "Disk Used",
      "description": "Disk Used in bytes",
      "units": "Bytes",
      "baseType": "number",
      "jsType": "number",
      "scaleMin": 0
    },
    {
      "name": "DiskUsedPercent",
      "displayName": "Percent disk Used",
      "description": "Disk usage (percent) ",
      "units": "Percent",
      "baseType": "number",
      "jsType": "number",
      "scaleMin": 0,
      "scaleMax": 100
    },
    {
      "name": "InterfaceRxBytes",
      "displayName": "Interface Rx",
      "description": "Interface Rx in bytes",
      "units": "Bytes",
      "baseType": "number",
      "jsType": "number",
      "scaleMin": 0
    },
    {
      "name": "InterfaceTxBytes",
      "displayName": "Interface Tx",
      "description": "Interface Tx in bytes",
      "units": "Bytes",
      "baseType": "number",
      "jsType": "number",
      "scaleMin": 0
    },
    {
      "name": "MemFree",
      "displayName": "Memory Free",
      "description": "Memory Free in bytes",
      "units": "Bytes",
      "baseType": "number",
      "jsType": "number",
      "scaleMin": 0
    },
    {
      "name": "MemTotal",
      "displayName": "Total Memory Space",
      "description": "Total Memory Space in bytes",
      "units": "Bytes",
      "baseType": "number",
      "jsType": "number",
      "scaleMin": 0
    },
    {
      "name": "MemUsed",
      "displayName": "Memory Used",
      "description": "Memory Used in bytes",
      "units": "Bytes",
      "baseType": "number",
      "jsType": "number",
      "scaleMin": 0
    },
    {
      "name": "MemUsedPercent",
      "displayName": "Percent Memory Used",
      "description": "Memory usage (percent) ",
      "units": "Percent",
      "baseType": "number",
      "jsType": "number",
      "scaleMin": 0,
      "scaleMax": 100
    },
    {
      "name": "reporterID",
      "displayName": "Reporting Node",
      "description": "Name of reporting node",
      "baseType": "string",
      "jsType": "string"
    }
  ]
},
  SmartNIC: {
  "name": "SmartNIC",
  "objectKind": "SmartNIC",
  "displayName": "Naples",
  "description": "Contains metrics reported from the Naples",
  "tags": [
    "Level7"
  ],
  "fields": [
    {
      "name": "CPUUsedPercent",
      "displayName": "Percent CPU Used",
      "description": "CPU usage (percent) ",
      "units": "Percent",
      "baseType": "number",
      "jsType": "number",
      "scaleMin": 0,
      "scaleMax": 100
    },
    {
      "name": "DiskFree",
      "displayName": "Disk Free",
      "description": "Disk Free in bytes",
      "units": "Bytes",
      "baseType": "number",
      "jsType": "number",
      "scaleMin": 0
    },
    {
      "name": "DiskTotal",
      "displayName": "Total Disk Space",
      "description": "Total Disk Space in bytes",
      "units": "Bytes",
      "baseType": "number",
      "jsType": "number",
      "scaleMin": 0
    },
    {
      "name": "DiskUsed",
      "displayName": "Disk Used",
      "description": "Disk Used in bytes",
      "units": "Bytes",
      "baseType": "number",
      "jsType": "number",
      "scaleMin": 0
    },
    {
      "name": "DiskUsedPercent",
      "displayName": "Percent Disk Used",
      "description": "Disk usage (percent) ",
      "units": "Percent",
      "baseType": "number",
      "jsType": "number",
      "scaleMin": 0,
      "scaleMax": 100
    },
    {
      "name": "InterfaceRxBytes",
      "displayName": "Interface Rx",
      "description": "Interface Rx in bytes",
      "units": "Bytes",
      "baseType": "number",
      "jsType": "number",
      "scaleMin": 0
    },
    {
      "name": "InterfaceTxBytes",
      "displayName": "Interface Tx",
      "description": "Interface Tx in bytes",
      "units": "Bytes",
      "baseType": "number",
      "jsType": "number",
      "scaleMin": 0
    },
    {
      "name": "MemFree",
      "displayName": "Memory Free",
      "description": "Memory Free in bytes",
      "units": "Bytes",
      "baseType": "number",
      "jsType": "number",
      "scaleMin": 0
    },
    {
      "name": "MemTotal",
      "displayName": "Total Memory Space",
      "description": "Total Memory Space in bytes",
      "units": "Bytes",
      "baseType": "number",
      "jsType": "number",
      "scaleMin": 0
    },
    {
      "name": "MemUsed",
      "displayName": "Memory Used",
      "description": "Memory Used in bytes",
      "units": "Bytes",
      "baseType": "number",
      "jsType": "number",
      "scaleMin": 0
    },
    {
      "name": "MemUsedPercent",
      "displayName": "Percent Memory Used",
      "units": "Percent",
      "description": "Memory usage (percent) ",
      "baseType": "number",
      "jsType": "number",
      "scaleMin": 0,
      "scaleMax": 100
    },
    {
      "name": "reporterID",
      "displayName": "Reporting SmartNIC",
      "description": "Name of reporting SmartNIC",
      "baseType": "string",
      "jsType": "string"
    }
  ]
},
  Cluster: {
  "name": "Cluster",
  "objectKind": "Cluster",
  "displayName": "Cluster",
  "description": "Contains metrics related to the cluster",
  "tags": [
    "Level6"
  ],
  "fields": [
    {
      "name": "AdmittedNICs",
      "displayName": "Admitted NICs",
      "description": "Number of admitted NICs",
      "units": "count",
      "baseType": "number",
      "jsType": "number",
      "scaleMin": 0,
      "tags": [
        "Level6"
      ]
    },
    {
      "name": "DecommissionedNICs",
      "displayName": "Decomissioned NICs",
      "description": "Number of decommissioned NICs",
      "units": "count",
      "baseType": "number",
      "jsType": "number",
      "scaleMin": 0,
      "tags": [
        "Level6"
      ]
    },
    {
      "name": "DisconnectedNICs",
      "displayName": "Disconnected NICs",
      "description": "Number of disconnected NICs",
      "units": "count",
      "baseType": "number",
      "jsType": "number",
      "scaleMin": 0,
      "tags": [
        "Level6"
      ]
    },
    {
      "name": "HealthyNICs",
      "displayName": "Healthy NICs",
      "description": "Number of healthy NICs",
      "units": "count",
      "baseType": "number",
      "jsType": "number",
      "scaleMin": 0,
      "tags": [
        "Level6"
      ]
    },
    {
      "name": "PendingNICs",
      "displayName": "Pending NICs",
      "description": "Number of pending NICs",
      "units": "count",
      "baseType": "number",
      "jsType": "number",
      "scaleMin": 0,
      "tags": [
        "Level6"
      ]
    },
    {
      "name": "RejectedNICs",
      "displayName": "Rejected NICs",
      "description": "Number of rejected NICs",
      "units": "count",
      "baseType": "number",
      "jsType": "number",
      "scaleMin": 0,
      "tags": [
        "Level6"
      ]
    },
    {
      "name": "UnhealthyNICs",
      "displayName": "Unhealthy NICs",
      "description": "Number of unhealthy NICs",
      "units": "count",
      "baseType": "number",
      "jsType": "number",
      "scaleMin": 0,
      "tags": [
        "Level6"
      ]
    }
  ]
},
}