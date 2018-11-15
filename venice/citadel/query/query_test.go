package query

import (
	"context"
	"errors"
	"fmt"
	"reflect"
	"testing"

	"github.com/influxdata/influxdb/models"

	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"

	"github.com/golang/mock/gomock"
	"github.com/influxdata/influxdb/query"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/metrics_query"
	"github.com/pensando/sw/api/labels"
	"github.com/pensando/sw/venice/citadel/query/mocks"
	"github.com/pensando/sw/venice/utils/testutils"
	. "github.com/pensando/sw/venice/utils/testutils"
)

var testServerURL = "localhost:0"

func TestBuildCitadelQuery(t *testing.T) {
	startTime := &api.Timestamp{}
	startTime.Parse("2018-11-09T23:16:17Z")
	endTime := &api.Timestamp{}
	endTime.Parse("2018-11-09T23:22:17Z")
	testQs := []struct {
		qs   *metrics_query.QuerySpec
		resp string
		pass bool
	}{
		{
			qs: &metrics_query.QuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "test-db",
				},
			},
			resp: "SELECT * FROM test-db",
			pass: true,
		},
		{
			qs: &metrics_query.QuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "test-db",
				},
				Fields: []string{"cpu"},
			},
			resp: "SELECT cpu FROM test-db",
			pass: true,
		},
		{
			qs: &metrics_query.QuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "test-db",
				},
				Function: "MEAN",
			},
			resp: "SELECT MEAN(*) FROM test-db",
			pass: true,
		},
		{
			qs: &metrics_query.QuerySpec{
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
			qs: &metrics_query.QuerySpec{
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
			qs: &metrics_query.QuerySpec{
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
			qs: &metrics_query.QuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "test-db",
				},
				ObjectSelector: metrics_query.ObjectSelector{
					Tenant: "default",
					Selector: &labels.Selector{
						Requirements: []*labels.Requirement{
							&labels.Requirement{
								Key:      "meta.name",
								Operator: "equals",
								Values:   []string{"test"},
							},
						},
					},
				},
				Fields: []string{"cpu"},
			},
			resp: "SELECT cpu FROM test-db WHERE \"meta.name\" = 'test'",
			pass: true,
		},
		{
			qs: &metrics_query.QuerySpec{
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
			qs: &metrics_query.QuerySpec{
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
			qs: &metrics_query.QuerySpec{
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
		resp, err := buildCitadelQuery(i.qs)
		if i.pass {
			testutils.AssertOk(t, err, fmt.Sprintf("failed to build query %+v", i.qs))
			testutils.Assert(t, resp == i.resp, fmt.Sprintf("query didn't match, got:{%s} expected: {%s}", resp, i.resp))
		} else {
			testutils.Assert(t, err != nil, fmt.Sprintf("build query didn't fail %+v", i.qs))
		}
	}
}

func TestValidate(t *testing.T) {
	q := &Server{
		grpcSrv: nil,
		broker:  nil,
	}
	testQs := []struct {
		qs      *metrics_query.QuerySpec
		errMsg  string
		errCode codes.Code
		pass    bool
	}{
		{
			qs:      nil,
			errMsg:  "query parameter required",
			errCode: codes.InvalidArgument,
			pass:    false,
		},
		{
			qs:      &metrics_query.QuerySpec{},
			errMsg:  "kind required",
			errCode: codes.InvalidArgument,
			pass:    false,
		},
		{
			qs: &metrics_query.QuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "",
				},
			},
			errMsg:  "kind required",
			errCode: codes.InvalidArgument,
			pass:    false,
		},
		{
			qs: &metrics_query.QuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "invalid kind",
				},
			},
			errMsg:  "invalid kind",
			errCode: codes.InvalidArgument,
			pass:    false,
		},
		{
			qs: &metrics_query.QuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "Node",
				},
			},
			errMsg:  "tenant required",
			errCode: codes.InvalidArgument,
			pass:    false,
		},
		{
			qs: &metrics_query.QuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "Node",
				},
				ObjectSelector: metrics_query.ObjectSelector{
					Tenant: "default",
				},
				Function: "fakeFunction",
			},
			errMsg:  "function FAKEFUNCTION is not an accepted function",
			errCode: codes.InvalidArgument,
			pass:    false,
		},
		{
			qs: &metrics_query.QuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "Node",
				},
				ObjectSelector: metrics_query.ObjectSelector{
					Tenant: "default",
				},
				Function: "max",
			},
			errMsg:  "Function MAX requires exactly one field",
			errCode: codes.InvalidArgument,
			pass:    false,
		},
		{
			qs: &metrics_query.QuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "Node",
				},
				ObjectSelector: metrics_query.ObjectSelector{
					Tenant: "default",
				},
				Function: "max",
				Fields:   []string{"f1"},
			},
			errMsg:  "",
			errCode: codes.OK,
			pass:    true,
		},
		{
			qs: &metrics_query.QuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "Node",
				},
				ObjectSelector: metrics_query.ObjectSelector{
					Tenant: "default",
				},
				Function: "max",
				Fields:   []string{"f1", "f2"},
			},
			errMsg:  "Function MAX requires exactly one field",
			errCode: codes.InvalidArgument,
			pass:    false,
		},
		{
			qs: &metrics_query.QuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "Node",
				},
				ObjectSelector: metrics_query.ObjectSelector{
					Tenant: "default",
				},
				Function: "mean",
			},
			errMsg:  "",
			errCode: codes.OK,
			pass:    true,
		},
		{
			qs: &metrics_query.QuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "Node",
				},
				ObjectSelector: metrics_query.ObjectSelector{
					Tenant: "default",
				},
				Function: "mean",
				Fields:   []string{"f1"},
			},
			errMsg:  "",
			errCode: codes.OK,
			pass:    true,
		},
		{
			qs: &metrics_query.QuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "Node",
				},
				ObjectSelector: metrics_query.ObjectSelector{
					Tenant: "default",
				},
				Function: "mean",
				Fields:   []string{"f1", "f2"},
			},
			errMsg:  "",
			errCode: codes.OK,
			pass:    true,
		},
		{
			qs: &metrics_query.QuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "Node",
				},
				ObjectSelector: metrics_query.ObjectSelector{
					Tenant: "default",
				},
				Function: "mean",
				Fields:   []string{"valid", "invalid field"},
			},
			errMsg:  "field invalid field was not a valid field",
			errCode: codes.InvalidArgument,
			pass:    false,
		},
		{
			qs: &metrics_query.QuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "Node",
				},
				ObjectSelector: metrics_query.ObjectSelector{
					Tenant:   "default",
					Selector: &labels.Selector{},
				},
				Function:     "mean",
				Fields:       []string{},
				GroupbyField: "invalid field",
			},
			errMsg:  "group-by-field invalid field was not a valid field",
			errCode: codes.InvalidArgument,
			pass:    false,
		},
		{
			qs: &metrics_query.QuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "Node",
				},
				ObjectSelector: metrics_query.ObjectSelector{
					Tenant: "default",
					Selector: &labels.Selector{
						Requirements: nil,
					},
				},
				Function:     "mean",
				Fields:       []string{},
				GroupbyField: "validField",
			},
			errMsg:  "",
			errCode: codes.OK,
			pass:    true,
		},
		{
			qs: &metrics_query.QuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "Node",
				},
				ObjectSelector: metrics_query.ObjectSelector{
					Tenant: "default",
					Selector: &labels.Selector{
						Requirements: []*labels.Requirement{},
					},
				},
				Function:     "mean",
				Fields:       []string{},
				GroupbyField: "validField",
			},
			errMsg:  "",
			errCode: codes.OK,
			pass:    true,
		},
		{
			qs: &metrics_query.QuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "Node",
				},
				ObjectSelector: metrics_query.ObjectSelector{
					Tenant: "default",
					Selector: &labels.Selector{
						Requirements: []*labels.Requirement{
							&labels.Requirement{
								Key:      "meta.name",
								Operator: "equals",
							},
						},
					},
				},
				Function:     "mean",
				Fields:       []string{},
				GroupbyField: "validField",
			},
			errMsg:  "Failed to parse selector requirements: Only a single value supported",
			errCode: codes.InvalidArgument,
			pass:    false,
		},
		{
			qs: &metrics_query.QuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "Node",
				},
				ObjectSelector: metrics_query.ObjectSelector{
					Tenant: "default",
					Selector: &labels.Selector{
						Requirements: []*labels.Requirement{
							&labels.Requirement{
								Key:      "meta.name",
								Operator: "equals",
								Values:   []string{"test"},
							},
						},
					},
				},
				Function:     "mean",
				Fields:       []string{},
				GroupbyField: "validField",
			},
			errMsg:  "",
			errCode: codes.OK,
			pass:    true,
		},
		{
			qs: &metrics_query.QuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "Node",
				},
				ObjectSelector: metrics_query.ObjectSelector{
					Tenant: "default",
					Selector: &labels.Selector{
						Requirements: []*labels.Requirement{
							&labels.Requirement{
								Key:      "meta.name",
								Operator: "equals",
								Values:   []string{"test"},
							},
						},
					},
				},
				Function:     "mean",
				Fields:       []string{},
				GroupbyField: "validField",
				GroupbyTime:  "invalidDuration",
			},
			errMsg:  "group-by-time value invalidDuration was not a valid duration",
			errCode: codes.InvalidArgument,
			pass:    false,
		},
	}

	for _, i := range testQs {
		err := q.validate(i.qs)
		if i.pass && err != nil {
			t.Errorf("Expected error to be nil but was %v", err)
		} else if !i.pass && err == nil {
			t.Errorf("Expected test to fail but err was nil")
		} else if err != nil {
			errStatus, ok := status.FromError(err)
			if !ok {
				t.Errorf("Expected GRPC error, but was unable to parse the returned error %v", err)
				return
			}
			if i.errMsg != errStatus.Message() {
				t.Errorf("Expected error message to be %s but error was %v", i.errMsg, errStatus.Message())
			}
			if i.errCode != errStatus.Code() {
				t.Errorf("Expected error code to be %d but error was %v", i.errCode, errStatus.Code())
			}
		}
	}
}

