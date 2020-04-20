package types

// DscMetricsList is the list of metrics saved in Venice, last updated: 03/11/2020
var DscMetricsList = []string{
	"AsicFrequencyMetrics",
	"AsicPowerMetrics",
	"AsicTemperatureMetrics",
	"DropMetrics",
	"EgressDropMetrics",
	"IPv4FlowDropMetrics",
	"FteCPSMetrics",
	"FteLifQMetrics",
	"MacMetrics",
	"MgmtMacMetrics",
	"LifMetrics",
	"RuleMetrics",
	"SessionSummaryMetrics",
}

// CloudDscMetricsList is the list of metrics saved in Venice for cloud pipeline, last updated: 03/11/2020
var CloudDscMetricsList = []string{
	"MacMetrics",
	"MgmtMacMetrics",
	"LifMetrics",
}
