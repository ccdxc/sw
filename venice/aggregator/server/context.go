package server

import (
	"github.com/influxdata/influxdb/coordinator"
	"github.com/influxdata/influxdb/query"
	"github.com/influxdata/influxdb/services/meta"

	"github.com/pensando/sw/venice/aggregator/gatherer"
	"github.com/pensando/sw/venice/aggregator/vstore"
)

// QueryContext holds information required to service a query
type QueryContext struct {
	Store         *vstore.Store
	Space         *gatherer.QuerySpace
	PointsWriter  *coordinator.PointsWriter
	QueryExecutor *query.QueryExecutor
}

// NewQueryContext returns an instance of QueryContext
func NewQueryContext(mc *meta.Client) *QueryContext {
	qc := &QueryContext{}

	qc.Store = vstore.NewStore()
	qc.PointsWriter = coordinator.NewPointsWriter()
	qc.PointsWriter.TSDBStore = qc.Store
	qc.PointsWriter.MetaClient = mc
	qc.Space = gatherer.NewQuerySpace(qc.PointsWriter)
	qc.QueryExecutor = query.NewQueryExecutor()
	qc.QueryExecutor.StatementExecutor = &coordinator.StatementExecutor{
		MetaClient:  mc,
		TaskManager: qc.QueryExecutor.TaskManager,
		TSDBStore:   qc.Store,
		ShardMapper: &coordinator.LocalShardMapper{
			TSDBStore:  qc.Store,
			MetaClient: mc,
		},
		PointsWriter: qc.PointsWriter,
	}

	return qc
}
