// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package gometrics

// Metrics is the interface all meterics objects have to implement
type Metrics interface {
	// GetKey() string          // returns the key for the metrics
	String() string                      // return json string
	GetCounter(offset int) Counter       // get a counter attribute
	GetGauge(offset int) Gauge           // get gauge attribute
	SetCounter(cntr Counter, offset int) // set counter value
	SetGauge(val Gauge, offset int)      // set gauge value
}

// MetricsIterator can iterate over a kind of metrics objects
type MetricsIterator interface {
	HasNext() bool                      // does iterator has next value?
	Next() Metrics                      // get the next value
	Find(key string) (Metrics, error)   // get a specific metrics entry
	Create(key string, len int) Metrics // create a metrics entry
	Delete(key string) error            // delete a metrics entry
}

// type definitions
type Counter uint64
type Gauge float64

// Size returns len of counter
func (c Counter) Size() int {
	return 8
}

// Size returns byte length of gauge
func (g Gauge) Size() int {
	return 8
}

// NewMetricsIterator returns new metrics iterator for a kind
func NewMetricsIterator(kind string) (MetricsIterator, error) {
	return newMetricsIterator(kind)
}
