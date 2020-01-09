package query

import (
	"context"
	"errors"
	"fmt"
	"reflect"
	"strings"
	"testing"

	"github.com/influxdata/influxdb/models"

	"github.com/golang/mock/gomock"
	"github.com/influxdata/influxdb/query"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/errors"
	"github.com/pensando/sw/api/fields"
	"github.com/pensando/sw/api/generated/telemetry_query"
	"github.com/pensando/sw/venice/citadel/broker/mock"
	. "github.com/pensando/sw/venice/utils/testutils"
)

type ExecuteQueryResponse struct {
	qr  []*query.Result
	err error
}

type ClusterCheckResponse struct {
	err error
}

var testServerURL = "localhost:0"

func TestBuildMetricsCitadelQuery(t *testing.T) {
	startTime := &api.Timestamp{}
	startTime.Parse("2018-11-09T23:16:17Z")
	endTime := &api.Timestamp{}
	endTime.Parse("2018-11-09T23:22:17Z")
	testQs := []struct {
		qs   *telemetry_query.MetricsQuerySpec
		resp string
		pass bool
		desc string
	}{
		{
			desc: "Selecting kind",
			qs: &telemetry_query.MetricsQuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "test-db",
				},
			},
			resp: "SELECT * FROM test-db ORDER BY time ASC",
			pass: true,
		},
		{
			desc: "Selecting measurement",
			qs: &telemetry_query.MetricsQuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "test-db",
				},
				Fields: []string{"cpu"},
			},
			resp: "SELECT cpu FROM test-db ORDER BY time ASC",
			pass: true,
		},
		{
			desc: "Using MEAN function",
			qs: &telemetry_query.MetricsQuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "test-db",
				},
				Function: telemetry_query.TsdbFunctionType_MEAN.String(),
			},
			resp: "SELECT mean(*) FROM test-db ORDER BY time ASC",
			pass: true,
		},
		{
			desc: "Using MEAN function with a measurement",
			qs: &telemetry_query.MetricsQuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "test-db",
				},
				Fields:   []string{"cpu"},
				Function: telemetry_query.TsdbFunctionType_MEAN.String(),
			},
			resp: "SELECT mean(cpu) FROM test-db ORDER BY time ASC",
			pass: true,
		},
		{
			desc: "Using MEAN function with multiple measurement",
			qs: &telemetry_query.MetricsQuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "test-db",
				},
				Fields:   []string{"cpu", "memory"},
				Function: telemetry_query.TsdbFunctionType_MEAN.String(),
			},
			resp: "SELECT mean(cpu),mean(memory) FROM test-db ORDER BY time ASC",
			pass: true,
		},
		{
			desc: "Using LAST function",
			qs: &telemetry_query.MetricsQuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "test-db",
				},
				Function: telemetry_query.TsdbFunctionType_LAST.String(),
			},
			resp: "SELECT last(*) FROM test-db ORDER BY time ASC",
			pass: true,
		},
		{
			desc: "Using LAST function with groupby",
			qs: &telemetry_query.MetricsQuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "test-db",
				},
				Function:    telemetry_query.TsdbFunctionType_LAST.String(),
				GroupbyTime: "3m",
			},
			resp: "SELECT last(*) FROM test-db GROUP BY time(3m) ORDER BY time ASC",
			pass: true,
		},

		{
			desc: "Using MEDIAN function",
			qs: &telemetry_query.MetricsQuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "test-db",
				},
				Function: "median",
			},
			resp: "SELECT median(*) FROM test-db ORDER BY time ASC",
			pass: true,
		},
		{
			desc: "Using DERIVATIVE function",
			qs: &telemetry_query.MetricsQuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "test-db",
				},
				Function: "derivative",
			},
			resp: "SELECT derivative(*) FROM test-db ORDER BY time ASC",
			pass: true,
		},
		{
			desc: "Using DERIVATIVE function with group by time",
			qs: &telemetry_query.MetricsQuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "test-db",
				},
				GroupbyTime: "5m",
				Function:    "derivative",
			},
			resp: "SELECT derivative(mean(*)) FROM test-db GROUP BY time(5m) ORDER BY time ASC",
			pass: true,
		},
		{
			desc: "Using DIFFERENCE function",
			qs: &telemetry_query.MetricsQuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "test-db",
				},
				Function: "difference",
			},
			resp: "SELECT difference(*) FROM test-db ORDER BY time ASC",
			pass: true,
		},
		{
			desc: "Using DIFFERENCE function with group by time",
			qs: &telemetry_query.MetricsQuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "test-db",
				},
				GroupbyTime: "5m",
				Function:    "difference",
			},
			resp: "SELECT difference(mean(*)) FROM test-db GROUP BY time(5m) ORDER BY time ASC",
			pass: true,
		},
		{
			desc: "Using MAX function",
			qs: &telemetry_query.MetricsQuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "test-db",
				},
				Fields:   []string{"cpu"},
				Function: telemetry_query.TsdbFunctionType_MAX.String(),
			},
			resp: "SELECT max(cpu),* FROM test-db ORDER BY time ASC",
			pass: true,
		},
		{
			desc: "Name field",
			qs: &telemetry_query.MetricsQuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "test-db",
				},
				Name:   "test",
				Fields: []string{"cpu"},
			},
			resp: "SELECT cpu FROM test-db WHERE \"Name\" = 'test' ORDER BY time ASC",
			pass: true,
		},
		{
			desc: "Selector clause",
			qs: &telemetry_query.MetricsQuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "test-db",
				},
				Selector: &fields.Selector{
					Requirements: []*fields.Requirement{
						&fields.Requirement{
							Key:      "Name",
							Operator: "gt",
							Values:   []string{"2"},
						},
					},
				},
				Fields: []string{"cpu"},
			},
			resp: "SELECT cpu FROM test-db WHERE \"Name\" > 2 ORDER BY time ASC",
			pass: true,
		},
		{
			desc: "time clause",
			qs: &telemetry_query.MetricsQuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "test-db",
				},
				Fields:    []string{"cpu"},
				StartTime: startTime,
				EndTime:   endTime,
			},
			resp: "SELECT cpu FROM test-db WHERE time > '2018-11-09T23:16:17Z' AND time < '2018-11-09T23:22:17Z' ORDER BY time ASC",
			pass: true,
		},
		{
			desc: "Group by field",
			qs: &telemetry_query.MetricsQuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "test-db",
				},
				Fields:       []string{"cpu"},
				GroupbyField: "ReporterID",
			},
			resp: "SELECT cpu FROM test-db GROUP BY \"ReporterID\" ORDER BY time ASC",
			pass: true,
		},
		{
			desc: "Group by time",
			qs: &telemetry_query.MetricsQuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "test-db",
				},
				Fields:      []string{"cpu"},
				Function:    telemetry_query.TsdbFunctionType_MEAN.String(),
				GroupbyTime: "30s",
			},
			resp: "SELECT mean(cpu) FROM test-db GROUP BY time(30s) ORDER BY time ASC",
			pass: true,
		},
		{
			desc: "sortOrder",
			qs: &telemetry_query.MetricsQuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "test-db",
				},
				Fields:      []string{"cpu"},
				Function:    telemetry_query.TsdbFunctionType_MEAN.String(),
				GroupbyTime: "30s",
				SortOrder:   telemetry_query.SortOrder_Descending.String(),
			},
			resp: "SELECT mean(cpu) FROM test-db GROUP BY time(30s) ORDER BY time DESC",
			pass: true,
		},
		{
			desc: "pagination",
			qs: &telemetry_query.MetricsQuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "test-db",
				},
				Fields:      []string{"cpu"},
				Function:    telemetry_query.TsdbFunctionType_MEAN.String(),
				GroupbyTime: "30s",
				Pagination: &telemetry_query.PaginationSpec{
					Count:  10,
					Offset: 10,
				},
			},
			resp: "SELECT mean(cpu) FROM test-db GROUP BY time(30s) ORDER BY time ASC LIMIT 10 OFFSET 10",
			pass: true,
		},
	}

	for _, i := range testQs {
		resp, err := buildCitadelMetricsQuery(i.qs)
		if i.pass {
			AssertOk(t, err, fmt.Sprintf("%s: failed to build query %+v", i.desc, i.qs))
			Assert(t, resp == i.resp, fmt.Sprintf("%s: query didn't match, got:{%s} expected: {%s}", i.desc, resp, i.resp))
		} else {
			Assert(t, err != nil, fmt.Sprintf("%s: build query didn't fail %+v", i.desc, i.qs))
		}
	}
}

