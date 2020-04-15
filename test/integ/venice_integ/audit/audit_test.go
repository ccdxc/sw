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
	"github.com/pensando/sw/api/bulkedit"
	"github.com/pensando/sw/api/fields"
	"github.com/pensando/sw/api/generated/apiclient"
	auditapi "github.com/pensando/sw/api/generated/audit"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/search"
	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/api/generated/staging"
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
	ti := TestInfo{Name: t.Name()}
	err := ti.SetupElastic()
	AssertOk(t, err, "setupElastic failed")
	defer ti.TeardownElastic()

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
	auditor := auditmgr.WithAuditors(elasticauditor.NewSynchAuditor(ti.ElasticSearchAddr, ti.Rslvr, ti.Logger, elasticauditor.WithElasticClient(ti.ESClient)))
	err = auditor.Run()
	AssertOk(t, err, "error starting elastic auditor")
	defer auditor.Shutdown()

	for _, test := range tests {
		err = auditor.ProcessEvents(test.events...)
		Assert(t, test.err == (err != nil), fmt.Sprintf("[%v] test failed, err: %v", test.name, err))
		if !test.err && test.query != nil {
			AssertEventually(t, func() (bool, interface{}) {
				resp, err := ti.ESClient.Search(context.Background(),
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
	ti := TestInfo{Name: t.Name()}
	err := ti.SetupElastic()
	AssertOk(t, err, "setupElastic failed")
	defer ti.TeardownElastic()
	auditor := auditmgr.WithAuditors(elasticauditor.NewSynchAuditor(ti.ElasticSearchAddr, ti.Rslvr, ti.Logger, elasticauditor.WithElasticClient(ti.ESClient)))
	err = auditor.Run()
	AssertOk(t, err, "error starting elastic auditor")
	// shutdown elastic server and test ProcessEvents
	testutils.StopElasticsearch(ti.ElasticSearchName, ti.ElasticSearchDir)
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
	ti := TestInfo{Name: t.Name()}
	err := ti.SetupElastic()
	AssertOk(t, err, "setupElastic failed")
	defer ti.TeardownElastic()
	err = ti.StartSpyglass()
	AssertOk(t, err, "failed to start spyglass")
	defer ti.Fdr.Stop()
	err = ti.StartAPIServer()
	AssertOk(t, err, "failed to start API server")
	defer ti.APIServer.Stop()
	err = ti.StartAPIGateway()
	AssertOk(t, err, "failed to start API Gateway")
	defer ti.APIGw.Stop()

	adminCred := &auth.PasswordCredential{
		Username: testUser,
		Password: testPassword,
		Tenant:   globals.DefaultTenant,
	}
	// create default tenant and global admin user
	if err := SetupAuth(ti.APIServerAddr, true, nil, nil, adminCred, ti.Logger); err != nil {
		t.Fatalf("auth setupElastic failed")
	}
	defer CleanupAuth(ti.APIServerAddr, true, false, adminCred, ti.Logger)

	const reqID = "abc-123"
	superAdminCtx := loginctx.NewContextWithExtRequestIDHeader(context.TODO(), reqID)
	superAdminCtx, err = NewLoggedInContext(superAdminCtx, ti.APIGwAddr, adminCred)
	AssertOk(t, err, "error creating logged in context")
	// test audit log for successful login
	loginEventObj := &auditapi.AuditEvent{}
	AssertEventually(t, func() (bool, interface{}) {
		query := es.NewBoolQuery().Must(es.NewTermQuery("resource.kind.keyword", string(auth.KindUser)),
			es.NewTermQuery("action.keyword", svc.LoginAction),
			es.NewTermQuery("outcome.keyword", auditapi.Outcome_Success.String()),
			es.NewTermQuery("external-id.keyword", reqID))
		resp, err := ti.ESClient.Search(context.Background(),
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
		err := getEvent(superAdminCtx, ti.APIGwAddr, loginEventObj.SelfLink, resp)
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
		_, err := ti.Restcl.ClusterV1().Tenant().Create(superAdminCtx, tenant)
		if err != nil {
			return false, err.Error()
		}
		return true, nil
	}, fmt.Sprintf("error creating tenant [%s]", testTenant), "100ms")
	defer DeleteTenant(ti.Apicl, testTenant)
	AssertEventually(t, func() (bool, interface{}) {
		query := es.NewBoolQuery().Must(es.NewTermQuery("resource.kind.keyword", string(cluster.KindTenant)),
			es.NewTermQuery("action.keyword", strings.Title(string(apiintf.CreateOper))),
			es.NewTermQuery("external-id.keyword", reqID))
		resp, err := ti.ESClient.Search(context.Background(),
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
	_, err = ti.Restcl.AuthV1().Role().Delete(superAdminCtx, &api.ObjectMeta{Name: globals.AdminRole, Tenant: globals.DefaultTenant})
	Assert(t, err != nil, "admin role shouldn't be deleted")
	AssertEventually(t, func() (bool, interface{}) {
		query := es.NewBoolQuery().Must(es.NewTermQuery("resource.kind.keyword", string(auth.KindRole)),
			es.NewTermQuery("action.keyword", strings.Title(string(apiintf.DeleteOper))),
			es.NewTermQuery("outcome.keyword", auditapi.Outcome_Failure.String()),
			es.NewTermQuery("stage.keyword", auditapi.Stage_RequestProcessing.String()))
		resp, err := ti.ESClient.Search(context.Background(),
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
	MustCreateTestUser(ti.Apicl, testUser, testPassword, testTenant)
	defer MustDeleteUser(ti.Apicl, testUser, testTenant)
	MustUpdateRoleBinding(ti.Apicl, globals.AdminRoleBinding, testTenant, globals.AdminRole, []string{testUser}, nil)
	defer MustUpdateRoleBinding(ti.Apicl, globals.AdminRoleBinding, testTenant, globals.AdminRole, nil, nil)
	unauthzCtx, err := NewLoggedInContext(context.Background(), ti.APIGwAddr, &auth.PasswordCredential{
		Username: testUser,
		Password: testPassword,
		Tenant:   testTenant,
	})
	AssertOk(t, err, "error creating logged in context for user in testtenant")
	_, err = ti.Restcl.ClusterV1().Tenant().Create(unauthzCtx, tenant)
	Assert(t, err != nil, "user should be unauthorized to create tenant")
	AssertEventually(t, func() (bool, interface{}) {
		query := es.NewBoolQuery().Must(es.NewTermQuery("resource.kind.keyword", string(cluster.KindTenant)),
			es.NewTermQuery("action.keyword", strings.Title(string(apiintf.CreateOper))),
			es.NewTermQuery("outcome.keyword", auditapi.Outcome_Failure.String()),
			es.NewTermQuery("stage.keyword", auditapi.Stage_RequestAuthorization.String()))
		resp, err := ti.ESClient.Search(context.Background(),
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
	Assert(t, getEvent(unauthzCtx, ti.APIGwAddr, loginEventObj.SelfLink, &auditapi.AuditEvent{}) != nil, "testtenant user should not be able to get event in default tenant")
	// test audit log for call failure
	_, err = ti.Restcl.ClusterV1().Tenant().Create(superAdminCtx, tenant)
	Assert(t, err != nil, "call to create duplicate tenant should fail")
	AssertEventually(t, func() (bool, interface{}) {
		query := es.NewBoolQuery().Must(es.NewTermQuery("resource.kind.keyword", string(cluster.KindTenant)),
			es.NewTermQuery("action.keyword", strings.Title(string(apiintf.CreateOper))),
			es.NewTermQuery("outcome.keyword", auditapi.Outcome_Failure.String()),
			es.NewTermQuery("stage.keyword", auditapi.Stage_RequestProcessing.String()))
		resp, err := ti.ESClient.Search(context.Background(),
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
	ti := TestInfo{Name: t.Name()}
	err := ti.SetupElastic()
	AssertOk(t, err, "setupElastic failed")
	defer ti.TeardownElastic()
	err = ti.StartSpyglass()
	AssertOk(t, err, "failed to start spyglass")
	defer ti.Fdr.Stop()
	err = ti.StartAPIServer()
	AssertOk(t, err, "failed to start API server")
	defer ti.APIServer.Stop()
	err = ti.StartAPIGateway()
	AssertOk(t, err, "failed to start API Gateway")
	defer ti.APIGw.Stop()

	adminCred := &auth.PasswordCredential{
		Username: testUser,
		Password: testPassword,
		Tenant:   globals.DefaultTenant,
	}
	// create default tenant and global admin user
	if err := SetupAuth(ti.APIServerAddr, true, nil, nil, adminCred, ti.Logger); err != nil {
		t.Fatalf("auth setupElastic failed")
	}
	defer CleanupAuth(ti.APIServerAddr, true, false, adminCred, ti.Logger)

	_, err = NewLoggedInContext(context.Background(), ti.APIGwAddr, adminCred)
	AssertOk(t, err, "error creating super admin logged in context")
	MustCreateTenant(ti.Apicl, testTenant)
	defer MustDeleteTenant(ti.Apicl, testTenant)
	MustCreateTestUser(ti.Apicl, testUser, testPassword, testTenant)
	defer MustDeleteUser(ti.Apicl, testUser, testTenant)
	MustCreateRole(ti.Apicl, "NoAuditingPerms", testTenant,
		login.NewPermission(testTenant, string(apiclient.GroupAuth), "", authz.ResourceNamespaceAll, "", auth.Permission_AllActions.String()),
		login.NewPermission(testTenant, string(apiclient.GroupSecurity), "", authz.ResourceNamespaceAll, "", auth.Permission_AllActions.String()),
		login.NewPermission(testTenant, string(apiclient.GroupStaging), "", authz.ResourceNamespaceAll, "", auth.Permission_AllActions.String()),
		login.NewPermission(testTenant, string(apiclient.GroupNetwork), "", authz.ResourceNamespaceAll, "", auth.Permission_AllActions.String()),
		login.NewPermission(testTenant, string(apiclient.GroupWorkload), "", authz.ResourceNamespaceAll, "", auth.Permission_AllActions.String()),
	)
	defer MustDeleteRole(ti.Apicl, "NoAuditingPerms", testTenant)
	MustCreateRoleBinding(ti.Apicl, "NoAuditingPermsRB", testTenant, "NoAuditingPerms", []string{testUser}, nil)
	defer MustDeleteRoleBinding(ti.Apicl, "NoAuditingPermsRB", testTenant)
	userCred := &auth.PasswordCredential{
		Username: testUser,
		Password: testPassword,
		Tenant:   testTenant,
	}
	userCtx, err := NewLoggedInContext(context.Background(), ti.APIGwAddr, userCred)
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
		if err := Search(userCtx, ti.APIGwAddr, query, &resp); err != nil {
			return false, err
		}
		return true, nil
	}, "error performing audit log search")
	Assert(t, resp.ActualHits == 0, fmt.Sprintf("user with no auditing permissions was able to retrieve audit logs: %#v", resp))
	MustCreateRole(ti.Apicl, "AuditingPerms", testTenant,
		login.NewPermission(testTenant, "", auth.Permission_AuditEvent.String(), authz.ResourceNamespaceAll, "", auth.Permission_Read.String()),
	)
	defer MustDeleteRole(ti.Apicl, "AuditingPerms", testTenant)
	MustCreateRoleBinding(ti.Apicl, "AuditingPermsRB", testTenant, "AuditingPerms", []string{testUser}, nil)
	defer MustDeleteRoleBinding(ti.Apicl, "AuditingPermsRB", testTenant)
	resp = search.SearchResponse{}
	AssertEventually(t, func() (bool, interface{}) {
		if err := Search(userCtx, ti.APIGwAddr, query, &resp); err != nil {
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
	ti := TestInfo{Name: t.Name()}
	err := ti.SetupElastic()
	AssertOk(t, err, "setupElastic failed")
	defer ti.TeardownElastic()
	err = ti.StartSpyglass()
	AssertOk(t, err, "failed to start spyglass")
	defer ti.Fdr.Stop()
	err = ti.StartAPIServer()
	AssertOk(t, err, "failed to start API server")
	defer ti.APIServer.Stop()
	err = ti.StartAPIGateway()
	AssertOk(t, err, "failed to start API Gateway")
	defer ti.APIGw.Stop()

	adminCred := &auth.PasswordCredential{
		Username: testUser,
		Password: testPassword,
		Tenant:   globals.DefaultTenant,
	}
	// create default tenant and global admin user
	if err := SetupAuth(ti.APIServerAddr, true, nil, nil, adminCred, ti.Logger); err != nil {
		t.Fatalf("auth setupElastic failed")
	}
	defer CleanupAuth(ti.APIServerAddr, true, false, adminCred, ti.Logger)

	superAdminCtx, err := NewLoggedInContext(context.Background(), ti.APIGwAddr, adminCred)
	AssertOk(t, err, "error creating logged in context")
	tuser := &auth.User{}
	tuser.Defaults("all")
	tuser.Name = "testuser2"
	tuser.Spec.Password = testPassword
	tuser.Spec.Email = "testuser2@pensando.io"
	_, err = ti.Restcl.AuthV1().User().Create(superAdminCtx, tuser)
	AssertOk(t, err, "error creating test user")
	defer MustDeleteUser(ti.Apicl, "testuser2", globals.DefaultTenant)
	// test auth policy
	apolicy, err := ti.Apicl.AuthV1().AuthenticationPolicy().Get(superAdminCtx, &api.ObjectMeta{})
	AssertOk(t, err, "unable to fetch auth policy")
	apolicy.Spec.Authenticators.Ldap = &auth.Ldap{
		Domains: []*auth.LdapDomain{
			{
				BindDN:       "CN=admin, DC=Pensando, DC=io",
				BindPassword: testPassword,
			},
		},
	}
	_, err = ti.Restcl.AuthV1().AuthenticationPolicy().Update(superAdminCtx, apolicy)
	AssertOk(t, err, "unable to update auth policy")
	// test reset password
	passwdResetReq := &auth.PasswordResetRequest{}
	passwdResetReq.Defaults("all")
	passwdResetReq.ObjectMeta.Name = "testuser2"
	passwdResetReq.ObjectMeta.Tenant = globals.DefaultTenant
	passwdResetReq.Kind = string(auth.KindUser)
	tuser, err = ti.Restcl.AuthV1().User().PasswordReset(superAdminCtx, passwdResetReq)
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
			err := Search(superAdminCtx, ti.APIGwAddr, query, &resp)
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
			err := Search(superAdminCtx, ti.APIGwAddr, query, &resp)
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
			err := Search(superAdminCtx, ti.APIGwAddr, query, &resp)
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
	ti := TestInfo{Name: t.Name()}
	err := ti.SetupElastic()
	AssertOk(t, err, "setupElastic failed")
	err = ti.StartSpyglass()
	AssertOk(t, err, "failed to start spyglass")
	defer ti.Fdr.Stop()
	err = ti.StartAPIServer()
	AssertOk(t, err, "failed to start API server")
	defer ti.APIServer.Stop()
	err = ti.StartAPIGateway()
	AssertOk(t, err, "failed to start API Gateway")
	defer ti.APIGw.Stop()

	adminCred := &auth.PasswordCredential{
		Username: testUser,
		Password: testPassword,
		Tenant:   globals.DefaultTenant,
	}
	// create default tenant and global admin user
	if err := SetupAuth(ti.APIServerAddr, true, nil, nil, adminCred, ti.Logger); err != nil {
		t.Fatalf("auth setupElastic failed")
	}
	defer CleanupAuth(ti.APIServerAddr, true, false, adminCred, ti.Logger)

	// stop elastic
	testutils.StopElasticsearch(ti.ElasticSearchName, ti.ElasticSearchDir)
	// login should work if elastic is down
	ctx, err := login.NewLoggedInContext(context.Background(), ti.APIGwAddr, adminCred)
	AssertOk(t, err, "super admin should be able to log in when elastic is down")
	// get operations should work
	policy, err := ti.Restcl.AuthV1().AuthenticationPolicy().Get(ctx, &api.ObjectMeta{})
	AssertOk(t, err, "unable to fetch auth policy")
	// update operations should fail
	nctx, cancel := context.WithTimeout(ctx, 2*time.Second)
	_, err = ti.Restcl.AuthV1().AuthenticationPolicy().Update(nctx, policy)
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
	tsr, err = ti.Restcl.MonitoringV1().TechSupportRequest().Create(ctx, tsr)
	AssertOk(t, err, "unable to create techsupport request")
	_, err = ti.Restcl.MonitoringV1().TechSupportRequest().Delete(ctx, &tsr.ObjectMeta)
	AssertOk(t, err, "unable to delete techsupport request")
}

func TestAPIServerNotAvailable(t *testing.T) {
	ti := TestInfo{Name: t.Name()}
	err := ti.SetupElastic()
	AssertOk(t, err, "setupElastic failed")
	err = ti.StartSpyglass()
	AssertOk(t, err, "failed to start spyglass")
	defer ti.Fdr.Stop()
	err = ti.StartAPIServer()
	AssertOk(t, err, "failed to start API server")
	defer ti.APIServer.Stop()
	err = ti.StartAPIGateway()
	AssertOk(t, err, "failed to start API Gateway")
	defer ti.APIGw.Stop()
	userCred := &auth.PasswordCredential{
		Username: testUser,
		Password: testPassword,
		Tenant:   globals.DefaultTenant,
	}
	// create tenant and admin user
	if err := SetupAuth(ti.APIServerAddr, true, nil, nil, userCred, ti.Logger); err != nil {
		t.Fatalf("auth setup failed")
	}
	_, err = NewLoggedInContext(context.TODO(), ti.APIGwAddr, userCred)
	AssertOk(t, err, "unable to get logged in context")
	ti.APIServer.Stop()
	AssertEventually(t, func() (bool, interface{}) {
		_, _, err := login.UserLogin(context.TODO(), ti.APIGwAddr, userCred)
		if err != nil && strings.Contains(err.Error(), "503") {
			return true, nil
		}
		return false, err
	}, "unexpected error when API server is unreachable")
	err = ti.StartAPIServer()
	AssertOk(t, err, "failed to start API server")
	CleanupAuth(ti.APIServerAddr, true, false, userCred, ti.Logger)
}

func TestSyslogAuditor(t *testing.T) {
	ti := TestInfo{Name: t.Name()}
	err := ti.SetupElastic()
	AssertOk(t, err, "setupElastic failed")
	defer ti.TeardownElastic()
	err = ti.StartSyslogServers()
	AssertOk(t, err, "failed to start syslog server")
	defer ti.StopSyslogServers()
	err = ti.StartSpyglass()
	AssertOk(t, err, "failed to start spyglass")
	defer ti.Fdr.Stop()
	err = ti.StartAPIServer()
	AssertOk(t, err, "failed to start API server")
	defer ti.APIServer.Stop()
	err = ti.StartAPIGateway()
	AssertOk(t, err, "failed to start API Gateway")
	defer ti.APIGw.Stop()

	adminCred := &auth.PasswordCredential{
		Username: testUser,
		Password: testPassword,
		Tenant:   globals.DefaultTenant,
	}
	// create default tenant and global admin user
	if err := SetupAuth(ti.APIServerAddr, true, nil, nil, adminCred, ti.Logger); err != nil {
		t.Fatalf("auth setup failed")
	}
	defer CleanupAuth(ti.APIServerAddr, true, false, adminCred, ti.Logger)

	superAdminCtx, err := NewLoggedInContext(context.Background(), ti.APIGwAddr, adminCred)
	AssertOk(t, err, "error creating logged in context")
	bsdPolicy := &monitoring.AuditPolicy{
		TypeMeta: api.TypeMeta{
			Kind: string(monitoring.KindAuditPolicy),
		},
		ObjectMeta: api.ObjectMeta{
			Name:      "audit-policy-1",
			Tenant:    globals.DefaultTenant,
			Namespace: globals.DefaultNamespace,
		},
		Spec: monitoring.AuditPolicySpec{
			Syslog: &monitoring.SyslogAuditor{
				Enabled: true,
				Format:  monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
				Targets: []*monitoring.ExportConfig{

					{
						Destination: ti.SyslogBSDInfo.IP,
						Transport:   fmt.Sprintf("%s/%s", "UDP", ti.SyslogBSDInfo.Port),
					},
				},
				SyslogConfig: &monitoring.SyslogExportConfig{
					FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
					Prefix:           "pen-audit-events",
				},
			},
		},
	}
	AssertEventually(t, func() (bool, interface{}) {
		policy, err := ti.Restcl.MonitoringV1().AuditPolicy().Create(superAdminCtx, bsdPolicy)
		if err != nil {
			return false, err
		}
		return true, policy
	}, "failed to create BSD syslog audit policy")
	defer ti.Apicl.MonitoringV1().AuditPolicy().Delete(superAdminCtx, &bsdPolicy.ObjectMeta)
	rfcPolicy := &monitoring.AuditPolicy{}
	bsdPolicy.Clone(rfcPolicy)
	rfcPolicy.Name = "audit-policy-2"
	rfcPolicy.Spec.Syslog.Format = monitoring.MonitoringExportFormat_SYSLOG_RFC5424.String()
	_, err = ti.Restcl.MonitoringV1().AuditPolicy().Create(superAdminCtx, rfcPolicy)
	Assert(t, err != nil, "no error in creating duplicate audit policy")
	// create syslog
	AssertEventually(t, func() (bool, interface{}) {
		superAdminCtx, err = NewLoggedInContext(context.Background(), ti.APIGwAddr, adminCred)
		if err != nil {
			return false, err
		}
		select {
		case logparts := <-ti.SyslogBSDInfo.Ch:
			val, ok := logparts["content"]
			if !ok {
				return false, fmt.Errorf("no message field: %+v", logparts)
			}
			msg := val.(string)
			if !strings.Contains(msg, "\"res-kind\":\"User\"") {
				return false, fmt.Errorf("unexpected syslog message %v", msg)
			}
			return true, nil
		case <-time.After(time.Second):
			return false, fmt.Errorf("failed to receive any syslog message")
		}
	}, "failed to receive any BSD syslog audit message for user login", "1s", "10s")
	// update audit policy to point to rfc5424 syslog server
	bsdPolicy.Spec.Syslog.Format = monitoring.MonitoringExportFormat_SYSLOG_RFC5424.String()
	bsdPolicy.Spec.Syslog.Targets[0].Destination = ti.SyslogRFC5424Info.IP
	bsdPolicy.Spec.Syslog.Targets[0].Transport = fmt.Sprintf("%s/%s", "UDP", ti.SyslogRFC5424Info.Port)
	AssertEventually(t, func() (bool, interface{}) {
		policy, err := ti.Restcl.MonitoringV1().AuditPolicy().Update(superAdminCtx, bsdPolicy)
		if err != nil {
			return false, err
		}
		return true, policy
	}, "failed to update syslog audit policy")
	// create rfc5424 syslog
	AssertEventually(t, func() (bool, interface{}) {
		superAdminCtx, err = NewLoggedInContext(context.Background(), ti.APIGwAddr, adminCred)
		if err != nil {
			return false, err
		}
		select {
		case logparts := <-ti.SyslogRFC5424Info.Ch:
			val, ok := logparts["structured_data"]
			if !ok {
				return false, fmt.Errorf("no message field: %+v", logparts)
			}
			msg := val.(string)
			if !strings.Contains(msg, "res-kind=\"User\"") {
				return false, fmt.Errorf("unexpected syslog message %v", msg)
			}
			return true, nil
		case <-time.After(time.Second):
			return false, fmt.Errorf("failed to receive any syslog message")
		}
	}, "failed to receive any rfc5424 syslog audit message for user login", "1s", "10s")
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
	ti := TestInfo{Name: b.Name()}
	err := ti.SetupElastic()
	if err != nil {
		panic(fmt.Sprintf("setupElastic failed with error: %v", err))
	}
	defer ti.TeardownElastic()
	auditor := auditmgr.WithAuditors(elasticauditor.NewSynchAuditor(ti.ElasticSearchAddr, ti.Rslvr, ti.Logger, elasticauditor.WithElasticClient(ti.ESClient)))
	err = auditor.Run()
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

// TestStagingBufferBulkeditAuditLogs Checks to ensure that multiple audit logs are being generated,
// one for each action when performing a bulkedit operation on a staging buffer
func TestStagingBufferBulkeditAuditLogs(t *testing.T) {
	ti := TestInfo{Name: t.Name()}
	err := ti.SetupElastic()
	AssertOk(t, err, "setupElastic failed")
	defer ti.TeardownElastic()
	err = ti.StartSpyglass()
	AssertOk(t, err, "failed to start spyglass")
	defer ti.Fdr.Stop()
	err = ti.StartAPIServer()
	AssertOk(t, err, "failed to start API server")
	defer ti.APIServer.Stop()
	err = ti.StartAPIGateway()
	AssertOk(t, err, "failed to start API Gateway")
	defer ti.APIGw.Stop()

	adminCred := &auth.PasswordCredential{
		Username: testUser,
		Password: testPassword,
		Tenant:   globals.DefaultTenant,
	}
	// create default tenant and global admin user
	if err := SetupAuth(ti.APIServerAddr, true, nil, nil, adminCred, ti.Logger); err != nil {
		t.Fatalf("auth setupElastic failed")
	}
	defer CleanupAuth(ti.APIServerAddr, true, false, adminCred, ti.Logger)

	ctx, err := NewLoggedInContext(context.Background(), ti.APIGwAddr, adminCred)
	AssertOk(t, err, "error creating logged in context")

	// Create staging buffer
	stagingBufferName := "IntegTestStagingBuffer"
	AssertEventually(t, func() (bool, interface{}) {
		_, err = ti.Restcl.StagingV1().Buffer().Create(ctx, &staging.Buffer{ObjectMeta: api.ObjectMeta{Name: stagingBufferName, Tenant: globals.DefaultTenant}})
		return err == nil, nil
	}, fmt.Sprintf("unable to create staging buffer, err: %v", err))
	defer ti.Restcl.StagingV1().Buffer().Delete(ctx, &api.ObjectMeta{Name: stagingBufferName, Tenant: globals.DefaultTenant})

	netw1 := network.Network{
		TypeMeta: api.TypeMeta{
			Kind:       "Network",
			APIVersion: "v1",
		},
		ObjectMeta: api.ObjectMeta{
			Tenant:    globals.DefaultTenant,
			Namespace: globals.DefaultNamespace,
			Name:      "TestStagingNetw1",
		},
		Spec: network.NetworkSpec{
			Type:        network.NetworkType_Bridged.String(),
			IPv4Subnet:  "10.1.1.1/24",
			IPv4Gateway: "10.1.1.1",
			VlanID:      11,
		},
	}

	n1, err := types.MarshalAny(&netw1)
	AssertOk(t, err, "error marshalling network netw1")
	netw2 := netw1
	netw2.ObjectMeta.Name = "TestStagingNetw2"
	netw2.Spec.IPv4Subnet = "12.1.1.1/24"
	netw2.Spec.VlanID = 12
	n2, err := types.MarshalAny(&netw2)
	AssertOk(t, err, "error marshalling network netw2")
	netw3 := netw1
	netw3.ObjectMeta.Name = "TestStagingNetw3"
	netw3.Spec.IPv4Subnet = "13.1.1.1/24"
	netw3.Spec.VlanID = 13
	n3, err := types.MarshalAny(&netw3)
	AssertOk(t, err, "error marshalling network netw3")

	bulkEditReq := &staging.BulkEditAction{
		ObjectMeta: api.ObjectMeta{
			Name:      stagingBufferName,
			Tenant:    globals.DefaultTenant,
			Namespace: globals.DefaultNamespace,
		},
		Spec: bulkedit.BulkEditActionSpec{
			Items: []*bulkedit.BulkEditItem{
				&bulkedit.BulkEditItem{
					Method: bulkedit.BulkEditItem_CREATE.String(),
					Object: &api.Any{Any: *n1},
				},
				&bulkedit.BulkEditItem{
					Method: bulkedit.BulkEditItem_CREATE.String(),
					Object: &api.Any{Any: *n2},
				},
				&bulkedit.BulkEditItem{
					Method: bulkedit.BulkEditItem_CREATE.String(),
					Object: &api.Any{Any: *n3},
				},
			},
		},
	}

	bEResp, err := ti.Restcl.StagingV1().Buffer().Bulkedit(ctx, bulkEditReq)
	AssertOk(t, err, "error Creating networks via bulkedit")
	Assert(t, bEResp.Status.ValidationResult == "success", "Bulkedit Validation failure")

	ca := staging.CommitAction{}
	ca.ObjectMeta = bulkEditReq.ObjectMeta
	_, err = ti.Restcl.StagingV1().Buffer().Commit(ctx, &ca)
	AssertOk(t, err, "error Committing networks via bulkedit")

	// search for audit events for create oper
	stages := []string{auditapi.Stage_RequestAuthorization.String(), auditapi.Stage_RequestProcessing.String()}
	netwNames := []string{"TestStagingNetw1", "TestStagingNetw2", "TestStagingNetw3"}
	for _, netw := range netwNames {
		for _, stage := range stages {
			query := &search.SearchRequest{
				Query: &search.SearchQuery{
					Kinds: []string{auth.Permission_AuditEvent.String()},
					Fields: &fields.Selector{
						Requirements: []*fields.Requirement{
							{
								Key:      "action",
								Operator: "equals",
								Values:   []string{string(apiintf.CreateOper)},
							},
							{
								Key:      "outcome",
								Operator: "equals",
								Values:   []string{strings.ToLower(auditapi.Outcome_Success.String())},
							},
							{
								Key:      "resource.kind",
								Operator: "equals",
								Values:   []string{string("Network")},
							},
							{
								Key:      "resource.name",
								Operator: "equals",
								Values:   []string{netw},
							},
							{
								Key:      "stage",
								Operator: "equals",
								Values:   []string{strings.ToLower(stage)},
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
				err := Search(ctx, ti.APIGwAddr, query, &resp)
				if err != nil {
					return false, err
				}
				if resp.ActualHits == 0 {
					return false, fmt.Errorf("no audit logs for network create at stage %s", stage)
				}
				if resp.ActualHits > 1 {
					return false, fmt.Errorf("unexpected number of audit logs: %d", resp.ActualHits)
				}
				return true, nil
			}, "error performing audit log search")

			events := resp.AggregatedEntries.Tenants[globals.DefaultTenant].Categories[globals.Kind2Category("AuditEvent")].Kinds[auth.Permission_AuditEvent.String()].Entries
			Assert(t, (events[0].Object.Action == string(apiintf.CreateOper)) &&
				(events[0].Object.Resource.Kind == string("Network")) &&
				(events[0].Object.Outcome == auditapi.Outcome_Success.String()) &&
				(events[0].Object.Stage == strings.ToLower(stage)), fmt.Sprintf("unexpected audit event: %#v", *events[0]))
		}
	}

	netw1.Spec.VlanID = 101
	n1, err = types.MarshalAny(&netw1)
	AssertOk(t, err, "error marshalling network netw1")
	netw2.Spec.VlanID = 102
	n2, err = types.MarshalAny(&netw2)
	AssertOk(t, err, "error marshalling network netw2")
	netw3.Spec.VlanID = 103
	n3, err = types.MarshalAny(&netw3)
	AssertOk(t, err, "error marshalling network netw3")

	bulkEditReq = &staging.BulkEditAction{
		ObjectMeta: api.ObjectMeta{
			Name:      stagingBufferName,
			Tenant:    globals.DefaultTenant,
			Namespace: globals.DefaultNamespace,
		},
		Spec: bulkedit.BulkEditActionSpec{
			Items: []*bulkedit.BulkEditItem{
				&bulkedit.BulkEditItem{
					Method: bulkedit.BulkEditItem_UPDATE.String(),
					Object: &api.Any{Any: *n1},
				},
				&bulkedit.BulkEditItem{
					Method: bulkedit.BulkEditItem_UPDATE.String(),
					Object: &api.Any{Any: *n2},
				},
				&bulkedit.BulkEditItem{
					Method: bulkedit.BulkEditItem_UPDATE.String(),
					Object: &api.Any{Any: *n3},
				},
			},
		},
	}

	bEResp, err = ti.Restcl.StagingV1().Buffer().Bulkedit(ctx, bulkEditReq)
	AssertOk(t, err, "error Creating networks via bulkedit")
	Assert(t, bEResp.Status.ValidationResult == "success", "Bulkedit Validation failure")

	ca = staging.CommitAction{}
	ca.ObjectMeta = bulkEditReq.ObjectMeta
	_, err = ti.Restcl.StagingV1().Buffer().Commit(ctx, &ca)
	AssertOk(t, err, "error Committing networks via bulkedit")

	// search for audit events for update oper
	stages = []string{auditapi.Stage_RequestAuthorization.String(), auditapi.Stage_RequestProcessing.String()}
	for _, netw := range netwNames {
		for _, stage := range stages {
			query := &search.SearchRequest{
				Query: &search.SearchQuery{
					Kinds: []string{auth.Permission_AuditEvent.String()},
					Fields: &fields.Selector{
						Requirements: []*fields.Requirement{
							{
								Key:      "action",
								Operator: "equals",
								Values:   []string{string(apiintf.UpdateOper)},
							},
							{
								Key:      "outcome",
								Operator: "equals",
								Values:   []string{auditapi.Outcome_Success.String()},
							},
							{
								Key:      "resource.kind",
								Operator: "equals",
								Values:   []string{string("Network")},
							},
							{
								Key:      "resource.name",
								Operator: "equals",
								Values:   []string{netw},
							},
							{
								Key:      "stage",
								Operator: "equals",
								Values:   []string{strings.ToLower(stage)},
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
				err := Search(ctx, ti.APIGwAddr, query, &resp)
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
			Assert(t, (events[0].Object.Action == string(apiintf.UpdateOper)) &&
				(events[0].Object.Resource.Kind == string("Network")) &&
				(events[0].Object.Outcome == auditapi.Outcome_Success.String()) &&
				(events[0].Object.Stage == strings.ToLower(stage)), fmt.Sprintf("unexpected audit event: %#v", *events[0]))
		}
	}

	bulkEditReq = &staging.BulkEditAction{
		ObjectMeta: api.ObjectMeta{
			Name:      stagingBufferName,
			Tenant:    globals.DefaultTenant,
			Namespace: globals.DefaultNamespace,
		},
		Spec: bulkedit.BulkEditActionSpec{
			Items: []*bulkedit.BulkEditItem{
				&bulkedit.BulkEditItem{
					Method: bulkedit.BulkEditItem_DELETE.String(),
					Object: &api.Any{Any: *n1},
				},
				&bulkedit.BulkEditItem{
					Method: bulkedit.BulkEditItem_DELETE.String(),
					Object: &api.Any{Any: *n2},
				},
				&bulkedit.BulkEditItem{
					Method: bulkedit.BulkEditItem_DELETE.String(),
					Object: &api.Any{Any: *n3},
				},
			},
		},
	}

	bEResp, err = ti.Restcl.StagingV1().Buffer().Bulkedit(ctx, bulkEditReq)
	AssertOk(t, err, "error Creating networks via bulkedit")
	Assert(t, bEResp.Status.ValidationResult == "success", "Bulkedit Validation failure")

	ca = staging.CommitAction{}
	ca.ObjectMeta = bulkEditReq.ObjectMeta
	_, err = ti.Restcl.StagingV1().Buffer().Commit(ctx, &ca)
	AssertOk(t, err, "error Committing networks via bulkedit")

	// search for audit events for delete oper
	stages = []string{auditapi.Stage_RequestAuthorization.String(), auditapi.Stage_RequestProcessing.String()}
	for _, netw := range netwNames {
		for _, stage := range stages {
			query := &search.SearchRequest{
				Query: &search.SearchQuery{
					Kinds: []string{auth.Permission_AuditEvent.String()},
					Fields: &fields.Selector{
						Requirements: []*fields.Requirement{
							{
								Key:      "action",
								Operator: "equals",
								Values:   []string{string(apiintf.DeleteOper)},
							},
							{
								Key:      "outcome",
								Operator: "equals",
								Values:   []string{auditapi.Outcome_Success.String()},
							},
							{
								Key:      "resource.kind",
								Operator: "equals",
								Values:   []string{string("Network")},
							},
							{
								Key:      "resource.name",
								Operator: "equals",
								Values:   []string{netw},
							},
							{
								Key:      "stage",
								Operator: "equals",
								Values:   []string{strings.ToLower(stage)},
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
				err := Search(ctx, ti.APIGwAddr, query, &resp)
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
			Assert(t, (events[0].Object.Action == string(apiintf.DeleteOper)) &&
				(events[0].Object.Resource.Kind == string("Network")) &&
				(events[0].Object.Outcome == auditapi.Outcome_Success.String()) &&
				(events[0].Object.Stage == strings.ToLower(stage)), fmt.Sprintf("unexpected audit event: %#v", *events[0]))
		}
	}
}
