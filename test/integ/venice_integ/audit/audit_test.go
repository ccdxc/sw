package audit

import (
	"context"
	"crypto/tls"
	"encoding/json"
	"fmt"
	"net/http"
	"strings"
	"testing"
	"time"

	"github.com/gogo/protobuf/types"
	es "github.com/olivere/elastic"
	uuid "github.com/satori/go.uuid"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/fields"
	"github.com/pensando/sw/api/generated/apiclient"
	auditapi "github.com/pensando/sw/api/generated/audit"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/api/generated/search"
	"github.com/pensando/sw/api/generated/security"
	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/api/login"
	loginctx "github.com/pensando/sw/api/login/context"
	. "github.com/pensando/sw/test/utils"
	testutils "github.com/pensando/sw/test/utils"
	"github.com/pensando/sw/venice/apigw/svc"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/audit"
	elasticauditor "github.com/pensando/sw/venice/utils/audit/elastic"
	auditmgr "github.com/pensando/sw/venice/utils/audit/manager"
	. "github.com/pensando/sw/venice/utils/authn/testutils"
	"github.com/pensando/sw/venice/utils/authz"
	"github.com/pensando/sw/venice/utils/elastic"
	"github.com/pensando/sw/venice/utils/netutils"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestAuditManager(t *testing.T) {
	ti := tInfo{}
	err := ti.setupElastic()
	AssertOk(t, err, "setupElastic failed")
	defer ti.teardownElastic()

	ts, _ := types.TimestampProto(time.Now())
	ts1 := api.Timestamp{Timestamp: *ts}
	tests := []struct {
		name   string
		events []*auditapi.AuditEvent
		err    bool
		query  es.Query
		hits   int
	}{
		{
			name: "single audit event",
			events: []*auditapi.AuditEvent{
				{
					TypeMeta:   api.TypeMeta{Kind: "AuditEvent"},
					ObjectMeta: api.ObjectMeta{Name: "auditevent1", UUID: uuid.NewV4().String(), Tenant: "default", CreationTime: ts1, ModTime: ts1},
					EventAttributes: auditapi.EventAttributes{
						Level:       auditapi.Level_RequestResponse.String(),
						Stage:       auditapi.Stage_RequestProcessing.String(),
						User:        &api.ObjectRef{Kind: "User", Namespace: "default", Tenant: "default", Name: "admin"},
						Resource:    &api.ObjectRef{Kind: "Network", Tenant: "default", Namespace: "default", Name: "network1"},
						ClientIPs:   []string{"192.168.75.133"},
						Action:      auth.Permission_Create.String(),
						Outcome:     auditapi.Outcome_Success.String(),
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
			events: []*auditapi.AuditEvent{
				{
					TypeMeta:   api.TypeMeta{Kind: "AuditEvent"},
					ObjectMeta: api.ObjectMeta{Name: "auditevent2", UUID: uuid.NewV4().String(), Tenant: "default", CreationTime: ts1, ModTime: ts1},
					EventAttributes: auditapi.EventAttributes{
						Level:       auditapi.Level_RequestResponse.String(),
						Stage:       auditapi.Stage_RequestProcessing.String(),
						User:        &api.ObjectRef{Kind: "User", Namespace: "default", Tenant: "default", Name: "admin"},
						Resource:    &api.ObjectRef{Kind: "Role", Tenant: "default", Namespace: "default", Name: "networkadmin"},
						ClientIPs:   []string{"192.168.75.133"},
						Action:      auth.Permission_Create.String(),
						Outcome:     auditapi.Outcome_Failure.String(),
						GatewayNode: "node2",
						GatewayIP:   "192.168.75.139",
						Data:        make(map[string]string),
					},
				},
				{
					TypeMeta:   api.TypeMeta{Kind: "AuditEvent"},
					ObjectMeta: api.ObjectMeta{Name: "auditevent3", UUID: uuid.NewV4().String(), Tenant: "default", CreationTime: ts1, ModTime: ts1},
					EventAttributes: auditapi.EventAttributes{
						Level:       auditapi.Level_RequestResponse.String(),
						Stage:       auditapi.Stage_RequestProcessing.String(),
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
			events: []*auditapi.AuditEvent{},
			err:    false,
			query:  nil,
		},
		{
			name:   "nil events",
			events: []*auditapi.AuditEvent{nil, nil},
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
	event := &auditapi.AuditEvent{
		TypeMeta:   api.TypeMeta{Kind: "AuditEvent"},
		ObjectMeta: api.ObjectMeta{Name: "auditevent1", UUID: uuid.NewV4().String(), Tenant: "default"},
		EventAttributes: auditapi.EventAttributes{
			Level:       auditapi.Level_RequestResponse.String(),
			Stage:       auditapi.Stage_RequestProcessing.String(),
			User:        &api.ObjectRef{Kind: "User", Namespace: "default", Tenant: "default", Name: "admin"},
			Resource:    &api.ObjectRef{Kind: "Network", Tenant: "default", Namespace: "default", Name: "network1"},
			ClientIPs:   []string{"192.168.75.133"},
			Action:      auth.Permission_Create.String(),
			Outcome:     auditapi.Outcome_Success.String(),
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
	if err := SetupAuth(ti.apiServerAddr, true, nil, nil, adminCred, ti.logger); err != nil {
		t.Fatalf("auth setupElastic failed")
	}
	defer CleanupAuth(ti.apiServerAddr, true, false, adminCred, ti.logger)

	const reqID = "abc-123"
	superAdminCtx := loginctx.NewContextWithExtRequestIDHeader(context.TODO(), reqID)
	superAdminCtx, err = NewLoggedInContext(superAdminCtx, ti.apiGwAddr, adminCred)
	AssertOk(t, err, "error creating logged in context")
	// test audit log for successful login
	loginEventObj := &auditapi.AuditEvent{}
	AssertEventually(t, func() (bool, interface{}) {
		query := es.NewBoolQuery().Must(es.NewTermQuery("resource.kind.keyword", string(auth.KindUser)),
			es.NewTermQuery("action.keyword", svc.LoginAction),
			es.NewTermQuery("outcome.keyword", auditapi.Outcome_Success.String()),
			es.NewTermQuery("external-id.keyword", reqID))
		resp, err := ti.esClient.Search(context.Background(),
			elastic.GetIndex(globals.AuditLogs, globals.DefaultTenant), elastic.GetDocType(globals.AuditLogs), query, nil, 0, 10000, "", true)
		if err != nil {
			return false, err.Error()
		}
		hits := len(resp.Hits.Hits)
		if !(hits > 0) {
			return false, fmt.Sprintf("expected at least 1 hit, got [%d]", hits)
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
		resp := &auditapi.AuditEvent{}
		err := getEvent(superAdminCtx, ti.apiGwAddr, loginEventObj.SelfLink, resp)
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
			es.NewTermQuery("action.keyword", strings.Title(string(apiintf.CreateOper))),
			es.NewTermQuery("external-id.keyword", reqID))
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
			es.NewTermQuery("action.keyword", strings.Title(string(apiintf.DeleteOper))),
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
	MustUpdateRoleBinding(ti.apicl, globals.AdminRoleBinding, testTenant, globals.AdminRole, []string{testUser}, nil)
	defer MustUpdateRoleBinding(ti.apicl, globals.AdminRoleBinding, testTenant, globals.AdminRole, nil, nil)
	unauthzCtx, err := NewLoggedInContext(context.Background(), ti.apiGwAddr, &auth.PasswordCredential{
		Username: testUser,
		Password: testPassword,
		Tenant:   testTenant,
	})
	_, err = ti.restcl.ClusterV1().Tenant().Create(unauthzCtx, tenant)
	Assert(t, err != nil, "user should be unauthorized to create tenant")
	AssertEventually(t, func() (bool, interface{}) {
		query := es.NewBoolQuery().Must(es.NewTermQuery("resource.kind.keyword", string(cluster.KindTenant)),
			es.NewTermQuery("action.keyword", strings.Title(string(apiintf.CreateOper))),
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
	Assert(t, getEvent(unauthzCtx, ti.apiGwAddr, loginEventObj.SelfLink, &auditapi.AuditEvent{}) != nil, "testtenant user should not be able to get event in default tenant")
	// test audit log for call failure
	_, err = ti.restcl.ClusterV1().Tenant().Create(superAdminCtx, tenant)
	Assert(t, err != nil, "call to create duplicate tenant should fail")
	AssertEventually(t, func() (bool, interface{}) {
		query := es.NewBoolQuery().Must(es.NewTermQuery("resource.kind.keyword", string(cluster.KindTenant)),
			es.NewTermQuery("action.keyword", strings.Title(string(apiintf.CreateOper))),
			es.NewTermQuery("outcome.keyword", auditapi.Outcome_Failure.String()),
			es.NewTermQuery("stage.keyword", auditapi.Stage_RequestProcessing.String()))
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

func TestAuditAuthz(t *testing.T) {
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

	_, err = NewLoggedInContext(context.Background(), ti.apiGwAddr, adminCred)
	AssertOk(t, err, "error creating super admin logged in context")
	MustCreateTenant(ti.apicl, testTenant)
	defer MustDeleteTenant(ti.apicl, testTenant)
	MustCreateTestUser(ti.apicl, testUser, testPassword, testTenant)
	defer MustDeleteUser(ti.apicl, testUser, testTenant)
	MustCreateRole(ti.apicl, "NoAuditingPerms", testTenant,
		login.NewPermission(testTenant, string(apiclient.GroupAuth), "", authz.ResourceNamespaceAll, "", auth.Permission_AllActions.String()),
		login.NewPermission(testTenant, string(apiclient.GroupSecurity), "", authz.ResourceNamespaceAll, "", auth.Permission_AllActions.String()),
		login.NewPermission(testTenant, string(apiclient.GroupStaging), "", authz.ResourceNamespaceAll, "", auth.Permission_AllActions.String()),
		login.NewPermission(testTenant, string(apiclient.GroupNetwork), "", authz.ResourceNamespaceAll, "", auth.Permission_AllActions.String()),
		login.NewPermission(testTenant, string(apiclient.GroupWorkload), "", authz.ResourceNamespaceAll, "", auth.Permission_AllActions.String()),
	)
	defer MustDeleteRole(ti.apicl, "NoAuditingPerms", testTenant)
	MustCreateRoleBinding(ti.apicl, "NoAuditingPermsRB", testTenant, "NoAuditingPerms", []string{testUser}, nil)
	defer MustDeleteRoleBinding(ti.apicl, "NoAuditingPermsRB", testTenant)
	userCred := &auth.PasswordCredential{
		Username: testUser,
		Password: testPassword,
		Tenant:   testTenant,
	}
	userCtx, err := NewLoggedInContext(context.Background(), ti.apiGwAddr, userCred)
	AssertOk(t, err, "error creating testtenant user logged in context")
	// query by category and kind
	query := &search.SearchRequest{
		Query: &search.SearchQuery{
			Categories: []string{globals.Kind2Category("AuditEvent")},
			Kinds:      []string{auth.Permission_AuditEvent.String()},
		},
		From:       0,
		MaxResults: 50,
		Aggregate:  true,
	}
	resp := search.SearchResponse{}
	AssertEventually(t, func() (bool, interface{}) {
		if err := Search(userCtx, ti.apiGwAddr, query, &resp); err != nil {
			return false, err
		}
		return true, nil
	}, "error performing audit log search")
	Assert(t, resp.ActualHits == 0, fmt.Sprintf("user with no auditing permissions was able to retrieve audit logs: %#v", resp))
	MustCreateRole(ti.apicl, "AuditingPerms", testTenant,
		login.NewPermission(testTenant, "", auth.Permission_AuditEvent.String(), authz.ResourceNamespaceAll, "", auth.Permission_Read.String()),
	)
	defer MustDeleteRole(ti.apicl, "AuditingPerms", testTenant)
	MustCreateRoleBinding(ti.apicl, "AuditingPermsRB", testTenant, "AuditingPerms", []string{testUser}, nil)
	defer MustDeleteRoleBinding(ti.apicl, "AuditingPermsRB", testTenant)
	resp = search.SearchResponse{}
	AssertEventually(t, func() (bool, interface{}) {
		if err := Search(userCtx, ti.apiGwAddr, query, &resp); err != nil {
			return false, err
		}
		if resp.ActualHits == 0 {
			return false, resp
		}
		return true, nil
	}, "user with auditing permissions should able to retrieve audit logs for its tenant")
	Assert(t, len(resp.AggregatedEntries.Tenants[testTenant].Categories[globals.Kind2Category("AuditEvent")].Kinds[auth.Permission_AuditEvent.String()].Entries) != 0,
		fmt.Sprintf("user with auditing permissions should able to retrieve audit logs for its tenant: %#v", resp))
	_, ok := resp.AggregatedEntries.Tenants[globals.DefaultTenant]
	Assert(t, !ok, fmt.Sprintf("user should not be able to retrieve audit logs for other tenants: %#v", resp))
}

func TestSecretsInAuditLogs(t *testing.T) {
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

	superAdminCtx, err := NewLoggedInContext(context.Background(), ti.apiGwAddr, adminCred)
	AssertOk(t, err, "error creating logged in context")
	tuser := &auth.User{}
	tuser.Defaults("all")
	tuser.Name = "testuser2"
	tuser.Spec.Password = testPassword
	tuser.Spec.Email = "testuser2@pensando.io"
	_, err = ti.restcl.AuthV1().User().Create(superAdminCtx, tuser)
	AssertOk(t, err, "error creating test user")
	defer MustDeleteUser(ti.apicl, "testuser2", globals.DefaultTenant)
	// test auth policy
	apolicy, err := ti.apicl.AuthV1().AuthenticationPolicy().Get(superAdminCtx, &api.ObjectMeta{})
	AssertOk(t, err, "unable to fetch auth policy")
	apolicy.Spec.Authenticators.Ldap = &auth.Ldap{
		Domains: []*auth.LdapDomain{
			{
				BindDN:       "CN=admin, DC=Pensando, DC=io",
				BindPassword: testPassword,
			},
		},
	}
	_, err = ti.restcl.AuthV1().AuthenticationPolicy().Update(superAdminCtx, apolicy)
	AssertOk(t, err, "unable to update auth policy")
	// test reset password
	passwdResetReq := &auth.PasswordResetRequest{}
	passwdResetReq.Defaults("all")
	passwdResetReq.ObjectMeta.Name = "testuser2"
	passwdResetReq.ObjectMeta.Tenant = globals.DefaultTenant
	passwdResetReq.Kind = string(auth.KindUser)
	tuser, err = ti.restcl.AuthV1().User().PasswordReset(superAdminCtx, passwdResetReq)
	AssertOk(t, err, "unable to reset testuser2 password")
	// search for audit events
	stages := []string{auditapi.Stage_RequestAuthorization.String(), auditapi.Stage_RequestProcessing.String()}
	for _, stage := range stages {
		query := &search.SearchRequest{
			Query: &search.SearchQuery{
				Kinds: []string{auth.Permission_AuditEvent.String()},
				Fields: &fields.Selector{
					Requirements: []*fields.Requirement{
						{
							Key:      "action",
							Operator: "equals",
							Values:   []string{strings.Title(string(apiintf.CreateOper))},
						},
						{
							Key:      "outcome",
							Operator: "equals",
							Values:   []string{auditapi.Outcome_Success.String()},
						},
						{
							Key:      "resource.kind",
							Operator: "equals",
							Values:   []string{string(auth.KindUser)},
						},
						{
							Key:      "resource.name",
							Operator: "equals",
							Values:   []string{"testuser2"},
						},
						{
							Key:      "stage",
							Operator: "equals",
							Values:   []string{stage},
						},
					},
				},
			},
			From:       0,
			MaxResults: 50,
			Aggregate:  true,
		}
		resp := testutils.AuditSearchResponse{}
		AssertEventually(t, func() (bool, interface{}) {
			err := Search(superAdminCtx, ti.apiGwAddr, query, &resp)
			if err != nil {
				return false, err
			}
			if resp.ActualHits == 0 {
				return false, fmt.Errorf("no audit logs for user create at stage %s", stage)
			}
			if resp.ActualHits > 1 {
				return false, fmt.Errorf("unexpected number of audit logs: %d", resp.ActualHits)
			}
			return true, nil
		}, "error performing audit log search")

		events := resp.AggregatedEntries.Tenants[globals.DefaultTenant].Categories[globals.Kind2Category("AuditEvent")].Kinds[auth.Permission_AuditEvent.String()].Entries

		Assert(t, (events[0].Object.Action == strings.Title(auth.Permission_Create.String())) &&
			(events[0].Object.Resource.Kind == string(auth.KindUser)) &&
			(events[0].Object.Outcome == auditapi.Outcome_Success.String()) &&
			(events[0].Object.Stage == stage), fmt.Sprintf("unexpected audit event: %#v", *events[0]))
		Assert(t, (!strings.Contains(events[0].Object.RequestObject, testPassword)) &&
			(!strings.Contains(events[0].Object.ResponseObject, testPassword)), fmt.Sprintf("audit event contains password: %#v", events[0].Object))

		// search for auth policy update audit event
		query = &search.SearchRequest{
			Query: &search.SearchQuery{
				Kinds: []string{auth.Permission_AuditEvent.String()},
				Fields: &fields.Selector{
					Requirements: []*fields.Requirement{
						{
							Key:      "action",
							Operator: "equals",
							Values:   []string{strings.Title(string(apiintf.UpdateOper))},
						},
						{
							Key:      "outcome",
							Operator: "equals",
							Values:   []string{auditapi.Outcome_Success.String()},
						},
						{
							Key:      "resource.kind",
							Operator: "equals",
							Values:   []string{string(auth.KindAuthenticationPolicy)},
						},
						{
							Key:      "stage",
							Operator: "equals",
							Values:   []string{stage},
						},
					},
				},
			},
			From:       0,
			MaxResults: 50,
			Aggregate:  true,
		}
		resp = testutils.AuditSearchResponse{}
		AssertEventually(t, func() (bool, interface{}) {
			err := Search(superAdminCtx, ti.apiGwAddr, query, &resp)
			if err != nil {
				return false, err
			}
			if resp.ActualHits == 0 {
				return false, fmt.Errorf("no audit logs for user create at stage %s", stage)
			}
			if resp.ActualHits > 1 {
				return false, fmt.Errorf("unexpected number of audit logs: %d", resp.ActualHits)
			}
			return true, nil
		}, "error performing audit log search")

		events = resp.AggregatedEntries.Tenants[globals.DefaultTenant].Categories[globals.Kind2Category("AuditEvent")].Kinds[auth.Permission_AuditEvent.String()].Entries

		Assert(t, (events[0].Object.Action == strings.Title(auth.Permission_Update.String())) &&
			(events[0].Object.Resource.Kind == string(auth.KindAuthenticationPolicy)) &&
			(events[0].Object.Outcome == auditapi.Outcome_Success.String()) &&
			(events[0].Object.Stage == stage) &&
			((strings.Contains(events[0].Object.RequestObject, "DC=Pensando") && events[0].Object.Stage == auditapi.Stage_RequestAuthorization.String()) ||
				(strings.Contains(events[0].Object.ResponseObject, "") && events[0].Object.Stage == auditapi.Stage_RequestProcessing.String())), fmt.Sprintf("unexpected audit event: %#v", *events[0]))
		Assert(t, (!strings.Contains(events[0].Object.RequestObject, testPassword)) &&
			(!strings.Contains(events[0].Object.ResponseObject, testPassword)), fmt.Sprintf("audit event contains password: %#v", events[0].Object))

		query = &search.SearchRequest{
			Query: &search.SearchQuery{
				Kinds: []string{auth.Permission_AuditEvent.String()},
				Fields: &fields.Selector{
					Requirements: []*fields.Requirement{
						{
							Key:      "action",
							Operator: "equals",
							Values:   []string{"PasswordReset"},
						},
						{
							Key:      "outcome",
							Operator: "equals",
							Values:   []string{auditapi.Outcome_Success.String()},
						},
						{
							Key:      "resource.kind",
							Operator: "equals",
							Values:   []string{string(auth.KindUser)},
						},
						{
							Key:      "resource.name",
							Operator: "equals",
							Values:   []string{"testuser2"},
						},
						{
							Key:      "stage",
							Operator: "equals",
							Values:   []string{stage},
						},
					},
				},
			},
			From:       0,
			MaxResults: 50,
			Aggregate:  true,
		}
		resp = testutils.AuditSearchResponse{}
		AssertEventually(t, func() (bool, interface{}) {
			err := Search(superAdminCtx, ti.apiGwAddr, query, &resp)
			if err != nil {
				return false, err
			}
			if resp.ActualHits == 0 {
				return false, fmt.Errorf("no audit logs for user reset password at stage %s", stage)
			}
			if resp.ActualHits > 1 {
				return false, fmt.Errorf("unexpected number of audit logs: %d", resp.ActualHits)
			}
			return true, nil
		}, "error performing audit log search")
		events = resp.AggregatedEntries.Tenants[globals.DefaultTenant].Categories[globals.Kind2Category("AuditEvent")].Kinds[auth.Permission_AuditEvent.String()].Entries

		Assert(t, (events[0].Object.Action == "PasswordReset") &&
			(events[0].Object.Resource.Kind == string(auth.KindUser)) &&
			(events[0].Object.Outcome == auditapi.Outcome_Success.String()) &&
			(events[0].Object.Stage == stage), fmt.Sprintf("unexpected audit event: %#v", *events[0]))
		Assert(t, !strings.Contains(events[0].Object.ResponseObject, tuser.Spec.Password), fmt.Sprintf("audit event contains password: %#v", events[0].Object))
	}
}

func TestAuditingWithElasticNotAvailable(t *testing.T) {
	ti := tInfo{}
	err := ti.setupElastic()
	AssertOk(t, err, "setupElastic failed")
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

	// stop elastic
	testutils.StopElasticsearch(ti.elasticSearchName, ti.elasticSearchDir)
	// login should work if elastic is down
	ctx, err := login.NewLoggedInContext(context.Background(), ti.apiGwAddr, adminCred)
	AssertOk(t, err, "super admin should be able to log in when elastic is down")
	// get operations should work
	policy, err := ti.restcl.AuthV1().AuthenticationPolicy().Get(ctx, &api.ObjectMeta{})
	AssertOk(t, err, "unable to fetch auth policy")
	// update operations should fail
	nctx, cancel := context.WithTimeout(ctx, 2*time.Second)
	_, err = ti.restcl.AuthV1().AuthenticationPolicy().Update(nctx, policy)
	defer cancel()
	Assert(t, err != nil, "update of auth policy should fail")
	// should be able to create techsupport request
	tsr := &monitoring.TechSupportRequest{}
	tsr.Defaults("all")
	tsr.Name = "t1"
	tsr.Spec = monitoring.TechSupportRequestSpec{
		NodeSelector: &monitoring.TechSupportRequestSpec_NodeSelectorSpec{
			Names: []string{"host1-dsc1"},
		},
	}
	tsr, err = ti.restcl.MonitoringV1().TechSupportRequest().Create(ctx, tsr)
	AssertOk(t, err, "unable to create techsupport request")
	_, err = ti.restcl.MonitoringV1().TechSupportRequest().Delete(ctx, &tsr.ObjectMeta)
	AssertOk(t, err, "unable to delete techsupport request")
}

func TestAPIServerNotAvailable(t *testing.T) {
	ti := tInfo{}
	err := ti.setupElastic()
	AssertOk(t, err, "setupElastic failed")
	err = ti.startSpyglass()
	AssertOk(t, err, "failed to start spyglass")
	defer ti.fdr.Stop()
	err = ti.startAPIServer()
	AssertOk(t, err, "failed to start API server")
	err = ti.startAPIGateway()
	AssertOk(t, err, "failed to start API Gateway")
	defer ti.apiGw.Stop()
	userCred := &auth.PasswordCredential{
		Username: testUser,
		Password: testPassword,
		Tenant:   globals.DefaultTenant,
	}
	// create tenant and admin user
	if err := SetupAuth(ti.apiServerAddr, true, nil, nil, userCred, ti.logger); err != nil {
		t.Fatalf("auth setup failed")
	}
	defer CleanupAuth(ti.apiServerAddr, true, false, userCred, ti.logger)
	_, err = NewLoggedInContext(context.TODO(), ti.apiGwAddr, userCred)
	AssertOk(t, err, "unable to get logged in context")
	ti.apiServer.Stop()
	AssertEventually(t, func() (bool, interface{}) {
		_, _, err := login.UserLogin(context.TODO(), ti.apiGwAddr, userCred)
		if err != nil && strings.Contains(err.Error(), "503") {
			return true, nil
		}
		return false, err
	}, "unexpected error when API server is unreachable")
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
	sgPolicy := getNetworkSecurityPolicyData(nRules)
	event := &auditapi.AuditEvent{
		TypeMeta:   api.TypeMeta{Kind: "AuditEvent"},
		ObjectMeta: api.ObjectMeta{Name: "auditevent1", UUID: uuid.NewV4().String(), Tenant: "default"},
		EventAttributes: auditapi.EventAttributes{
			Level:       auditapi.Level_RequestResponse.String(),
			Stage:       auditapi.Stage_RequestProcessing.String(),
			User:        &api.ObjectRef{Kind: "User", Namespace: "default", Tenant: "default", Name: "admin"},
			Resource:    &api.ObjectRef{Kind: sgPolicy.Kind, Tenant: sgPolicy.Tenant, Namespace: sgPolicy.Namespace, Name: sgPolicy.Name},
			ClientIPs:   []string{"192.168.75.133"},
			Action:      auth.Permission_Create.String(),
			Outcome:     auditapi.Outcome_Success.String(),
			GatewayNode: "node1",
			GatewayIP:   "192.168.75.136",
			Data:        make(map[string]string),
		},
	}
	b.ResetTimer()
	_, _, err = audit.NewPolicyChecker().PopulateEvent(event,
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

func getEvent(ctx context.Context, apiGwAddr, selfLink string, resp *auditapi.AuditEvent) error {
	auditURL := fmt.Sprintf("https://%s%s", apiGwAddr, selfLink)
	restcl := netutils.NewHTTPClient()
	restcl.WithTLSConfig(&tls.Config{InsecureSkipVerify: true})
	restcl.DisableKeepAlives()
	defer restcl.CloseIdleConnections()

	// get authz header
	authzHeader, ok := loginctx.AuthzHeaderFromContext(ctx)
	if !ok {
		return fmt.Errorf("no authorization header in context")
	}
	restcl.SetHeader("Authorization", authzHeader)
	status, err := restcl.Req("GET", auditURL, &auditapi.AuditEventRequest{}, resp)
	if status != http.StatusOK {
		return fmt.Errorf("GET request failed with http status code (%d) for event (%s)", status, auditURL)
	}
	return err
}

func getNetworkSecurityPolicyData(nRules int) *security.NetworkSecurityPolicy {
	sgPolicy := &security.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "policy1",
		},
		Spec: security.NetworkSecurityPolicySpec{
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
