package indexer

import (
	"expvar"
	"strconv"
)

var metric = newMetrics()

type metrics struct {
	fwlogDrops   *expvar.Int
	fwlogRetries *expvar.Map
}

func newMetrics() *metrics {
	return &metrics{
		fwlogDrops:   expvar.NewInt("elasticFwlogDrops"),
		fwlogRetries: expvar.NewMap("elasticFwlogRetries"),
	}
}

func (m *metrics) addDrop() {
	m.fwlogDrops.Add(1)
}

func (m *metrics) addRetries(retryCount int) {
	m.fwlogRetries.Add(strconv.Itoa(retryCount), 1)
}
