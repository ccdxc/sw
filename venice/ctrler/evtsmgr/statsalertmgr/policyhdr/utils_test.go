package policyhdr

import (
	"reflect"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/api/generated/telemetry_query"
	"github.com/pensando/sw/venice/globals"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/testutils/policygen"
)

func TestStatsAlertPolicyToCitadelQuery(t *testing.T) {
	tests := []struct {
		name  string
		sap   monitoring.StatsAlertPolicy
		query telemetry_query.MetricsQueryList
	}{
		{
			name: "valid stats alert policy",
			sap: *policygen.CreateStatsAlertPolicyObj(globals.DefaultTenant, globals.DefaultNamespace,
				CreateAlphabetString(5),
				monitoring.MetricIdentifier{Kind: "Node", FieldName: "CPUUsedPercent"},
				&monitoring.MeasurementCriteria{Window: "10s", Function: monitoring.MeasurementFunction_MAX.String()},
				monitoring.Thresholds{Operator: monitoring.Operator_GREATER_OR_EQUAL_THAN.String(), Values: []monitoring.Threshold{
					{
						Severity:   monitoring.AlertSeverity_CRITICAL.String(),
						RaiseValue: "95",
					},
					{
						Severity:   monitoring.AlertSeverity_WARN.String(),
						RaiseValue: "85",
					},
				}}, []string{}),
			query: telemetry_query.MetricsQueryList{
				Tenant:    globals.DefaultTenant,
				Namespace: globals.DefaultNamespace,
				Queries: []*telemetry_query.MetricsQuerySpec{
					{
						TypeMeta: api.TypeMeta{
							Kind: "Node",
						},
						Fields:       []string{"CPUUsedPercent"},
						Function:     telemetry_query.TsdbFunctionType_MAX.String(),
						GroupbyField: "reporterID",
						SortOrder:    "descending",
						Pagination: &telemetry_query.PaginationSpec{
							Count: 1,
						},
					},
				},
			},
		},
		{
			name: "measurement function - difference",
			sap: *policygen.CreateStatsAlertPolicyObj(globals.DefaultTenant, globals.DefaultNamespace,
				CreateAlphabetString(5),
				monitoring.MetricIdentifier{Kind: "Node", FieldName: "MemUsedPercent"},
				&monitoring.MeasurementCriteria{Window: "20s", Function: monitoring.MeasurementFunction_DIFFERENCE.String()},
				monitoring.Thresholds{Operator: monitoring.Operator_GREATER_OR_EQUAL_THAN.String(), Values: []monitoring.Threshold{
					{
						Severity:   monitoring.AlertSeverity_CRITICAL.String(),
						RaiseValue: "95",
					},
					{
						Severity:   monitoring.AlertSeverity_WARN.String(),
						RaiseValue: "85",
					},
				}}, []string{}),
			query: telemetry_query.MetricsQueryList{
				Tenant:    globals.DefaultTenant,
				Namespace: globals.DefaultNamespace,
				Queries: []*telemetry_query.MetricsQuerySpec{
					{
						TypeMeta: api.TypeMeta{
							Kind: "Node",
						},
						Fields:       []string{"MemUsedPercent"},
						Function:     telemetry_query.TsdbFunctionType_DIFFERENCE.String(),
						GroupbyField: "reporterID",
						SortOrder:    "descending",
						Pagination: &telemetry_query.PaginationSpec{
							Count: 1,
						},
					},
				},
			},
		},
		{
			name: "measurement function - min",
			sap: *policygen.CreateStatsAlertPolicyObj(globals.DefaultTenant, globals.DefaultNamespace,
				CreateAlphabetString(5),
				monitoring.MetricIdentifier{Kind: "Node", FieldName: "MemUsedPercent"},
				&monitoring.MeasurementCriteria{Window: "20s", Function: monitoring.MeasurementFunction_MIN.String()},
				monitoring.Thresholds{Operator: monitoring.Operator_GREATER_OR_EQUAL_THAN.String(), Values: []monitoring.Threshold{
					{
						Severity:   monitoring.AlertSeverity_CRITICAL.String(),
						RaiseValue: "95",
					},
					{
						Severity:   monitoring.AlertSeverity_WARN.String(),
						RaiseValue: "85",
					},
				}}, []string{}),
			query: telemetry_query.MetricsQueryList{
				Tenant:    globals.DefaultTenant,
				Namespace: globals.DefaultNamespace,
				Queries: []*telemetry_query.MetricsQuerySpec{
					{
						TypeMeta: api.TypeMeta{
							Kind: "Node",
						},
						Fields:       []string{"MemUsedPercent"},
						Function:     telemetry_query.TsdbFunctionType_NONE.String(),
						GroupbyField: "reporterID",
						SortOrder:    "descending",
						Pagination: &telemetry_query.PaginationSpec{
							Count: 1,
						},
					},
				},
			},
		},
		{
			name: "nil measurement criteria",
			sap: *policygen.CreateStatsAlertPolicyObj(globals.DefaultTenant, globals.DefaultNamespace,
				CreateAlphabetString(5),
				monitoring.MetricIdentifier{Kind: "Node", FieldName: "CPUUsedPercent"},
				nil,
				monitoring.Thresholds{Operator: monitoring.Operator_GREATER_OR_EQUAL_THAN.String(), Values: []monitoring.Threshold{
					{
						Severity:   monitoring.AlertSeverity_CRITICAL.String(),
						RaiseValue: "95",
					},
					{
						Severity:   monitoring.AlertSeverity_WARN.String(),
						RaiseValue: "85",
					},
				}}, []string{}),
			query: telemetry_query.MetricsQueryList{
				Tenant:    globals.DefaultTenant,
				Namespace: globals.DefaultNamespace,
				Queries: []*telemetry_query.MetricsQuerySpec{
					{
						TypeMeta: api.TypeMeta{
							Kind: "Node",
						},
						Fields:       []string{"CPUUsedPercent"},
						Function:     telemetry_query.TsdbFunctionType_MEAN.String(),
						GroupbyField: "reporterID",
						SortOrder:    "descending",
						Pagination: &telemetry_query.PaginationSpec{
							Count: 1,
						},
					},
				},
			},
		},
	}

	for i := range tests {
		query := StatsAlertPolicyToCitadelQuery(tests[i].sap)
		query.Queries[0].StartTime = nil
		query.Queries[0].EndTime = nil
		Assert(t, reflect.DeepEqual(tests[i].query, query), "{%s} expected: %v, got: %v", tests[i].name, tests[i].query, query)
	}
}
