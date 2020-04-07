package policyhdr

import (
	"time"

	"github.com/gogo/protobuf/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/api/generated/telemetry_query"
)

// TODO:
//  1. check if we really need the default function or can directly work on the instantaneous values.

// StatsAlertPolicyToCitadelQuery converts the given stats alert policy to citadel metrics query.
func StatsAlertPolicyToCitadelQuery(sap monitoring.StatsAlertPolicy) telemetry_query.MetricsQueryList {
	measurementWindow, _ := time.ParseDuration(defaultMeasurementWindow)
	measurementFunction := telemetry_query.TsdbFunctionType_MEAN.String() // default function - mean; for now

	// e.g. now()-10m to now()
	if sap.Spec.MeasurementCriteria != nil {
		measurementWindow, _ = time.ParseDuration(sap.Spec.MeasurementCriteria.Window)
		measurementFunction = getAggFunction(monitoring.MeasurementFunction(monitoring.MeasurementFunction_vvalue[sap.Spec.MeasurementCriteria.Function]))
	}

	sTime, _ := types.TimestampProto(time.Now().Add(-1 * measurementWindow))
	eTime, _ := types.TimestampProto(time.Now())

	query := telemetry_query.MetricsQueryList{
		Tenant:    sap.Tenant,
		Namespace: sap.Namespace,
		Queries: []*telemetry_query.MetricsQuerySpec{
			{
				TypeMeta: api.TypeMeta{
					Kind: sap.Spec.Metric.Kind,
				},
				Fields:       []string{sap.Spec.Metric.FieldName},
				Function:     measurementFunction,
				StartTime:    &api.Timestamp{Timestamp: *sTime},
				EndTime:      &api.Timestamp{Timestamp: *eTime},
				GroupbyField: "reporterID",
				SortOrder:    "descending",
				Pagination: &telemetry_query.PaginationSpec{
					Count: 1,
				},
			},
		},
	}

	return query
}

func getAggFunction(mFunction monitoring.MeasurementFunction) string {
	switch mFunction {
	case monitoring.MeasurementFunction_MAX:
		return telemetry_query.TsdbFunctionType_MAX.String()
	case monitoring.MeasurementFunction_MEAN:
		return telemetry_query.TsdbFunctionType_MEAN.String()
	case monitoring.MeasurementFunction_DIFFERENCE:
		return telemetry_query.TsdbFunctionType_DIFFERENCE.String()
	case monitoring.MeasurementFunction_MIN:
		// TODO: telemetry is yet to support min function
		fallthrough
	default:
		return telemetry_query.TsdbFunctionType_NONE.String()

	}
}
