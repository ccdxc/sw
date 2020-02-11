package query

import (
	"context"
	"fmt"
	"strings"
	"time"

	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"

	"github.com/pensando/sw/api/fields"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/errors"
	"github.com/pensando/sw/api/generated/telemetry_query"
	"github.com/pensando/sw/venice/utils/apigen/validators"
	"github.com/pensando/sw/venice/utils/log"
)

// validateMetricsQueryList validates a query list request
func (q *Server) validateMetricsQueryList(ql *telemetry_query.MetricsQueryList) error {
	errorStrings := []string{}
	if ql == nil {
		errorStrings = append(errorStrings, "query required")
	} else {
		if len(ql.Queries) == 0 {
			errorStrings = append(errorStrings, "query required")
		}

		if ql.Tenant == "" {
			errorStrings = append(errorStrings, "tenant required")
		}

		for _, qs := range ql.Queries {
			if qsStrings := q.validateMetricsQuerySpec(qs); len(qsStrings) != 0 {
				errorStrings = append(errorStrings, qsStrings...)
			}
		}
		errs := ql.Validate("v1", "", true, false)
		for _, e := range errs {
			errorStrings = append(errorStrings, e.Error())
		}
	}

	if len(errorStrings) != 0 {
		return apierrors.ToGrpcError("Validation Failed", errorStrings, int32(codes.InvalidArgument), "", nil)
	}
	return nil
}

// validateMetricsQuerySpec validates individual query parameters
func (q *Server) validateMetricsQuerySpec(qs *telemetry_query.MetricsQuerySpec) []string {
	errorStrings := []string{}
	if qs == nil {
		errorStrings = append(errorStrings, "query parameter required")
		return errorStrings
	}

	if qs.Kind == "" {
		errorStrings = append(errorStrings, "kind required")
	} else if validators.RegExp(qs.Kind, []string{"name"}) != nil {
		errorStrings = append(errorStrings, "invalid kind")
	}

	if qs.Function == "" {
		qs.Function = telemetry_query.TsdbFunctionType_NONE.String()
	}

	if qs.GroupbyTime != "" {
		dur, err := time.ParseDuration(qs.GroupbyTime)
		if err != nil {
			errorStrings = append(errorStrings, fmt.Sprintf("failed to parse groupby-time, %v", err))
		} else {
			if dur < time.Minute {
				errorStrings = append(errorStrings, fmt.Sprintf("too low groupby-time, %v", dur))
			}
		}
	}

	switch qs.Function {
	case telemetry_query.TsdbFunctionType_MAX.String(),
		telemetry_query.TsdbFunctionType_TOP.String(),
		telemetry_query.TsdbFunctionType_BOTTOM.String():

		// Can only specify one field
		if len(qs.Fields) != 1 {
			errorStrings = append(errorStrings, "Function "+qs.Function+" requires exactly one field")
		}
	case telemetry_query.TsdbFunctionType_MEAN.String():
		//none
	case telemetry_query.TsdbFunctionType_MEDIAN.String():
		//none
	case telemetry_query.TsdbFunctionType_DERIVATIVE.String():
		//none
	case telemetry_query.TsdbFunctionType_DIFFERENCE.String():
		//none
	case telemetry_query.TsdbFunctionType_NONE.String():
		//none
	case telemetry_query.TsdbFunctionType_LAST.String():
		//none
	}

	if qs.Selector != nil && len(qs.Selector.Requirements) > 0 {
		if _, err := qs.Selector.PrintSQL(); err != nil {
			errorStrings = append(errorStrings, fmt.Sprintf("Failed to parse selector requirements: %v", err))
		}
	}

	return errorStrings
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
		return nil, status.Errorf(codes.Internal, err.Error())
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
	selectedFields := []string{"*"}
	var selectors []string

	if len(qs.Fields) > 0 {
		selectedFields = qs.Fields
	}

	if qs.Function != "" {
		switch qs.Function {
		case telemetry_query.TsdbFunctionType_DERIVATIVE.String(),
			telemetry_query.TsdbFunctionType_DIFFERENCE.String():

			newFields := []string{}
			if len(qs.GroupbyTime) > 0 {
				// Must use advanced syntax when using group by time
				innerFunc := telemetry_query.TsdbFunctionType_MEAN.String()
				for _, field := range selectedFields {
					newFields = append(newFields, fmt.Sprintf("%s(%s(%s))", qs.Function, innerFunc, field))
				}
				selectedFields = newFields
				break
			}
			// If group by time is not present, we use normal syntax
			fallthrough

		case telemetry_query.TsdbFunctionType_MEAN.String(),
			telemetry_query.TsdbFunctionType_MAX.String(),
			telemetry_query.TsdbFunctionType_LAST.String(),
			telemetry_query.TsdbFunctionType_MEDIAN.String(),
			telemetry_query.TsdbFunctionType_DIFFERENCE.String():
			newFields := []string{}

			for _, field := range selectedFields {
				newFields = append(newFields, fmt.Sprintf("%s(%s)", qs.Function, field))
			}
			if qs.Function == telemetry_query.TsdbFunctionType_MAX.String() {
				// We add * to select the other fields to give the max query context
				newFields = append(newFields, "*")
			}
			selectedFields = newFields
		case telemetry_query.TsdbFunctionType_TOP.String(),
			telemetry_query.TsdbFunctionType_BOTTOM.String():
			selectedFields = []string{fmt.Sprintf("%s(%s,reporterID,10)", qs.Function, selectedFields[0])}

		case telemetry_query.TsdbFunctionType_NONE.String():
			//none
		}
	}

	q := fmt.Sprintf("SELECT %s FROM %s", strings.Join(selectedFields, ","), measurement)

	if qs.Name != "" {
		req := &fields.Requirement{
			Key:      "Name",
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
		// Need to escape with quotes incase value is an influx keyword (ex. name)
		groupby = append(groupby, fmt.Sprintf("\"%s\"", qs.GroupbyField))
	}

	if len(groupby) > 0 {
		q += fmt.Sprintf(" GROUP BY %s", strings.Join(groupby, ","))
	}

	order := "ASC"
	if qs.SortOrder == telemetry_query.SortOrder_Descending.String() {
		order = "DESC"
	}
	q += fmt.Sprintf(" ORDER BY time %s", order)

	if qs.Pagination != nil {
		q += fmt.Sprintf(" LIMIT %d", qs.Pagination.Count)
		// Count must be used with offset
		if qs.Pagination.Offset != 0 {
			q += fmt.Sprintf(" OFFSET %d", qs.Pagination.Offset)
		}
	}

	return q, nil
}

// AutoWatchSvcTelemetryV1 is not implemented
func (q *Server) AutoWatchSvcTelemetryV1(lwo *api.ListWatchOptions, s telemetry_query.TelemetryV1_AutoWatchSvcTelemetryV1Server) error {
	return status.Errorf(codes.Unimplemented, "not supported")
}