func TestValidateMetricsQueryList(t *testing.T) {
	q := &Server{
		grpcSrv: nil,
		broker:  nil,
	}
	testQs := []struct {
		ql      *telemetry_query.MetricsQueryList
		errMsgs []string
		errCode int32
		pass    bool
		desc    string
	}{
		{
			desc:    "nil query",
			ql:      nil,
			errMsgs: []string{"query required"},
			errCode: 400,
			pass:    false,
		},
		{
			desc: "blank query",
			ql:   &telemetry_query.MetricsQueryList{},
			errMsgs: []string{
				"query required",
				"tenant required",
			},
			errCode: 400,
			pass:    false,
		},
		{
			desc: "empty tenant",
			ql: &telemetry_query.MetricsQueryList{
				Queries: []*telemetry_query.MetricsQuerySpec{
					&telemetry_query.MetricsQuerySpec{
						TypeMeta: api.TypeMeta{
							Kind: "Node",
						},
						Function:  "none",
						SortOrder: telemetry_query.SortOrder_Ascending.String(),
					},
				},
			},
			errMsgs: []string{
				"tenant required",
			},
			errCode: 400,
			pass:    false,
		},
		{
			desc: "invalid name selector",
			ql: &telemetry_query.MetricsQueryList{
				Queries: []*telemetry_query.MetricsQuerySpec{
					&telemetry_query.MetricsQuerySpec{
						TypeMeta: api.TypeMeta{
							Kind: "Node",
						},
						Name:      "invalid name",
						Function:  "none",
						SortOrder: telemetry_query.SortOrder_Ascending.String(),
					},
				},
				Tenant: "testTenant",
			},
			errMsgs: []string{
				"Queries[0].Name failed validation",
			},
			errCode: 400,
			pass:    false,
		},
		{
			desc: "invalid function",
			ql: &telemetry_query.MetricsQueryList{
				Queries: []*telemetry_query.MetricsQuerySpec{
					&telemetry_query.MetricsQuerySpec{
						TypeMeta: api.TypeMeta{
							Kind: "Node",
						},
						Function:  "fakeFunction",
						SortOrder: telemetry_query.SortOrder_Ascending.String(),
					},
				},
				Tenant: "testTenant",
			},
			errMsgs: []string{
				"Queries[0].Function did not match allowed strings",
			},
			errCode: 400,
			pass:    false,
		},
		{
			desc: "invalid group by field",
			ql: &telemetry_query.MetricsQueryList{
				Queries: []*telemetry_query.MetricsQuerySpec{
					&telemetry_query.MetricsQuerySpec{
						TypeMeta: api.TypeMeta{
							Kind: "Node",
						},
						GroupbyField: "invalid field",
						SortOrder:    telemetry_query.SortOrder_Ascending.String(),
					},
				},
				Tenant: "testTenant",
			},
			errMsgs: []string{
				"Queries[0].GroupbyField failed validation: Value must start",
			},
			errCode: 400,
			pass:    false,
		},
		{
			desc: "invalid field",
			ql: &telemetry_query.MetricsQueryList{
				Queries: []*telemetry_query.MetricsQuerySpec{
					&telemetry_query.MetricsQuerySpec{
						TypeMeta: api.TypeMeta{
							Kind: "Node",
						},
						Function:  "mean",
						Fields:    []string{"valid", "invalid field"},
						SortOrder: telemetry_query.SortOrder_Ascending.String(),
					},
				},
				Tenant: "testTenant",
			},
			errMsgs: []string{
				"Queries[0].Fields failed validation",
			},
			errCode: 400,
			pass:    false,
		},
		{
			desc: "Invalid group by time",
			ql: &telemetry_query.MetricsQueryList{
				Queries: []*telemetry_query.MetricsQuerySpec{
					&telemetry_query.MetricsQuerySpec{
						TypeMeta: api.TypeMeta{
							Kind: "Node",
						},
						Function:    "mean",
						GroupbyTime: "invalidDuration",
						SortOrder:   telemetry_query.SortOrder_Ascending.String(),
					},
				},
				Tenant: "testTenant",
			},
			errMsgs: []string{
				"failed to parse groupby-time",
			},
			errCode: 400,
			pass:    false,
		},
		{
			desc: "Valid query",
			ql: &telemetry_query.MetricsQueryList{
				Queries: []*telemetry_query.MetricsQuerySpec{
					&telemetry_query.MetricsQuerySpec{
						TypeMeta: api.TypeMeta{
							Kind: "Node",
						},
						Function:  "none",
						SortOrder: telemetry_query.SortOrder_Ascending.String(),
					},
				},
				Tenant: "testTenant",
			},
			errMsgs: []string{},
			errCode: 200,
			pass:    true,
		},
		{
			desc: "invalid empty pagination",
			ql: &telemetry_query.MetricsQueryList{
				Queries: []*telemetry_query.MetricsQuerySpec{
					&telemetry_query.MetricsQuerySpec{
						TypeMeta: api.TypeMeta{
							Kind: "Node",
						},
						Function:   "none",
						Pagination: &telemetry_query.PaginationSpec{},
						SortOrder:  telemetry_query.SortOrder_Ascending.String(),
					},
				},
				Tenant: "testTenant",
			},
			errMsgs: []string{
				"Queries[0].Pagination.Count failed validation",
			},
			errCode: 400,
			pass:    false,
		},
		{
			desc: "invalid count for pagination",
			ql: &telemetry_query.MetricsQueryList{
				Queries: []*telemetry_query.MetricsQuerySpec{
					&telemetry_query.MetricsQuerySpec{
						TypeMeta: api.TypeMeta{
							Kind: "Node",
						},
						Function: "none",
						Pagination: &telemetry_query.PaginationSpec{
							Count: -10,
						},
						SortOrder: telemetry_query.SortOrder_Ascending.String(),
					},
				},
				Tenant: "testTenant",
			},
			errMsgs: []string{
				"Queries[0].Pagination.Count failed validation",
			},
			errCode: 400,
			pass:    false,
		},
		{
			desc: "invalid offset for pagination",
			ql: &telemetry_query.MetricsQueryList{
				Queries: []*telemetry_query.MetricsQuerySpec{
					&telemetry_query.MetricsQuerySpec{
						TypeMeta: api.TypeMeta{
							Kind: "Node",
						},
						Function: "none",
						Pagination: &telemetry_query.PaginationSpec{
							Count:  10,
							Offset: -10,
						},
						SortOrder: telemetry_query.SortOrder_Ascending.String(),
					},
				},
				Tenant: "testTenant",
			},
			errMsgs: []string{
				"Queries[0].Pagination.Offset failed validation",
			},
			errCode: 400,
			pass:    false,
		},
		{
			desc: "valid query with groupby-time < 1m",
			ql: &telemetry_query.MetricsQueryList{
				Queries: []*telemetry_query.MetricsQuerySpec{
					&telemetry_query.MetricsQuerySpec{
						TypeMeta: api.TypeMeta{
							Kind: "Node",
						},
						Function:    "last",
						SortOrder:   telemetry_query.SortOrder_Ascending.String(),
						GroupbyTime: "30s",
					},
				},
				Tenant: "testTenant",
			},
			errMsgs: []string{"too low groupby-time"},
			pass:    false,
			errCode: 400,
		},

		{
			desc: "valid query with pagination",
			ql: &telemetry_query.MetricsQueryList{
				Queries: []*telemetry_query.MetricsQuerySpec{
					&telemetry_query.MetricsQuerySpec{
						TypeMeta: api.TypeMeta{
							Kind: "Node",
						},
						Function: "none",
						Pagination: &telemetry_query.PaginationSpec{
							Count:  10,
							Offset: 10,
						},
						SortOrder: telemetry_query.SortOrder_Ascending.String(),
					},
				},
				Tenant: "testTenant",
			},
			errMsgs: []string{},
			errCode: 200,
			pass:    true,
		},
	}

	for _, i := range testQs {
		err := q.validateMetricsQueryList(i.ql)
		if i.pass && err != nil {
			t.Errorf("%s: Expected error to be nil but was %v", i.desc, err)
		} else if !i.pass && err == nil {
			t.Errorf("%s: Expected test to fail but err was nil", i.desc)
		} else if err != nil {
			errStatus := apierrors.FromError(err)
			if !strings.HasPrefix(errStatus.GetMessage()[0], i.errMsgs[0]) {
				t.Errorf("test:%s, Expected error message to be %v but error was %v", i.desc, i.errMsgs, errStatus.GetMessage())
			}
			if i.errCode != errStatus.GetCode() {
				t.Errorf("%s: Expected error code to be %d but error was %v", i.desc, i.errCode, errStatus.GetCode())
			}
		}
	}
}

