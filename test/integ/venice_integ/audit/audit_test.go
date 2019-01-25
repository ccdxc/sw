package audit

import (
	"context"
	"crypto/tls"
	"encoding/json"
	"fmt"
	"net/http"
	"testing"

	es "github.com/olivere/elastic"
	"github.com/satori/go.uuid"

	"github.com/pensando/sw/api"
	auditapi "github.com/pensando/sw/api/generated/audit"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/security"
	loginctx "github.com/pensando/sw/api/login/context"
	. "github.com/pensando/sw/test/utils"
	testutils "github.com/pensando/sw/test/utils"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/audit"
	elasticauditor "github.com/pensando/sw/venice/utils/audit/elastic"
	auditmgr "github.com/pensando/sw/venice/utils/audit/manager"
	. "github.com/pensando/sw/venice/utils/authn/testutils"
	"github.com/pensando/sw/venice/utils/elastic"
	"github.com/pensando/sw/venice/utils/netutils"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestAuditManager(t *testing.T) {
	ti := tInfo{}
	err := ti.setupElastic()
	AssertOk(t, err, "setupElastic failed")
	defer ti.teardownElastic()
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
			err:    false,
			query:  nil,
		},
		{
			name:   "nil events",
			events: []*auditapi.Event{nil, nil},
			err:    false,
			query:  nil,
		},
	}
	auditor := auditmgr.WithAuditors(elasticauditor.NewSynchAuditor(ti.elasticSearchAddr, ti.rslvr, ti.logger, elasticauditor.WithElasticClient(ti.esClient)))
	err = auditor.Run(make(<-chan struct{}))
	AssertOk(t, err, "error starting elastic auditor")
	defer auditor.Shutdown()

	for _, test := range tests {
		err = auditor.ProcessEvents(test.events...)
		Assert(t, test.err == (err != nil), fmt.Sprintf("[%v] test failed, err: %v", test.name, err))
		if !test.err && test.query != nil {
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
}

func TestElasticServerDown(t *testing.T) {
	ti := tInfo{}
	err := ti.setupElastic()
	AssertOk(t, err, "setupElastic failed")
	defer ti.teardownElastic()
	auditor := auditmgr.WithAuditors(elasticauditor.NewSynchAuditor(ti.elasticSearchAddr, ti.rslvr, ti.logger, elasticauditor.WithElasticClient(ti.esClient)))
	err = auditor.Run(make(<-chan struct{}))
	AssertOk(t, err, "error starting elastic auditor")
	// shutdown elastic server and test ProcessEvents
	testutils.StopElasticsearch(ti.elasticSearchName, ti.elasticSearchDir)
	event := &auditapi.Event{
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
	}
	defer auditor.Shutdown()
	err = auditor.ProcessEvents(event)
	Assert(t, err != nil, "elastic auditor should fail to process events if elastic server is down")
}

func TestAuditLogs(t *testing.T) {
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
	if err := SetupAuth(ti.apiServerAddr, true, &auth.Ldap{Enabled: false}, &auth.Radius{Enabled: false}, adminCred, ti.logger); err != nil {
		t.Fatalf("auth setupElastic failed")
	}
	defer CleanupAuth(ti.apiServerAddr, true, false, adminCred, ti.logger)

	superAdminCtx, err := NewLoggedInContext(context.Background(), ti.apiGwAddr, adminCred)
	AssertOk(t, err, "error creating logged in context")
	// test audit log for successful login
	loginEventObj := &auditapi.Event{}
	AssertEventually(t, func() (bool, interface{}) {
		query := es.NewBoolQuery().Must(es.NewTermQuery("resource.kind.keyword", string(auth.KindUser)),
			es.NewTermQuery("action.keyword", "login"),
			es.NewTermQuery("outcome.keyword", auditapi.Outcome_Success.String()))
		resp, err := ti.esClient.Search(context.Background(),
			elastic.GetIndex(globals.AuditLogs, globals.DefaultTenant), elastic.GetDocType(globals.AuditLogs), query, nil, 0, 10000, "", true)
		if err != nil {
			return false, err.Error()
		}
		hits := len(resp.Hits.Hits)
		if hits != 1 {
			return false, fmt.Sprintf("expected [%d] hits, got [%d]", 1, hits)
		}
		databytes, err := resp.Hits.Hits[0].Source.MarshalJSON()
		if err != nil {
			return false, err.Error()
		}
		err = json.Unmarshal(databytes, loginEventObj)
		if err != nil {
			return false, err.Error()
		}
		return true, nil
	}, "expected audit log for user login", "100ms")
	AssertEventually(t, func() (bool, interface{}) {
		resp := &auditapi.Event{}
		err := getEvent(superAdminCtx, ti.apiGwAddr, loginEventObj.GetUUID(), resp)
		if err != nil {
			return false, err.Error()
		}
		return resp.GetUUID() == loginEventObj.GetUUID(), resp
	}, fmt.Sprintf("expected REST call to get event by UUID (%s) to succeed", loginEventObj.GetUUID()), "100ms")
	// test audit log for successful operation
	tenant := &cluster.Tenant{
		TypeMeta: api.TypeMeta{Kind: string(cluster.KindTenant)},
		ObjectMeta: api.ObjectMeta{
			Name: testTenant,
		},
		Spec: cluster.TenantSpec{},
	}
	AssertEventually(t, func() (bool, interface{}) {
		_, err := ti.restcl.ClusterV1().Tenant().Create(superAdminCtx, tenant)
		if err != nil {
			return false, err.Error()
		}
		return true, nil
	}, fmt.Sprintf("error creating tenant [%s]", testTenant), "100ms")
	defer DeleteTenant(ti.apicl, testTenant)
	AssertEventually(t, func() (bool, interface{}) {
		query := es.NewBoolQuery().Must(es.NewTermQuery("resource.kind.keyword", string(cluster.KindTenant)),
			es.NewTermQuery("action.keyword", auth.Permission_Create.String()))
		resp, err := ti.esClient.Search(context.Background(),
			elastic.GetIndex(globals.AuditLogs, globals.DefaultTenant), elastic.GetDocType(globals.AuditLogs), query, nil, 0, 10000, "", true)
		if err != nil {
			return false, err.Error()
		}
		hits := len(resp.Hits.Hits)
		if hits != 2 {
			return false, fmt.Errorf("expected [%d] hits for tenant creation, got [%d]", 2, hits)
		}
		return true, nil
	}, fmt.Sprintf("expected two audit logs for [%s] tenant creation", testTenant), "100ms")
	// test audit log for pre-call hook failure
	_, err = ti.restcl.AuthV1().Role().Delete(superAdminCtx, &api.ObjectMeta{Name: globals.AdminRole, Tenant: globals.DefaultTenant})
	Assert(t, err != nil, "admin role shouldn't be deleted")
	AssertEventually(t, func() (bool, interface{}) {
		query := es.NewBoolQuery().Must(es.NewTermQuery("resource.kind.keyword", string(auth.KindRole)),
			es.NewTermQuery("action.keyword", auth.Permission_Delete.String()),
			es.NewTermQuery("outcome.keyword", auditapi.Outcome_Failure.String()),
			es.NewTermQuery("stage.keyword", auditapi.Stage_RequestProcessing.String()))
		resp, err := ti.esClient.Search(context.Background(),
			elastic.GetIndex(globals.AuditLogs, globals.DefaultTenant), elastic.GetDocType(globals.AuditLogs), query, nil, 0, 10000, "", true)
		if err != nil {
			return false, err.Error()
		}
		hits := len(resp.Hits.Hits)
		if hits != 1 {
			return false, fmt.Sprintf("expected [%d] hits for admin role deletion failure, got [%d]", 1, hits)
		}
		return true, nil
	}, fmt.Sprintf("expected one audit log for [%s] admin role deletion failure", testTenant), "100ms")
	// test audit log for authorization failure
	MustCreateTestUser(ti.apicl, testUser, testPassword, testTenant)
	defer MustDeleteUser(ti.apicl, testUser, testTenant)
	MustCreateRoleBinding(ti.apicl, "AdminRoleBinding", testTenant, globals.AdminRole, []string{testUser}, nil)
	defer MustDeleteRoleBinding(ti.apicl, "AdminRoleBinding", testTenant)
	unauthzCtx, err := NewLoggedInContext(context.Background(), ti.apiGwAddr, &auth.PasswordCredential{
		Username: testUser,
		Password: testPassword,
		Tenant:   testTenant,
	})
	_, err = ti.restcl.ClusterV1().Tenant().Create(unauthzCtx, tenant)
	Assert(t, err != nil, "user should be unauthorized to create tenant")
	AssertEventually(t, func() (bool, interface{}) {
		query := es.NewBoolQuery().Must(es.NewTermQuery("resource.kind.keyword", string(cluster.KindTenant)),
			es.NewTermQuery("action.keyword", auth.Permission_Create.String()),
			es.NewTermQuery("outcome.keyword", auditapi.Outcome_Failure.String()),
			es.NewTermQuery("stage.keyword", auditapi.Stage_RequestAuthorization.String()))
		resp, err := ti.esClient.Search(context.Background(),
			elastic.GetIndex(globals.AuditLogs, testTenant), elastic.GetDocType(globals.AuditLogs), query, nil, 0, 10000, "", true)
		if err != nil {
			return false, err.Error()
		}
		hits := len(resp.Hits.Hits)
		if hits != 1 {
			return false, fmt.Sprintf("expected [%d] hits for tenant creation authorization failure, got [%d]", 1, hits)
		}
		return true, nil
	}, fmt.Sprintf("expected one audit log for [%s] tenant creation authorization failure", testTenant), "100ms")
	// test authorization check in spyglass controller when audit event is fetched given UUID
	Assert(t, getEvent(unauthzCtx, ti.apiGwAddr, loginEventObj.GetUUID(), &auditapi.Event{}) != nil, "testtenant user should not be able to get event in default tenant")
	// test audit log for call failure
	_, err = ti.restcl.ClusterV1().Tenant().Create(superAdminCtx, tenant)
	Assert(t, err != nil, "call to create duplicate tenant should fail")
	AssertEventually(t, func() (bool, interface{}) {
		query := es.NewBoolQuery().Must(es.NewTermQuery("resource.kind.keyword", string(cluster.KindTenant)),
			es.NewTermQuery("action.keyword", auth.Permission_Create.String()),
			es.NewTermQuery("outcome.keyword", auditapi.Outcome_Failure.String()),
			es.NewTermQuery("stage.keyword", auditapi.Stage_RequestCompleted.String()))
		resp, err := ti.esClient.Search(context.Background(),
			elastic.GetIndex(globals.AuditLogs, globals.DefaultTenant), elastic.GetDocType(globals.AuditLogs), query, nil, 0, 10000, "", true)
		if err != nil {
			return false, err.Error()
		}
		hits := len(resp.Hits.Hits)
		if hits != 1 {
			return false, fmt.Sprintf("expected [%d] hits for duplicate tenant creation failure, got [%d]", 1, hits)
		}
		return true, nil
	}, fmt.Sprintf("expected one audit log for [%s] duplicate tenant creation failure", testTenant), "100ms")
}

// BenchmarkProcessEvents1Rule/auditor.ProcessEvents()-8         	     500	   6800263 ns/op
func BenchmarkProcessEvents1Rule(b *testing.B) {
	benchmarkProcessEvents(1, b)
}

// BenchmarkProcessEvents1kRules/auditor.ProcessEvents()-8         	     200	  26087556 ns/op
func BenchmarkProcessEvents1kRules(b *testing.B) {
	benchmarkProcessEvents(1000, b)
}

// BenchmarkProcessEvents10kRules/auditor.ProcessEvents()-8         	     100	 160981666 ns/op
func BenchmarkProcessEvents10kRules(b *testing.B) {
	benchmarkProcessEvents(10000, b)
}

// BenchmarkProcessEvents70kRules/auditor.ProcessEvents()-8         	      30	1287068761 ns/op
func BenchmarkProcessEvents70kRules(b *testing.B) {
	benchmarkProcessEvents(70000, b)
}

func benchmarkProcessEvents(nRules int, b *testing.B) {
	ti := tInfo{}
	err := ti.setupElastic()
	if err != nil {
		panic(fmt.Sprintf("setupElastic failed with error: %v", err))
	}
	defer ti.teardownElastic()
	auditor := auditmgr.WithAuditors(elasticauditor.NewSynchAuditor(ti.elasticSearchAddr, ti.rslvr, ti.logger, elasticauditor.WithElasticClient(ti.esClient)))
	err = auditor.Run(make(<-chan struct{}))
	if err != nil {
		panic(fmt.Sprintf("error starting elastic auditor: %v", err))
	}
	defer auditor.Shutdown()
	sgPolicy := getSGPolicyData(nRules)
	event := &auditapi.Event{
		TypeMeta:   api.TypeMeta{Kind: "AuditEvent"},
		ObjectMeta: api.ObjectMeta{Name: "auditevent1", UUID: uuid.NewV4().String(), Tenant: "default"},
		EventAttributes: auditapi.EventAttributes{
			Level:       auditapi.Level_RequestResponse.String(),
			Stage:       auditapi.Stage_RequestProcessing.String(),
			User:        &api.ObjectRef{Kind: "User", Namespace: "default", Tenant: "default", Name: "admin"},
			Resource:    &api.ObjectRef{Kind: sgPolicy.Kind, Tenant: sgPolicy.Tenant, Namespace: sgPolicy.Namespace, Name: sgPolicy.Name},
			ClientIPs:   []string{"192.168.75.133"},
			Action:      auth.Permission_Create.String(),
			Outcome:     auditapi.Outcome_Unknown.String(),
			GatewayNode: "node1",
			GatewayIP:   "192.168.75.136",
			Data:        make(map[string]string),
		},
	}
	b.ResetTimer()
	_, err = audit.NewPolicyChecker().PopulateEvent(event,
		audit.NewRequestObjectPopulator(sgPolicy, true),
		audit.NewResponseObjectPopulator(sgPolicy, true))
	if err != nil {
		panic(fmt.Sprintf("error populating audit event: %v", err))
	}
	b.Run("auditor.ProcessEvents()", func(b *testing.B) {
		for i := 0; i < b.N; i++ {
			err = auditor.ProcessEvents(event)
			if err != nil {
				panic(fmt.Sprintf("error processing audit event: %v", err))
			}
		}
	})
}

func getEvent(ctx context.Context, apiGwAddr, eventID string, resp *auditapi.Event) error {
	auditURL := fmt.Sprintf("https://%s/audit/v1/events/%s", apiGwAddr, eventID)
	restcl := netutils.NewHTTPClient()
	restcl.WithTLSConfig(&tls.Config{InsecureSkipVerify: true})
	// get authz header
	authzHeader, ok := loginctx.AuthzHeaderFromContext(ctx)
	if !ok {
		return fmt.Errorf("no authorizaton header in context")
	}
	restcl.SetHeader("Authorization", authzHeader)
	status, err := restcl.Req("GET", auditURL, &auditapi.EventRequest{}, resp)
	if status != http.StatusOK {
		return fmt.Errorf("GET request failed with http status code (%d) for event (%s)", status, auditURL)
	}
	return err
}

func getSGPolicyData(nRules int) *security.SGPolicy {
	sgPolicy := &security.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "policy1",
		},
		Spec: security.SGPolicySpec{
			AttachTenant: true,
		},
	}
	for i := 0; i < nRules; i++ {
		sgPolicy.Spec.Rules = append(sgPolicy.Spec.Rules, security.SGRule{
			FromIPAddresses: []string{"10.0.0.0/24"},
			ToIPAddresses:   []string{"11.0.0.0/24"},
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "tcp",
					Ports:    "80",
				},
			},
			Action: "PERMIT",
		})
	}
	return sgPolicy
}
