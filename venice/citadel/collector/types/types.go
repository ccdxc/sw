package types

import (
	"context"

	"github.com/influxdata/influxdb/models"
	"github.com/influxdata/influxdb/query"
)

// TSDBIntf contains methods required to implement a backend for the collector
type TSDBIntf interface {
	CreateDatabase(ctx context.Context, database string) error
	WritePoints(ctx context.Context, database string, points []models.Point) error
	ExecuteQuery(ctx context.Context, database string, qry string) ([]*query.Result, error)
	WriteLines(ctx context.Context, database string, lines []string) error
}
