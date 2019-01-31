package query

import (
	"context"
	"fmt"
	"strings"
	"time"

	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"

	"github.com/pensando/sw/api/labels"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/telemetry_query"
	validators "github.com/pensando/sw/venice/utils/apigen/validators"
	"github.com/pensando/sw/venice/utils/log"
)

// validateMetricsQueryList validates a query list request
func (q *Server) validateMetricsQueryList(ql *telemetry_query.MetricsQueryList) error {
	if ql == nil || len(ql.Queries) == 0 {
		return status.Errorf(codes.InvalidArgument, "query required")
	}

	if ql.Tenant == "" {
		return status.Errorf(codes.InvalidArgument, "tenant required")
	}

	for _, qs := range ql.Queries {
		if err := q.validateMetricsQuerySpec(qs); err != nil {
			return err
		}
	}
	return nil
}

// validateMetricsQuerySpec validates individual query parameters
func (q *Server) validateMetricsQuerySpec(qs *telemetry_query.MetricsQuerySpec) error {

	if qs == nil {
		return status.Errorf(codes.InvalidArgument, "query parameter required")
	}

	if qs.Kind == "" {
		return status.Errorf(codes.InvalidArgument, "kind required")
	}

	if !validators.RegExp(qs.Kind, []string{"name"}) {
		return status.Errorf(codes.InvalidArgument, "invalid kind")
	}

	qs.Function = strings.ToUpper(qs.Function)

	if _, ok := telemetry_query.TsdbFunctionType_value[qs.Function]; !ok {
		return status.Errorf(codes.InvalidArgument, "function %s is not an accepted function", qs.Function)
	}

	if qs.Function != "" {
		switch qs.Function {
		case telemetry_query.TsdbFunctionType_MAX.String():
			// Can only specify one field when using MAX
			if len(qs.Fields) != 1 {
				return status.Errorf(codes.InvalidArgument, "Function MAX requires exactly one field")
			}
		case telemetry_query.TsdbFunctionType_MEAN.String():
			//none
		case telemetry_query.TsdbFunctionType_NONE.String():
			//none
		}
	}

	for _, v := range qs.Fields {
		if !validators.RegExp(v, []string{"name"}) {
			return status.Errorf(codes.InvalidArgument, "field %s was not a valid field", v)
		}
	}

	if qs.GroupbyField != "" {
		if !validators.RegExp(qs.GroupbyField, []string{"name"}) {
			return status.Errorf(codes.InvalidArgument, "group-by-field %s was not a valid field", qs.GroupbyField)
		}
	}

	if qs.Name != "" && !validators.RegExp(qs.Name, []string{"name"}) {
		return status.Errorf(codes.InvalidArgument, "Name selector %s was invalid", qs.Name)
	}

	if qs.Selector != nil && len(qs.Selector.Requirements) > 0 {
		if _, err := qs.Selector.PrintSQL(); err != nil {
			return status.Errorf(codes.InvalidArgument, "Failed to parse selector requirements: %v", err)
		}
	}

	if qs.GroupbyTime != "" && !validators.Duration(qs.GroupbyTime, []string{"0", "0"}) {
		return status.Errorf(codes.InvalidArgument, "group-by-time value %s was not a valid duration", qs.GroupbyTime)
	}

	return nil
}

func (q *Server) executeMetricsQuery(c context.Context, tenant string, qs string) ([]*telemetry_query.MetricsQueryResult, error) {
	citadelResults, err := q.broker.ExecuteQuery(c, tenant, qs)
	if err != nil {
		return nil, err
	}
	queryResults := []*telemetry_query.MetricsQueryResult{}

	for _, citadelResp := range citadelResults {
		result := &telemetry_query.MetricsQueryResult{
			StatementID: int32(citadelResp.StatementID),
			Series:      []*telemetry_query.ResultSeries{},
		}

		for _, s := range citadelResp.Series {
			rs := &telemetry_query.ResultSeries{
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
	return queryResults, nil
}

// Metrics implements the metrics query method
func (q *Server) Metrics(c context.Context, ql *telemetry_query.MetricsQueryList) (*telemetry_query.MetricsQueryResponse, error) {
	if err := q.validateMetricsQueryList(ql); err != nil {
		return nil, err
	}

	queries := []string{}
	for _, qs := range ql.Queries {
		qc, err := buildCitadelMetricsQuery(qs)
		if err != nil {
			return nil, err
		}
		queries = append(queries, qc)
	}

	if err := q.broker.ClusterCheck(); err != nil {
		return nil, err
	}
	queryString := strings.Join(queries, "; ")

	log.Infof("citadel metrics query: %v", queryString)
	queryRes, err := q.executeMetricsQuery(c, ql.Tenant, queryString)
	if err != nil {
		return nil, err
	}

	return &telemetry_query.MetricsQueryResponse{
		Results:   queryRes,
		Tenant:    ql.Tenant,
		Namespace: ql.Namespace,
	}, nil
}

func buildCitadelMetricsQuery(qs *telemetry_query.MetricsQuerySpec) (string, error) {
	measurement := qs.Kind
	fields := []string{"*"}
	var selectors []string

	if len(qs.Fields) > 0 {
		fields = qs.Fields
	}

	if qs.Function != "" {
		switch qs.Function {
		case telemetry_query.TsdbFunctionType_MEAN.String(), telemetry_query.TsdbFunctionType_MAX.String():
			newFields := []string{}
			for _, field := range fields {
				newFields = append(newFields, fmt.Sprintf("%s(%s)", qs.Function, field))
			}
			if qs.Function == telemetry_query.TsdbFunctionType_MAX.String() {
				// We add * to select the other fields to give the max query context
				newFields = append(newFields, "*")
			}
			fields = newFields
		case telemetry_query.TsdbFunctionType_NONE.String():
			//none
		}
	}

	q := fmt.Sprintf("SELECT %s FROM %s", strings.Join(fields, ","), measurement)

	if qs.Name != "" {
		req := &labels.Requirement{
			Key:      "meta.name",
			Operator: "equals",
			Values:   []string{qs.Name},
		}
		sel, err := req.PrintSQL()
		if err != nil {
			return "", err
		}
		selectors = append(selectors, sel)
	}

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

// AutoWatchSvcTelemetryV1 is not implemented
func (q *Server) AutoWatchSvcTelemetryV1(lwo *api.ListWatchOptions, s telemetry_query.TelemetryV1_AutoWatchSvcTelemetryV1Server) error {
	return status.Errorf(codes.Unimplemented, "not supported")
}
