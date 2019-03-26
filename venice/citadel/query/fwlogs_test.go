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
		desc    string
	}{
		{
			desc:    "nil query",
			ql:      nil,
			errMsg:  "[tenant required, query required]",
			errCode: codes.InvalidArgument,
		},
		{
			desc:    "blank query",
			ql:      &telemetry_query.FwlogsQueryList{},
			errMsg:  "[tenant required, query required]",
			errCode: codes.InvalidArgument,
		},
		{
			desc: "missing tenant and invalid source ip",
			ql: &telemetry_query.FwlogsQueryList{
				Queries: []*telemetry_query.FwlogsQuerySpec{
					&telemetry_query.FwlogsQuerySpec{
						SourceIPs: []string{"10"},
						SortOrder: telemetry_query.SortOrder_Descending.String(),
					},
				},
			},
			errMsg:  "[tenant required, Queries[0].SourceIPs[0] failed validation: Value must be a valid IP in dot notation]",
			errCode: codes.InvalidArgument,
		},
		{
			desc: "invalid source ip",
			ql: &telemetry_query.FwlogsQueryList{
				Tenant: "tenant",
				Queries: []*telemetry_query.FwlogsQuerySpec{
					&telemetry_query.FwlogsQuerySpec{
						SourceIPs: []string{"10"},
						SortOrder: telemetry_query.SortOrder_Descending.String(),
					},
				},
			},
			errMsg:  "[Queries[0].SourceIPs[0] failed validation: Value must be a valid IP in dot notation]",
			errCode: codes.InvalidArgument,
		},
		{
			desc: "valid query with tenant and src ip",
			ql: &telemetry_query.FwlogsQueryList{
				Tenant: "tenant",
				Queries: []*telemetry_query.FwlogsQuerySpec{
					&telemetry_query.FwlogsQuerySpec{
						SourceIPs: []string{"10.1.1.1"},
						SortOrder: telemetry_query.SortOrder_Descending.String(),
					},
				},
			},
			pass: true,
		},
		{
			desc: "Pagination spec with no count",
			ql: &telemetry_query.FwlogsQueryList{
				Tenant: "tenant",
				Queries: []*telemetry_query.FwlogsQuerySpec{
					&telemetry_query.FwlogsQuerySpec{
						SourceIPs:  []string{"10.1.1.1"},
						Pagination: &telemetry_query.PaginationSpec{},
						SortOrder:  telemetry_query.SortOrder_Descending.String(),
					},
				},
			},
			errMsg:  "[Queries[0].Pagination.Count failed validation: Value must be at least 1]",
			errCode: codes.InvalidArgument,
		},
		{
			desc: "Pagination spec with negative count",
			ql: &telemetry_query.FwlogsQueryList{
				Tenant: "tenant",
				Queries: []*telemetry_query.FwlogsQuerySpec{
					&telemetry_query.FwlogsQuerySpec{
						SourceIPs: []string{"10.1.1.1"},
						Pagination: &telemetry_query.PaginationSpec{
							Count: -15,
						},
						SortOrder: telemetry_query.SortOrder_Descending.String(),
					},
				},
			},
			errMsg:  "[Queries[0].Pagination.Count failed validation: Value must be at least 1]",
			errCode: codes.InvalidArgument,
		},
		{
			desc: "Pagination spec with negative offset",
			ql: &telemetry_query.FwlogsQueryList{
				Tenant: "tenant",
				Queries: []*telemetry_query.FwlogsQuerySpec{
					&telemetry_query.FwlogsQuerySpec{
						SourceIPs: []string{"10.1.1.1"},
						Pagination: &telemetry_query.PaginationSpec{
							Count:  100,
							Offset: -15,
						},
						SortOrder: telemetry_query.SortOrder_Descending.String(),
					},
				},
			},
			errMsg:  "[Queries[0].Pagination.Offset failed validation: Value must be at least 0]",
			errCode: codes.InvalidArgument,
		},
		{
			desc: "query with valid pagination spec",
			ql: &telemetry_query.FwlogsQueryList{
				Tenant: "tenant",
				Queries: []*telemetry_query.FwlogsQuerySpec{
					&telemetry_query.FwlogsQuerySpec{
						SourceIPs: []string{"10.1.1.1"},
						Pagination: &telemetry_query.PaginationSpec{
							Count:  100,
							Offset: 100,
						},
						SortOrder: telemetry_query.SortOrder_Descending.String(),
					},
				},
			},
			pass: true,
		},
	}
	for _, i := range testQs {
		err := q.validateFwlogsQueryList(i.ql)
		if i.pass && err != nil {
			t.Errorf("%s: Expected error to be nil but was %v", i.desc, err)
		} else if !i.pass && err == nil {
			t.Errorf("%s: Expected test to fail but err was nil", i.desc)
		} else if err != nil {
			errStatus, ok := status.FromError(err)
			if !ok {
				t.Errorf("%s: Expected GRPC error, but was unable to parse the returned error %v", i.desc, err)
				return
			}
			if i.errMsg != errStatus.Message() {
				t.Errorf("%s: Expected error message to be %s but error was %v", i.desc, i.errMsg, errStatus.Message())
			}
			if i.errCode != errStatus.Code() {
				t.Errorf("%s: Expected error code to be %d but error was %v", i.desc, i.errCode, errStatus.Code())
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
		desc string
	}{
		{
			desc: "Empty query",
			qs:   &telemetry_query.FwlogsQuerySpec{},
			resp: `SELECT * FROM Fwlogs ORDER BY time ASC`,
			pass: true,
		},
		{
			desc: "Query with source ip",
			qs: &telemetry_query.FwlogsQuerySpec{
				SourceIPs: []string{"10.1.1.10"},
				SortOrder: telemetry_query.SortOrder_Descending.String(),
			},
			resp: `SELECT * FROM Fwlogs WHERE ("source" = '10.1.1.10') ORDER BY time DESC`,
			pass: true,
		},
		{
			desc: "Query with multiple source ips",
			qs: &telemetry_query.FwlogsQuerySpec{
				SourceIPs: []string{"10.1.1.10", "10.1.1.11", "10.1.1.12"},
				SortOrder: telemetry_query.SortOrder_Descending.String(),
			},
			resp: `SELECT * FROM Fwlogs WHERE ("source" = '10.1.1.10' OR "source" = '10.1.1.11' OR "source" = '10.1.1.12') ORDER BY time DESC`,
			pass: true,
		},
		{
			desc: "Query with multiple src and dest ips",
			qs: &telemetry_query.FwlogsQuerySpec{
				SourceIPs: []string{"10.1.1.10", "10.1.1.11", "10.1.1.12"},
				DestIPs:   []string{"11.1.1.10", "11.1.1.11", "11.1.1.12"},
				SortOrder: telemetry_query.SortOrder_Descending.String(),
			},
			resp: `SELECT * FROM Fwlogs WHERE ("source" = '10.1.1.10' OR "source" = '10.1.1.11' OR "source" = '10.1.1.12') AND ("destination" = '11.1.1.10' OR "destination" = '11.1.1.11' OR "destination" = '11.1.1.12') ORDER BY time DESC`,
			pass: true,
		},
		{
			desc: "Query with multiple ports",
			qs: &telemetry_query.FwlogsQuerySpec{
				SourceIPs:   []string{"10.1.1.10", "10.1.1.11", "10.1.1.12"},
				SourcePorts: []uint32{8000, 9000},
				DestPorts:   []uint32{6000, 7000},
				SortOrder:   telemetry_query.SortOrder_Descending.String(),
			},
			resp: `SELECT * FROM Fwlogs WHERE ("source" = '10.1.1.10' OR "source" = '10.1.1.11' OR "source" = '10.1.1.12') AND ("source-port" = '8000' OR "source-port" = '9000') AND ("destination-port" = '6000' OR "destination-port" = '7000') ORDER BY time DESC`,
			pass: true,
		},
		{
			desc: "Query with multiple actions",
			qs: &telemetry_query.FwlogsQuerySpec{
				SourceIPs: []string{"10.1.1.10", "10.1.1.11", "10.1.1.12"},
				Actions:   []string{"deny", "reject"},
				SortOrder: telemetry_query.SortOrder_Descending.String(),
			},
			resp: `SELECT * FROM Fwlogs WHERE ("source" = '10.1.1.10' OR "source" = '10.1.1.11' OR "source" = '10.1.1.12') AND ("action" = 'deny' OR "action" = 'reject') ORDER BY time DESC`,
			pass: true,
		},
		{
			desc: "Query with multiple directions",
			qs: &telemetry_query.FwlogsQuerySpec{
				SourceIPs:  []string{"10.1.1.10", "10.1.1.11", "10.1.1.12"},
				Directions: []string{"from_uplink", "from_host"},
				SortOrder:  telemetry_query.SortOrder_Descending.String(),
			},
			resp: `SELECT * FROM Fwlogs WHERE ("source" = '10.1.1.10' OR "source" = '10.1.1.11' OR "source" = '10.1.1.12') AND ("direction" = 'from_uplink' OR "direction" = 'from_host') ORDER BY time DESC`,
			pass: true,
		},
		{
			desc: "Query with time query",
			qs: &telemetry_query.FwlogsQuerySpec{
				SourceIPs: []string{"10.1.1.10", "10.1.1.11", "10.1.1.12"},
				StartTime: startTime,
				EndTime:   endTime,
				SortOrder: telemetry_query.SortOrder_Descending.String(),
			},
			resp: `SELECT * FROM Fwlogs WHERE ("source" = '10.1.1.10' OR "source" = '10.1.1.11' OR "source" = '10.1.1.12') AND time > '2018-11-09T23:16:17Z' AND time < '2018-11-09T23:22:17Z' ORDER BY time DESC`,
			pass: true,
		},
		{
			desc: "Query with pagination",
			qs: &telemetry_query.FwlogsQuerySpec{
				SourceIPs: []string{"10.1.1.10", "10.1.1.11", "10.1.1.12"},
				StartTime: startTime,
				EndTime:   endTime,
				Pagination: &telemetry_query.PaginationSpec{
					Count:  100,
					Offset: 200,
				},
				SortOrder: telemetry_query.SortOrder_Descending.String(),
			},
			resp: `SELECT * FROM Fwlogs WHERE ("source" = '10.1.1.10' OR "source" = '10.1.1.11' OR "source" = '10.1.1.12') AND time > '2018-11-09T23:16:17Z' AND time < '2018-11-09T23:22:17Z' ORDER BY time DESC LIMIT 100 OFFSET 200`,
			pass: true,
		},
		{
			desc: "Query with sorting",
			qs: &telemetry_query.FwlogsQuerySpec{
				SourceIPs: []string{"10.1.1.10", "10.1.1.11", "10.1.1.12"},
				StartTime: startTime,
				EndTime:   endTime,
				SortOrder: telemetry_query.SortOrder_Ascending.String(),
				Pagination: &telemetry_query.PaginationSpec{
					Count:  100,
					Offset: 200,
				},
			},
			resp: `SELECT * FROM Fwlogs WHERE ("source" = '10.1.1.10' OR "source" = '10.1.1.11' OR "source" = '10.1.1.12') AND time > '2018-11-09T23:16:17Z' AND time < '2018-11-09T23:22:17Z' ORDER BY time ASC LIMIT 100 OFFSET 200`,
			pass: true,
		},
	}

	for _, i := range testQs {
		resp, err := buildCitadelFwlogsQuery(i.qs)
		if i.pass {
			AssertOk(t, err, fmt.Sprintf("%s: failed to build query %+v", i.desc, i.qs))
			Assert(t, resp == i.resp, fmt.Sprintf("%s: query didn't match, got:{%s} expected: {%s}", i.desc, resp, i.resp))
		} else {
			Assert(t, err != nil, fmt.Sprintf("%s: build query didn't fail %+v", i.desc, i.qs))
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
								Columns: []string{"time", "source", "destination", "action", "direction", "reporterID"},
								Values: [][]interface{}{
									[]interface{}{
										"2018-11-09T23:16:17Z", "10.1.1.1", "10.1.1.2", "allow", "from_host", "naples1",
									},
									[]interface{}{
										"2018-11-09T23:16:17Z", "10.1.1.1", "10.1.1.2", "deny", "from_uplink", "naples2",
									},
									[]interface{}{
										"2018-11-09T23:16:17Z", "10.1.1.1", "10.1.1.2", "reject", "from_uplink", "naples3",
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
								Columns: []string{"time", "source", "destination", "randomField", "action", "direction"},
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
								Columns: []string{"time", "source", "destination"},
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
							Src:        "10.1.1.1",
							Dest:       "10.1.1.2",
							Action:     "allow",
							Direction:  "from_host",
							ReporterID: "naples1",
							Time:       timestamp,
						},
						&telemetry_query.Fwlog{
							Src:        "10.1.1.1",
							Dest:       "10.1.1.2",
							Action:     "deny",
							Direction:  "from_uplink",
							ReporterID: "naples2",
							Time:       timestamp,
						},
						&telemetry_query.Fwlog{
							Src:        "10.1.1.1",
							Dest:       "10.1.1.2",
							Action:     "reject",
							Direction:  "from_uplink",
							ReporterID: "naples3",
							Time:       timestamp,
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
							Time:      timestamp,
						},
					},
				},
				&telemetry_query.FwlogsQueryResult{
					StatementID: 1,
					Logs: []*telemetry_query.Fwlog{
						&telemetry_query.Fwlog{
							Src:  "10.1.1.3",
							Dest: "10.1.1.4",
							Time: timestamp,
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
			Assert(t, reflect.DeepEqual(resp, i.exp), fmt.Sprintf("[%d] resp didn't match exp, got:{%s} expected: {%s}", index, resp, i.exp))
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
					&telemetry_query.FwlogsQuerySpec{
						SortOrder: telemetry_query.SortOrder_Descending.String(),
					},
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
					&telemetry_query.FwlogsQuerySpec{
						SortOrder: telemetry_query.SortOrder_Descending.String(),
					},
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
						Actions:     []string{"deny"},
						Directions:  []string{"from_host"},
						RuleIDs:     []string{"1234"},
						ReporterIDs: []string{"naples1", "naples2"},
						// Policy name will be ignored since
						// there is no matching tsdb tag
						PolicyNames: []string{"policy1"},
						StartTime:   startTime,
						EndTime:     endTime,
						SortOrder:   telemetry_query.SortOrder_Descending.String(),
					},
				},
			},
			brokerQuery: `SELECT * FROM Fwlogs WHERE ("source" = '10.1.1.1') AND ("destination" = '10.1.1.2') AND ("source-port" = '8000') AND ("destination-port" = '9000') AND ("protocol" = 'TCP') AND ("action" = 'deny') AND ("direction" = 'from_host') AND ("rule-id" = '1234') AND ("reporterID" = 'naples1' OR "reporterID" = 'naples2') AND time > '2018-11-09T23:15:17Z' AND time < '2018-11-09T23:20:17Z' ORDER BY time DESC`,
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
								Columns: []string{"time", "source", "destination", "source-port", "destination-port", "protocol", "action", "direction", "rule-id", "reporterID"},
								Values: [][]interface{}{
									[]interface{}{
										"2018-11-09T23:20:17Z", "10.1.1.1", "10.1.1.2", 8000, 9000, "TCP", "deny", "from_host", "1234", "naples1",
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
								Src:        "10.1.1.1",
								Dest:       "10.1.1.2",
								SrcPort:    8000,
								DestPort:   9000,
								Protocol:   "TCP",
								Action:     "deny",
								Direction:  "from_host",
								RuleID:     "1234",
								ReporterID: "naples1",
								Time:       endTime,
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
