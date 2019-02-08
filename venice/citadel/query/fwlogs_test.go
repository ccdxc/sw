package query

import (
	"context"
	"errors"
	"fmt"
	"reflect"
	"strings"
	"testing"

	"github.com/golang/mock/gomock"

	"github.com/influxdata/influxdb/models"

	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"

	"github.com/influxdata/influxdb/query"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/telemetry_query"
	"github.com/pensando/sw/venice/citadel/broker/mock"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestValidateFwlogsQuery(t *testing.T) {
	q := &Server{
		grpcSrv: nil,
		broker:  nil,
	}
	testQs := []struct {
		ql      *telemetry_query.FwlogsQueryList
		errMsg  string
		errCode codes.Code
		pass    bool
	}{
		{
			ql:      nil,
			errMsg:  "[tenant required, query required]",
			errCode: codes.InvalidArgument,
		},
		{
			ql:      &telemetry_query.FwlogsQueryList{},
			errMsg:  "[tenant required, query required]",
			errCode: codes.InvalidArgument,
		},
		{
			ql: &telemetry_query.FwlogsQueryList{
				Queries: []*telemetry_query.FwlogsQuerySpec{
					&telemetry_query.FwlogsQuerySpec{
						SourceIPs: []string{"10"},
					},
				},
			},
			errMsg:  "[tenant required, Queries[0].SourceIPs[0] validation failed]",
			errCode: codes.InvalidArgument,
		},
		{
			ql: &telemetry_query.FwlogsQueryList{
				Tenant: "tenant",
				Queries: []*telemetry_query.FwlogsQuerySpec{
					&telemetry_query.FwlogsQuerySpec{
						SourceIPs: []string{"10"},
					},
				},
			},
			errMsg:  "[Queries[0].SourceIPs[0] validation failed]",
			errCode: codes.InvalidArgument,
		},
		{
			ql: &telemetry_query.FwlogsQueryList{
				Tenant: "tenant",
				Queries: []*telemetry_query.FwlogsQuerySpec{
					&telemetry_query.FwlogsQuerySpec{
						SourceIPs: []string{"10.1.1.1"},
					},
				},
			},
			pass: true,
		},
	}
	for _, i := range testQs {
		err := q.validateFwlogsQueryList(i.ql)
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

func TestBuildCitadelFwlogsQuery(t *testing.T) {
	startTime := &api.Timestamp{}
	startTime.Parse("2018-11-09T23:16:17Z")
	endTime := &api.Timestamp{}
	endTime.Parse("2018-11-09T23:22:17Z")
	testQs := []struct {
		qs   *telemetry_query.FwlogsQuerySpec
		resp string
		pass bool
	}{
		{
			qs:   &telemetry_query.FwlogsQuerySpec{},
			resp: `SELECT * FROM Fwlogs`,
			pass: true,
		},
		{
			qs: &telemetry_query.FwlogsQuerySpec{
				SourceIPs: []string{"10.1.1.10"},
			},
			resp: `SELECT * FROM Fwlogs WHERE ("src" = '10.1.1.10')`,
			pass: true,
		},
		{
			qs: &telemetry_query.FwlogsQuerySpec{
				SourceIPs: []string{"10.1.1.10", "10.1.1.11", "10.1.1.12"},
			},
			resp: `SELECT * FROM Fwlogs WHERE ("src" = '10.1.1.10' OR "src" = '10.1.1.11' OR "src" = '10.1.1.12')`,
			pass: true,
		},
		{
			qs: &telemetry_query.FwlogsQuerySpec{
				SourceIPs: []string{"10.1.1.10", "10.1.1.11", "10.1.1.12"},
				DestIPs:   []string{"11.1.1.10", "11.1.1.11", "11.1.1.12"},
			},
			resp: `SELECT * FROM Fwlogs WHERE ("src" = '10.1.1.10' OR "src" = '10.1.1.11' OR "src" = '10.1.1.12') AND ("dest" = '11.1.1.10' OR "dest" = '11.1.1.11' OR "dest" = '11.1.1.12')`,
			pass: true,
		},
		{
			qs: &telemetry_query.FwlogsQuerySpec{
				SourceIPs:   []string{"10.1.1.10", "10.1.1.11", "10.1.1.12"},
				SourcePorts: []uint32{8000, 9000},
				DestPorts:   []uint32{6000, 7000},
			},
			resp: `SELECT * FROM Fwlogs WHERE ("src" = '10.1.1.10' OR "src" = '10.1.1.11' OR "src" = '10.1.1.12') AND ("src-port" = '8000' OR "src-port" = '9000') AND ("dest-port" = '6000' OR "dest-port" = '7000')`,
			pass: true,
		},
		{
			qs: &telemetry_query.FwlogsQuerySpec{
				SourceIPs: []string{"10.1.1.10", "10.1.1.11", "10.1.1.12"},
				Actions:   []string{"ALL"},
			},
			resp: `SELECT * FROM Fwlogs WHERE ("src" = '10.1.1.10' OR "src" = '10.1.1.11' OR "src" = '10.1.1.12')`,
			pass: true,
		},
		{
			qs: &telemetry_query.FwlogsQuerySpec{
				SourceIPs: []string{"10.1.1.10", "10.1.1.11", "10.1.1.12"},
				Actions:   []string{"ACTION_DENY", "ALL"},
			},
			resp: `SELECT * FROM Fwlogs WHERE ("src" = '10.1.1.10' OR "src" = '10.1.1.11' OR "src" = '10.1.1.12')`,
			pass: true,
		},
		{
			qs: &telemetry_query.FwlogsQuerySpec{
				SourceIPs: []string{"10.1.1.10", "10.1.1.11", "10.1.1.12"},
				Actions:   []string{"ACTION_DENY", "ACTION_REJECT"},
			},
			resp: `SELECT * FROM Fwlogs WHERE ("src" = '10.1.1.10' OR "src" = '10.1.1.11' OR "src" = '10.1.1.12') AND ("action" = 'SECURITY_RULE_ACTION_DENY' OR "action" = 'SECURITY_RULE_ACTION_REJECT')`,
			pass: true,
		},
		{
			qs: &telemetry_query.FwlogsQuerySpec{
				SourceIPs:  []string{"10.1.1.10", "10.1.1.11", "10.1.1.12"},
				Directions: []string{"DIRECTION_ALL", "DIRECTION_FROM_HOST"},
			},
			resp: `SELECT * FROM Fwlogs WHERE ("src" = '10.1.1.10' OR "src" = '10.1.1.11' OR "src" = '10.1.1.12')`,
			pass: true,
		},
		{
			qs: &telemetry_query.FwlogsQuerySpec{
				SourceIPs:  []string{"10.1.1.10", "10.1.1.11", "10.1.1.12"},
				Directions: []string{"DIRECTION_FROM_UPLINK", "DIRECTION_FROM_HOST"},
			},
			resp: `SELECT * FROM Fwlogs WHERE ("src" = '10.1.1.10' OR "src" = '10.1.1.11' OR "src" = '10.1.1.12') AND ("direction" = 'FLOW_DIRECTION_FROM_UPLINK' OR "direction" = 'FLOW_DIRECTION_FROM_HOST')`,
			pass: true,
		},
		{
			qs: &telemetry_query.FwlogsQuerySpec{
				SourceIPs: []string{"10.1.1.10", "10.1.1.11", "10.1.1.12"},
				StartTime: startTime,
				EndTime:   endTime,
			},
			resp: `SELECT * FROM Fwlogs WHERE ("src" = '10.1.1.10' OR "src" = '10.1.1.11' OR "src" = '10.1.1.12') AND time > '2018-11-09T23:16:17Z' AND time < '2018-11-09T23:22:17Z'`,
			pass: true,
		},
	}

	for _, i := range testQs {
		resp, err := buildCitadelFwlogsQuery(i.qs)
		if i.pass {
			AssertOk(t, err, fmt.Sprintf("failed to build query %+v", i.qs))
			Assert(t, resp == i.resp, fmt.Sprintf("query didn't match, got:{%s} expected: {%s}", resp, i.resp))
		} else {
			Assert(t, err != nil, fmt.Sprintf("build query didn't fail %+v", i.qs))
		}
	}
}

func TestExecuteFwlogsQuery(t *testing.T) {

	mockCtrl := gomock.NewController(t)
	defer mockCtrl.Finish()
	mockBroker := mock.NewMockInf(mockCtrl)
	srv := Server{
		broker: mockBroker,
	}

	timestamp := &api.Timestamp{}
	timestamp.Parse("2018-11-09T23:16:17Z")

	testQs := []struct {
		executeQueryResp *ExecuteQueryResponse
		exp              []*telemetry_query.FwlogsQueryResult
		pass             bool
	}{
		{
			executeQueryResp: &ExecuteQueryResponse{
				err: errors.New("Query failed"),
			},
			pass: false,
		},
		{
			executeQueryResp: &ExecuteQueryResponse{
				qr: []*query.Result{
					&query.Result{
						StatementID: 0,
						Series: models.Rows{
							&models.Row{
								// Column doesn't have all of the fwlog fields
								Columns: []string{"time", "src", "dest", "action", "direction"},
								Values: [][]interface{}{
									[]interface{}{
										"2018-11-09T23:16:17Z", "10.1.1.1", "10.1.1.2", "SECURITY_RULE_ACTION_ALLOW", "FLOW_DIRECTION_FROM_HOST",
									},
									[]interface{}{
										"2018-11-09T23:16:17Z", "10.1.1.1", "10.1.1.2", "SECURITY_RULE_ACTION_DENY", "FLOW_DIRECTION_FROM_UPLINK",
									},
									[]interface{}{
										"2018-11-09T23:16:17Z", "10.1.1.1", "10.1.1.2", "SECURITY_RULE_ACTION_REJECT", "FLOW_DIRECTION_FROM_UPLINK",
									},
								},
							},
						},
					},
					&query.Result{
						// Statement ID should be preserved in results
						StatementID: 0,
						Series: models.Rows{
							&models.Row{
								// Column has extra unknown fields
								Columns: []string{"time", "src", "dest", "randomField", "action", "direction"},
								Values: [][]interface{}{
									[]interface{}{
										"2018-11-09T23:16:17Z", "10.1.1.3", "10.1.1.4", "random", "UNKNOWN", "UNKNOWN",
									},
								},
							},
						},
					},
					&query.Result{
						// Statement ID should be preserved in results
						StatementID: 1,
						Series: models.Rows{
							&models.Row{
								Columns: []string{"time", "src", "dest"},
								Values: [][]interface{}{
									[]interface{}{
										"2018-11-09T23:16:17Z", "10.1.1.3", "10.1.1.4",
									},
								},
							},
						},
					},
				},
				err: nil,
			},
			exp: []*telemetry_query.FwlogsQueryResult{
				&telemetry_query.FwlogsQueryResult{
					StatementID: 0,
					Logs: []*telemetry_query.Fwlog{
						&telemetry_query.Fwlog{
							Src:       "10.1.1.1",
							Dest:      "10.1.1.2",
							Action:    "ALLOW",
							Direction: "FROM_HOST",
							Timestamp: timestamp,
						},
						&telemetry_query.Fwlog{
							Src:       "10.1.1.1",
							Dest:      "10.1.1.2",
							Action:    "DENY",
							Direction: "FROM_UPLINK",
							Timestamp: timestamp,
						},
						&telemetry_query.Fwlog{
							Src:       "10.1.1.1",
							Dest:      "10.1.1.2",
							Action:    "REJECT",
							Direction: "FROM_UPLINK",
							Timestamp: timestamp,
						},
					},
				},
				&telemetry_query.FwlogsQueryResult{
					StatementID: 0,
					Logs: []*telemetry_query.Fwlog{
						&telemetry_query.Fwlog{
							Src:       "10.1.1.3",
							Dest:      "10.1.1.4",
							Action:    "UNKNOWN",
							Direction: "UNKNOWN",
							Timestamp: timestamp,
						},
					},
				},
				&telemetry_query.FwlogsQueryResult{
					StatementID: 1,
					Logs: []*telemetry_query.Fwlog{
						&telemetry_query.Fwlog{
							Src:       "10.1.1.3",
							Dest:      "10.1.1.4",
							Timestamp: timestamp,
						},
					},
				},
			},
			pass: true,
		},
	}
	for index, i := range testQs {
		ctx := context.Background()
		mockBroker.EXPECT().ExecuteQuery(ctx, "tenant", "query_string").Return(i.executeQueryResp.qr, i.executeQueryResp.err)

		resp, err := srv.executeFwlogsQuery(ctx, "tenant", "query_string")
		if i.pass {
			AssertOk(t, err, fmt.Sprintf("execute query failed for test case %d %+v", index, err))
			Assert(t, reflect.DeepEqual(resp, i.exp), fmt.Sprintf("resp didn't match exp, got:{%s} expected: {%s}", resp, i.exp))
		} else {
			Assert(t, err != nil, fmt.Sprintf("build query didn't fail %+v", index))
		}
	}
}

func TestFwlogs(t *testing.T) {
	startTime := &api.Timestamp{}
	startTime.Parse("2018-11-09T23:15:17Z")
	endTime := &api.Timestamp{}
	endTime.Parse("2018-11-09T23:20:17Z")

	mockCtrl := gomock.NewController(t)
	defer mockCtrl.Finish()
	mockBroker := mock.NewMockInf(mockCtrl)
	srv, err := NewQueryService(testServerURL, mockBroker)
	if err != nil {
		t.Errorf("Recieved unexpected error from NewQueryService: %v", err)
	}
	defer srv.Stop()

	testQs := []struct {
		queryList   *telemetry_query.FwlogsQueryList
		brokerQuery string
		errMsg      string
		// If clusterCheckResponse is not nil, it will mock the given value
		clusterCheckResponse *ClusterCheckResponse
		// If executeQueryResponse is not nil, it will mock the given value
		executeQueryResponse *ExecuteQueryResponse
		queryResponse        *telemetry_query.FwlogsQueryResponse
	}{
		{
			queryList: &telemetry_query.FwlogsQueryList{
				Queries: []*telemetry_query.FwlogsQuerySpec{
					&telemetry_query.FwlogsQuerySpec{},
				},
			},
			brokerQuery:          "",
			errMsg:               "tenant required",
			clusterCheckResponse: nil,
			executeQueryResponse: nil,
			queryResponse:        nil,
		},
		{
			queryList: &telemetry_query.FwlogsQueryList{
				Tenant: "test",
				Queries: []*telemetry_query.FwlogsQuerySpec{
					&telemetry_query.FwlogsQuerySpec{},
				},
			},
			brokerQuery: "",
			errMsg:      "Cluster check failed",
			clusterCheckResponse: &ClusterCheckResponse{
				err: errors.New("Cluster check failed"),
			},
			executeQueryResponse: nil,
			queryResponse:        nil,
		},
		{
			queryList: &telemetry_query.FwlogsQueryList{
				Tenant: "test",
				// Using all filter options
				Queries: []*telemetry_query.FwlogsQuerySpec{
					&telemetry_query.FwlogsQuerySpec{
						SourceIPs:   []string{"10.1.1.1"},
						DestIPs:     []string{"10.1.1.2"},
						SourcePorts: []uint32{8000},
						DestPorts:   []uint32{9000},
						Protocols:   []string{"TCP"},
						Actions:     []string{"ACTION_DENY"},
						Directions:  []string{"DIRECTION_FROM_HOST"},
						RuleIDs:     []string{"1234"},
						// Policy name will be ignored since
						// there is no matching tsdb tag
						PolicyNames: []string{"policy1"},
						StartTime:   startTime,
						EndTime:     endTime,
					},
				},
			},
			brokerQuery: `SELECT * FROM Fwlogs WHERE ("src" = '10.1.1.1') AND ("dest" = '10.1.1.2') AND ("src-port" = '8000') AND ("dest-port" = '9000') AND ("protocol" = 'TCP') AND ("action" = 'SECURITY_RULE_ACTION_DENY') AND ("direction" = 'FLOW_DIRECTION_FROM_HOST') AND ("rule-id" = '1234') AND time > '2018-11-09T23:15:17Z' AND time < '2018-11-09T23:20:17Z'`,
			errMsg:      "",
			clusterCheckResponse: &ClusterCheckResponse{
				err: nil,
			},
			executeQueryResponse: &ExecuteQueryResponse{
				qr: []*query.Result{
					&query.Result{
						// Statement ID should be preserved in results
						StatementID: 0,
						Series: models.Rows{
							&models.Row{
								Columns: []string{"time", "src", "dest", "src-port", "dest-port", "protocol", "action", "direction", "rule-id"},
								Values: [][]interface{}{
									[]interface{}{
										"2018-11-09T23:20:17Z", "10.1.1.1", "10.1.1.2", 8000, 9000, "TCP", "DENY", "FROM_HOST", "1234",
									},
								},
							},
						},
					},
				},
			},
			queryResponse: &telemetry_query.FwlogsQueryResponse{
				Tenant: "test",
				Results: []*telemetry_query.FwlogsQueryResult{

					&telemetry_query.FwlogsQueryResult{
						StatementID: 0,
						Logs: []*telemetry_query.Fwlog{
							&telemetry_query.Fwlog{
								Src:       "10.1.1.1",
								Dest:      "10.1.1.2",
								SrcPort:   8000,
								DestPort:  9000,
								Protocol:  "TCP",
								Action:    "DENY",
								Direction: "FROM_HOST",
								RuleID:    "1234",
								Timestamp: endTime,
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
			mockBroker.EXPECT().ExecuteQuery(ctx, i.queryList.Tenant, i.brokerQuery).Return(i.executeQueryResponse.qr, i.executeQueryResponse.err)
		}

		res, err := srv.Fwlogs(context.Background(), i.queryList)
		if i.errMsg == "" {
			AssertOk(t, err, "Query returned unexpected error %v", err)
		} else {
			Assert(t, strings.Contains(err.Error(), i.errMsg), "Expected query to return error that contains %s but got err %s", i.errMsg, err.Error())
		}
		Assert(t, reflect.DeepEqual(res, i.queryResponse), "Query response did not match expected response, expected %v, but got %v", i.queryResponse, res)
	}

}
