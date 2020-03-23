package cq

// ContinuousQuerySpec spec to save info for CreateContinuousQuery API
type ContinuousQuerySpec struct {
	CQName                 string
	DBName                 string
	RetentionPolicyName    string
	RetentionPolicyInHours uint64
	Query                  string
}

// ContinuousQueryRetentionSpec spec to save retention info for continuous query
type ContinuousQueryRetentionSpec struct {
	Name    string
	Hours   uint64
	GroupBy string
}

// RetentionPolicyMap use suffix label to get corresponded retention policy name
var RetentionPolicyMap = map[string]ContinuousQueryRetentionSpec{
	"1day": ContinuousQueryRetentionSpec{
		Name:    "rp_1y",
		Hours:   365 * 24,
		GroupBy: "1d",
	},
	"1hour": ContinuousQueryRetentionSpec{
		Name:    "rp_30d",
		Hours:   30 * 24,
		GroupBy: "1h",
	},
	"5minutes": ContinuousQueryRetentionSpec{
		Name:    "rp_5d",
		Hours:   5 * 24,
		GroupBy: "5m",
	},
}

// ContinuousQueryMap info for running continuous query on running service
var ContinuousQueryMap = map[string]ContinuousQuerySpec{}

// IsContinuousQueryMeasurement check whether a measurement is a continuous query measurement or not
func IsContinuousQueryMeasurement(name string) bool {
	_, ok := ContinuousQueryMap[name]
	return ok
}

// InitContinuousQueryMap build ContinuousQuerySpec map in preparation of CreateContinuousQuery API
func InitContinuousQueryMap() {
	for suffix, rpSpec := range RetentionPolicyMap {
		generateContinuousQueryMap(suffix, rpSpec)
	}
}

