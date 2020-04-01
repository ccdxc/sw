package fwlog

import (
	"context"
	"fmt"
	"reflect"
	"sort"
	"testing"
	"time"

	"github.com/gogo/protobuf/types"
	uuid "github.com/satori/go.uuid"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/fwlog"
	. "github.com/pensando/sw/test/utils"
	"github.com/pensando/sw/venice/globals"
	. "github.com/pensando/sw/venice/utils/authn/testutils"
	"github.com/pensando/sw/venice/utils/elastic"
	. "github.com/pensando/sw/venice/utils/testutils"
)

const (
	ipPrefix = "192.168.10"
	srcPort  = 10000
	destPort = 10001
	ruleID   = 1
	sessID   = 2
)

func TestFwLogQuery(t *testing.T) {
	ti := tInfo{}
	err := ti.setupElastic()
	AssertOk(t, err, "setupElastic failed")
	defer ti.teardownElastic()
	err = ti.startSpyglass()
	AssertOk(t, err, "failed to start spyglass")
	defer ti.fdr.Stop()
	err = ti.startAPIServer()
	AssertOk(t, err, "failed to start API server")
	defer ti.apiServer.Stop()
	err = ti.startAPIGateway()
	AssertOk(t, err, "failed to start API Gateway")
	defer ti.apiGw.Stop()

	adminCred := &auth.PasswordCredential{
		Username: testUser,
		Password: testPassword,
		Tenant:   globals.DefaultTenant,
	}
	// create default tenant and global admin user
	if err := SetupAuth(ti.apiServerAddr, true, nil, nil, adminCred, ti.logger); err != nil {
		t.Fatalf("auth setupElastic failed")
	}
	defer CleanupAuth(ti.apiServerAddr, true, false, adminCred, ti.logger)
	superAdminCtx, err := NewLoggedInContext(context.TODO(), ti.apiGwAddr, adminCred)
	AssertOk(t, err, "error creating logged in context")

	createFwLogs(t, &ti, 254)
	fwlogURL := fmt.Sprintf("https://%s/fwlog/v1/query", ti.apiGwAddr)
	tests := []struct {
		name         string
		query        *fwlog.FwLogQuery
		expectedHits int
		out          []*fwlog.FwLog
		err          error
	}{
		{
			name: "query by destination ip",
			query: &fwlog.FwLogQuery{
				DestIPs:    []string{fmt.Sprintf("%s.%d", ipPrefix, 1)},
				MaxResults: 50,
				Tenants:    []string{globals.DefaultTenant},
			},
			expectedHits: 1,
			out: []*fwlog.FwLog{
				{
					TypeMeta: api.TypeMeta{
						Kind: auth.Permission_FwLog.String(),
					},
					ObjectMeta: api.ObjectMeta{
						Tenant: globals.DefaultTenant,
					},
					SrcIP:      fmt.Sprintf("%s.%d", ipPrefix, 0),
					DestIP:     fmt.Sprintf("%s.%d", ipPrefix, 1),
					SrcPort:    srcPort,
					DestPort:   destPort,
					Protocol:   "tcp",
					Action:     "allow",
					Direction:  "from-host",
					RuleID:     ruleID,
					SessionID:  sessID,
					FlowAction: "create",
				},
			},
			err: nil,
		},
		{
			name: "query by source ip",
			query: &fwlog.FwLogQuery{
				SourceIPs:  []string{fmt.Sprintf("%s.%d", ipPrefix, 0)},
				MaxResults: 50,
				Tenants:    []string{globals.DefaultTenant},
			},
			expectedHits: 1,
			out: []*fwlog.FwLog{
				{
					TypeMeta: api.TypeMeta{
						Kind: auth.Permission_FwLog.String(),
					},
					ObjectMeta: api.ObjectMeta{
						Tenant: globals.DefaultTenant,
					},
					SrcIP:      fmt.Sprintf("%s.%d", ipPrefix, 0),
					DestIP:     fmt.Sprintf("%s.%d", ipPrefix, 1),
					SrcPort:    srcPort,
					DestPort:   destPort,
					Protocol:   "tcp",
					Action:     "allow",
					Direction:  "from-host",
					RuleID:     ruleID,
					SessionID:  sessID,
					FlowAction: "create",
				},
			},
			err: nil,
		},
		{
			name: "query by only destination port",
			query: &fwlog.FwLogQuery{
				DestPorts:  []uint32{destPort},
				MaxResults: 50,
				Tenants:    []string{globals.DefaultTenant},
			},
			expectedHits: 0,
			err:          fmt.Errorf("POST request failed with http status code (400) for fwlog query (%s)", fwlogURL),
		},
		{
			name: "query by only source port",
			query: &fwlog.FwLogQuery{
				SourcePorts: []uint32{srcPort},
				MaxResults:  50,
				Tenants:     []string{globals.DefaultTenant},
			},
			expectedHits: 0,
			err:          fmt.Errorf("POST request failed with http status code (400) for fwlog query (%s)", fwlogURL),
		},
		{
			name: "query by destination and source ips and ports",
			query: &fwlog.FwLogQuery{
				DestIPs:     []string{fmt.Sprintf("%s.%d", ipPrefix, 2)},
				SourceIPs:   []string{fmt.Sprintf("%s.%d", ipPrefix, 1)},
				SourcePorts: []uint32{srcPort},
				DestPorts:   []uint32{destPort},
				MaxResults:  50,
				Tenants:     []string{globals.DefaultTenant},
			},
			expectedHits: 1,
			out: []*fwlog.FwLog{
				{
					TypeMeta: api.TypeMeta{
						Kind: auth.Permission_FwLog.String(),
					},
					ObjectMeta: api.ObjectMeta{
						Tenant: globals.DefaultTenant,
					},
					SrcIP:      fmt.Sprintf("%s.%d", ipPrefix, 1),
					DestIP:     fmt.Sprintf("%s.%d", ipPrefix, 2),
					SrcPort:    srcPort,
					DestPort:   destPort,
					Protocol:   "tcp",
					Action:     "allow",
					Direction:  "from-host",
					RuleID:     ruleID,
					SessionID:  sessID,
					FlowAction: "create",
				},
			},
			err: nil,
		},
		{
			name: "query by source IP and action",
			query: &fwlog.FwLogQuery{
				SourceIPs:  []string{fmt.Sprintf("%s.%d", ipPrefix, 0)},
				Actions:    []string{fwlog.FwLogActions_allow.String()},
				MaxResults: 50,
				Tenants:    []string{globals.DefaultTenant},
			},
			expectedHits: 1,
			out: []*fwlog.FwLog{
				{
					TypeMeta: api.TypeMeta{
						Kind: auth.Permission_FwLog.String(),
					},
					ObjectMeta: api.ObjectMeta{
						Tenant: globals.DefaultTenant,
					},
					SrcIP:      fmt.Sprintf("%s.%d", ipPrefix, 0),
					DestIP:     fmt.Sprintf("%s.%d", ipPrefix, 1),
					SrcPort:    srcPort,
					DestPort:   destPort,
					Protocol:   "tcp",
					Action:     "allow",
					Direction:  "from-host",
					RuleID:     ruleID,
					SessionID:  sessID,
					FlowAction: "create",
				},
			},
			err: nil,
		},
		{
			name: "query by only action",
			query: &fwlog.FwLogQuery{
				Actions:    []string{fwlog.FwLogActions_allow.String()},
				MaxResults: 50,
				Tenants:    []string{globals.DefaultTenant},
			},
			expectedHits: 0,
			err:          fmt.Errorf("POST request failed with http status code (400) for fwlog query (%s)", fwlogURL),
		},
		{
			name: "query by action- no match",
			query: &fwlog.FwLogQuery{
				DestIPs:    []string{fmt.Sprintf("%s.%d", ipPrefix, 2)},
				Actions:    []string{fwlog.FwLogActions_deny.String()},
				MaxResults: 50,
				Tenants:    []string{globals.DefaultTenant},
			},
			expectedHits: 0,
			err:          nil,
		},
		{
			name: "query by source IP and protocol",
			query: &fwlog.FwLogQuery{
				SourceIPs:  []string{fmt.Sprintf("%s.%d", ipPrefix, 0)},
				Protocols:  []string{"tcp"},
				MaxResults: 50,
				Tenants:    []string{globals.DefaultTenant},
			},
			expectedHits: 1,
			out: []*fwlog.FwLog{
				{
					TypeMeta: api.TypeMeta{
						Kind: auth.Permission_FwLog.String(),
					},
					ObjectMeta: api.ObjectMeta{
						Tenant: globals.DefaultTenant,
					},
					SrcIP:      fmt.Sprintf("%s.%d", ipPrefix, 0),
					DestIP:     fmt.Sprintf("%s.%d", ipPrefix, 1),
					SrcPort:    srcPort,
					DestPort:   destPort,
					Protocol:   "tcp",
					Action:     "allow",
					Direction:  "from-host",
					RuleID:     ruleID,
					SessionID:  sessID,
					FlowAction: "create",
				},
			},
			err: nil,
		},
		{
			name: "query by only protocol",
			query: &fwlog.FwLogQuery{
				Protocols:  []string{"tcp"},
				MaxResults: 50,
				Tenants:    []string{globals.DefaultTenant},
			},
			expectedHits: 0,
			err:          fmt.Errorf("POST request failed with http status code (400) for fwlog query (%s)", fwlogURL),
		},
		{
			name: "query by ip and protocol- no match",
			query: &fwlog.FwLogQuery{
				DestIPs:    []string{fmt.Sprintf("%s.%d", ipPrefix, 2)},
				Protocols:  []string{"udp"},
				MaxResults: 50,
				Tenants:    []string{globals.DefaultTenant},
			},
			expectedHits: 0,
			err:          nil,
		},
		{
			name: "query by timestamp",
			query: &fwlog.FwLogQuery{
				StartTime: func() *api.Timestamp {
					ts, _ := types.TimestampProto(time.Now().Add(-60 * time.Minute))
					return &api.Timestamp{Timestamp: *ts}
				}(),
				EndTime: func() *api.Timestamp {
					ts, _ := types.TimestampProto(time.Now())
					return &api.Timestamp{Timestamp: *ts}
				}(),
				MaxResults: 50,
				Tenants:    []string{globals.DefaultTenant},
			},
			expectedHits: 50,
			err:          nil,
		},
		{
			name: "query by timestamp- no match",
			query: &fwlog.FwLogQuery{
				StartTime: func() *api.Timestamp {
					ts, _ := types.TimestampProto(time.Now())
					return &api.Timestamp{Timestamp: *ts}
				}(),
				EndTime: func() *api.Timestamp {
					ts, _ := types.TimestampProto(time.Now().Add(60 * time.Minute))
					return &api.Timestamp{Timestamp: *ts}
				}(),
				MaxResults: 50,
				Tenants:    []string{globals.DefaultTenant},
			},
			expectedHits: 0,
			err:          nil,
		},
		{
			name: "query all fwlogs",
			query: &fwlog.FwLogQuery{
				MaxResults: 50,
				Tenants:    []string{globals.DefaultTenant},
			},
			expectedHits: 50,
			err:          nil,
		},
	}
	for _, test := range tests {
		var err error
		resp := fwlog.FwLogList{}
		AssertEventually(t, func() (bool, interface{}) {
			err = FwLogQuery(superAdminCtx, ti.apiGwAddr, test.query, &resp)
			if err != nil {
				if reflect.DeepEqual(test.err, err) {
					return true, nil
				}
				return false, err
			}
			if len(resp.Items) == 0 && test.expectedHits != 0 {
				return false, fmt.Errorf("no fw logs")
			}
			return true, nil
		}, fmt.Sprintf("test [%s] failed", test.name), "1s", "6s")
		if test.err == nil {
			Assert(t, len(resp.Items) == test.expectedHits, fmt.Sprintf("test [%s] failed, expected fwlog count:%d, got: %d", test.name, test.expectedHits, len(resp.Items)))
			sortFwLogs(test.out)
			sortFwLogs(resp.Items)
			for i := 0; i < len(test.out); i++ {
				Assert(t, verifyFwLog(test.out[i], resp.Items[i]), fmt.Sprintf("test [%s] failed, expected: %#v, got: %#v", test.name, test.out[i], resp.Items[i]))
			}
		}

	}

}

