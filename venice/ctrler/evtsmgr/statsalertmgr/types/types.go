package types

// QueryResponse represents the metrics query response for a policy, this response will be fed to alert engine
// for creating/updating/deleting the underlying alert.
type QueryResponse struct {
	ByReporterID map[string]*MetricValue
}

// MetricValue represents the metric value for a single node/reporter.
type MetricValue struct {
	Time string
	Val  float64
}
