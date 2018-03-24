package api

import "time"

// metrics apis defines basic unit of metric collection and export

// Counter implements an incrementing metric
type Counter interface {
	Add(int64)
	Inc()
}

// Gauge implements a changeable metric i.e. can be set to any value
type Gauge interface {
	Set(float64, time.Time)
}

// Bool implements a binary metric i.e. can be set to true or false
type Bool interface {
	Set(bool, time.Time)
}

// String implements a metric with string enumeration
type String interface {
	Set(string, time.Time)
}

// Histogram implements a distribution of values within a range
type Histogram interface {
	SetRanges(ranges []int64) Histogram
	AddSample(int64)
}

// Summary implements a metric that computes averages over time
type Summary interface {
	AddSample(float64)
}
