package metrics

// todo: use common metric types in delphi & ntsdb to avoid conversion
// type definition used in delphi metrics

// Counter is the metric type used in delphi metrics
type Counter uint64

// Gauge is the metric type used in delphi metrics
type Gauge float64

// Size returns len of counter
func (c Counter) Size() int {
	return 8
}

// Size returns byte length of gauge
func (g Gauge) Size() int {
	return 8
}
