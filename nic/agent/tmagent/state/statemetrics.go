package state

import (
	"expvar"
	"strconv"
)

var metric = newMetrics()

type stateMetrics struct {
	fwlogDrops   *expvar.Int
	fwlogSuccess *expvar.Int
	fwlogRetries *expvar.Map
}

func newMetrics() *stateMetrics {
	return &stateMetrics{
		fwlogDrops:   expvar.NewInt("objstoreFwlogDrops"),
		fwlogSuccess: expvar.NewInt("objstoreFwlogSuccess"),
		fwlogRetries: expvar.NewMap("objstoreFwlogRetries"),
	}
}

func (m *stateMetrics) addDrop() {
	m.fwlogDrops.Add(1)
}

func (m *stateMetrics) addSuccess() {
	m.fwlogSuccess.Add(1)
}

func (m *stateMetrics) addRetries(retryCount int) {
	m.fwlogRetries.Add(strconv.Itoa(retryCount), 1)
}