func TestValidateQuerySpec(t *testing.T) {
	q := &Server{
		grpcSrv: nil,
		broker:  nil,
	}
	testQs := []struct {
		qs      *telemetry_query.MetricsQuerySpec
		errMsgs []string
		pass    bool
		desc    string
	}{
		{
			desc: "nil query",
			qs:   nil,
			errMsgs: []string{
				"query parameter required",
			},
			pass: false,
		},
		{
			desc: "no kind",
			qs:   &telemetry_query.MetricsQuerySpec{},
			errMsgs: []string{
				"kind required",
			},
			pass: false,
		},
		{
			desc: "mepty kind",
			qs: &telemetry_query.MetricsQuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "",
				},
			},
			errMsgs: []string{
				"kind required",
			},
			pass: false,
		},
		{
			desc: "invalid kind",
			qs: &telemetry_query.MetricsQuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "invalid kind",
				},
			},
			errMsgs: []string{
				"invalid kind",
			},
			pass: false,
		},
		{
			desc: "valid query on a kind",
			qs: &telemetry_query.MetricsQuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "Node",
				},
				Name:      "validname",
				Function:  "none",
				SortOrder: telemetry_query.SortOrder_Ascending.String(),
			},
			errMsgs: []string{},
			pass:    true,
		},
		{
			desc: "Max function with no field",
			qs: &telemetry_query.MetricsQuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "Node",
				},
				Function:  "max",
				SortOrder: telemetry_query.SortOrder_Ascending.String(),
			},
			errMsgs: []string{
				"Function MAX requires exactly one field",
			},
			pass: false,
		},
		{
			desc: "valid Max function query",
			qs: &telemetry_query.MetricsQuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "Node",
				},
				Function:  "max",
				Fields:    []string{"f1"},
				SortOrder: telemetry_query.SortOrder_Ascending.String(),
			},
			errMsgs: []string{},
			pass:    true,
		},
		{
			desc: "Max function query with more than one field",
			qs: &telemetry_query.MetricsQuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "Node",
				},
				Function:  "max",
				Fields:    []string{"f1", "f2"},
				SortOrder: telemetry_query.SortOrder_Ascending.String(),
			},
			errMsgs: []string{
				"Function MAX requires exactly one field",
			},
			pass: false,
		},
		{
			desc: "Mean query with no fields",
			qs: &telemetry_query.MetricsQuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "Node",
				},
				Function:  "mean",
				SortOrder: telemetry_query.SortOrder_Ascending.String(),
			},
			errMsgs: []string{},
			pass:    true,
		},
		{
			desc: "Mean query with one field",
			qs: &telemetry_query.MetricsQuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "Node",
				},
				Function:  "mean",
				Fields:    []string{"f1"},
				SortOrder: telemetry_query.SortOrder_Ascending.String(),
			},
			errMsgs: []string{},
			pass:    true,
		},
		{
			desc: "Mean query with more than one fields",
			qs: &telemetry_query.MetricsQuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "Node",
				},
				Function:  "mean",
				Fields:    []string{"f1", "f2"},
				SortOrder: telemetry_query.SortOrder_Ascending.String(),
			},
			errMsgs: []string{},
			pass:    true,
		},
		{
			desc: "nil requirements",
			qs: &telemetry_query.MetricsQuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "Node",
				},
				Selector: &fields.Selector{
					Requirements: nil,
				},
				Function:     "mean",
				Fields:       []string{},
				GroupbyField: "validField",
				SortOrder:    telemetry_query.SortOrder_Ascending.String(),
			},
			errMsgs: []string{},
			pass:    true,
		},
		{
			desc: "empty requirements",
			qs: &telemetry_query.MetricsQuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "Node",
				},
				Selector: &fields.Selector{
					Requirements: []*fields.Requirement{},
				},
				Function:     "mean",
				Fields:       []string{},
				GroupbyField: "validField",
				SortOrder:    telemetry_query.SortOrder_Ascending.String(),
			},
			errMsgs: []string{},
			pass:    true,
		},
		{
			desc: "invalid requirements",
			qs: &telemetry_query.MetricsQuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "Node",
				},
				Selector: &fields.Selector{
					Requirements: []*fields.Requirement{
						&fields.Requirement{
							Key:      "Name",
							Operator: "equals",
						},
					},
				},
				Function:     "mean",
				Fields:       []string{},
				GroupbyField: "validField",
				SortOrder:    telemetry_query.SortOrder_Ascending.String(),
			},
			errMsgs: []string{
				"Failed to parse selector requirements: Values cannot be empty",
			},
			pass: false,
		},
		{
			desc: "query with valid requirements",
			qs: &telemetry_query.MetricsQuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "Node",
				},
				Selector: &fields.Selector{
					Requirements: []*fields.Requirement{
						&fields.Requirement{
							Key:      "Name",
							Operator: "equals",
							Values:   []string{"test"},
						},
					},
				},
				Function:     "mean",
				Fields:       []string{},
				GroupbyField: "validField",
				SortOrder:    telemetry_query.SortOrder_Ascending.String(),
			},
			errMsgs: []string{},
			pass:    true,
		},
	}

	for _, i := range testQs {
		err := q.validateMetricsQuerySpec(i.qs)
		if i.pass && len(err) != 0 {
			t.Errorf("%s: Expected error to be nil but was %v", i.desc, err)
		} else if !i.pass && len(err) == 0 {
			t.Errorf("test:%s,Expected test to fail but err was nil", i.desc)
		} else if err != nil {
			if !reflect.DeepEqual(i.errMsgs, err) {
				t.Errorf("%s: Expected error message to be %v but error was %v", i.desc, i.errMsgs, err)
			}
		}
	}
}

