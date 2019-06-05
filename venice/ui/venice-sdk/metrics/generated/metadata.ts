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
}

export interface MetricMeasurement {
  name: string,
  description: string,
  displayName: string,
  objectKind?: string,
  fields: MetricField[],
}

export const MetricsMetadata: { [key: string]: MetricMeasurement } = {
  DropMetrics: {
  "name": "DropMetrics",
  "description": "Key index - Global ID",
  "displayName": "Global Drop Statistics",
  "fields": [
    {
      "name": "drop_malformed_pkt",
      "displayName": "Malformed Packet Drops",
      "description": " malformed pkt drop",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "drop_parser_icrc_error",
      "displayName": "RDMA ICRC Errors",
      "description": "number of packets dropped due to RDMA ICRC Errors",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "drop_parse_len_error",
      "displayName": "Packet Length Errors",
      "description": "number of packets dropped due to parse length errors",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "drop_hardware_error",
      "displayName": "Hardware Errors",
      "description": "number of packets dropped due to hardware errors seen",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "drop_input_mapping",
      "displayName": "Input Mapping Table Drops",
      "description": "number of packets dropped due to missing lookup in input mapping table",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "drop_input_mapping_dejavu",
      "displayName": "Input Mapping Deja-vu Drops",
      "description": "number of packets dropped due to dejavu in input mapping table",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "drop_multi_dest_not_pinned_uplink",
      "displayName": "Multi-dest-not-pinned-uplink Drops",
      "description": "number of multi-destination (multicast) packets dropped because they were not seen on right pinned uplink",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "drop_flow_hit",
      "displayName": "Drop-flow-hit Drops",
      "description": "number of packets dropped due to hitting drop flows",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "drop_flow_miss",
      "displayName": "Flow-miss Drops",
      "description": "number of packets dropped due to missing a flow-hit",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "drop_nacl",
      "displayName": "Drop-NACL-hit Drops",
      "description": "number of packets dropped due to drop-nacl hit",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "drop_ipsg",
      "displayName": "Drop-IPSG Drops",
      "description": "number of packets dropped due to drop-ipsg hit",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "drop_ip_normalization",
      "displayName": "IP-Normalization Drops",
      "description": "number of packets dropped due to IP packet normalization",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "drop_tcp_normalization",
      "displayName": "TCP-Normalization Drops",
      "description": "number of TCP packets dropped due to TCP normalization",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "drop_tcp_rst_with_invalid_ack_num",
      "displayName": "TCP-RST-Invalid-ACK Drops",
      "description": "number of TCP RST packets dropped due to invalid ACK number",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "drop_tcp_non_syn_first_pkt",
      "displayName": "TCP-RST-Invalid-ACK Drops",
      "description": "number of TCP non-SYN first packets dropped",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "drop_icmp_normalization",
      "displayName": "ICMP-Normalization Drops",
      "description": "number of packets dropped due to ICMP packet normalization",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "drop_input_properties_miss",
      "displayName": "Input-properties-miss Drops",
      "description": "number of packets dropped due to input properties miss",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "drop_tcp_out_of_window",
      "displayName": "TCP-out-of-window Drops",
      "description": "number of TCP packets dropped due to out-of-window",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "drop_tcp_split_handshake",
      "displayName": "TCP-split-handshake Drops",
      "description": "number of TCP packets dropped due to split handshake",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "drop_tcp_win_zero_drop",
      "displayName": "TCP-zero-window Drops",
      "description": "number of TCP packets dropped due to window size being zero",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "drop_tcp_data_after_fin",
      "displayName": "TCP-data-after-FIN Drops",
      "description": "number of TCP packets dropped due to data received after FIN was seen",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "drop_tcp_non_rst_pkt_after_rst",
      "displayName": "TCP-non-RST-after-RST Drops",
      "description": "number of TCP packets dropped due to non-RST seen after RST",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "drop_tcp_invalid_responder_first_pkt",
      "displayName": "TCP-responder-first-packet Drops",
      "description": "number of TCP packets dropped due to invalid first packet seen from responder",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "drop_tcp_unexpected_pkt",
      "displayName": "TCP-unexpected-packet Drops",
      "description": "number of TCP packets dropped due to unexpected packet seen",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "drop_src_lif_mismatch",
      "displayName": "Source-LIF-mismatch Drops",
      "description": "number of packets dropped due to packets received on unexpected source LIF",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "drop_vf_ip_label_mismatch",
      "displayName": "VF-IP-Label-mismatch Drops",
      "description": "number of packets dropped due to packets VF IP Label mismatch",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "drop_vf_bad_rr_dst_ip",
      "displayName": "VF-Bad-RR-Destination-IP Drops",
      "description": "number of packets dropped due to packets VF bad RR destination IP",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    }
  ],
  "objectKind": "SmartNIC"
},
  EgressDropMetrics: {
  "name": "EgressDropMetrics",
  "description": "Key index - Global ID",
  "displayName": "Global Egress Drop Statistics",
  "fields": [
    {
      "name": "drop_output_mapping",
      "displayName": "Output-mapping-miss Drops",
      "description": "number of packets dropped due to output mapping miss",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "drop_prune_src_port",
      "displayName": "Source-port-prune Drops",
      "description": "number of packets dropped due to source port pruning",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "drop_mirror",
      "displayName": "Mirror-hit Drops",
      "description": "number of packets dropped due to mirror hit",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "drop_policer",
      "displayName": "Policer Drops",
      "description": "number of packets dropped due to policer",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "drop_copp",
      "displayName": "COPP Drops",
      "description": "number of packets dropped due to COPP",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "drop_checksum_err",
      "displayName": "Checksum-error Drops",
      "description": "number of packets dropped due to checksum errors",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    }
  ],
  "objectKind": "SmartNIC"
},
  FteCPSMetrics: {
  "name": "FteCPSMetrics",
  "description": "Key index - FTE ID",
  "displayName": "per-FTE CPS Statistics",
  "fields": [
    {
      "name": "connections_per_second",
      "displayName": "CPS",
      "description": "Connections per second",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "max_connections_per_second",
      "displayName": "Max-CPS",
      "description": "Max Connections per second",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    }
  ],
  "objectKind": "SmartNIC"
},
  FteLifQMetrics: {
  "name": "FteLifQMetrics",
  "description": "Key index - FTE ID",
  "displayName": "per-FTE Queue Statistics",
  "fields": [
    {
      "name": "flow_miss_packets",
      "displayName": "Flow-miss Packets",
      "description": "Number of flow miss packets processed by this FTE",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "flow_retransmit_packets",
      "displayName": "Flow-retransmit Packets",
      "description": "Number of flow retransmits seen",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "l4_redirect_packets",
      "displayName": "L4-redirect Packets",
      "description": "Number of packets that hit the L4 redirect queue",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "alg_control_flow_packets",
      "displayName": "ALG-control-flow Packets",
      "description": "Number of packets that hit the ALG control flow queue",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "tcp_close_packets",
      "displayName": "TCP-Close Packets",
      "description": "Number of packets that hit the TCP close queue",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "tls_proxy_packets",
      "displayName": "TLS-proxy Packets",
      "description": "Number of packets that hit the TLS Proxy queue",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "fte_span_packets",
      "displayName": "FTE-Span Packets",
      "description": "Number of packets that hit the FTE SPAN queue",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "software_queue_packets",
      "displayName": "Software-config-Q Requests",
      "description": "Number of packets that hit the FTE config path",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "queued_tx_packets",
      "displayName": "Queued-Tx Packets",
      "description": "Number of packets enqueue in the FTE TX queue",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    }
  ],
  "objectKind": "SmartNIC"
},
  SessionSummaryMetrics: {
  "name": "SessionSummaryMetrics",
  "description": "Key index - Global",
  "displayName": "Global Session Summary Statistics",
  "fields": [
    {
      "name": "total_active_sessions",
      "displayName": "Total Active Sessions",
      "description": "Total Number of active sessions",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "num_l2_sessions",
      "displayName": "L2 Sessions",
      "description": "Total Number of L2 Sessions",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "num_tcp_sessions",
      "displayName": "TCP Sessions",
      "description": "Total Number of TCP sessions",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "num_udp_sessions",
      "displayName": "UDP Sessions",
      "description": "Total Number of UDP sessions",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "num_icmp_sessions",
      "displayName": "ICMP Sessions",
      "description": "Total Number of ICMP sessions",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "num_drop_sessions",
      "displayName": "Drop Sessions",
      "description": "Total Number of Drop sessions",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "num_aged_sessions",
      "displayName": "Aged Sessions",
      "description": "Total Number of Aged sessions",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "num_tcp_resets",
      "displayName": "TCP RST Sent",
      "description": "Total Number of TCP Resets sent as a result of SFW Reject",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "num_icmp_errors",
      "displayName": "ICMP Error Sent",
      "description": "Total Number of ICMP Errors sent as a result of SFW Reject",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "num_tcp_cxnsetup_timeouts",
      "displayName": "Connection-timeout Sessions",
      "description": "Total Number of sessions that timed out at connection setup",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "num_session_create_errors",
      "displayName": "Session Create Errors",
      "description": "Total Number of sessions that errored out during creation",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    }
  ],
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
      "jsType": "number"
    },
    {
      "name": "CIndex",
      "displayName": "C Index",
      "description": "ring consumer index",
      "units": "ID",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "InputBytes",
      "displayName": "Input Bytes",
      "description": "total input bytes (not available for cp_hot, dc_hot, xts_enc/dec, gcm_enc/dec)",
      "units": "Bytes",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "OutputBytes",
      "displayName": "Output Bytes",
      "description": "total output bytes (not available for cp_hot, dc_hot, xts_enc/dec, gcm_enc/dec)",
      "units": "Bytes",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "SoftResets",
      "displayName": "Soft Resets",
      "description": "number of soft resets executed",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    }
  ],
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
      "jsType": "number"
    },
    {
      "name": "NumSeqQueues",
      "displayName": "Num Seq Queues",
      "description": "number of sequencer queues available",
      "units": "Count",
      "baseType": "uint32",
      "jsType": "number"
    },
    {
      "name": "CryptoKeyIdxBase",
      "displayName": "Crypto Key Idx Base",
      "description": "crypto key base index",
      "units": "Count",
      "baseType": "uint32",
      "jsType": "number"
    },
    {
      "name": "NumCryptoKeysMax",
      "displayName": "Num Crypto Keys Max",
      "description": "maximum number of crypto keys allowed",
      "units": "Count",
      "baseType": "uint32",
      "jsType": "number"
    },
    {
      "name": "IntrBase",
      "displayName": "Intr Base",
      "description": "CPU interrupt base",
      "units": "Count",
      "baseType": "uint32",
      "jsType": "number"
    },
    {
      "name": "IntrCount",
      "displayName": "Intr Count",
      "description": "CPU interrupt vectors available",
      "units": "Count",
      "baseType": "uint32",
      "jsType": "number"
    }
  ],
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
      "jsType": "number"
    },
    {
      "name": "QGroup",
      "displayName": "Q Group",
      "description": "queue group\\n           : 0 - compress/decompress\\n           : 1 - compress/decompress status\\n           : 2 - crypto\\n           : 3 - crypto status\\n",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "CoreId",
      "displayName": "Core Id",
      "description": "CPU core ID (not available currently",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    }
  ],
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
      "jsType": "number"
    },
    {
      "name": "NextDBsRung",
      "displayName": "Next DBs Rung",
      "description": "chaining doorbells rung",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "SeqDescsProcessed",
      "displayName": "Seq Descs Processed",
      "description": "sequencer descriptors processed",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "SeqDescsAborted",
      "displayName": "Seq Descs Aborted",
      "description": "sequencer descriptors aborted (due to reset)",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "StatusPdmaXfers",
      "displayName": "Status Pdma Xfers",
      "description": "status descriptors copied",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "HwDescXfers",
      "displayName": "Hw Desc Xfers",
      "description": "descriptors transferred to hardware",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "HwBatchErrors",
      "displayName": "Hw Batch Errors",
      "description": "hardware batch (length) errors",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "HwOpErrors",
      "displayName": "Hw Op Errors",
      "description": "hardware operation errors",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "AolUpdateReqs",
      "displayName": "Aol Update Reqs",
      "description": "AOL list updates requested",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "SglUpdateReqs",
      "displayName": "Sgl Update Reqs",
      "description": "scatter/gather list updates requested",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "SglPdmaXfers",
      "displayName": "Sgl Pdma Xfers",
      "description": "payload DMA transfers executed",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "SglPdmaErrors",
      "displayName": "Sgl Pdma Errors",
      "description": "payload DMA errors encountered",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "SglPadOnlyXfers",
      "displayName": "Sgl Pad Only Xfers",
      "description": "pad-data-only DMA transfers executed",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "SglPadOnlyErrors",
      "displayName": "Sgl Pad Only Errors",
      "description": "pad-data-only DMA errors encountered",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "AltDescsTaken",
      "displayName": "Alt Descs Taken",
      "description": "alternate (bypass-onfail) descriptors executed",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "AltBufsTaken",
      "displayName": "Alt Bufs Taken",
      "description": "alternate buffers taken",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "LenUpdateReqs",
      "displayName": "Len Update Reqs",
      "description": "length updates requested",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "CpHeaderUpdates",
      "displayName": "Cp Header Updates",
      "description": "compression header updates requested",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "SeqHwBytes",
      "displayName": "Seq Hw Bytes",
      "description": "bytes processed",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    }
  ],
  "objectKind": "SmartNIC"
},
  RuleMetrics: {
  "name": "RuleMetrics",
  "description": "Key index - Rule ID",
  "displayName": "per-Rule Statistics",
  "fields": [
    {
      "name": "tcp_hits",
      "displayName": "TCP Hits",
      "description": "Number of TCP packets that hit the rule",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "udp_hits",
      "displayName": "UDP Hits",
      "description": "Number of UDP packets that hit the rule",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "icmp_hits",
      "displayName": "ICMP Hits",
      "description": "Number of ICMP packets that hit the rule",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "esp_hits",
      "displayName": "ESP Hits",
      "description": "Number of ESP packets that hit the rule",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "other_hits",
      "displayName": "Other Hits",
      "description": "Number of non-TCP/UDP/ICMP/ESP packets that hit the rule",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    },
    {
      "name": "total_hits",
      "displayName": "Total Hits",
      "description": "Number of total packets that hit the rule",
      "units": "Count",
      "baseType": "Counter",
      "jsType": "number"
    }
  ],
  "objectKind": "SmartNIC"
},
  Node: {
  "name": "Node",
  "objectKind": "Node",
  "displayName": "Node",
  "description": "Contains metrics reported from the Venice Nodes",
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
}