package rpcserver

import (
	"fmt"

	"golang.org/x/net/context"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/telemetry_query"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"

	"github.com/influxdata/influxdb/services/meta"
)

const (
	// TODO have controller populate this
	objMetricsDB = "NaplesStats"
)

// AggRPCSrv defines an aggregator RPC server
type AggRPCSrv struct {
	grpcSrv    *rpckit.RPCServer
	metaClient *meta.Client
}

// NewAggRPCSrv creates and starts an aggregator RPC server
func NewAggRPCSrv(listenURL string, mc *meta.Client) (*AggRPCSrv, error) {
	s, err := rpckit.NewRPCServer(globals.Aggregator, listenURL, rpckit.WithLoggerEnabled(false))
	if err != nil {
		log.Infof("failed to start grpc server: %v", err)
		return nil, err
	}
	srv := &AggRPCSrv{
		metaClient: mc,
		grpcSrv:    s,
	}
	telemetry_query.RegisterTelemetryV1Server(s.GrpcServer, srv)
	s.Start()
	log.Infof("telemetry_query Aggregator RPC server started at %s", listenURL)
	return srv, nil
}

// ErrOut returns a channel that gives an error indication
func (a *AggRPCSrv) ErrOut() <-chan error {
	return a.grpcSrv.DoneCh
}

// URL gets the listen url of the server
func (a *AggRPCSrv) URL() string {
	return a.grpcSrv.GetListenURL()
}

// Stop stops the server
func (a *AggRPCSrv) Stop() {
	a.grpcSrv.Stop()
	<-a.grpcSrv.DoneCh
}

// Metrics implements the grpc method
func (a *AggRPCSrv) Metrics(c context.Context, qs *telemetry_query.MetricsQueryList) (*telemetry_query.MetricsQueryResponse, error) {
	/*
		// based on the query spec, build an influx query.
		iQuery, err := InfluxQuery(qs)
		if err != nil {
			return nil, err
		}

		for _, qStr := range iQuery {
			log.Infof("Influx Query: %s", qStr)
		}
		p := influxql.NewParser(strings.NewReader(iQuery[0]))
		// Parse query from query string.
		q, err := p.ParseQuery()
		if err != nil {
			return nil, errors.Wrap(err, "Parsing query")
		}

		// Query all shards and write to our cache before executing the query
		qc := server.NewQueryContext(a.metaClient)
		err = qc.Space.GatherPoints(q.Statements[0], objMetricsDB)
		if err != nil {
			return nil, err
		}

		// Execute query.
		opts := query.ExecutionOptions{
			Database: objMetricsDB,
			ReadOnly: true,
		}
		closing := make(chan struct{})
		results := qc.QueryExecutor.ExecuteQuery(q, opts, closing)
		var qr telemetry_query.QueryResponse
		for r := range results {
			for _, row := range r.Series {
				series := &telemetry_query.ResultSeries{
					Columns: row.Columns,
					Rows:    getSeriesRows(row.Values),
				}
				qr.Series = append(qr.Series, series)

			}
		}
		//TODO implement math functions
	*/
	return nil, nil

}

// AutoWatchSvcTelemetryV1 is not implemented
func (a *AggRPCSrv) AutoWatchSvcTelemetryV1(lwo *api.ListWatchOptions, s telemetry_query.TelemetryV1_AutoWatchSvcTelemetryV1Server) error {
	return fmt.Errorf("Not implemented")
}

/*
func getSeriesRows(values [][]interface{}) []*telemetry_query.Row {
	var out []*telemetry_query.Row

	//for _, v := range values {
	//	qrRow := telemetry_query.Row{}
	//	for _, vv := range v {
	//		qrRow.Values = append(qrRow.Values, fmt.Sprintf("%v", vv))
	//	}
	//
	//		out = append(out, &qrRow)
	//	}

	return out
}
*/
