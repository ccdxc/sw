package collector

import (
	"context"

	"github.com/influxdata/influxdb/models"

	"github.com/pensando/sw/venice/citadel/collector/types"
)

// Collector collects points in batches and sends to backends
type Collector struct {
	tsdbClient types.TSDBIntf // TSDB interface, citadel or influx
}

// NewCollector returns a new instance of Collector
func NewCollector(br types.TSDBIntf) *Collector {
	return &Collector{
		tsdbClient: br,
	}
}

// WritePoints writes points to TSDB
func (c *Collector) WritePoints(ctx context.Context, db, table string, points []models.Point) error {
	//TODO: tenant & namespace
	return c.tsdbClient.WritePoints(ctx, db, points)
}

// WriteLines writes lines to TSDB
func (c *Collector) WriteLines(ctx context.Context, db string, lines []string) error {
	return c.tsdbClient.WriteLines(ctx, db, lines)
}

// CreateDatabase creates a database in TSDB
func (c *Collector) CreateDatabase(ctx context.Context, dbName string) error {
	return c.tsdbClient.CreateDatabase(ctx, dbName)
}