func createFwLogs(t *testing.T, ti *tInfo, count int) {
	var fwlogs []*elastic.BulkRequest
	for i := 0; i < count; i++ {
		creationTime, _ := types.TimestampProto(time.Now())
		obj := fwlog.FwLog{
			TypeMeta: api.TypeMeta{
				Kind: auth.Permission_FwLog.String(),
			},
			ObjectMeta: api.ObjectMeta{
				UUID:   fmt.Sprintf("%d", i),
				Tenant: globals.DefaultTenant,
				CreationTime: api.Timestamp{
					Timestamp: *creationTime,
				},
				ModTime: api.Timestamp{
					Timestamp: *creationTime,
				},
			},
			SrcIP:      fmt.Sprintf("%s.%d", ipPrefix, i),
			DestIP:     fmt.Sprintf("%s.%d", ipPrefix, i+1),
			SrcPort:    srcPort,
			DestPort:   destPort,
			Protocol:   "tcp",
			Action:     "allow",
			Direction:  "from-host",
			RuleID:     ruleID,
			SessionID:  sessID,
			FlowAction: "create",
		}

		// prepare the index request
		request := &elastic.BulkRequest{
			RequestType: elastic.Index,
			Index:       elastic.GetIndex(globals.FwLogs, globals.DefaultTenant),
			IndexType:   elastic.GetDocType(globals.FwLogs),
			ID:          uuid.NewV4().String(),
			Obj:         obj, // req.object
		}

		fwlogs = append(fwlogs, request)
	}
	_, err := ti.esClient.Bulk(context.TODO(), fwlogs)
	AssertOk(t, err, "error creating fw logs in elastic")
}

func verifyFwLog(expected, got *fwlog.FwLog) bool {
	if expected == got {
		return true
	}
	if expected.SrcIP == got.SrcIP &&
		expected.SrcPort == got.SrcPort &&
		expected.DestIP == got.DestIP &&
		expected.DestPort == got.DestPort &&
		expected.Protocol == got.Protocol &&
		expected.Action == got.Action {
		return true
	}
	return false
}

func sortFwLogs(fwlogs []*fwlog.FwLog) {
	sort.Slice(fwlogs, func(i, j int) bool {
		return fwlogs[i].UUID < fwlogs[j].UUID
	})
}
