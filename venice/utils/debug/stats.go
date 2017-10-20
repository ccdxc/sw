package debug

import (
	"expvar"
	"strings"
)

// Stats stores counters for debug purpose
type Stats struct {
	debugs *expvar.Map
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
