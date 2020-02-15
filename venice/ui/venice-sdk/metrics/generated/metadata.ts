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
  "displayName": "Drop Statistics",
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
      "name": "DropIcmpFragPkt",
      "displayName": "ICMP/ICMPv6-Fragment Drops",
      "description": "ICMP/ICMPv6 fragmented packet drops",
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
  "tags": [
    "Level7"
  ],
  "scope": "PerASIC",
  "objectKind": "DistributedServiceCard"
},
  EgressDropMetrics: {
  "name": "EgressDropMetrics",
  "description": "Key index - Global ID",
  "displayName": "Asic Egress Drop Statistics",
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
  "tags": [
    "Level7"
  ],
  "scope": "PerASIC",
  "objectKind": "DistributedServiceCard"
},
  IPv4FlowBehavioralMetrics: {
  "name": "IPv4FlowBehavioralMetrics",
  "description": "Key index - IPv4-Flow Key",
  "displayName": "IPv4-Flow Behavioral-Metrics",
  "fields": [
    {
      "name": "Instances",
      "displayName": "Total Instances",
      "description": "Total Instances seen in this Flow-Context",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "PpsThreshold",
      "displayName": "PPS-Threshold",
      "description": "Packet-rate-Monitoring-Threshold set for this Flow-Context",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "PpsThresholdExceedEvents",
      "displayName": "PPS-Threshold-Exceed Events",
      "description": "Number of Packet-rate-Monitoring-Threshold-Exceed Events seen in this Flow-Context",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "PpsThresholdExceedEventFirstTimestamp",
      "displayName": "PPS-Threshold-Exceed-Event First-Timestamp",
      "description": "First-Time when Packet-rate-Monitoring-Threshold-Exceed-Event seen in this Flow-Context",
      "units": "Count",
      "baseType": "Gauge",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "PpsThresholdExceedEventLastTimestamp",
      "displayName": "PPS-Threshold-Exceed-Event Last-Timestamp",
      "description": "Last-Time when Packet-rate-Monitoring-Threshold-Exceed-Event seen in this Flow-Context",
      "units": "Count",
      "baseType": "Gauge",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "BwThreshold",
      "displayName": "BW-Threshold",
      "description": "Bandwidth-Monitoring-Threshold set for this Flow-Context",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "BwThresholdExceedEvents",
      "displayName": "BW-Threshold-Exceed Events",
      "description": "Number of Bandwidth-Monitoring-Threshold-Exceed Events seen in this Flow-Context",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "BwThresholdExceedEventFirstTimestamp",
      "displayName": "BW-Threshold-Exceed-Event First-Timestamp",
      "description": "First-Time when Bandwidth-Monitoring-Threshold-Exceed-Event seen in this Flow-Context",
      "units": "Count",
      "baseType": "Gauge",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "BwThresholdExceedEventLastTimestamp",
      "displayName": "BW-Threshold-Exceed-Event Last-Timestamp",
      "description": "Last-Time when Bandwidth-Monitoring-Threshold-Exceed-Event seen in this Flow-Context",
      "units": "Count",
      "baseType": "Gauge",
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
  "tags": [
    "Level7"
  ],
  "scope": "PerASIC",
  "objectKind": "DistributedServiceCard"
},
  IPv4FlowDropMetrics: {
  "name": "IPv4FlowDropMetrics",
  "description": "Key index - IPv4-Flow Key",
  "displayName": "IPv4-Flow Drop-Metrics",
  "fields": [
    {
      "name": "Instances",
      "displayName": "Total Instances",
      "description": "Total Instances seen in this Flow-Context",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "DropPackets",
      "displayName": "Drop Packets",
      "description": "Number of packets dropped in this Flow-Context",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "DropBytes",
      "displayName": "Drop Bytes",
      "description": "Number of bytes dropped in this Flow-Context",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "DropFirstTimestamp",
      "displayName": "Drop First-Timestamp",
      "description": "First-time when drops occurred in this Flow-Context",
      "units": "Count",
      "baseType": "Gauge",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "DropLastTimestamp",
      "displayName": "Drop Last-Timestamp",
      "description": "Last-time when drops occurred in this Flow-Context",
      "units": "Count",
      "baseType": "Gauge",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "DropReason",
      "displayName": "Drop Reason",
      "description": "Drop-Reason Bitmap for packets dropped in this Flow-Context",
      "units": "Bitmap",
      "baseType": "Bitmap",
      "allowedValues": [
        "malformed_pkt_drop",
        "parser_icrc_err_drop",
        "parser_len_err_drop",
        "hardware_err_drop",
        "input_mapping_drop",
        "input_mapping_dejavu_drop",
        "multi_dest_not_pinned_uplink_drop",
        "flow_hit_drop",
        "flow_miss_drop",
        "nacl_drop",
        "ipsg_drop",
        "ip_normalization_drop",
        "tcp_normalization_err_drop",
        "tcp_rst_with_invalid_ack_num_drop",
        "tcp_non_syn_first_pkt_drop",
        "icmp_normalization_drop",
        "input_properties_miss_drop",
        "tcp_out_of_window_drop",
        "tcp_split_handshake_drop",
        "tcp_win_zero_err_drop",
        "tcp_data_after_fin_drop",
        "tcp_non_rst_pkt_after_rst_drop",
        "tcp_invalid_responder_first_pkt_drop",
        "tcp_unexpected_pkt_drop",
        "src_lif_mismatch_drop",
        "vf_ip_label_mismatch_drop",
        "vf_bad_rr_dst_ip_drop"
      ],
      "tags": [
        "Level7"
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
  "tags": [
    "Level7"
  ],
  "scope": "PerASIC",
  "objectKind": "DistributedServiceCard"
},
  IPv4FlowLatencyMetrics: {
  "name": "IPv4FlowLatencyMetrics",
  "description": "Key index - IPv4-Flow Key",
  "displayName": "IPv4-Flow Latency-Metrics",
  "fields": [
    {
      "name": "Instances",
      "displayName": "Total Instances",
      "description": "Total Instances seen in this Flow-Context",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "MinSetupLatency",
      "displayName": "Min TCP-Syn-Ack-Latency",
      "description": "Min Syn-Ack-Latency seen in this TCP-Flow-Context",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "MinSetupLatencyTimestamp",
      "displayName": "Min TCP-Syn-Ack-Latency",
      "description": "Min Syn-Ack-Latency seen in this TCP-Flow-Context",
      "units": "Count",
      "baseType": "Gauge",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "MaxSetupLatency",
      "displayName": "Max TCP-Syn-Ack-Latency",
      "description": "Max Syn-Ack-Latency seen in this TCP-Flow-Context",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "MaxSetupLatencyTimestamp",
      "displayName": "Max TCP-Syn-Ack-Latency",
      "description": "Max Syn-Ack-Latency seen in this TCP-Flow-Context",
      "units": "Count",
      "baseType": "Gauge",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "MinRttLatency",
      "displayName": "Min TCP-RTT-Latency",
      "description": "Min RTT-Latency seen in this TCP-Flow-Context",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "MinRttLatencyTimestamp",
      "displayName": "Min TCP-RTT-Latency",
      "description": "Min RTT-Latency seen in this TCP-Flow-Context",
      "units": "Count",
      "baseType": "Gauge",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "MaxRttLatency",
      "displayName": "Max TCP-RTT-Latency",
      "description": "Max RTT-Latency seen in this TCP-Flow-Context",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "MaxRttLatencyTimestamp",
      "displayName": "Max TCP-RTT-Latency",
      "description": "Max RTT-Latency seen in this TCP-Flow-Context",
      "units": "Count",
      "baseType": "Gauge",
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
  "tags": [
    "Level7"
  ],
  "scope": "PerASIC",
  "objectKind": "DistributedServiceCard"
},
  IPv4FlowPerformanceMetrics: {
  "name": "IPv4FlowPerformanceMetrics",
  "description": "Key index - IPv4-Flow Key",
  "displayName": "IPv4-Flow Performance-Metrics",
  "fields": [
    {
      "name": "Instances",
      "displayName": "Total Instances",
      "description": "Total Instances seen in this Flow-Context",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "PeakPps",
      "displayName": "Peak PPS",
      "description": "Peak Packet-rate seen in this Flow-Context",
      "units": "Count",
      "baseType": "Gauge",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "PeakPpsTimestamp",
      "displayName": "Peak PPS Timestamp",
      "description": "Timestamp when Peak Packet-rate seen in this Flow-Context",
      "units": "Count",
      "baseType": "Gauge",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "PeakBw",
      "displayName": "Peak BW",
      "description": "Peak Bandwidth seen in this Flow-Context",
      "units": "Count",
      "baseType": "Gauge",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "PeakBwTimestamp",
      "displayName": "Peak BW Timestamp",
      "description": "Timestamp when Peak Bandwidth seen in this Flow-Context",
      "units": "Count",
      "baseType": "Gauge",
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
  "tags": [
    "Level7"
  ],
  "scope": "PerASIC",
  "objectKind": "DistributedServiceCard"
},
  IPv4FlowRawMetrics: {
  "name": "IPv4FlowRawMetrics",
  "description": "Key index - IPv4-Flow Key",
  "displayName": "IPv4-Flow Raw-Metrics",
  "fields": [
    {
      "name": "Instances",
      "displayName": "Total Instances",
      "description": "Total Instances seen in this Flow-Context",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Packets",
      "displayName": "Total Packets",
      "description": "Total Number of packets in this Flow-Context",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Bytes",
      "displayName": "Total Bytes",
      "description": "Total Number of bytes in this Flow-Context",
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
  "tags": [
    "Level7"
  ],
  "scope": "PerASIC",
  "objectKind": "DistributedServiceCard"
},
  IPv6FlowBehavioralMetrics: {
  "name": "IPv6FlowBehavioralMetrics",
  "description": "Key index - IPv6-Flow Key",
  "displayName": "IPv6-Flow Behavioral-Metrics",
  "fields": [
    {
      "name": "Instances",
      "displayName": "Total Instances",
      "description": "Total Instances seen in this Flow-Context",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "PpsThreshold",
      "displayName": "PPS-Threshold",
      "description": "Packet-rate-Monitoring-Threshold set for this Flow-Context",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "PpsThresholdExceedEvents",
      "displayName": "PPS-Threshold-Exceed Events",
      "description": "Number of Packet-rate-Monitoring-Threshold-Exceed Events seen in this Flow-Context",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "PpsThresholdExceedEventFirstTimestamp",
      "displayName": "PPS-Threshold-Exceed-Event First-Timestamp",
      "description": "First-Time when Packet-rate-Monitoring-Threshold-Exceed-Event seen in this Flow-Context",
      "units": "Count",
      "baseType": "Gauge",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "PpsThresholdExceedEventLastTimestamp",
      "displayName": "PPS-Threshold-Exceed-Event Last-Timestamp",
      "description": "Last-Time when Packet-rate-Monitoring-Threshold-Exceed-Event seen in this Flow-Context",
      "units": "Count",
      "baseType": "Gauge",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "BwThreshold",
      "displayName": "BW-Threshold",
      "description": "Bandwidth-Monitoring-Threshold set for this Flow-Context",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "BwThresholdExceedEvents",
      "displayName": "BW-Threshold-Exceed Events",
      "description": "Number of Bandwidth-Monitoring-Threshold-Exceed Events seen in this Flow-Context",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "BwThresholdExceedEventFirstTimestamp",
      "displayName": "BW-Threshold-Exceed-Event First-Timestamp",
      "description": "First-Time when Bandwidth-Monitoring-Threshold-Exceed-Event seen in this Flow-Context",
      "units": "Count",
      "baseType": "Gauge",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "BwThresholdExceedEventLastTimestamp",
      "displayName": "BW-Threshold-Exceed-Event Last-Timestamp",
      "description": "Last-Time when Bandwidth-Monitoring-Threshold-Exceed-Event seen in this Flow-Context",
      "units": "Count",
      "baseType": "Gauge",
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
  "tags": [
    "Level7"
  ],
  "scope": "PerASIC",
  "objectKind": "DistributedServiceCard"
},
  IPv6FlowDropMetrics: {
  "name": "IPv6FlowDropMetrics",
  "description": "Key index - IPv6-Flow Key",
  "displayName": "IPv6-Flow Drop-Metrics",
  "fields": [
    {
      "name": "Instances",
      "displayName": "Total Instances",
      "description": "Total Instances seen in this Flow-Context",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "DropPackets",
      "displayName": "Drop Packets",
      "description": "Number of packets dropped in this Flow-Context",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "DropBytes",
      "displayName": "Drop Bytes",
      "description": "Number of bytes dropped in this Flow-Context",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "DropFirstTimestamp",
      "displayName": "Drop First-Timestamp",
      "description": "First-time when drops occurred in this Flow-Context",
      "units": "Count",
      "baseType": "Gauge",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "DropLastTimestamp",
      "displayName": "Drop Last-Timestamp",
      "description": "Last-time when drops occurred in this Flow-Context",
      "units": "Count",
      "baseType": "Gauge",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "DropReason",
      "displayName": "Drop Reason",
      "description": "Drop-Reason Bitmap for packets dropped in this Flow-Context",
      "units": "Bitmap",
      "baseType": "Bitmap",
      "allowedValues": [
        "malformed_pkt_drop",
        "parser_icrc_err_drop",
        "parser_len_err_drop",
        "hardware_err_drop",
        "input_mapping_drop",
        "input_mapping_dejavu_drop",
        "multi_dest_not_pinned_uplink_drop",
        "flow_hit_drop",
        "flow_miss_drop",
        "nacl_drop",
        "ipsg_drop",
        "ip_normalization_drop",
        "tcp_normalization_err_drop",
        "tcp_rst_with_invalid_ack_num_drop",
        "tcp_non_syn_first_pkt_drop",
        "icmp_normalization_drop",
        "input_properties_miss_drop",
        "tcp_out_of_window_drop",
        "tcp_split_handshake_drop",
        "tcp_win_zero_err_drop",
        "tcp_data_after_fin_drop",
        "tcp_non_rst_pkt_after_rst_drop",
        "tcp_invalid_responder_first_pkt_drop",
        "tcp_unexpected_pkt_drop",
        "src_lif_mismatch_drop",
        "vf_ip_label_mismatch_drop",
        "vf_bad_rr_dst_ip_drop"
      ],
      "tags": [
        "Level7"
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
  "tags": [
    "Level7"
  ],
  "scope": "PerASIC",
  "objectKind": "DistributedServiceCard"
},
  IPv6FlowLatencyMetrics: {
  "name": "IPv6FlowLatencyMetrics",
  "description": "Key index - IPv6-Flow Key",
  "displayName": "IPv6-Flow Latency-Metrics",
  "fields": [
    {
      "name": "Instances",
      "displayName": "Total Instances",
      "description": "Total Instances seen in this Flow-Context",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "MinSetupLatency",
      "displayName": "Min TCP-Syn-Ack-Latency",
      "description": "Min Syn-Ack-Latency seen in this TCP-Flow-Context",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "MinSetupLatencyTimestamp",
      "displayName": "Min TCP-Syn-Ack-Latency",
      "description": "Min Syn-Ack-Latency seen in this TCP-Flow-Context",
      "units": "Count",
      "baseType": "Gauge",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "MaxSetupLatency",
      "displayName": "Max TCP-Syn-Ack-Latency",
      "description": "Max Syn-Ack-Latency seen in this TCP-Flow-Context",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "MaxSetupLatencyTimestamp",
      "displayName": "Max TCP-Syn-Ack-Latency",
      "description": "Max Syn-Ack-Latency seen in this TCP-Flow-Context",
      "units": "Count",
      "baseType": "Gauge",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "MinRttLatency",
      "displayName": "Min TCP-RTT-Latency",
      "description": "Min RTT-Latency seen in this TCP-Flow-Context",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "MinRttLatencyTimestamp",
      "displayName": "Min TCP-RTT-Latency",
      "description": "Min RTT-Latency seen in this TCP-Flow-Context",
      "units": "Count",
      "baseType": "Gauge",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "MaxRttLatency",
      "displayName": "Max TCP-RTT-Latency",
      "description": "Max RTT-Latency seen in this TCP-Flow-Context",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "MaxRttLatencyTimestamp",
      "displayName": "Max TCP-RTT-Latency",
      "description": "Max RTT-Latency seen in this TCP-Flow-Context",
      "units": "Count",
      "baseType": "Gauge",
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
  "tags": [
    "Level7"
  ],
  "scope": "PerASIC",
  "objectKind": "DistributedServiceCard"
},
  IPv6FlowPerformanceMetrics: {
  "name": "IPv6FlowPerformanceMetrics",
  "description": "Key index - IPv6-Flow Key",
  "displayName": "IPv6-Flow Performance-Metrics",
  "fields": [
    {
      "name": "Instances",
      "displayName": "Total Instances",
      "description": "Total Instances seen in this Flow-Context",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "PeakPps",
      "displayName": "Peak PPS",
      "description": "Peak Packet-rate seen in this Flow-Context",
      "units": "Count",
      "baseType": "Gauge",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "PeakPpsTimestamp",
      "displayName": "Peak PPS Timestamp",
      "description": "Timestamp when Peak Packet-rate seen in this Flow-Context",
      "units": "Count",
      "baseType": "Gauge",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "PeakBw",
      "displayName": "Peak BW",
      "description": "Peak Bandwidth seen in this Flow-Context",
      "units": "Count",
      "baseType": "Gauge",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "PeakBwTimestamp",
      "displayName": "Peak BW Timestamp",
      "description": "Timestamp when Peak Bandwidth seen in this Flow-Context",
      "units": "Count",
      "baseType": "Gauge",
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
  "tags": [
    "Level7"
  ],
  "scope": "PerASIC",
  "objectKind": "DistributedServiceCard"
},
  IPv6FlowRawMetrics: {
  "name": "IPv6FlowRawMetrics",
  "description": "Key index - IPv6-Flow Key",
  "displayName": "IPv6-Flow Raw-Metrics",
  "fields": [
    {
      "name": "Instances",
      "displayName": "Total Instances",
      "description": "Total Instances seen in this Flow-Context",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Packets",
      "displayName": "Total Packets",
      "description": "Total Number of packets in this Flow-Context",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Bytes",
      "displayName": "Total Bytes",
      "description": "Total Number of bytes in this Flow-Context",
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
  "tags": [
    "Level7"
  ],
  "scope": "PerASIC",
  "objectKind": "DistributedServiceCard"
},
  L2FlowBehavioralMetrics: {
  "name": "L2FlowBehavioralMetrics",
  "description": "Key index - L2-Flow Key",
  "displayName": "L2-Flow Behavioral-Metrics",
  "fields": [
    {
      "name": "Instances",
      "displayName": "Total Instances",
      "description": "Total Instances seen in this Flow-Context",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "PpsThreshold",
      "displayName": "PPS-Threshold",
      "description": "Packet-rate-Monitoring-Threshold set for this Flow-Context",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "PpsThresholdExceedEvents",
      "displayName": "PPS-Threshold-Exceed Events",
      "description": "Number of Packet-rate-Monitoring-Threshold-Exceed Events seen in this Flow-Context",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "PpsThresholdExceedEventFirstTimestamp",
      "displayName": "PPS-Threshold-Exceed-Event First-Timestamp",
      "description": "First-Time when Packet-rate-Monitoring-Threshold-Exceed-Event seen in this Flow-Context",
      "units": "Count",
      "baseType": "Gauge",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "PpsThresholdExceedEventLastTimestamp",
      "displayName": "PPS-Threshold-Exceed-Event Last-Timestamp",
      "description": "Last-Time when Packet-rate-Monitoring-Threshold-Exceed-Event seen in this Flow-Context",
      "units": "Count",
      "baseType": "Gauge",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "BwThreshold",
      "displayName": "BW-Threshold",
      "description": "Bandwidth-Monitoring-Threshold set for this Flow-Context",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "BwThresholdExceedEvents",
      "displayName": "BW-Threshold-Exceed Events",
      "description": "Number of Bandwidth-Monitoring-Threshold-Exceed Events seen in this Flow-Context",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "BwThresholdExceedEventFirstTimestamp",
      "displayName": "BW-Threshold-Exceed-Event First-Timestamp",
      "description": "First-Time when Bandwidth-Monitoring-Threshold-Exceed-Event seen in this Flow-Context",
      "units": "Count",
      "baseType": "Gauge",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "BwThresholdExceedEventLastTimestamp",
      "displayName": "BW-Threshold-Exceed-Event Last-Timestamp",
      "description": "Last-Time when Bandwidth-Monitoring-Threshold-Exceed-Event seen in this Flow-Context",
      "units": "Count",
      "baseType": "Gauge",
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
  "tags": [
    "Level7"
  ],
  "scope": "PerASIC",
  "objectKind": "DistributedServiceCard"
},
  L2FlowDropMetrics: {
  "name": "L2FlowDropMetrics",
  "description": "Key index - L2-Flow Key",
  "displayName": "L2-Flow Drop-Metrics",
  "fields": [
    {
      "name": "Instances",
      "displayName": "Total Instances",
      "description": "Total Instances seen in this Flow-Context",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "DropPackets",
      "displayName": "Drop Packets",
      "description": "Number of packets dropped in this Flow-Context",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "DropBytes",
      "displayName": "Drop Bytes",
      "description": "Number of bytes dropped in this Flow-Context",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "DropFirstTimestamp",
      "displayName": "Drop First-Timestamp",
      "description": "First-time when drops occurred in this Flow-Context",
      "units": "Count",
      "baseType": "Gauge",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "DropLastTimestamp",
      "displayName": "Drop Last-Timestamp",
      "description": "Last-time when drops occurred in this Flow-Context",
      "units": "Count",
      "baseType": "Gauge",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "DropReason",
      "displayName": "Drop Reason",
      "description": "Drop-Reason Bitmap for packets dropped in this Flow-Context",
      "units": "Bitmap",
      "baseType": "Bitmap",
      "allowedValues": [
        "malformed_pkt_drop",
        "parser_icrc_err_drop",
        "parser_len_err_drop",
        "hardware_err_drop",
        "input_mapping_drop",
        "input_mapping_dejavu_drop",
        "multi_dest_not_pinned_uplink_drop",
        "flow_hit_drop",
        "flow_miss_drop",
        "nacl_drop",
        "ipsg_drop",
        "ip_normalization_drop",
        "tcp_normalization_err_drop",
        "tcp_rst_with_invalid_ack_num_drop",
        "tcp_non_syn_first_pkt_drop",
        "icmp_normalization_drop",
        "input_properties_miss_drop",
        "tcp_out_of_window_drop",
        "tcp_split_handshake_drop",
        "tcp_win_zero_err_drop",
        "tcp_data_after_fin_drop",
        "tcp_non_rst_pkt_after_rst_drop",
        "tcp_invalid_responder_first_pkt_drop",
        "tcp_unexpected_pkt_drop",
        "src_lif_mismatch_drop",
        "vf_ip_label_mismatch_drop",
        "vf_bad_rr_dst_ip_drop"
      ],
      "tags": [
        "Level7"
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
  "tags": [
    "Level7"
  ],
  "scope": "PerASIC",
  "objectKind": "DistributedServiceCard"
},
  L2FlowPerformanceMetrics: {
  "name": "L2FlowPerformanceMetrics",
  "description": "Key index - L2-Flow Key",
  "displayName": "L2-Flow Performance-Metrics",
  "fields": [
    {
      "name": "Instances",
      "displayName": "Total Instances",
      "description": "Total Instances seen in this Flow-Context",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "PeakPps",
      "displayName": "Peak PPS",
      "description": "Peak Packet-rate seen in this Flow-Context",
      "units": "Count",
      "baseType": "Gauge",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "PeakPpsTimestamp",
      "displayName": "Peak PPS Timestamp",
      "description": "Timestamp when Peak Packet-rate seen in this Flow-Context",
      "units": "Count",
      "baseType": "Gauge",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "PeakBw",
      "displayName": "Peak BW",
      "description": "Peak Bandwidth seen in this Flow-Context",
      "units": "Count",
      "baseType": "Gauge",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "PeakBwTimestamp",
      "displayName": "Peak BW Timestamp",
      "description": "Timestamp when Peak Bandwidth seen in this Flow-Context",
      "units": "Count",
      "baseType": "Gauge",
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
  "tags": [
    "Level7"
  ],
  "scope": "PerASIC",
  "objectKind": "DistributedServiceCard"
},
  L2FlowRawMetrics: {
  "name": "L2FlowRawMetrics",
  "description": "Key index - L2-Flow Key",
  "displayName": "L2-Flow Raw-Metrics",
  "fields": [
    {
      "name": "Instances",
      "displayName": "Total Instances",
      "description": "Total Instances seen in this Flow-Context",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Packets",
      "displayName": "Total Packets",
      "description": "Total Number of packets in this Flow-Context",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "Bytes",
      "displayName": "Total Bytes",
      "description": "Total Number of bytes in this Flow-Context",
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
  "tags": [
    "Level7"
  ],
  "scope": "PerASIC",
  "objectKind": "DistributedServiceCard"
},
  FteCPSMetrics: {
  "name": "FteCPSMetrics",
  "description": "Key index - FTE ID",
  "displayName": "CPS Statistics",
  "fields": [
    {
      "name": "ConnectionsPerSecond",
      "displayName": "CPS",
      "description": "Connections per second",
      "units": "Gauge",
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
      "units": "Gauge",
      "baseType": "Counter",
      "tags": [
        "Level7"
      ],
      "jsType": "number"
    },
    {
      "name": "PacketsPerSecond",
      "displayName": "PPS",
      "description": "Packets per second",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level7"
      ],
      "jsType": "number"
    },
    {
      "name": "MaxPacketsPerSecond",
      "displayName": "Max-PPS",
      "description": "Max packets per second",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level7"
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
  "tags": [
    "Level4"
  ],
  "scope": "PerFTE",
  "objectKind": "DistributedServiceCard"
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
      "name": "FreedTxPackets",
      "displayName": "Freed-Tx Packets",
      "description": "Number of dropped or non-flowmiss packets for which the CPU resources are freed",
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
  "tags": [
    "Level7"
  ],
  "scope": "PerFTE",
  "objectKind": "DistributedServiceCard"
},
  SessionSummaryMetrics: {
  "name": "SessionSummaryMetrics",
  "description": "Key index - FTE ID",
  "displayName": "Session Summary Statistics",
  "fields": [
    {
      "name": "TotalActiveSessions",
      "displayName": "Total Active Sessions",
      "description": "Total Number of active sessions",
      "units": "Gauge",
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
      "name": "NumTcpHalfOpenSessions",
      "displayName": "Half Open TCP Sessions",
      "description": "Total Number of Half Open TCP sessions",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "NumOtherActiveSessions",
      "displayName": "Other Active Sessions",
      "description": "Total Number of sessions other than TCP/UDP/ICMP",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "NumTcpSessionLimitDrops",
      "displayName": "TCP Session limit exceed drops",
      "description": "Total Number of sessions dropped due to TCP session limit",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "NumUdpSessionLimitDrops",
      "displayName": "UDP Session limit exceed drops",
      "description": "Total Number of sessions dropped due to UDP session limit",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "NumIcmpSessionLimitDrops",
      "displayName": "ICMP Session limit exceed drops",
      "description": "Total Number of sessions dropped due to ICMP session limit",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "NumOtherSessionLimitDrops",
      "displayName": "Other Session limit exceed drops",
      "description": "Total Number of sessions other than TCP/UDP/ICMP dropped due to session limit",
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
  "tags": [
    "Level4"
  ],
  "scope": "PerFTE",
  "objectKind": "DistributedServiceCard"
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
  "objectKind": "DistributedServiceCard"
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
  "objectKind": "DistributedServiceCard"
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
  "objectKind": "DistributedServiceCard"
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
  "objectKind": "DistributedServiceCard"
},
  LifMetrics: {
  "name": "LifMetrics",
  "description": "Key index - LIF ID",
  "displayName": "Lif Packet Statistics",
  "fields": [
    {
      "name": "RxUnicastBytes",
      "displayName": "Rx Unicast Bytes",
      "description": "Rx Unicast Bytes",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "RxUnicastPackets",
      "displayName": "Rx Unicast Packets",
      "description": "Rx Unicast Packets",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "RxMulticastBytes",
      "displayName": "Rx Multicast Bytes",
      "description": "Rx Multicast Bytes",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "RxMulticastPackets",
      "displayName": "Rx Multicast Packets",
      "description": "Rx Multicast Packets",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "RxBroadcastBytes",
      "displayName": "Rx Broadcast Bytes",
      "description": "Rx Broadcast Bytes",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "RxBroadcastPackets",
      "displayName": "Rx Broadcast Packets",
      "description": "Rx Broadcast Packets",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "RxDropUnicastBytes",
      "displayName": "Rx Drop Unicast Bytes",
      "description": "Rx Drop Unicast Bytes",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "RxDropUnicastPackets",
      "displayName": "Rx Drop Unicast Packets",
      "description": "Rx Drop Unicast Packets",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "RxDropMulticastBytes",
      "displayName": "Rx Drop Multicast Bytes",
      "description": "Rx Drop Multicast Bytes",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "RxDropMulticastPackets",
      "displayName": "Rx Drop Multicast Packets",
      "description": "Rx Drop Multicast Packets",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "RxDropBroadcastBytes",
      "displayName": "Rx Drop Broadcast Bytes",
      "description": "Rx Drop Broadcast Bytes",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "RxDropBroadcastPackets",
      "displayName": "Rx Drop Broadcast Packets",
      "description": "Rx Drop Broadcast Packets",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "TxUnicastBytes",
      "displayName": "Tx Unicast Bytes",
      "description": "Tx Unicast Bytes",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "TxUnicastPackets",
      "displayName": "Tx Unicast Packets",
      "description": "Tx Unicast Packets",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "TxMulticastBytes",
      "displayName": "Tx Multicast Bytes",
      "description": "Tx Multicast Bytes",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "TxMulticastPackets",
      "displayName": "Tx Multicast Packets",
      "description": "Tx Multicast Packets",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "TxBroadcastBytes",
      "displayName": "Tx Broadcast Bytes",
      "description": "Tx Broadcast Bytes",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "TxBroadcastPackets",
      "displayName": "Tx Broadcast Packets",
      "description": "Tx Broadcast Packets",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "TxDropUnicastBytes",
      "displayName": "Tx Drop Unicast Bytes",
      "description": "Tx Drop Unicast Bytes",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "TxDropUnicastPackets",
      "displayName": "Tx Drop Unicast Packets",
      "description": "Tx Drop Unicast Packets",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "TxDropMulticastBytes",
      "displayName": "Tx Drop Multicast Bytes",
      "description": "Tx Drop Multicast Bytes",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "TxDropMulticastPackets",
      "displayName": "Tx Drop Multicast Packets",
      "description": "Tx Drop Multicast Packets",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "TxDropBroadcastBytes",
      "displayName": "Tx Drop Broadcast Bytes",
      "description": "Tx Drop Broadcast Bytes",
      "units": "Count",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "TxDropBroadcastPackets",
      "displayName": "Tx Drop Broadcast Packets",
      "description": "Tx Drop Broadcast Packets",
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
  "tags": [
    "Level7"
  ],
  "scope": "PerLIF",
  "objectKind": "DistributedServiceCard"
},
  NMDMetrics: {
  "name": "NMDMetrics",
  "description": "Statistics related to NMD",
  "displayName": "NMD Metrics",
  "fields": [
    {
      "name": "GetCalls",
      "displayName": "GET calls",
      "description": "Number of GET calls to NMD",
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
  "scope": "PerNode",
  "objectKind": "DistributedServiceCard"
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
  "tags": [
    "Level7"
  ],
  "scope": "PerFwRule",
  "objectKind": "DistributedServiceCard"
},
  AsicFrequencyMetrics: {
  "name": "AsicFrequencyMetrics",
  "description": "Asic Frequency",
  "displayName": "Asic Frequency",
  "fields": [
    {
      "name": "Frequency",
      "displayName": "Frequency of the system",
      "description": "Frequency of the system in MHz",
      "units": "MHz",
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
  "tags": [
    "Level7"
  ],
  "scope": "PerASIC",
  "objectKind": "DistributedServiceCard"
},
  AsicMemoryMetrics: {
  "name": "AsicMemoryMetrics",
  "description": "System Memory",
  "displayName": "System Memory",
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
  "tags": [
    "Level7"
  ],
  "scope": "UnknownScope",
  "objectKind": "DistributedServiceCard"
},
  AsicPowerMetrics: {
  "name": "AsicPowerMetrics",
  "description": "Asic Power",
  "displayName": "Asic Power",
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
  "tags": [
    "Level7"
  ],
  "scope": "PerASIC",
  "objectKind": "DistributedServiceCard"
},
  AsicTemperatureMetrics: {
  "name": "AsicTemperatureMetrics",
  "description": "Asic Temperature",
  "displayName": "Asic Temperature",
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
      "name": "QsfpPort1Temperature",
      "displayName": "QSFP port1 temperature",
      "description": "QSFP port 1 temperature in celsius",
      "units": "Celsius",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "QsfpPort2Temperature",
      "displayName": "QSFP port2 temperature",
      "description": "QSFP port 2 temperature in celsius",
      "units": "Celsius",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "QsfpPort1WarningTemperature",
      "displayName": "QSFP port1 warning temperature",
      "description": "QSFP port 1 warning temperature in celsius",
      "units": "Celsius",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "QsfpPort2WarningTemperature",
      "displayName": "QSFP port2 warning temperature",
      "description": "QSFP port 2 warning temperature in celsius",
      "units": "Celsius",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "QsfpPort1AlarmTemperature",
      "displayName": "QSFP port1 alarm temperature",
      "description": "QSFP port 1 alarm temperature in celsius",
      "units": "Celsius",
      "baseType": "Counter",
      "tags": [
        "Level4"
      ],
      "jsType": "number"
    },
    {
      "name": "QsfpPort2AlarmTemperature",
      "displayName": "QSFP port2 alarm temperature",
      "description": "QSFP port 2 alarm temperature in celsius",
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
  "tags": [
    "Level7"
  ],
  "scope": "PerASIC",
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