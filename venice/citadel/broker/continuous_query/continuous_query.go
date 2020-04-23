package cq

import "strings"

// groupbyFieldType define different groupby field
var groupbyFieldType = map[int][]string{
	1: {`"name"`, `"reporterID"`, `"tenant"`},
	2: {`"Name"`, `"reporterID"`},
}

// CQMeasurementGroupByMap list for measurement to be applied to CQ
// key: measurement name
// value: groupby field type
var CQMeasurementGroupByMap = map[string]int{
	"AsicFrequencyMetrics":   1,
	"AsicPowerMetrics":       1,
	"AsicTemperatureMetrics": 1,
	"IPv4FlowDropMetrics":    1,
	"FteCPSMetrics":          1,
	"LifMetrics":             1,
	"SessionSummaryMetrics":  1,
	"RuleMetrics":            1,
	"FteLifQMetrics":         1,
	"MgmtMacMetrics":         1,
	"MacMetrics":             1,
	"EgressDropMetrics":      1,
	"Cluster":                2,
	"DistributedServiceCard": 2,
	"Node":                   2,
}

// tagMap all column name in this list will NOT be considered as CQ field
var tagMap = map[string]bool{
	"time":       true,
	"Kind":       true,
	"Name":       true,
	"name":       true,
	"tenant":     true,
	"reporterID": true,
}

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

// AllCQMeasurementMap collects all saved continuous query measureement
var AllCQMeasurementMap = map[string]bool{}

// IsContinuousQueryMeasurement check whether a measurement is a continuous query measurement or not
func IsContinuousQueryMeasurement(name string) bool {
	_, ok := AllCQMeasurementMap[name]
	return ok
}

// IsTag check whether a column is a tag or not
func IsTag(columnName string) bool {
	_, ok := tagMap[columnName]
	return ok
}

// getFieldString get field string
func getFieldString(aggFunc string, fields []string) string {
	fieldStringList := []string{}
	for _, field := range fields {
		fieldStringList = append(fieldStringList, aggFunc+`("`+field+`") AS "`+field+`"`)
	}
	return strings.Join(fieldStringList, ", ")
}

// groupbyFieldString get groupby field string
func groupbyFieldString(measurement string) string {
	return strings.Join(groupbyFieldType[CQMeasurementGroupByMap[measurement]], ", ")
}

// GenerateContinuousQueryMap generate cq map for specific measurement
func GenerateContinuousQueryMap(database string, measurement string, aggFunc string, fields []string) map[string]ContinuousQuerySpec {
	cqMap := map[string]ContinuousQuerySpec{}
	for suffix, rpSpec := range RetentionPolicyMap {
		cqMap[measurement+"_"+suffix] = ContinuousQuerySpec{
			CQName:                 measurement + "_" + suffix,
			DBName:                 database,
			RetentionPolicyName:    rpSpec.Name,
			RetentionPolicyInHours: rpSpec.Hours,
			Query: `CREATE CONTINUOUS QUERY ` + measurement + `_` + suffix + ` ON "` + database + `"
					BEGIN
						SELECT ` + getFieldString(aggFunc, fields) + `
						INTO "` + database + `"."` + rpSpec.Name + `"."` + measurement + `_` + suffix + `"
						FROM "` + measurement + `"
						GROUP BY time(` + rpSpec.GroupBy + `), ` + groupbyFieldString(measurement) + `
					END`,
		}
	}
	return cqMap
}
