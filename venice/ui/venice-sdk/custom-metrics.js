metrics  = [
  {
    name: 'Node',
    displayName: 'Node',
    description: 'Contains metrics reported from the Venice Nodes',
    fields: [
      {
        name: 'CPUUsedPercent',
        displayName: 'Percent CPU Used',
        description: 'CPU usage (percent) ',
        baseType: 'number',
        scaleMin: 0,
        scaleMax: 100,
      },
      {
        name: 'DiskFree',
        displayName: 'Disk Free',
        description: 'Disk Free in bytes',
        baseType: 'number',
      },
      {
        name: 'DiskTotal',
        displayName: 'Total Disk Space',
        description: 'Total Disk Space in bytes',
        baseType: 'number',
      },
      {
        name: 'DiskUsed',
        displayName: 'Disk Used',
        description: 'Disk Used in bytes',
        baseType: 'number',
      },
      {
        name: 'DiskUsedPercent',
        displayName: 'Percent disk Used',
        description: 'Disk usage (percent) ',
        baseType: 'number',
        scaleMin: 0,
        scaleMax: 100,
      },
      {
        name: 'InterfaceRxBytes',
        displayName: 'Interface Rx',
        description: 'Interface Rx in bytes',
        baseType: 'number',
      },
      {
        name: 'InterfaceTxBytes',
        displayName: 'Interface Tx',
        description: 'Interface Tx in bytes',
        baseType: 'number',
      },
      {
        name: 'MemFree',
        displayName: 'Memory Free',
        description: 'Memory Free in bytes',
        baseType: 'number',
      },
      {
        name: 'MemTotal',
        displayName: 'Total Memory Space',
        description: 'Total Memory Space in bytes',
        baseType: 'number',
      },
      {
        name: 'MemUsed',
        displayName: 'Memory Used',
        description: 'Memory Used in bytes',
        baseType: 'number',
      },
      {
        name: 'MemUsedPercent',
        displayName: 'Percent Memory Used',
        description: 'Memory usage (percent) ',
        baseType: 'number',
        scaleMin: 0,
        scaleMax: 100,
      },
      {
        name: 'reporterID',
        displayName: 'Reporting Node',
        description: 'Name of reporting node',
        baseType: 'string',
        scaleMin: 0,
        scaleMax: 100,
      },
    ]
  },
  {
    name: 'SmartNIC',
    displayName: 'Naples',
    description: 'Contains metrics reported from the Naples',
    fields: [
      {
        name: 'CPUUsedPercent',
        displayName: 'Percent CPU Used',
        description: 'CPU usage (percent) ',
        baseType: 'number',
        scaleMin: 0,
        scaleMax: 100,
      },
      {
        name: 'DiskFree',
        displayName: 'Disk Free',
        description: 'Disk Free in bytes',
        baseType: 'number',
      },
      {
        name: 'DiskTotal',
        displayName: 'Total Disk Space',
        description: 'Total Disk Space in bytes',
        baseType: 'number',
      },
      {
        name: 'DiskUsed',
        displayName: 'Disk Used',
        description: 'Disk Used in bytes',
        baseType: 'number',
      },
      {
        name: 'DiskUsedPercent',
        displayName: 'Percent Disk Used',
        description: 'Disk usage (percent) ',
        baseType: 'number',
        scaleMin: 0,
        scaleMax: 100,
      },
      {
        name: 'InterfaceRxBytes',
        displayName: 'Interface Rx',
        description: 'Interface Rx in bytes',
        baseType: 'number',
      },
      {
        name: 'InterfaceTxBytes',
        displayName: 'Interface Tx',
        description: 'Interface Tx in bytes',
        baseType: 'number',
      },
      {
        name: 'MemFree',
        displayName: 'Memory Free',
        description: 'Memory Free in bytes',
        baseType: 'number',
      },
      {
        name: 'MemTotal',
        displayName: 'Total Memory Space',
        description: 'Total Memory Space in bytes',
        baseType: 'number',
      },
      {
        name: 'MemUsed',
        displayName: 'Memory Used',
        description: 'Memory Used in bytes',
        baseType: 'number'
      },
      {
        name: 'MemUsedPercent',
        displayName: 'Percent Memory Used',
        description: 'Memory usage (percent) ',
        baseType: 'number',
        scaleMin: 0,
        scaleMax: 100,
      },
      {
        name: 'reporterID',
        displayName: 'Reporting SmartNIC',
        description: 'Name of reporting SmartNIC',
        baseType: 'string',
        scaleMin: 0,
        scaleMax: 100,
      },
    ]
  }
];

module.exports = {
  metrics: metrics
}