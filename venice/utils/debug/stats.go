package debug

import (
	"expvar"
	"runtime"
	"strings"
	"sync"
)

// Stats stores counters for debug purpose
type Stats struct {
	debugs *expvar.Map
}

// CPUStats is collection of runtime process stats
type CPUStats struct {
	NumCPUs       uint16
	NumGoroutines uint64
	NumCgoCalls   uint64
}

var once sync.Once

// Publish runtime process stats
func init() {
	once.Do(func() {
		expvar.Publish("cpustats", expvar.Func(GetCPUStats))
	})
}

// New debug stats
func New(instance string) *Stats {
	// tools are more happy without ':'  Hence convert that to _
	debugInstance := "debugStats_" + strings.Replace(instance, ":", "_", -1)
	return &Stats{
		debugs: expvar.NewMap(debugInstance),
	}
}

// Increment increments this counter
func (st *Stats) Increment(counterName string) {
	st.debugs.Add(counterName, 1)
}

// GetCPUStats returns runtime process stats
func GetCPUStats() interface{} {
	stats := CPUStats{
		NumCPUs:       uint16(runtime.NumCPU()),
		NumGoroutines: uint64(runtime.NumGoroutine()),
		NumCgoCalls:   uint64(runtime.NumCgoCall()),
	}
	return &stats
}