func TestMetricsQuery(t *testing.T) {
	mockCtrl := gomock.NewController(t)
	defer mockCtrl.Finish()
	mockBroker := mock.NewMockInf(mockCtrl)
	srv, err := NewQueryService(testServerURL, mockBroker)
	if err != nil {
		t.Errorf("Recieved unexpected error from NewQueryService: %v", err)
	}
	defer srv.Stop()

	testQs := []struct {
		queryList    *telemetry_query.MetricsQueryList
		citadelQuery string
		errMsg       string
		// If clusterCheckResponse is not nil, it will mock the given value
		clusterCheckResponse *ClusterCheckResponse
		// If executeQueryResponse is not nil, it will mock the given value
		executeQueryResponse *ExecuteQueryResponse
		queryResponse        *telemetry_query.MetricsQueryResponse
	}{
		{
			queryList:            &telemetry_query.MetricsQueryList{},
			citadelQuery:         "",
			errMsg:               "rpc error: code = InvalidArgument desc = Validation Failed",
			clusterCheckResponse: nil,
			executeQueryResponse: nil,
			queryResponse:        nil,
		},
		{
			queryList: &telemetry_query.MetricsQueryList{
				Tenant: "test",
				Queries: []*telemetry_query.MetricsQuerySpec{
					&telemetry_query.MetricsQuerySpec{
						TypeMeta: api.TypeMeta{
							Kind: "Node",
						},
						Function:  "none",
						SortOrder: telemetry_query.SortOrder_Ascending.String(),
					},
				},
			},
			citadelQuery: "",
			errMsg:       "Cluster check failed",
			clusterCheckResponse: &ClusterCheckResponse{
				err: errors.New("Cluster check failed"),
			},
			executeQueryResponse: nil,
			queryResponse:        nil,
		},
		{
			queryList: &telemetry_query.MetricsQueryList{
				Tenant:    "testTenant",
				Namespace: "testNamespace",
				Queries: []*telemetry_query.MetricsQuerySpec{
					&telemetry_query.MetricsQuerySpec{
						TypeMeta: api.TypeMeta{
							Kind: "Node",
						},
						Selector: &fields.Selector{
							Requirements: []*fields.Requirement{
								&fields.Requirement{
									Key:      "Name",
									Operator: "equals",
									Values:   []string{"test"},
								},
							},
						},
						Function:  "none",
						SortOrder: telemetry_query.SortOrder_Ascending.String(),
					},
					&telemetry_query.MetricsQuerySpec{
						TypeMeta: api.TypeMeta{
							Kind: "Node",
						},
						Selector: &fields.Selector{
							Requirements: []*fields.Requirement{
								&fields.Requirement{
									Key:      "Name",
									Operator: "equals",
									Values:   []string{"test1"},
								},
							},
						},
						Function:  "none",
						SortOrder: telemetry_query.SortOrder_Ascending.String(),
					},
					&telemetry_query.MetricsQuerySpec{
						TypeMeta: api.TypeMeta{
							Kind: "Node",
						},
						Selector: &fields.Selector{
							Requirements: []*fields.Requirement{
								&fields.Requirement{
									Key:      "Name",
									Operator: "equals",
									Values:   []string{"test2"},
								},
							},
						},
						Function:  "none",
						SortOrder: telemetry_query.SortOrder_Ascending.String(),
					},
				},
			},
			citadelQuery: "SELECT * FROM Node WHERE \"Name\" = 'test' ORDER BY time ASC; SELECT * FROM Node WHERE \"Name\" = 'test1' ORDER BY time ASC; SELECT * FROM Node WHERE \"Name\" = 'test2' ORDER BY time ASC",
			errMsg:       "",
			clusterCheckResponse: &ClusterCheckResponse{
				err: nil,
			},
			executeQueryResponse: &ExecuteQueryResponse{
				qr: []*query.Result{
					&query.Result{
						StatementID: 0,
						Series: models.Rows{
							&models.Row{
								Name:    "test",
								Tags:    nil,
								Columns: []string{"time", "value"},
								Values: [][]interface{}{
									{8, 10.123, "string", true},
								},
								Partial: false,
							},
						},
						Messages: nil,
						Partial:  false,
						Err:      nil,
					},
					&query.Result{
						StatementID: 1,
						Series: models.Rows{
							&models.Row{
								Name:    "test1",
								Tags:    nil,
								Columns: []string{"time", "value"},
								Values: [][]interface{}{
									{8, 10.123, "string", true},
								},
								Partial: false,
							},
						},
						Messages: nil,
						Partial:  false,
						Err:      nil,
					},
					&query.Result{
						StatementID: 2,
						Series: models.Rows{
							&models.Row{
								Name:    "test2",
								Tags:    nil,
								Columns: []string{"time", "value"},
								Values: [][]interface{}{
									{8, 10.123, "string", true},
								},
								Partial: false,
							},
						},
						Messages: nil,
						Partial:  false,
						Err:      nil,
					},
				},
				err: nil,
			},
			queryResponse: &telemetry_query.MetricsQueryResponse{
				Results: []*telemetry_query.MetricsQueryResult{
					&telemetry_query.MetricsQueryResult{
						StatementID: 0,
						Series: []*telemetry_query.ResultSeries{
							&telemetry_query.ResultSeries{
								Name:    "test",
								Tags:    nil,
								Columns: []string{"time", "value"},
								Values: []*api.InterfaceSlice{
									&api.InterfaceSlice{
										Values: []*api.Interface{
											&api.Interface{
												Value: &api.Interface_Int64{
													Int64: 8,
												},
											},
											&api.Interface{
												Value: &api.Interface_Float{
													Float: 10.123,
												},
											},
											&api.Interface{
												Value: &api.Interface_Str{
													Str: "string",
												},
											},
											&api.Interface{
												Value: &api.Interface_Bool{
													Bool: true,
												},
											},
										},
									},
								},
							},
						},
					},
					&telemetry_query.MetricsQueryResult{
						StatementID: 1,
						Series: []*telemetry_query.ResultSeries{
							&telemetry_query.ResultSeries{
								Name:    "test1",
								Tags:    nil,
								Columns: []string{"time", "value"},
								Values: []*api.InterfaceSlice{
									&api.InterfaceSlice{
										Values: []*api.Interface{
											&api.Interface{
												Value: &api.Interface_Int64{
													Int64: 8,
												},
											},
											&api.Interface{
												Value: &api.Interface_Float{
													Float: 10.123,
												},
											},
											&api.Interface{
												Value: &api.Interface_Str{
													Str: "string",
												},
											},
											&api.Interface{
												Value: &api.Interface_Bool{
													Bool: true,
												},
											},
										},
									},
								},
							},
						},
					},
					&telemetry_query.MetricsQueryResult{
						StatementID: 2,
						Series: []*telemetry_query.ResultSeries{
							&telemetry_query.ResultSeries{
								Name:    "test2",
								Tags:    nil,
								Columns: []string{"time", "value"},
								Values: []*api.InterfaceSlice{
									&api.InterfaceSlice{
										Values: []*api.Interface{
											&api.Interface{
												Value: &api.Interface_Int64{
													Int64: 8,
												},
											},
											&api.Interface{
												Value: &api.Interface_Float{
													Float: 10.123,
												},
											},
											&api.Interface{
												Value: &api.Interface_Str{
													Str: "string",
												},
											},
											&api.Interface{
												Value: &api.Interface_Bool{
													Bool: true,
												},
											},
										},
									},
								},
							},
						},
					},
				},
				Tenant:    "testTenant",
				Namespace: "testNamespace",
			},
		},
	}

	ctx := context.Background()

	for _, i := range testQs {
		if i.clusterCheckResponse != nil {
			mockBroker.EXPECT().ClusterCheck().Return(i.clusterCheckResponse.err)
		}
		if i.executeQueryResponse != nil {
			mockBroker.EXPECT().ExecuteQuery(ctx, i.queryList.Tenant, i.citadelQuery).Return(i.executeQueryResponse.qr, i.executeQueryResponse.err)
		}

		res, err := srv.Metrics(context.Background(), i.queryList)
		if i.errMsg == "" {
			AssertOk(t, err, "Query returned unexpected error %v", err)
		} else {
			Assert(t, strings.Contains(err.Error(), i.errMsg), "Expected query to return error that contains %s but got err %s", i.errMsg, err.Error())
		}
		Assert(t, reflect.DeepEqual(res, i.queryResponse), "Query response did not match expected response, expected %v, but got %v", i.queryResponse, res)
	}
}
