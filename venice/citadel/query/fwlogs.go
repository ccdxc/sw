package query

import (
	"context"
	"fmt"
	"reflect"
	"strconv"
	"strings"
	"time"

	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/telemetry_query"
	"github.com/pensando/sw/venice/utils/log"
)

// Tag names are set in
// nic/agent/tmagent/state/state.go
// Mapping from FwlogQuery fields to tag names in tsdb
var queryFieldToTsdbField = map[string]string{
	"SourceIPs":   "src",
	"DestIPs":     "dest",
	"SourcePorts": "src-port",
	"DestPorts":   "dest-port",
	"Protocols":   "protocol",
	"Actions":     "action",
	"Directions":  "direction",
	"RuleIDs":     "rule-id",
}

// Maps query enum values to values in tsdb
var actionEnumMapping = map[string]string{
	"ACTION_ALLOW":  "SECURITY_RULE_ACTION_ALLOW",
	"ACTION_DENY":   "SECURITY_RULE_ACTION_DENY",
	"ACTION_REJECT": "SECURITY_RULE_ACTION_REJECT",
}

// Maps tsdb enum value to fwlog values
var actionTsdbEnumMapping = map[string]string{
	"SECURITY_RULE_ACTION_ALLOW":  "ALLOW",
	"SECURITY_RULE_ACTION_DENY":   "DENY",
	"SECURITY_RULE_ACTION_REJECT": "REJECT",
}

// Maps query enum values to values in tsdb
var directionEnumMapping = map[string]string{
	"DIRECTION_FROM_HOST":   "FLOW_DIRECTION_FROM_HOST",
	"DIRECTION_FROM_UPLINK": "FLOW_DIRECTION_FROM_UPLINK",
}

// Maps tsdb enum value to fwlog values
var directionTsdbEnumMapping = map[string]string{
	"FLOW_DIRECTION_FROM_HOST":   "FROM_HOST",
	"FLOW_DIRECTION_FROM_UPLINK": "FROM_UPLINK",
}

// Measurement name in tsdb
const measurement string = "Fwlogs"

// validateFwlogsQueryList validates a query list request
func (q *Server) validateFwlogsQueryList(ql *telemetry_query.FwlogsQueryList) error {
	var errs []string
	if ql == nil {
		ql = &telemetry_query.FwlogsQueryList{}
	}
	if ql.Tenant == "" {
		errs = append(errs, "tenant required")
	}
	if len(ql.Queries) == 0 {
		errs = append(errs, "query required")
	}

	// TODO: remove hardcoded versioning
	errors := ql.Validate("v1", "", true)
	for _, e := range errors {
		errs = append(errs, e.Error())
	}

	if len(errs) != 0 {
		return status.Errorf(codes.InvalidArgument, "[%v]", strings.Join(errs, ", "))
	}
	return nil
}

func generateClause(fieldName string, values []interface{}) string {
	clause := []string{}
	if len(values) == 0 {
		return ""
	}
	for _, v := range values {
		clause = append(clause, fmt.Sprintf(`"%s" = '%v'`, fieldName, v))
	}
	return fmt.Sprintf("(%s)", strings.Join(clause, " OR "))
}

