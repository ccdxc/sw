package state

import (
	"expvar"
	"strconv"
)

var metric = newMetrics()

type stateMetrics struct {
	fwlogDrops        *expvar.Int
	fwlogSuccess      *expvar.Int
	fwlogRetries      *expvar.Map
	fwlogPendingItems *expvar.Int
}

func newMetrics() *stateMetrics {
	return &stateMetrics{
		fwlogDrops:        expvar.NewInt("objstoreFwlogDrops"),
		fwlogSuccess:      expvar.NewInt("objstoreFwlogSuccess"),
		fwlogRetries:      expvar.NewMap("objstoreFwlogRetries"),
		fwlogPendingItems: expvar.NewInt("objstoreFwlogPendingItems"),
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

func (m *stateMetrics) addPendingItem() {
	m.fwlogPendingItems.Add(1)
}

func (m *stateMetrics) subtractPendingItem() {
	m.fwlogPendingItems.Add(-1)
}
