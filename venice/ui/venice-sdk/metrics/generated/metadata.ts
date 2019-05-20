export interface MetricField {
  name: string,
  description?: string,
  displayName: string,
  units?: string,
  baseType?: string,
  scaleMin?: number,
  scaleMax?: number,
}

export interface MetricMeasurement {
  name: string,
  description: string,
  displayName: string,
  fields: MetricField[],
}

export const MetricsMetadata: { [key: string]: MetricMeasurement } = {
  AccelHwRingMetrics: {
  "name": "AccelHwRingMetrics",
  "description": "Key indices - RId: ring ID, SubRId: sub-ring ID",
  "displayName": "Metrics for hardware rings",
  "fields": [
    {
      "name": "Key",
      "displayName": "Key indices",
      "units": "ID",
      "baseType": ".accel_metrics.AccelHwRingKey"
    },
    {
      "name": "PIndex",
      "displayName": "PIndex",
      "description": "ring producer index",
      "units": "ID",
      "baseType": "Counter"
    },
    {
      "name": "CIndex",
      "displayName": "CIndex",
      "description": "ring consumer index",
      "units": "ID",
      "baseType": "Counter"
    },
    {
      "name": "InputBytes",
      "displayName": "InputBytes",
      "description": "total input bytes (not available for cp_hot, dc_hot, xts_enc/dec, gcm_enc/dec)",
      "units": "Bytes",
      "baseType": "Counter"
    },
    {
      "name": "OutputBytes",
      "displayName": "OutputBytes",
      "description": "total output bytes (not available for cp_hot, dc_hot, xts_enc/dec, gcm_enc/dec)",
      "units": "Bytes",
      "baseType": "Counter"
    },
    {
      "name": "SoftResets",
      "displayName": "SoftResets",
      "description": "number of soft resets executed",
      "units": "Count",
      "baseType": "Counter"
    }
  ]
},
  AccelPfInfo: {
  "name": "AccelPfInfo",
  "description": "Key index - logical interface ID",
  "displayName": "Device information",
  "fields": [
    {
      "name": "HwLifId",
      "displayName": "HwLifId",
      "description": "hardware logical interface ID",
      "units": "ID",
      "baseType": "uint64"
    },
    {
      "name": "NumSeqQueues",
      "displayName": "NumSeqQueues",
      "description": "number of sequencer queues available",
      "units": "Count",
      "baseType": "uint32"
    },
    {
      "name": "CryptoKeyIdxBase",
      "displayName": "CryptoKeyIdxBase",
      "description": "crypto key base index",
      "units": "Count",
      "baseType": "uint32"
    },
    {
      "name": "NumCryptoKeysMax",
      "displayName": "NumCryptoKeysMax",
      "description": "maximum number of crypto keys allowed",
      "units": "Count",
      "baseType": "uint32"
    },
    {
      "name": "IntrBase",
      "displayName": "IntrBase",
      "description": "CPU interrupt base",
      "units": "Count",
      "baseType": "uint32"
    },
    {
      "name": "IntrCount",
      "displayName": "IntrCount",
      "description": "CPU interrupt vectors available",
      "units": "Count",
      "baseType": "uint32"
    }
  ]
},
  AccelSeqQueueInfoMetrics: {
  "name": "AccelSeqQueueInfoMetrics",
  "description": "Key indices are - LifID and QId",
  "displayName": "Sequencer queues information",
  "fields": [
    {
      "name": "Key",
      "displayName": "Key indices",
      "units": "ID",
      "baseType": ".accel_metrics.AccelSeqQueueKey"
    },
    {
      "name": "QStateAddr",
      "displayName": "QStateAdd",
      "description": "queue state memory address",
      "units": "Count",
      "baseType": "Counter"
    },
    {
      "name": "QGroup",
      "displayName": "QGroup",
      "description": "queue group\\n           : 0 - compress/decompress\\n           : 1 - compress/decompress status\\n           : 2 - crypto\\n           : 3 - crypto status\\n",
      "units": "Count",
      "baseType": "Counter"
    },
    {
      "name": "CoreId",
      "displayName": "CoreId",
      "description": "CPU core ID (not available currently",
      "units": "Count",
      "baseType": "Counter"
    }
  ]
},
  AccelSeqQueueMetrics: {
  "name": "AccelSeqQueueMetrics",
  "description": "Key indices are - LifID and QId",
  "displayName": "Metrics for sequencer queues",
  "fields": [
    {
      "name": "Key",
      "displayName": "Key indices",
      "units": "ID",
      "baseType": ".accel_metrics.AccelSeqQueueKey"
    },
    {
      "name": "InterruptsRaised",
      "displayName": "InterruptsRaised",
      "description": "CPU interrupts raised",
      "units": "Count",
      "baseType": "Counter"
    },
    {
      "name": "NextDBsRung",
      "displayName": "NextDBsRung",
      "description": "chaining doorbells rung",
      "units": "Count",
      "baseType": "Counter"
    },
    {
      "name": "SeqDescsProcessed",
      "displayName": "SeqDescsProcessed",
      "description": "sequencer descriptors processed",
      "units": "Count",
      "baseType": "Counter"
    },
    {
      "name": "SeqDescsAborted",
      "displayName": "SeqDescsAborted",
      "description": "sequencer descriptors aborted (due to reset)",
      "units": "Count",
      "baseType": "Counter"
    },
    {
      "name": "StatusPdmaXfers",
      "displayName": "StatusPdmaXfers",
      "description": "status descriptors copied",
      "units": "Count",
      "baseType": "Counter"
    },
    {
      "name": "HwDescXfers",
      "displayName": "HwDescXfers",
      "description": "descriptors transferred to hardware",
      "units": "Count",
      "baseType": "Counter"
    },
    {
      "name": "HwBatchErrors",
      "displayName": "HwBatchErrors",
      "description": "hardware batch (length) errors",
      "units": "Count",
      "baseType": "Counter"
    },
    {
      "name": "HwOpErrors",
      "displayName": "HwOpErrors",
      "description": "hardware operation errors",
      "units": "Count",
      "baseType": "Counter"
    },
    {
      "name": "AolUpdateReqs",
      "displayName": "AolUpdateReqs",
      "description": "AOL list updates requested",
      "units": "Count",
      "baseType": "Counter"
    },
    {
      "name": "SglUpdateReqs",
      "displayName": "AolUpdateReqs",
      "description": "scatter/gather list updates requested",
      "units": "Count",
      "baseType": "Counter"
    },
    {
      "name": "SglPdmaXfers",
      "displayName": "SglUpdateReqs",
      "description": "payload DMA transfers executed",
      "units": "Count",
      "baseType": "Counter"
    },
    {
      "name": "SglPdmaErrors",
      "displayName": "SglPdmaXfers",
      "description": "payload DMA errors encountered",
      "units": "Count",
      "baseType": "Counter"
    },
    {
      "name": "SglPadOnlyXfers",
      "displayName": "SglPdmaErrors",
      "description": "pad-data-only DMA transfers executed",
      "units": "Count",
      "baseType": "Counter"
    },
    {
      "name": "SglPadOnlyErrors",
      "displayName": "SglPadOnlyXfers",
      "description": "pad-data-only DMA errors encountered",
      "units": "Count",
      "baseType": "Counter"
    },
    {
      "name": "AltDescsTaken",
      "displayName": "AltDescsTaken",
      "description": "alternate (bypass-onfail) descriptors executed",
      "units": "Count",
      "baseType": "Counter"
    },
    {
      "name": "AltBufsTaken",
      "displayName": "AltBufsTaken",
      "description": "alternate buffers taken",
      "units": "Count",
      "baseType": "Counter"
    },
    {
      "name": "LenUpdateReqs",
      "displayName": "LenUpdateReqs",
      "description": "length updates requested",
      "units": "Count",
      "baseType": "Counter"
    },
    {
      "name": "CpHeaderUpdates",
      "displayName": "CpHeaderUpdates",
      "description": "compression header updates requested",
      "units": "Count",
      "baseType": "Counter"
    },
    {
      "name": "SeqHwBytes",
      "displayName": "SeqHwBytes",
      "description": "bytes processed",
      "units": "Count",
      "baseType": "Counter"
    }
  ]
},
  Node: {
  "name": "Node",
  "displayName": "Node",
  "description": "Contains metrics reported from the Venice Nodes",
  "fields": [
    {
      "name": "CPUUsedPercent",
      "displayName": "Percent CPU Used",
      "description": "CPU usage (percent) ",
      "baseType": "number",
      "scaleMin": 0,
      "scaleMax": 100
    },
    {
      "name": "DiskFree",
      "displayName": "Disk Free",
      "description": "Disk Free in bytes",
      "baseType": "number"
    },
    {
      "name": "DiskTotal",
      "displayName": "Total Disk Space",
      "description": "Total Disk Space in bytes",
      "baseType": "number"
    },
    {
      "name": "DiskUsed",
      "displayName": "Disk Used",
      "description": "Disk Used in bytes",
      "baseType": "number"
    },
    {
      "name": "DiskUsedPercent",
      "displayName": "Percent disk Used",
      "description": "Disk usage (percent) ",
      "baseType": "number",
      "scaleMin": 0,
      "scaleMax": 100
    },
    {
      "name": "InterfaceRxBytes",
      "displayName": "Interface Rx",
      "description": "Interface Rx in bytes",
      "baseType": "number"
    },
    {
      "name": "InterfaceTxBytes",
      "displayName": "Interface Tx",
      "description": "Interface Tx in bytes",
      "baseType": "number"
    },
    {
      "name": "MemFree",
      "displayName": "Memory Free",
      "description": "Memory Free in bytes",
      "baseType": "number"
    },
    {
      "name": "MemTotal",
      "displayName": "Total Memory Space",
      "description": "Total Memory Space in bytes",
      "baseType": "number"
    },
    {
      "name": "MemUsed",
      "displayName": "Memory Used",
      "description": "Memory Used in bytes",
      "baseType": "number"
    },
    {
      "name": "MemUsedPercent",
      "displayName": "Percent Memory Used",
      "description": "Memory usage (percent) ",
      "baseType": "number",
      "scaleMin": 0,
      "scaleMax": 100
    },
    {
      "name": "reporterID",
      "displayName": "Reporting Node",
      "description": "Name of reporting node",
      "baseType": "string",
      "scaleMin": 0,
      "scaleMax": 100
    }
  ]
},
  SmartNIC: {
  "name": "SmartNIC",
  "displayName": "Naples",
  "description": "Contains metrics reported from the Naples",
  "fields": [
    {
      "name": "CPUUsedPercent",
      "displayName": "Percent CPU Used",
      "description": "CPU usage (percent) ",
      "baseType": "number",
      "scaleMin": 0,
      "scaleMax": 100
    },
    {
      "name": "DiskFree",
      "displayName": "Disk Free",
      "description": "Disk Free in bytes",
      "baseType": "number"
    },
    {
      "name": "DiskTotal",
      "displayName": "Total Disk Space",
      "description": "Total Disk Space in bytes",
      "baseType": "number"
    },
    {
      "name": "DiskUsed",
      "displayName": "Disk Used",
      "description": "Disk Used in bytes",
      "baseType": "number"
    },
    {
      "name": "DiskUsedPercent",
      "displayName": "Percent Disk Used",
      "description": "Disk usage (percent) ",
      "baseType": "number",
      "scaleMin": 0,
      "scaleMax": 100
    },
    {
      "name": "InterfaceRxBytes",
      "displayName": "Interface Rx",
      "description": "Interface Rx in bytes",
      "baseType": "number"
    },
    {
      "name": "InterfaceTxBytes",
      "displayName": "Interface Tx",
      "description": "Interface Tx in bytes",
      "baseType": "number"
    },
    {
      "name": "MemFree",
      "displayName": "Memory Free",
      "description": "Memory Free in bytes",
      "baseType": "number"
    },
    {
      "name": "MemTotal",
      "displayName": "Total Memory Space",
      "description": "Total Memory Space in bytes",
      "baseType": "number"
    },
    {
      "name": "MemUsed",
      "displayName": "Memory Used",
      "description": "Memory Used in bytes",
      "baseType": "number"
    },
    {
      "name": "MemUsedPercent",
      "displayName": "Percent Memory Used",
      "description": "Memory usage (percent) ",
      "baseType": "number",
      "scaleMin": 0,
      "scaleMax": 100
    },
    {
      "name": "reporterID",
      "displayName": "Reporting SmartNIC",
      "description": "Name of reporting SmartNIC",
      "baseType": "string",
      "scaleMin": 0,
      "scaleMax": 100
    }
  ]
},
}