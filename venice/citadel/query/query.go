package query

import (
	"context"
	"errors"
	"fmt"
	"strings"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/metrics_query"
	"github.com/pensando/sw/venice/citadel/broker"
	"github.com/pensando/sw/venice/globals"
	validators "github.com/pensando/sw/venice/utils/apigen/validators"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
)

// Server defines a metrics query server
type Server struct {
	grpcSrv *rpckit.RPCServer
	broker  *broker.Broker
}

// NewQueryService creates an RPC server to handle queries from APIGW/gRPC
func NewQueryService(listenURL string, br *broker.Broker) (*Server, error) {
	s, err := rpckit.NewRPCServer(globals.Citadel, listenURL, rpckit.WithLoggerEnabled(false))
	if err != nil {
		log.Errorf("failed to start query grpc server: %v", err)
		return nil, err
	}

	srv := &Server{
		grpcSrv: s,
		broker:  br,
	}

	metrics_query.RegisterMetricsV1Server(s.GrpcServer, srv)
	s.Start()

	log.Infof("citadel metrics query RPC server started at %s", listenURL)

	return srv, nil
}

// Stop stops the server
func (q *Server) Stop() {
	q.grpcSrv.Stop()
}

// validate is the function to validate query parameters
func (q *Server) validate(qs *metrics_query.QuerySpec) error {

	if qs == nil {
		return fmt.Errorf("query parameter required")
	}

	if qs.Kind == "" {
		return fmt.Errorf("kind required")
	}

	if qs.Tenant == "" {
		return fmt.Errorf("tenant required")
	}

	qs.Function = strings.ToUpper(qs.Function)

	if _, ok := metrics_query.TsdbFunctionType_value[qs.Function]; !ok {
		return fmt.Errorf("function %s did not match allowed strings", qs.Function)
	}

	for _, v := range qs.Fields {
		if !validators.RegExp(v, []string{"name"}) {
			return fmt.Errorf("failed to validate field %s", v)
		}
	}

	if qs.GroupbyField != "" {
		if !validators.RegExp(qs.GroupbyField, []string{"name"}) {
			return fmt.Errorf("failed to validate field %s", qs.GroupbyField)
		}
	}

	if qs.Selector != nil && len(qs.Selector.Requirements) > 0 {
		if _, err := qs.Selector.PrintSQL(); err != nil {
			return err
		}
	}

	if !validators.Duration(qs.GroupbyTime) {
		return fmt.Errorf("failed to validate group-by-time %s", qs.GroupbyTime)
	}

	return nil
}

// Query implements the metrics query method
func (q *Server) Query(c context.Context, qs *metrics_query.QuerySpec) (*metrics_query.QueryResponse, error) {

	if err := q.validate(qs); err != nil {
		return nil, err
	}

	qc, err := buildCitadelQuery(qs)
	if err != nil {
		return nil, err
	}

	if err := q.broker.ClusterCheck(); err != nil {
		return nil, err
	}

	log.Infof("citadel query: %v", qc)

	citadelResults, err := q.broker.ExecuteQuery(c, qs.Tenant, qc)
	if err != nil {
		return nil, err
	}

	queryResults := []*metrics_query.QueryResult{}

	for _, citadelResp := range citadelResults {
		result := &metrics_query.QueryResult{
			StatementID: int32(citadelResp.StatementID),
			Series:      []*metrics_query.ResultSeries{},
		}

		for _, s := range citadelResp.Series {
			rs := &metrics_query.ResultSeries{
				Name:    s.Name,
				Tags:    s.Tags,
				Columns: s.Columns,
				Values:  []*api.InterfaceSlice{},
			}

			for _, qrow := range s.Values {
				valueList := &api.InterfaceSlice{}
				for _, intf := range qrow {
					switch v := intf.(type) {
					case int:
						obj := &api.Interface_Int64{}
						obj.Int64 = int64(v)
						valueList.Values = append(valueList.Values, &api.Interface{Value: obj})

					case float64:
						obj := &api.Interface_Float{}
						obj.Float = v
						valueList.Values = append(valueList.Values, &api.Interface{Value: obj})

					case string:
						obj := &api.Interface_Str{}
						obj.Str = v
						valueList.Values = append(valueList.Values, &api.Interface{Value: obj})

					case bool:
						obj := &api.Interface_Bool{}
						obj.Bool = v
						valueList.Values = append(valueList.Values, &api.Interface{Value: obj})

					default:
						valueList.Values = append(valueList.Values, &api.Interface{Value: nil})

					}
				}
				rs.Values = append(rs.Values, valueList)
			}

			result.Series = append(result.Series, rs)
		}
		queryResults = append(queryResults, result)
	}
	return &metrics_query.QueryResponse{Results: queryResults}, nil
}

func buildCitadelQuery(qs *metrics_query.QuerySpec) (string, error) {
	measurement := qs.Kind
	fields := "*"
	var selectors []string

	if len(qs.Fields) > 0 {
		fields = strings.Join(qs.Fields, ",")
	}

	if qs.Function != "" {
		switch qs.Function {
		case metrics_query.TsdbFunctionType_MEAN.String():
			fields = fmt.Sprintf("%s(%s)", qs.Function, fields)
		case metrics_query.TsdbFunctionType_MAX.String():
			fields = fmt.Sprintf("%s(%s),*", qs.Function, fields)
		case metrics_query.TsdbFunctionType_NONE.String():
			//none
		}
	}

	q := fmt.Sprintf("SELECT %s FROM %s", fields, measurement)

	if qs.Selector != nil && len(qs.Selector.Requirements) > 0 {
		sel, err := qs.Selector.PrintSQL()
		if err != nil {
			return "", err
		}
		selectors = append(selectors, sel)
	}

	if qs.StartTime != nil {
		t, err := qs.StartTime.Time()
		if err != nil {
			return "", err
		}
		selectors = append(selectors, fmt.Sprintf("time > '%s'", t.Format(time.RFC3339)))
	}

	if qs.EndTime != nil {
		t, err := qs.EndTime.Time()
		if err != nil {
			return "", err
		}
		selectors = append(selectors, fmt.Sprintf("time < '%s'", t.Format(time.RFC3339)))
	}

	if len(selectors) > 0 {
		q += fmt.Sprintf(" WHERE %s", strings.Join(selectors, " AND "))
	}

	var groupby []string
	if qs.GroupbyTime != "" {
		groupby = append(groupby, fmt.Sprintf("time(%s)", qs.GroupbyTime))
	}

	if qs.GroupbyField != "" {
		groupby = append(groupby, fmt.Sprintf("%s", qs.GroupbyField))
	}

	if len(groupby) > 0 {
		q += fmt.Sprintf(" GROUP BY %s", strings.Join(groupby, ","))
	}

	return q, nil
}

// AutoWatchSvcMetricsV1 is not implemented
func (q *Server) AutoWatchSvcMetricsV1(lwo *api.ListWatchOptions, s metrics_query.MetricsV1_AutoWatchSvcMetricsV1Server) error {
	return errors.New("not supported")
}