func buildCitadelFwlogsQuery(qs *telemetry_query.FwlogsQuerySpec) (string, error) {
	fields := []string{"*"}
	var selectors []string

	// Convert enum values from venice enum to naples enum
	if len(qs.Actions) > 0 {
		for i, action := range qs.Actions {
			actionVal, ok := actionEnumMapping[action]
			if !ok {
				// Action is ALL, or unrecognized
				qs.Actions = []string{}
				break
			}
			qs.Actions[i] = actionVal
		}
	}

	if len(qs.Directions) > 0 {
		for i, dir := range qs.Directions {
			dirVal, ok := directionEnumMapping[dir]
			if !ok {
				// Direction is ALL, or unrecognized
				qs.Directions = []string{}
				break
			}
			qs.Directions[i] = dirVal
		}
	}

	q := fmt.Sprintf("SELECT %s FROM %s", strings.Join(fields, ","), measurement)

	qFields := reflect.TypeOf(*qs)
	qValues := reflect.ValueOf(*qs)

	// For each field of fwlogQuerySpec, we check
	// if we have a corresponding tsdb tag
	// If so, we generate its clause and add it if it isn't empty
	for i := 0; i < qFields.NumField(); i++ {
		qFieldName := qFields.Field(i).Name
		fieldName, ok := queryFieldToTsdbField[qFieldName]
		if !ok {
			// don't have a tsdb tag
			continue
		}
		valInf := qValues.Field(i)
		val := make([]interface{}, valInf.Len())

		for i := 0; i < valInf.Len(); i++ {
			val[i] = valInf.Index(i).Interface()
		}
		if clause := generateClause(fieldName, val); len(clause) > 0 {
			selectors = append(selectors, clause)
		}
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

	if qs.Pagination != nil {
		q += fmt.Sprintf(" LIMIT %d", qs.Pagination.Count)
		// Count must be used with offset
		if qs.Pagination.Offset != 0 {
			q += fmt.Sprintf(" OFFSET %d", qs.Pagination.Offset)
		}
	}

	return q, nil
}

func (q *Server) executeFwlogsQuery(c context.Context, tenant string, qs string) ([]*telemetry_query.FwlogsQueryResult, error) {
	citadelResults, err := q.broker.ExecuteQuery(c, tenant, qs)
	if err != nil {
		return nil, err
	}
	queryResults := []*telemetry_query.FwlogsQueryResult{}

	for _, citadelResp := range citadelResults {
		result := &telemetry_query.FwlogsQueryResult{
			StatementID: int32(citadelResp.StatementID),
			Logs:        []*telemetry_query.Fwlog{},
		}

		for _, s := range citadelResp.Series {
			// Creating map from db column name to index
			// For every column name, we see if we have a
			// corresponding fwlog field. If we do, we add it's
			// index to the map
			colMapping := map[string]int{}
			for i, k := range s.Columns {
				colMapping[k] = i
			}

			for _, qrow := range s.Values {
				fwlog := &telemetry_query.Fwlog{}

				qFields := reflect.TypeOf(*fwlog)
				qValues := reflect.ValueOf(fwlog).Elem()

				// We iterate over all fields of fwlog
				// For each field, we see if we have an
				// entry for it in colMapping
				for i := 0; i < qFields.NumField(); i++ {
					qField := qFields.Field(i)
					qFieldName := qField.Name
					tag := qField.Tag.Get("json")
					if len(tag) == 0 {
						log.Errorf("Expected fwlog field to have json tag, field: %s", qFieldName)
						continue
					}
					// Split guarantees returning an array
					// of at least length 1 if tag isn't empty
					tag = strings.Split(tag, ",")[0]
					index, ok := colMapping[tag]
					if !ok {
						// Returned data doesn't have a column for this field
						continue
					}
					f := qValues.Field(i)
					// All fields we use in tsdb are tags, so they
					// are always strings
					valStr, ok := qrow[index].(string)
					if !ok {
						log.Errorf("Expected returned value to be of type string, val: %v", qrow[index])
						valStr = fmt.Sprintf("%v", qrow[index])
					}
					// We set the fwlog obj's field to the value
					if qFieldName == "SrcPort" || qFieldName == "DestPort" {
						val, err := strconv.ParseUint(valStr, 10, 32)
						if err != nil {
							log.Errorf("Failed to parse %s value into uint32, val: %s, err: %v", qFieldName, valStr, err)
							continue
						}
						f.SetUint(val)
					} else if qFieldName == "Action" {
						if val, ok := actionTsdbEnumMapping[valStr]; ok {
							f.SetString(val)
							continue
						}
						f.SetString(valStr)
					} else if qFieldName == "Direction" {
						if val, ok := directionTsdbEnumMapping[valStr]; ok {
							f.SetString(val)
							continue
						}
						f.SetString(valStr)
					} else {
						f.SetString(valStr)
					}
				}
				// Setting time, time is always the first column from influx
				timestamp := &api.Timestamp{}
				timestamp.Parse(qrow[0].(string))
				fwlog.Timestamp = timestamp
				result.Logs = append(result.Logs, fwlog)
			}
		}
		queryResults = append(queryResults, result)
	}
	return queryResults, nil
}

// Fwlogs implements the metrics query method
func (q *Server) Fwlogs(c context.Context, ql *telemetry_query.FwlogsQueryList) (*telemetry_query.FwlogsQueryResponse, error) {
	if err := q.validateFwlogsQueryList(ql); err != nil {
		return nil, err
	}

	queries := []string{}
	for _, qs := range ql.Queries {
		qc, err := buildCitadelFwlogsQuery(qs)
		if err != nil {
			return nil, err
		}
		queries = append(queries, qc)
	}

	if err := q.broker.ClusterCheck(); err != nil {
		return nil, status.Errorf(codes.Internal, err.Error())
	}
	queryString := strings.Join(queries, "; ")

	log.Infof("citadel fwlogs query: %v", queryString)
	queryRes, err := q.executeFwlogsQuery(c, ql.Tenant, queryString)
	if err != nil {
		return nil, err
	}

	return &telemetry_query.FwlogsQueryResponse{
		Results:   queryRes,
		Tenant:    ql.Tenant,
		Namespace: ql.Namespace,
	}, nil
}