func generateContinuousQueryMap(suffix string, rpSpec ContinuousQueryRetentionSpec) {
	ContinuousQueryMap["AsicFrequencyMetrics_"+suffix] = ContinuousQuerySpec{
		CQName:                 "AsicFrequencyMetrics_" + suffix,
		DBName:                 "default",
		RetentionPolicyName:    rpSpec.Name,
		RetentionPolicyInHours: rpSpec.Hours,
		Query: `CREATE CONTINUOUS QUERY AsicFrequencyMetrics_` + suffix + ` ON "default"
				BEGIN
					SELECT last("Frequency") AS "Frequency"
					INTO "default"."` + rpSpec.Name + `"."AsicFrequencyMetrics_` + suffix + `"
					FROM "AsicFrequencyMetrics"
					GROUP BY time(` + rpSpec.GroupBy + `), "name", "reporterID", "tenant"
				END`,
	}

	ContinuousQueryMap["AsicPowerMetrics_"+suffix] = ContinuousQuerySpec{
		CQName:                 "AsicPowerMetrics_" + suffix,
		DBName:                 "default",
		RetentionPolicyName:    rpSpec.Name,
		RetentionPolicyInHours: rpSpec.Hours,
		Query: `CREATE CONTINUOUS QUERY AsicPowerMetrics_` + suffix + ` ON "default"
				BEGIN
					SELECT last("Pin") AS "Pin",
							last("Pout1") AS "Pout1", 
							last("Pout2") AS "Pout2" 
					INTO "default"."` + rpSpec.Name + `"."AsicPowerMetrics_` + suffix + `"
					FROM "AsicPowerMetrics"
					GROUP BY time(` + rpSpec.GroupBy + `), "name", "reporterID", "tenant"
				END`,
	}

	ContinuousQueryMap["AsicTemperatureMetrics_"+suffix] = ContinuousQuerySpec{
		CQName:                 "AsicTemperatureMetrics_" + suffix,
		DBName:                 "default",
		RetentionPolicyName:    rpSpec.Name,
		RetentionPolicyInHours: rpSpec.Hours,
		Query: `CREATE CONTINUOUS QUERY AsicTemperatureMetrics_` + suffix + ` ON "default"
				BEGIN
					SELECT last("DieTemperature") AS "DieTemperature", 
							last("HbmTemperature") AS "HbmTemperature", 
							last("LocalTemperature") AS "LocalTemperature", 
							last("QsfpPort1Temperature") AS "QsfpPort1Temperature", 
							last("QsfpPort2Temperature") AS "QsfpPort2Temperature", 
							last("QsfpPort1WarningTemperature") AS "QsfpPort1WarningTemperature", 
							last("QsfpPort2WarningTemperature") AS "QsfpPort2WarningTemperature", 
							last("QsfpPort1AlarmTemperature") AS "QsfpPort1AlarmTemperature", 
							last("QsfpPort2AlarmTemperature") AS "QsfpPort2AlarmTemperature" 
					INTO "default"."` + rpSpec.Name + `"."AsicTemperatureMetrics_` + suffix + `"
					FROM "AsicTemperatureMetrics"
					GROUP BY time(` + rpSpec.GroupBy + `), "name", "reporterID", "tenant"
				END`,
	}

	ContinuousQueryMap["Cluster_"+suffix] = ContinuousQuerySpec{
		CQName:                 "Cluster_" + suffix,
		DBName:                 "default",
		RetentionPolicyName:    rpSpec.Name,
		RetentionPolicyInHours: rpSpec.Hours,
		Query: `CREATE CONTINUOUS QUERY Cluster_` + suffix + ` ON "default"
				BEGIN
					SELECT last("AdmittedNICs") AS "AdmittedNICs", 
							last("DecommissionedNICs") AS "DecommissionedNICs", 
							last("DisconnectedNICs") AS "DisconnectedNICs", 
							last("HealthyNICs") AS "HealthyNICs", 
							last("PendingNICs") AS "PendingNICs", 
							last("RejectedNICs") AS "RejectedNICs", 
							last("UnhealthyNICs") AS "UnhealthyNICs" 
					INTO "default"."` + rpSpec.Name + `"."Cluster_` + suffix + `"
					FROM "Cluster"
					GROUP BY time(` + rpSpec.GroupBy + `), "Name", "reporterID"
				END`,
	}

	ContinuousQueryMap["DistributedServiceCard_"+suffix] = ContinuousQuerySpec{
		CQName:                 "DistributedServiceCard_" + suffix,
		DBName:                 "default",
		RetentionPolicyName:    rpSpec.Name,
		RetentionPolicyInHours: rpSpec.Hours,
		Query: `CREATE CONTINUOUS QUERY DistributedServiceCard_` + suffix + ` ON "default"
				BEGIN
					SELECT last("CPUUsedPercent") AS "CPUUsedPercent", 
							last("DiskFree") AS "DiskFree", 
							last("DiskTotal") AS "DiskTotal", 
							last("DiskUsed") AS "DiskUsed", 
							last("DiskUsedPercent") AS "DiskUsedPercent", 
							last("InterfaceRxBytes") AS "InterfaceRxBytes", 
							last("InterfaceTxBytes") AS "InterfaceTxBytes", 
							last("MemAvailable") AS "MemAvailable", 
							last("MemFree") AS "MemFree", 
							last("MemTotal") AS "MemTotal", 
							last("MemUsed") AS "MemUsed", 
							last("MemUsedPercent") AS "MemUsedPercent" 
					INTO "default"."` + rpSpec.Name + `"."DistributedServiceCard_` + suffix + `"
					FROM "DistributedServiceCard"
					GROUP BY time(` + rpSpec.GroupBy + `), "Name", "reporterID"
				END`,
	}

	ContinuousQueryMap["IPv4FlowDropMetrics_"+suffix] = ContinuousQuerySpec{
		CQName:                 "IPv4FlowDropMetrics_" + suffix,
		DBName:                 "default",
		RetentionPolicyName:    rpSpec.Name,
		RetentionPolicyInHours: rpSpec.Hours,
		Query: `CREATE CONTINUOUS QUERY IPv4FlowDropMetrics_` + suffix + ` ON "default"
				BEGIN
					SELECT last("Instances") AS "Instances",
							last("DropPackets") AS "DropPackets",
							last("DropBytes") AS "DropBytes",
							last("DropFirstTimestamp") AS "DropFirstTimestamp",
							last("DropLastTimestamp") AS "DropLastTimestamp",
							last("DropReason") AS "DropReason"
					INTO "default"."` + rpSpec.Name + `"."IPv4FlowDropMetrics_` + suffix + `"
					FROM "IPv4FlowDropMetrics"
					GROUP BY time(` + rpSpec.GroupBy + `), "name", "reporterID", "tenant"
				END`,
	}

	ContinuousQueryMap["FteCPSMetrics_"+suffix] = ContinuousQuerySpec{
		CQName:                 "FteCPSMetrics_" + suffix,
		DBName:                 "default",
		RetentionPolicyName:    rpSpec.Name,
		RetentionPolicyInHours: rpSpec.Hours,
		Query: `CREATE CONTINUOUS QUERY FteCPSMetrics_` + suffix + ` ON "default"
				BEGIN
					SELECT last("ConnectionsPerSecond") AS "ConnectionsPerSecond", 
							last("MaxConnectionsPerSecond") AS "MaxConnectionsPerSecond", 
							last("PacketsPerSecond") AS "PacketsPerSecond", 
							last("MaxPacketsPerSecond") AS "MaxPacketsPerSecond" 
					INTO "default"."` + rpSpec.Name + `"."FteCPSMetrics_` + suffix + `"
					FROM "FteCPSMetrics"
					GROUP BY time(` + rpSpec.GroupBy + `), "name", "reporterID", "tenant"
				END`,
	}

	ContinuousQueryMap["LifMetrics_"+suffix] = ContinuousQuerySpec{
		CQName:                 "LifMetrics_" + suffix,
		DBName:                 "default",
		RetentionPolicyName:    rpSpec.Name,
		RetentionPolicyInHours: rpSpec.Hours,
		Query: `CREATE CONTINUOUS QUERY LifMetrics_` + suffix + ` ON "default"
				BEGIN
					SELECT last("ConnectionsPerSecond") AS "ConnectionsPerSecond", 
							last("RxBroadcastPackets") AS "RxBroadcastPackets",
							last("RxBroadcastBytes") AS "RxBroadcastBytes",
							last("RxDmaError") AS "RxDmaError",
							last("RxDropBroadcastBytes") AS "RxDropBroadcastBytes",
							last("RxDropBroadcastPackets") AS "RxDropBroadcastPackets",
							last("RxDropMulticastBytes") AS "RxDropMulticastBytes",
							last("RxDropMulticastPackets") AS "RxDropMulticastPackets",
							last("RxDropUnicastBytes") AS "RxDropUnicastBytes",
							last("RxDropUnicastPackets") AS "RxDropUnicastPackets",
							last("RxMulticastBytes") AS "RxMulticastBytes",
							last("RxMulticastPackets") AS "RxMulticastPackets",
							last("RxRdmaCnpPackets") AS "RxRdmaCnpPackets",
							last("RxRdmaEcnPackets") AS "RxRdmaEcnPackets",
							last("RxRdmaMcastBytes") AS "RxRdmaMcastBytes",
							last("RxRdmaMcastPackets") AS "RxRdmaMcastPackets",
							last("RxRdmaUcastBytes") AS "RxRdmaUcastBytes",
							last("RxRdmaUcastPackets") AS "RxRdmaUcastPackets",
							last("RxUnicastBytes") AS "RxUnicastBytes",
							last("RxUnicastPackets") AS "RxUnicastPackets",
							last("TxBroadcastBytes") AS "TxBroadcastBytes",
							last("TxBroadcastPackets") AS "TxBroadcastPackets",
							last("TxDescDataError") AS "TxDescDataError",
							last("TxDescFetchError") AS "TxDescFetchError",
							last("TxDropBroadcastBytes") AS "TxDropBroadcastBytes",
							last("TxDropBroadcastPackets") AS "TxDropBroadcastPackets",
							last("TxDropMulticastBytes") AS "TxDropMulticastBytes",
							last("TxDropMulticastPackets") AS "TxDropMulticastPackets",
							last("TxDropUnicastBytes") AS "TxDropUnicastBytes",
							last("TxDropUnicastPackets") AS "TxDropUnicastPackets",
							last("TxMulticastBytes") AS "TxMulticastBytes",
							last("TxMulticastPackets") AS "TxMulticastPackets",
							last("TxQueueDisabledDrops") AS "TxQueueDisabledDrops",
							last("TxQueueSched") AS "TxQueueSched",
							last("TxUnicastBytes") AS "TxUnicastBytes", 
							last("TxUnicastPackets") AS "TxUnicastPackets" 
					INTO "default"."` + rpSpec.Name + `"."LifMetrics_` + suffix + `"
					FROM "LifMetrics"
					GROUP BY time(` + rpSpec.GroupBy + `), "name", "reporterID", "tenant"
				END`,
	}

	ContinuousQueryMap["SessionSummaryMetrics_"+suffix] = ContinuousQuerySpec{
		CQName:                 "SessionSummaryMetrics_" + suffix,
		DBName:                 "default",
		RetentionPolicyName:    rpSpec.Name,
		RetentionPolicyInHours: rpSpec.Hours,
		Query: `CREATE CONTINUOUS QUERY SessionSummaryMetrics_` + suffix + ` ON "default"
				BEGIN
					SELECT last("NumAgedSessions") AS "NumAgedSessions",
							last("NumDropSessions") AS "NumDropSessions",
							last("NumIcmpErrors") AS "NumIcmpErrors",
							last("NumIcmpSessionLimitDrops") AS "NumIcmpSessionLimitDrops",
							last("NumIcmpSessions") AS "NumIcmpSessions",
							last("NumL2Sessions") AS "NumL2Sessions",
							last("NumOtherActiveSessions") AS "NumOtherActiveSessions",
							last("NumOtherSessionLimitDrops") AS "NumOtherSessionLimitDrops",
							last("NumSessionCreateErrors") AS "NumSessionCreateErrors",
							last("NumTcpCxnsetupTimeouts") AS "NumTcpCxnsetupTimeouts",
							last("NumTcpHalfOpenSessions") AS "NumTcpHalfOpenSessions",
							last("NumTcpResets") AS "NumTcpResets",
							last("NumTcpSessionLimitDrops") AS "NumTcpSessionLimitDrops",
							last("NumTcpSessions") AS "NumTcpSessions",
							last("NumUdpSessionLimitDrops") AS "NumUdpSessionLimitDrops",
							last("NumUdpSessions") AS "NumUdpSessions",
							last("TotalActiveSessions") AS "TotalActiveSessions"
					INTO "default"."` + rpSpec.Name + `"."SessionSummaryMetrics_` + suffix + `"
					FROM "SessionSummaryMetrics"
					GROUP BY time(` + rpSpec.GroupBy + `), "name", "reporterID", "tenant"
				END`,
	}

	ContinuousQueryMap["RuleMetrics_"+suffix] = ContinuousQuerySpec{
		CQName:                 "RuleMetrics_" + suffix,
		DBName:                 "default",
		RetentionPolicyName:    rpSpec.Name,
		RetentionPolicyInHours: rpSpec.Hours,
		Query: `CREATE CONTINUOUS QUERY RuleMetrics_` + suffix + ` ON "default"
				BEGIN
					SELECT last("EspHits") AS "EspHits",
							last("IcmpHits") AS "IcmpHits",
							last("OtherHits") AS "OtherHits",
							last("TcpHits") AS "TcpHits",
							last("TotalHits") AS "TotalHits",
							last("UdpHits") AS "UdpHits" 
					INTO "default"."` + rpSpec.Name + `"."RuleMetrics_` + suffix + `"
					FROM "RuleMetrics"
					GROUP BY time(` + rpSpec.GroupBy + `), "name", "reporterID", "tenant"
				END`,
	}

	ContinuousQueryMap["FteLifQMetrics_"+suffix] = ContinuousQuerySpec{
		CQName:                 "FteLifQMetrics_" + suffix,
		DBName:                 "default",
		RetentionPolicyName:    rpSpec.Name,
		RetentionPolicyInHours: rpSpec.Hours,
		Query: `CREATE CONTINUOUS QUERY FteLifQMetrics_` + suffix + ` ON "default"
				BEGIN
					SELECT last("FlowMissPackets") AS "FlowMissPackets",
							last("FlowRetransmitPackets") AS "FlowRetransmitPackets",
							last("L4RedirectPackets") AS "L4RedirectPackets",
							last("AlgControlFlowPackets") AS "AlgControlFlowPackets",
							last("TcpClosePackets") AS "TcpClosePackets",
							last("TlsProxyPackets") AS "TlsProxyPackets",
							last("FteSpanPackets") AS "FteSpanPackets",
							last("SoftwareQueuePackets") AS "SoftwareQueuePackets",
							last("QueuedTxPackets") AS "QueuedTxPackets",
							last("FreedTxPackets") AS "FreedTxPackets",
							last("MaxSessionThresholdDrops") AS "MaxSessionThresholdDrops"
					INTO "default"."` + rpSpec.Name + `"."FteLifQMetrics_` + suffix + `"
					FROM "FteLifQMetrics"
					GROUP BY time(` + rpSpec.GroupBy + `), "name", "reporterID", "tenant"
				END`,
	}

	ContinuousQueryMap["Node_"+suffix] = ContinuousQuerySpec{
		CQName:                 "Node_" + suffix,
		DBName:                 "default",
		RetentionPolicyName:    rpSpec.Name,
		RetentionPolicyInHours: rpSpec.Hours,
		Query: `CREATE CONTINUOUS QUERY Node_` + suffix + ` ON "default"
				BEGIN
					SELECT last("CPUUsedPercent") AS "CPUUsedPercent",
							last("DiskFree") AS "DiskFree",
							last("DiskTotal") AS "DiskTotal",
							last("DiskUsed") AS "DiskUsed",
							last("DiskUsedPercent") AS "DiskUsedPercent",
							last("InterfaceRxBytes") AS "InterfaceRxBytes",
							last("InterfaceTxBytes") AS "InterfaceTxBytes",
							last("MemAvailable") AS "MemAvailable",
							last("MemFree") AS "MemFree",
							last("MemTotal") AS "MemTotal",
							last("MemUsed") AS "MemUsed",
							last("MemUsedPercent") AS  "MemUsedPercent"
					INTO "default"."` + rpSpec.Name + `"."Node_` + suffix + `"
					FROM "Node"
					GROUP BY time(` + rpSpec.GroupBy + `), "Name", "reporterID"
				END`,
	}

	ContinuousQueryMap["MgmtMacMetrics_"+suffix] = ContinuousQuerySpec{
		CQName:                 "MgmtMacMetrics_" + suffix,
		DBName:                 "default",
		RetentionPolicyName:    rpSpec.Name,
		RetentionPolicyInHours: rpSpec.Hours,
		Query: `CREATE CONTINUOUS QUERY MgmtMacMetrics_` + suffix + ` ON "default"
				BEGIN
					SELECT last("FramesRxAll") AS "FramesRxAll",
							last("FramesRxBadAll") AS "FramesRxBadAll",
							last("FramesRxBadFcs") AS "FramesRxBadFcs",
							last("FramesRxBadLength") AS "FramesRxBadLength",
							last("FramesRxBroadcast") AS "FramesRxBroadcast",
							last("FramesRxFifoFull") AS "FramesRxFifoFull",
							last("FramesRxFragments") AS "FramesRxFragments",
							last("FramesRxGt_1518B") AS "FramesRxGt_1518B",
							last("FramesRxJabber") AS "FramesRxJabber",
							last("FramesRxMulticast") AS "FramesRxMulticast",
							last("FramesRxOk") AS "FramesRxOk",
							last("FramesRxOversized") AS "FramesRxOversized",
							last("FramesRxPause") AS "FramesRxPause",
							last("FramesRxUndersized") AS "FramesRxUndersized",
							last("FramesRxUnicast") AS  "FramesRxUnicast",
							last("FramesRx_1024B_1518B") AS "FramesRx_1024B_1518B",
							last("FramesRx_128B_255B") AS "FramesRx_128B_255B",
							last("FramesRx_256B_511B") AS "FramesRx_256B_511B",
							last("FramesRx_512B_1023B") AS "FramesRx_512B_1023B",
							last("FramesRx_64B") AS "FramesRx_64B",
							last("FramesRx_65B_127B") AS "FramesRx_65B_127B",
							last("FramesTxAll") AS "FramesTxAll",
							last("FramesTxBad") AS "FramesTxBad",
							last("FramesTxBroadcast") AS "FramesTxBroadcast",
							last("FramesTxMulticast") AS "FramesTxMulticast",
							last("FramesTxOk") AS "FramesTxOk",
							last("FramesTxPause") AS "FramesTxPause",
							last("FramesTxUnicast") AS "FramesTxUnicast",
							last("OctetsRxAll") AS "OctetsRxAll",
							last("OctetsRxOk") AS "OctetsRxOk",
							last("OctetsTxOk") AS "OctetsTxOk",
							last("OctetsTxTotal") AS "OctetsTxTotal"
					INTO "default"."` + rpSpec.Name + `"."MgmtMacMetrics_` + suffix + `"
					FROM "MgmtMacMetrics"
					GROUP BY time(` + rpSpec.GroupBy + `), "name", "reporterID", "tenant"
				END`,
	}

	ContinuousQueryMap["MacMetrics_"+suffix] = ContinuousQuerySpec{
		CQName:                 "MacMetrics_" + suffix,
		DBName:                 "default",
		RetentionPolicyName:    rpSpec.Name,
		RetentionPolicyInHours: rpSpec.Hours,
		Query: `CREATE CONTINUOUS QUERY MacMetrics_` + suffix + ` ON "default"
				BEGIN
					SELECT last("FramesRxAll") AS "FramesRxAll",
							last("FramesRxBadAll") AS "FramesRxBadAll",
							last("FramesRxBadFcs") AS "FramesRxBadFcs",
							last("FramesRxBadLength") AS "FramesRxBadLength",
							last("FramesRxBroadcast") AS "FramesRxBroadcast",
							last("FramesRxDropped") AS "FramesRxDropped",
							last("FramesRxFragments") AS "FramesRxFragments",
							last("FramesRxJabber") AS "FramesRxJabber",
							last("FramesRxLessThan_64B") AS "FramesRxLessThan_64B",
							last("FramesRxMulticast") AS "FramesRxMulticast",
							last("FramesRxOk") AS "FramesRxOk",
							last("FramesRxOther") AS "FramesRxOther",
							last("FramesRxOversized") AS "FramesRxOversized",
							last("FramesRxPause") AS "FramesRxPause",
							last("FramesRxPri_0") AS "FramesRxPri_0",
							last("FramesRxPri_1") AS "FramesRxPri_1",
							last("FramesRxPri_2") AS "FramesRxPri_2",
							last("FramesRxPri_3") AS "FramesRxPri_3",
							last("FramesRxPri_4") AS "FramesRxPri_4",
							last("FramesRxPri_5") AS "FramesRxPri_5",
							last("FramesRxPri_6") AS "FramesRxPri_6",
							last("FramesRxPri_7") AS "FramesRxPri_7",
							last("FramesRxPripause") AS "FramesRxPripause",
							last("FramesRxStompedCrc") AS "FramesRxStompedCrc",
							last("FramesRxTooLong") AS "FramesRxTooLong",
							last("FramesRxUndersized") AS "FramesRxUndersized",
							last("FramesRxUnicast") AS "FramesRxUnicast",
							last("FramesRxVlanGood") AS "FramesRxVlanGood",
							last("FramesRx_1024B_1518B") AS "FramesRx_1024B_1518B",
							last("FramesRx_128B_255B") AS "FramesRx_128B_255B",
							last("FramesRx_1519B_2047B") AS "FramesRx_1519B_2047B",
							last("FramesRx_2048B_4095B") AS "FramesRx_2048B_4095B",
							last("FramesRx_256B_511B") AS "FramesRx_256B_511B",
							last("FramesRx_4096B_8191B") AS "FramesRx_4096B_8191B",
							last("FramesRx_512B_1023B") AS "FramesRx_512B_1023B",
							last("FramesRx_64B") AS "FramesRx_64B",
							last("FramesRx_65B_127B") AS "FramesRx_65B_127B",
							last("FramesRx_8192B_9215B") AS "FramesRx_8192B_9215B",
							last("FramesTxAll") AS "FramesTxAll",
							last("FramesTxBad") AS "FramesTxBad",
							last("FramesTxBroadcast") AS "FramesTxBroadcast",
							last("FramesTxLessThan_64B") AS "FramesTxLessThan_64B",
							last("FramesTxMulticast") AS "FramesTxMulticast",
							last("FramesTxOk") AS "FramesTxOk",
							last("FramesTxOther") AS "FramesTxOther",
							last("FramesTxPause") AS "FramesTxPause",
							last("FramesTxPri_0") AS "FramesTxPri_0",
							last("FramesTxPri_1") AS "FramesTxPri_1",
							last("FramesTxPri_2") AS "FramesTxPri_2",
							last("FramesTxPri_3") AS "FramesTxPri_3",
							last("FramesTxPri_4") AS "FramesTxPri_4",
							last("FramesTxPri_5") AS "FramesTxPri_5",
							last("FramesTxPri_6") AS "FramesTxPri_6",
							last("FramesTxPri_7") AS "FramesTxPri_7",
							last("FramesTxPripause") AS "FramesTxPripause",
							last("FramesTxTruncated") AS "FramesTxTruncated",
							last("FramesTxUnicast") AS "FramesTxUnicast",
							last("FramesTxVlan") AS "FramesTxVlan",
							last("FramesTx_1024B_1518B") AS "FramesTx_1024B_1518B",
							last("FramesTx_128B_255B") AS "FramesTx_128B_255B",
							last("FramesTx_1519B_2047B") AS "FramesTx_1519B_2047B",
							last("FramesTx_2048B_4095B") AS "FramesTx_2048B_4095B",
							last("FramesTx_256B_511B") AS "FramesTx_256B_511B",
							last("FramesTx_4096B_8191B") AS "FramesTx_4096B_8191B",
							last("FramesTx_512B_1023B") AS "FramesTx_512B_1023B",
							last("FramesTx_64B") AS "FramesTx_64B",
							last("FramesTx_65B_127B") AS "FramesTx_65B_127B",
							last("FramesTx_8192B_9215B") AS "FramesTx_8192B_9215B",
							last("OctetsRxAll") AS "OctetsRxAll",
							last("OctetsRxOk") AS "OctetsRxOk",
							last("OctetsTxOk") AS "OctetsTxOk",
							last("OctetsTxTotal") AS "OctetsTxTotal",
							last("RxPause_1UsCount") AS "RxPause_1UsCount",
							last("RxPripause_0_1UsCount") AS "RxPripause_0_1UsCount",
							last("RxPripause_1_1UsCount") AS "RxPripause_1_1UsCount",
							last("RxPripause_2_1UsCount") AS "RxPripause_2_1UsCount",
							last("RxPripause_3_1UsCount") AS "RxPripause_3_1UsCount",
							last("RxPripause_4_1UsCount") AS "RxPripause_4_1UsCount",
							last("RxPripause_5_1UsCount") AS "RxPripause_5_1UsCount",
							last("RxPripause_6_1UsCount") AS "RxPripause_6_1UsCount",
							last("RxPripause_7_1UsCount") AS "RxPripause_7_1UsCount",
							last("TxPripause_0_1UsCount") AS "TxPripause_0_1UsCount",
							last("TxPripause_1_1UsCount") AS "TxPripause_1_1UsCount",
							last("TxPripause_2_1UsCount") AS "TxPripause_2_1UsCount",
							last("TxPripause_3_1UsCount") AS "TxPripause_3_1UsCount",
							last("TxPripause_4_1UsCount") AS "TxPripause_4_1UsCount",
							last("TxPripause_5_1UsCount") AS "TxPripause_5_1UsCount",
							last("TxPripause_6_1UsCount") AS "TxPripause_6_1UsCount",
							last("TxPripause_7_1UsCount") AS "TxPripause_7_1UsCount"
					INTO "default"."` + rpSpec.Name + `"."MacMetrics_` + suffix + `"
					FROM "MacMetrics"
					GROUP BY time(` + rpSpec.GroupBy + `), "name", "reporterID", "tenant"
				END`,
	}
}
