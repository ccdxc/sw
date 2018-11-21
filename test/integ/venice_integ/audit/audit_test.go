package audit

import (
	"context"
	"fmt"
	"testing"

	es "github.com/olivere/elastic"
	"github.com/satori/go.uuid"

	"github.com/pensando/sw/api"
	auditapi "github.com/pensando/sw/api/generated/audit"
	"github.com/pensando/sw/api/generated/auth"
	testutils "github.com/pensando/sw/test/utils"
	"github.com/pensando/sw/venice/globals"
	elasticauditor "github.com/pensando/sw/venice/utils/audit/elastic"
	auditmgr "github.com/pensando/sw/venice/utils/audit/manager"
	"github.com/pensando/sw/venice/utils/elastic"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestAuditManager(t *testing.T) {
	ti := tInfo{}
	ti.setup()
	defer ti.teardown()
	tests := []struct {
		name   string
		events []*auditapi.Event
		err    bool
		query  es.Query
		hits   int
	}{
		{
			name: "single audit event",
			events: []*auditapi.Event{
				{
					TypeMeta:   api.TypeMeta{Kind: "AuditEvent"},
					ObjectMeta: api.ObjectMeta{Name: "auditevent1", UUID: uuid.NewV4().String(), Tenant: "default"},
					EventAttributes: auditapi.EventAttributes{
						Level:       auditapi.Level_RequestResponse.String(),
						Stage:       auditapi.Stage_RequestProcessing.String(),
						User:        &api.ObjectRef{Kind: "User", Namespace: "default", Tenant: "default", Name: "admin"},
						Resource:    &api.ObjectRef{Kind: "Network", Tenant: "default", Namespace: "default", Name: "network1"},
						ClientIPs:   []string{"192.168.75.133"},
						Action:      auth.Permission_Create.String(),
						Outcome:     auditapi.Outcome_Unknown.String(),
						GatewayNode: "node1",
						GatewayIP:   "192.168.75.136",
						Data:        make(map[string]string),
					},
				},
			},
			err: false,
			query: es.NewBoolQuery().Must(es.NewTermQuery("resource.kind.keyword", "Network"),
				es.NewTermQuery("action.keyword", auth.Permission_Create.String())),
			hits: 1,
		},
		{
			name: "multiple audit events",
			events: []*auditapi.Event{
				{
					TypeMeta:   api.TypeMeta{Kind: "AuditEvent"},
					ObjectMeta: api.ObjectMeta{Name: "auditevent2", UUID: uuid.NewV4().String(), Tenant: "default"},
					EventAttributes: auditapi.EventAttributes{
						Level:       auditapi.Level_RequestResponse.String(),
						Stage:       auditapi.Stage_RequestProcessing.String(),
						User:        &api.ObjectRef{Kind: "User", Namespace: "default", Tenant: "default", Name: "admin"},
						Resource:    &api.ObjectRef{Kind: "Role", Tenant: "default", Namespace: "default", Name: "networkadmin"},
						ClientIPs:   []string{"192.168.75.133"},
						Action:      auth.Permission_Create.String(),
						Outcome:     auditapi.Outcome_Unknown.String(),
						GatewayNode: "node2",
						GatewayIP:   "192.168.75.139",
						Data:        make(map[string]string),
					},
				},
				{
					TypeMeta:   api.TypeMeta{Kind: "AuditEvent"},
					ObjectMeta: api.ObjectMeta{Name: "auditevent3", UUID: uuid.NewV4().String(), Tenant: "default"},
					EventAttributes: auditapi.EventAttributes{
						Level:       auditapi.Level_RequestResponse.String(),
						Stage:       auditapi.Stage_RequestCompleted.String(),
						User:        &api.ObjectRef{Kind: "User", Namespace: "default", Tenant: "default", Name: "admin"},
						Resource:    &api.ObjectRef{Kind: "Role", Tenant: "default", Namespace: "default", Name: "networkadmin"},
						ClientIPs:   []string{"192.168.75.133"},
						Action:      auth.Permission_Create.String(),
						Outcome:     auditapi.Outcome_Success.String(),
						GatewayNode: "node2",
						GatewayIP:   "192.168.75.139",
						Data:        make(map[string]string),
					},
				},
			},
			err: false,
			query: es.NewBoolQuery().Must(es.NewTermQuery("resource.kind.keyword", "Role"),
				es.NewTermQuery("action.keyword", auth.Permission_Create.String())),
			hits: 2,
		},
		{
			name:   "nil event slice",
			events: []*auditapi.Event{},
			err:    true,
		},
		{
			name:   "nil events",
			events: []*auditapi.Event{nil, nil},
			err:    true,
		},
	}
	auditor := auditmgr.WithAuditors(elasticauditor.NewSynchAuditor(ti.elasticSearchAddr, ti.rslvr, ti.logger, elasticauditor.WithElasticClient(ti.esClient)))
	err := auditor.Run(make(<-chan struct{}))
	AssertOk(t, err, "error starting elastic auditor")
	defer auditor.Shutdown()

	for _, test := range tests {
		err = auditor.ProcessEvents(test.events...)
		Assert(t, test.err == (err != nil), fmt.Sprintf("[%v] test failed, err: %v", test.name, err))
		if !test.err {
			AssertEventually(t, func() (bool, interface{}) {
				resp, err := ti.esClient.Search(context.Background(),
					elastic.GetIndex(globals.AuditLogs, globals.DefaultTenant), elastic.GetDocType(globals.AuditLogs), test.query, nil, 0, 10000, "", true)
				if err != nil {
					return false, err
				}
				hits := len(resp.Hits.Hits)
				if test.hits != hits {
					return false, fmt.Errorf("expected [%d] hits, got [%d]", test.hits, hits)
				}
				return true, nil
			}, fmt.Sprintf("[%v] test failed", test.name), "100ms")
		}
	}

	// shutdown elastic server and test ProcessEvents
	testutils.StopElasticsearch(ti.elasticSearchName, ti.elasticSearchAuthDir)
	for _, test := range tests {
		err = auditor.ProcessEvents(test.events...)
		Assert(t, err != nil, "elastic auditor should fail to process events if elastic server is down")
	}
}