func TestQuery(t *testing.T) {
	mockCtrl := gomock.NewController(t)
	defer mockCtrl.Finish()
	mockBroker := mocks.NewMockBrokerInf(mockCtrl)
	srv, err := NewQueryService(testServerURL, mockBroker)
	if err != nil {
		t.Errorf("Recieved unexpected error from NewQueryService: %v", err)
	}
	defer srv.Stop()

	type ExecuteQueryResponse struct {
		qr  []*query.Result
		err error
	}

	type ClusterCheckResponse struct {
		err error
	}

	testQs := []struct {
		querySpec    *metrics_query.QuerySpec
		citadelQuery string
		errMsg       string
		// If clusterCheckResponse is not nil, it will mock the given value
		clusterCheckResponse *ClusterCheckResponse
		// If executeQueryResponse is not nil, it will mock the given value
		executeQueryResponse *ExecuteQueryResponse
		queryResponse        *metrics_query.QueryResponse
	}{
		{
			querySpec:            &metrics_query.QuerySpec{},
			citadelQuery:         "",
			errMsg:               "rpc error: code = InvalidArgument desc = kind required",
			clusterCheckResponse: nil,
			executeQueryResponse: nil,
			queryResponse:        nil,
		},
		{
			querySpec: &metrics_query.QuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "Node",
				},
				ObjectSelector: metrics_query.ObjectSelector{
					Tenant: "default",
				},
				Function: "none",
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
			querySpec: &metrics_query.QuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: "Node",
				},
				ObjectSelector: metrics_query.ObjectSelector{
					Name:      "testName",
					Tenant:    "testTenant",
					Namespace: "testNamespace",
					Selector: &labels.Selector{
						Requirements: []*labels.Requirement{
							&labels.Requirement{
								Key:      "meta.name",
								Operator: "equals",
								Values:   []string{"test"},
							},
						},
					},
				},
				Function: "none",
			},
			citadelQuery: "SELECT * FROM Node WHERE \"meta.name\" = 'test'",
			errMsg:       "",
			clusterCheckResponse: &ClusterCheckResponse{
				err: nil,
			},
			executeQueryResponse: &ExecuteQueryResponse{
				qr: []*query.Result{
					&query.Result{
						StatementID: 1,
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
				},
				err: nil,
			},
			queryResponse: &metrics_query.QueryResponse{
				Results: []*metrics_query.QueryResult{
					&metrics_query.QueryResult{
						StatementID: 1,
						Series: []*metrics_query.ResultSeries{
							&metrics_query.ResultSeries{
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
				},
				ObjectSelector: metrics_query.ObjectSelector{
					Name:      "testName",
					Tenant:    "testTenant",
					Namespace: "testNamespace",
					Selector: &labels.Selector{
						Requirements: []*labels.Requirement{
							&labels.Requirement{
								Key:      "meta.name",
								Operator: "equals",
								Values:   []string{"test"},
							},
						},
					},
				},
			},
		},
	}

	ctx := context.Background()

	for _, i := range testQs {
		if i.clusterCheckResponse != nil {
			mockBroker.EXPECT().ClusterCheck().Return(i.clusterCheckResponse.err)
		}
		if i.executeQueryResponse != nil {
			mockBroker.EXPECT().ExecuteQuery(ctx, i.querySpec.Tenant, i.citadelQuery).Return(i.executeQueryResponse.qr, i.executeQueryResponse.err)
		}

		res, err := srv.Query(context.Background(), i.querySpec)
		if i.errMsg == "" {
			AssertOk(t, err, "Query returned unexpected error %v", err)
		} else {
			Assert(t, err.Error() == i.errMsg, "Expected query to return error %s but got err %s", i.errMsg, err.Error())
		}
		Assert(t, reflect.DeepEqual(res, i.queryResponse), "Query response did not match expected response, expected %v, but got %v", i.queryResponse, res)
	}
}
