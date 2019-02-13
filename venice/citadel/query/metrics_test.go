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
	"github.com/pensando/sw/api/generated/telemetry_query"
	"github.com/pensando/sw/api/labels"
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
	}{
		{
			qs: &telemetry_query.MetricsQuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "test-db",
				},
			},
			resp: "SELECT * FROM test-db",
			pass: true,
		},
		{
			qs: &telemetry_query.MetricsQuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "test-db",
				},
				Fields: []string{"cpu"},
			},
			resp: "SELECT cpu FROM test-db",
			pass: true,
		},
		{
			qs: &telemetry_query.MetricsQuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "test-db",
				},
				Function: "MEAN",
			},
			resp: "SELECT MEAN(*) FROM test-db",
			pass: true,
		},
		{
			qs: &telemetry_query.MetricsQuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "test-db",
				},
				Fields:   []string{"cpu"},
				Function: "MEAN",
			},
			resp: "SELECT MEAN(cpu) FROM test-db",
			pass: true,
		},
		{
			qs: &telemetry_query.MetricsQuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "test-db",
				},
				Fields:   []string{"cpu", "memory"},
				Function: "MEAN",
			},
			resp: "SELECT MEAN(cpu),MEAN(memory) FROM test-db",
			pass: true,
		},
		{
			qs: &telemetry_query.MetricsQuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "test-db",
				},
				Fields:   []string{"cpu"},
				Function: "MAX",
			},
			resp: "SELECT MAX(cpu),* FROM test-db",
			pass: true,
		},
		{
			qs: &telemetry_query.MetricsQuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "test-db",
				},
				Name:   "test",
				Fields: []string{"cpu"},
			},
			resp: "SELECT cpu FROM test-db WHERE \"meta.name\" = 'test'",
			pass: true,
		},
		{
			qs: &telemetry_query.MetricsQuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "test-db",
				},
				Selector: &labels.Selector{
					Requirements: []*labels.Requirement{
						&labels.Requirement{
							Key:      "meta.name",
							Operator: "equals",
							Values:   []string{"test"},
						},
					},
				},
				Fields: []string{"cpu"},
			},
			resp: "SELECT cpu FROM test-db WHERE \"meta.name\" = 'test'",
			pass: true,
		},
		{
			qs: &telemetry_query.MetricsQuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "test-db",
				},
				Fields:    []string{"cpu"},
				StartTime: startTime,
				EndTime:   endTime,
			},
			resp: "SELECT cpu FROM test-db WHERE time > '2018-11-09T23:16:17Z' AND time < '2018-11-09T23:22:17Z'",
			pass: true,
		},
		{
			qs: &telemetry_query.MetricsQuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "test-db",
				},
				Fields:       []string{"cpu"},
				GroupbyField: "ReporterID",
			},
			resp: "SELECT cpu FROM test-db GROUP BY ReporterID",
			pass: true,
		},
		{
			qs: &telemetry_query.MetricsQuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "test-db",
				},
				Fields:      []string{"cpu"},
				Function:    "MEAN",
				GroupbyTime: "30s",
			},
			resp: "SELECT MEAN(cpu) FROM test-db GROUP BY time(30s)",
			pass: true,
		},
	}

	for _, i := range testQs {
		resp, err := buildCitadelMetricsQuery(i.qs)
		if i.pass {
			AssertOk(t, err, fmt.Sprintf("failed to build query %+v", i.qs))
			Assert(t, resp == i.resp, fmt.Sprintf("query didn't match, got:{%s} expected: {%s}", resp, i.resp))
		} else {
			Assert(t, err != nil, fmt.Sprintf("build query didn't fail %+v", i.qs))
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
	}{
		{
			ql:      nil,
			errMsgs: []string{"query required"},
			errCode: 400,
			pass:    false,
		},
		{
			ql: &telemetry_query.MetricsQueryList{},
			errMsgs: []string{
				"query required",
				"tenant required",
			},
			errCode: 400,
			pass:    false,
		},
		{
			ql: &telemetry_query.MetricsQueryList{
				Queries: []*telemetry_query.MetricsQuerySpec{
					&telemetry_query.MetricsQuerySpec{
						TypeMeta: api.TypeMeta{
							Kind: "Node",
						},
						Function: "none",
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
			ql: &telemetry_query.MetricsQueryList{
				Queries: []*telemetry_query.MetricsQuerySpec{
					&telemetry_query.MetricsQuerySpec{
						TypeMeta: api.TypeMeta{
							Kind: "Node",
						},
						Name:     "invalid name",
						Function: "none",
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
			ql: &telemetry_query.MetricsQueryList{
				Queries: []*telemetry_query.MetricsQuerySpec{
					&telemetry_query.MetricsQuerySpec{
						TypeMeta: api.TypeMeta{
							Kind: "Node",
						},
						Function: "fakeFunction",
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
			ql: &telemetry_query.MetricsQueryList{
				Queries: []*telemetry_query.MetricsQuerySpec{
					&telemetry_query.MetricsQuerySpec{
						TypeMeta: api.TypeMeta{
							Kind: "Node",
						},
						GroupbyField: "invalid field",
					},
				},
				Tenant: "testTenant",
			},
			errMsgs: []string{
				"Queries[0].GroupbyField failed validation",
			},
			errCode: 400,
			pass:    false,
		},
		{
			ql: &telemetry_query.MetricsQueryList{
				Queries: []*telemetry_query.MetricsQuerySpec{
					&telemetry_query.MetricsQuerySpec{
						TypeMeta: api.TypeMeta{
							Kind: "Node",
						},
						Function: "mean",
						Fields:   []string{"valid", "invalid field"},
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
			ql: &telemetry_query.MetricsQueryList{
				Queries: []*telemetry_query.MetricsQuerySpec{
					&telemetry_query.MetricsQuerySpec{
						TypeMeta: api.TypeMeta{
							Kind: "Node",
						},
						Function:    "mean",
						GroupbyTime: "invalidDuration",
					},
				},
				Tenant: "testTenant",
			},
			errMsgs: []string{
				"Queries[0].GroupbyTime failed validation",
			},
			errCode: 400,
			pass:    false,
		},
		{
			ql: &telemetry_query.MetricsQueryList{
				Queries: []*telemetry_query.MetricsQuerySpec{
					&telemetry_query.MetricsQuerySpec{
						TypeMeta: api.TypeMeta{
							Kind: "Node",
						},
						Function: "none",
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
			t.Errorf("Expected error to be nil but was %v", err)
		} else if !i.pass && err == nil {
			t.Errorf("Expected test to fail but err was nil")
		} else if err != nil {
			errStatus := apierrors.FromError(err)
			if !reflect.DeepEqual(i.errMsgs, errStatus.GetMessage()) {
				t.Errorf("Expected error message to be %v but error was %v", i.errMsgs, errStatus.GetMessage())
			}
			if i.errCode != errStatus.GetCode() {
				t.Errorf("Expected error code to be %d but error was %v", i.errCode, errStatus.GetCode())
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
	}{
		{
			qs: nil,
			errMsgs: []string{
				"query parameter required",
			},
			pass: false,
		},
		{
			qs: &telemetry_query.MetricsQuerySpec{},
			errMsgs: []string{
				"kind required",
			},
			pass: false,
		},
		{
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
			qs: &telemetry_query.MetricsQuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "Node",
				},
				Name:     "validname",
				Function: "none",
			},
			errMsgs: []string{},
			pass:    true,
		},
		{
			qs: &telemetry_query.MetricsQuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "Node",
				},
				Function: "max",
			},
			errMsgs: []string{
				"Function MAX requires exactly one field",
			},
			pass: false,
		},
		{
			qs: &telemetry_query.MetricsQuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "Node",
				},
				Function: "max",
				Fields:   []string{"f1"},
			},
			errMsgs: []string{},
			pass:    true,
		},
		{
			qs: &telemetry_query.MetricsQuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "Node",
				},
				Function: "max",
				Fields:   []string{"f1", "f2"},
			},
			errMsgs: []string{
				"Function MAX requires exactly one field",
			},
			pass: false,
		},
		{
			qs: &telemetry_query.MetricsQuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "Node",
				},
				Function: "mean",
			},
			errMsgs: []string{},
			pass:    true,
		},
		{
			qs: &telemetry_query.MetricsQuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "Node",
				},
				Function: "mean",
				Fields:   []string{"f1"},
			},
			errMsgs: []string{},
			pass:    true,
		},
		{
			qs: &telemetry_query.MetricsQuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "Node",
				},
				Function: "mean",
				Fields:   []string{"f1", "f2"},
			},
			errMsgs: []string{},
			pass:    true,
		},
		{
			qs: &telemetry_query.MetricsQuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "Node",
				},
				Selector: &labels.Selector{
					Requirements: nil,
				},
				Function:     "mean",
				Fields:       []string{},
				GroupbyField: "validField",
			},
			errMsgs: []string{},
			pass:    true,
		},
		{
			qs: &telemetry_query.MetricsQuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "Node",
				},
				Selector: &labels.Selector{
					Requirements: []*labels.Requirement{},
				},
				Function:     "mean",
				Fields:       []string{},
				GroupbyField: "validField",
			},
			errMsgs: []string{},
			pass:    true,
		},
		{
			qs: &telemetry_query.MetricsQuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "Node",
				},
				Selector: &labels.Selector{
					Requirements: []*labels.Requirement{
						&labels.Requirement{
							Key:      "meta.name",
							Operator: "equals",
						},
					},
				},
				Function:     "mean",
				Fields:       []string{},
				GroupbyField: "validField",
			},
			errMsgs: []string{
				"Failed to parse selector requirements: Only a single value supported",
			},
			pass: false,
		},
		{
			qs: &telemetry_query.MetricsQuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "Node",
				},
				Selector: &labels.Selector{
					Requirements: []*labels.Requirement{
						&labels.Requirement{
							Key:      "meta.name",
							Operator: "equals",
							Values:   []string{"test"},
						},
					},
				},
				Function:     "mean",
				Fields:       []string{},
				GroupbyField: "validField",
			},
			errMsgs: []string{},
			pass:    true,
		},
	}

	for testCase, i := range testQs {
		err := q.validateMetricsQuerySpec(i.qs)
		if i.pass && len(err) != 0 {
			t.Errorf("Test Case %v: Expected error to be nil but was %v", testCase, err)
		} else if !i.pass && len(err) == 0 {
			t.Errorf("Test Case %v: Expected test to fail but err was nil", testCase)
		} else if err != nil {
			if !reflect.DeepEqual(i.errMsgs, err) {
				t.Errorf("Test Case %v: Expected error message to be %v but error was %v", testCase, i.errMsgs, err)
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
						Function: "none",
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
						Selector: &labels.Selector{
							Requirements: []*labels.Requirement{
								&labels.Requirement{
									Key:      "meta.name",
									Operator: "equals",
									Values:   []string{"test"},
								},
							},
						},
						Function: "none",
					},
					&telemetry_query.MetricsQuerySpec{
						TypeMeta: api.TypeMeta{
							Kind: "Node",
						},
						Selector: &labels.Selector{
							Requirements: []*labels.Requirement{
								&labels.Requirement{
									Key:      "meta.name",
									Operator: "equals",
									Values:   []string{"test1"},
								},
							},
						},
						Function: "none",
					},
					&telemetry_query.MetricsQuerySpec{
						TypeMeta: api.TypeMeta{
							Kind: "Node",
						},
						Selector: &labels.Selector{
							Requirements: []*labels.Requirement{
								&labels.Requirement{
									Key:      "meta.name",
									Operator: "equals",
									Values:   []string{"test2"},
								},
							},
						},
						Function: "none",
					},
				},
			},
			citadelQuery: "SELECT * FROM Node WHERE \"meta.name\" = 'test'; SELECT * FROM Node WHERE \"meta.name\" = 'test1'; SELECT * FROM Node WHERE \"meta.name\" = 'test2'",
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
