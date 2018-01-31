package server

import (
	"github.com/influxdata/influxdb/coordinator"
	"github.com/influxdata/influxdb/query"
	"github.com/influxdata/influxdb/services/meta"

	"github.com/pensando/sw/venice/aggregator/gatherer"
	"github.com/pensando/sw/venice/aggregator/vstore"
)

type queryContext struct {
	store         *vstore.Store
	space         *gatherer.QuerySpace
	pointsWriter  *coordinator.PointsWriter
	queryExecutor *query.QueryExecutor
}

// newQueryContext returns an instance of queryContext
func newQueryContext(mc *meta.Client) *queryContext {
	qc := &queryContext{}

	qc.store = vstore.NewStore()
	qc.pointsWriter = coordinator.NewPointsWriter()
	qc.pointsWriter.TSDBStore = qc.store
	qc.pointsWriter.MetaClient = mc
	qc.space = gatherer.NewQuerySpace(qc.pointsWriter)
	qc.queryExecutor = query.NewQueryExecutor()
	qc.queryExecutor.StatementExecutor = &coordinator.StatementExecutor{
		MetaClient:  mc,
		TaskManager: qc.queryExecutor.TaskManager,
		TSDBStore:   qc.store,
		ShardMapper: &coordinator.LocalShardMapper{
			TSDBStore:  qc.store,
			MetaClient: mc,
		},
		PointsWriter: qc.pointsWriter,
	}

	return qc
}
