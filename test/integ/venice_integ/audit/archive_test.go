package audit

import (
	"context"
	"fmt"
	"io"
	"strings"
	"sync"
	"testing"
	"time"

	"github.com/gogo/protobuf/types"
	uuid "github.com/satori/go.uuid"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/fields"
	"github.com/pensando/sw/api/generated/apiclient"
	auditapi "github.com/pensando/sw/api/generated/audit"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/objstore"
	"github.com/pensando/sw/api/login"
	. "github.com/pensando/sw/test/utils"
	"github.com/pensando/sw/venice/globals"
	. "github.com/pensando/sw/venice/utils/archive/testutils"
	elasticauditor "github.com/pensando/sw/venice/utils/audit/elastic"
	auditmgr "github.com/pensando/sw/venice/utils/audit/manager"
	. "github.com/pensando/sw/venice/utils/authn/testutils"
	"github.com/pensando/sw/venice/utils/authz"
	objstore2 "github.com/pensando/sw/venice/utils/objstore/client"
	. "github.com/pensando/sw/venice/utils/testutils"
)

type testData struct {
	name   string
	req    *monitoring.ArchiveRequest
	status string
	evtStr string
}

func TestAuditLogArchive(t *testing.T) {
	ti := TestInfo{Name: t.Name()}
	err := ti.SetupElastic()
	AssertOk(t, err, "setupElastic failed")
	defer ti.TeardownElastic()
	err = ti.StartAPIServer()
	AssertOk(t, err, "failed to start API server")
	defer ti.APIServer.Stop()
	err = ti.StartSpyglass()
	AssertOk(t, err, "failed to start spyglass")
	err = ti.StartAPIGateway()
	AssertOk(t, err, "failed to start API Gateway")
	defer ti.APIGw.Stop()

	pastTime1 := time.Now().Add(-6 * time.Second)
	pastTime2 := pastTime1.Add(3 * time.Second)
	ts, _ := types.TimestampProto(pastTime1)
	ts1 := api.Timestamp{Timestamp: *ts}
	ts, _ = types.TimestampProto(pastTime2)
	ts2 := api.Timestamp{Timestamp: *ts}

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
	superAdminCtx, err := NewLoggedInContext(context.TODO(), ti.APIGwAddr, adminCred)
	AssertOk(t, err, "error creating logged in context")
	Assert(t, createAuditLogs(t, ti, 50) > 0, "unable to create audit logs")
	currts, _ := types.TimestampProto(time.Now())
	tests := []testData{
		{
			name: "query all audit logs",
			req: &monitoring.ArchiveRequest{
				TypeMeta: api.TypeMeta{Kind: string(monitoring.KindArchiveRequest)},
				ObjectMeta: api.ObjectMeta{
					Name:   "testArch1",
					Tenant: globals.DefaultTenant,
				},
				Spec: monitoring.ArchiveRequestSpec{
					Type:  monitoring.ArchiveRequestSpec_AuditEvent.String(),
					Query: &monitoring.ArchiveQuery{},
				},
			},
			status: monitoring.ArchiveRequestStatus_Completed.String(),
			evtStr: "\"kind\":\"AuditEvent\"",
		},
		{
			name: "no audit logs matched based on time range",
			req: &monitoring.ArchiveRequest{
				TypeMeta: api.TypeMeta{Kind: string(monitoring.KindArchiveRequest)},
				ObjectMeta: api.ObjectMeta{
					Name:   "testArch2",
					Tenant: globals.DefaultTenant,
				},
				Spec: monitoring.ArchiveRequestSpec{
					Type: monitoring.ArchiveRequestSpec_AuditEvent.String(),
					Query: &monitoring.ArchiveQuery{
						StartTime: &ts1,
						EndTime:   &ts2,
					},
				},
			},
			status: monitoring.ArchiveRequestStatus_Completed.String(),
			evtStr: "",
		},
		{
			name: "audit logs matched based on time range",
			req: &monitoring.ArchiveRequest{
				TypeMeta: api.TypeMeta{Kind: string(monitoring.KindArchiveRequest)},
				ObjectMeta: api.ObjectMeta{
					Name:   "testArch3",
					Tenant: globals.DefaultTenant,
				},
				Spec: monitoring.ArchiveRequestSpec{
					Type: monitoring.ArchiveRequestSpec_AuditEvent.String(),
					Query: &monitoring.ArchiveQuery{
						StartTime: &ts2,
						EndTime:   &api.Timestamp{Timestamp: *currts},
					},
				},
			},
			status: monitoring.ArchiveRequestStatus_Completed.String(),
			evtStr: "\"kind\":\"AuditEvent\"",
		},
		{
			name: "audit logs matched based on field names",
			req: &monitoring.ArchiveRequest{
				TypeMeta: api.TypeMeta{Kind: string(monitoring.KindArchiveRequest)},
				ObjectMeta: api.ObjectMeta{
					Name:   "testArch4",
					Tenant: globals.DefaultTenant,
				},
				Spec: monitoring.ArchiveRequestSpec{
					Type: monitoring.ArchiveRequestSpec_AuditEvent.String(),
					Query: &monitoring.ArchiveQuery{
						Fields: &fields.Selector{
							Requirements: []*fields.Requirement{
								{
									Key:      "resource.kind",
									Operator: "equals",
									Values:   []string{string(network.KindNetwork)},
								},
							},
						},
					},
				},
			},
			status: monitoring.ArchiveRequestStatus_Completed.String(),
			evtStr: "\"kind\":\"AuditEvent\"",
		},
		{
			name: "no audit logs matched based on field names",
			req: &monitoring.ArchiveRequest{
				TypeMeta: api.TypeMeta{Kind: string(monitoring.KindArchiveRequest)},
				ObjectMeta: api.ObjectMeta{
					Name:   "testArch5",
					Tenant: globals.DefaultTenant,
				},
				Spec: monitoring.ArchiveRequestSpec{
					Type: monitoring.ArchiveRequestSpec_AuditEvent.String(),
					Query: &monitoring.ArchiveQuery{
						Fields: &fields.Selector{
							Requirements: []*fields.Requirement{
								{
									Key:      "resource.kind",
									Operator: "equals",
									Values:   []string{string(network.KindNetwork)},
								},
								{
									Key:      "outcome",
									Operator: "equals",
									Values:   []string{auditapi.Outcome_Failure.String()},
								},
							},
						},
					},
				},
			},
			status: monitoring.ArchiveRequestStatus_Completed.String(),
			evtStr: "",
		},
		{
			name: "nil query",
			req: &monitoring.ArchiveRequest{
				TypeMeta: api.TypeMeta{Kind: string(monitoring.KindArchiveRequest)},
				ObjectMeta: api.ObjectMeta{
					Name:   "testArch6",
					Tenant: globals.DefaultTenant,
				},
				Spec: monitoring.ArchiveRequestSpec{
					Type:  monitoring.ArchiveRequestSpec_AuditEvent.String(),
					Query: nil,
				},
			},
			status: monitoring.ArchiveRequestStatus_Completed.String(),
			evtStr: "\"kind\":\"AuditEvent\"",
		},
	}
	for i := 0; i < len(tests); i++ {
		AssertEventually(t, func() (bool, interface{}) {
			createArchiveRequests(superAdminCtx, t, ti, tests[i:i+1])
			err = verifyArchiveRequests(superAdminCtx, t, ti, tests[i:i+1])
			if err != nil {
				return false, err
			}
			deleteArchiveRequests(superAdminCtx, t, ti, tests[i:i+1])
			return true, nil
		}, "error verifying archive requests", "3s", "30s")
	}
	// stop spyglass
	ti.Fdr.Stop()
	// create archive requests
	createArchiveRequests(superAdminCtx, t, ti, tests[0:1])
	// start spyglass
	ti.StartSpyglass()
	defer ti.Fdr.Stop()
	AssertEventually(t, func() (bool, interface{}) {
		err = verifyArchiveRequests(superAdminCtx, t, ti, tests[0:1])
		if err != nil {
			return false, err
		}
		return true, nil
	}, "error verifying archive request after spyglass restart", "3s", "30s")
	deleteArchiveRequests(superAdminCtx, t, ti, tests[0:1])
}

func TestDuplicateArchiveRequests(t *testing.T) {
	ti := TestInfo{Name: t.Name()}
	err := ti.SetupElastic()
	AssertOk(t, err, "setupElastic failed")
	defer ti.TeardownElastic()
	err = ti.StartAPIServer()
	AssertOk(t, err, "failed to start API server")
	defer ti.APIServer.Stop()
	err = ti.StartSpyglass()
	AssertOk(t, err, "failed to start spyglass")
	defer ti.Fdr.Stop()
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
	superAdminCtx, err := NewLoggedInContext(context.TODO(), ti.APIGwAddr, adminCred)
	AssertOk(t, err, "error creating logged in context")
	var waitgrp sync.WaitGroup
	numReqs := 20
	for i := 0; i < numReqs; i++ {
		waitgrp.Add(1)
		go func(j int) {
			defer waitgrp.Done()
			req := &monitoring.ArchiveRequest{
				TypeMeta: api.TypeMeta{Kind: string(monitoring.KindArchiveRequest)},
				ObjectMeta: api.ObjectMeta{
					Name:   fmt.Sprintf("testArch%d", j),
					Tenant: globals.DefaultTenant,
				},
				Spec: monitoring.ArchiveRequestSpec{
					Type:  monitoring.ArchiveRequestSpec_AuditEvent.String(),
					Query: &monitoring.ArchiveQuery{},
				},
			}
			MustCreateArchiveRequest(superAdminCtx, ti.Restcl, req)
		}(i)
	}
	waitgrp.Wait()
	var mutex sync.RWMutex
	var failedCount, failedDupCount, successCount, cancelCount int
	for i := 0; i < numReqs; i++ {
		waitgrp.Add(1)
		go func(j int) {
			defer waitgrp.Done()
			req := &monitoring.ArchiveRequest{
				TypeMeta: api.TypeMeta{Kind: string(monitoring.KindArchiveRequest)},
				ObjectMeta: api.ObjectMeta{
					Name:   fmt.Sprintf("testArch%d", j),
					Tenant: globals.DefaultTenant,
				},
				Spec: monitoring.ArchiveRequestSpec{
					Type:  monitoring.ArchiveRequestSpec_AuditEvent.String(),
					Query: &monitoring.ArchiveQuery{},
				},
			}
			fetchedReq := &monitoring.ArchiveRequest{}
			if !CheckEventually(func() (bool, interface{}) {
				fetchedReq, err = ti.Restcl.MonitoringV1().ArchiveRequest().Get(superAdminCtx, &req.ObjectMeta)
				if err != nil {
					return false, err
				}
				switch fetchedReq.Status.Status {
				case monitoring.ArchiveRequestStatus_Failed.String():
					mutex.Lock()
					if strings.Contains(fetchedReq.Status.Reason, "already running for log type") {
						failedDupCount++
					} else {
						failedCount++
					}
					mutex.Unlock()
					return true, fetchedReq
				case monitoring.ArchiveRequestStatus_Completed.String():
					mutex.Lock()
					successCount++
					mutex.Unlock()
					return true, fetchedReq
				case monitoring.ArchiveRequestStatus_Canceled.String():
					mutex.Lock()
					cancelCount++
					mutex.Unlock()
					return true, fetchedReq
				default:
					return false, fetchedReq
				}
			}, "1s", "30s") {
				t.Logf("unexpected archive request status for %v", fetchedReq)
			}
		}(i)
	}
	waitgrp.Wait()
	Assert(t, failedDupCount > 0, fmt.Sprintf("unexpected failed count due to duplicate requests %d", failedCount))
	Assert(t, successCount > 0, fmt.Sprintf("unexpected success count %d", successCount))
	Assert(t, failedCount+failedDupCount+successCount+cancelCount == numReqs,
		fmt.Sprintf("unexpected total of failed [%d], dup failed [%d], success [%d], cancel [%d] count", failedCount, failedDupCount, successCount, cancelCount))
	t.Logf("failed %d, dup failed %d, success %d, cancel %d", failedCount, failedDupCount, successCount, cancelCount)
	for i := 0; i < numReqs; i++ {
		req := &monitoring.ArchiveRequest{
			TypeMeta: api.TypeMeta{Kind: string(monitoring.KindArchiveRequest)},
			ObjectMeta: api.ObjectMeta{
				Name:   fmt.Sprintf("testArch%d", i),
				Tenant: globals.DefaultTenant,
			},
			Spec: monitoring.ArchiveRequestSpec{
				Type:  monitoring.ArchiveRequestSpec_AuditEvent.String(),
				Query: &monitoring.ArchiveQuery{},
			},
		}
		MustDeleteArchiveRequest(superAdminCtx, ti.Restcl, &req.ObjectMeta)
	}
}

func TestCancelArchiveRequests(t *testing.T) {
	ti := TestInfo{Name: t.Name()}
	err := ti.SetupElastic()
	AssertOk(t, err, "setupElastic failed")
	defer ti.TeardownElastic()
	err = ti.StartAPIServer()
	AssertOk(t, err, "failed to start API server")
	defer ti.APIServer.Stop()
	err = ti.StartSpyglass()
	AssertOk(t, err, "failed to start spyglass")
	defer ti.Fdr.Stop()
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
	superAdminCtx, err := NewLoggedInContext(context.TODO(), ti.APIGwAddr, adminCred)
	AssertOk(t, err, "error creating logged in context")
	createAuditLogs(t, ti, 100)
	fetchedReq := &monitoring.ArchiveRequest{}
	AssertEventually(t, func() (bool, interface{}) {
		req := &monitoring.ArchiveRequest{
			TypeMeta: api.TypeMeta{Kind: string(monitoring.KindArchiveRequest)},
			ObjectMeta: api.ObjectMeta{
				Name:   "testArch",
				Tenant: globals.DefaultTenant,
			},
			Spec: monitoring.ArchiveRequestSpec{
				Type:  monitoring.ArchiveRequestSpec_AuditEvent.String(),
				Query: &monitoring.ArchiveQuery{},
			},
		}
		MustCreateArchiveRequest(superAdminCtx, ti.Restcl, req)
		defer MustDeleteArchiveRequest(superAdminCtx, ti.Restcl, &req.ObjectMeta)
		cancelReq := &monitoring.CancelArchiveRequest{
			TypeMeta:   api.TypeMeta{Kind: string(monitoring.KindArchiveRequest)},
			ObjectMeta: req.ObjectMeta,
		}
		_, err := ti.Restcl.MonitoringV1().ArchiveRequest().Cancel(superAdminCtx, cancelReq)
		if err != nil {
			return false, err
		}
		if !CheckEventually(func() (bool, interface{}) {
			fetchedReq, err = ti.Restcl.MonitoringV1().ArchiveRequest().Get(superAdminCtx, &req.ObjectMeta)
			if err != nil {
				return false, err
			}
			if fetchedReq.Status.Status == monitoring.ArchiveRequestStatus_Canceled.String() && strings.Contains(fetchedReq.Status.Reason, "context canceled") {
				return true, fetchedReq
			}
			return false, fetchedReq
		}, "10ms", "1s") {
			return false, fetchedReq
		}
		return true, fetchedReq

	}, fmt.Sprintf("unexpected archive request status for %v", fetchedReq), "1s", "30s")

}

func TestAuthzInArchiveRequests(t *testing.T) {
	ti := TestInfo{Name: t.Name()}
	err := ti.SetupElastic()
	AssertOk(t, err, "setupElastic failed")
	defer ti.TeardownElastic()
	err = ti.StartAPIServer()
	AssertOk(t, err, "failed to start API server")
	defer ti.APIServer.Stop()
	err = ti.StartSpyglass()
	AssertOk(t, err, "failed to start spyglass")
	defer ti.Fdr.Stop()
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
	MustCreateTestUser(ti.Apicl, "tuser1", testPassword, globals.DefaultTenant)
	defer MustDeleteUser(ti.Apicl, "tuser1", globals.DefaultTenant)
	archUserCtx, err := NewLoggedInContext(context.TODO(), ti.APIGwAddr, &auth.PasswordCredential{Username: "tuser1", Password: testPassword, Tenant: globals.DefaultTenant})
	AssertOk(t, err, "error creating logged in context for archive user")
	req := &monitoring.ArchiveRequest{
		ObjectMeta: api.ObjectMeta{
			Name:      "arch-req1",
			Tenant:    globals.DefaultTenant,
			Namespace: globals.DefaultNamespace,
		},
		Spec: monitoring.ArchiveRequestSpec{
			Type: monitoring.ArchiveRequestSpec_AuditEvent.String(),
			Query: &monitoring.ArchiveQuery{
				Tenants: []string{globals.DefaultTenant},
			},
		},
	}
	_, err = ti.Restcl.MonitoringV1().ArchiveRequest().Create(archUserCtx, req)
	Assert(t, err != nil, "expected authorization error in creating archive request")
	t.Logf("error in creating archive request by tuser1: %v", err)
	MustCreateRole(ti.Apicl, "ArchivingPerms", globals.DefaultTenant,
		login.NewPermission(globals.DefaultTenant, string(apiclient.GroupMonitoring), "", authz.ResourceNamespaceAll, "", auth.Permission_AllActions.String()),
		login.NewPermission(globals.DefaultTenant, "", auth.Permission_AuditEvent.String(), authz.ResourceNamespaceAll, "", auth.Permission_Read.String()),
		login.NewPermission(globals.DefaultTenant, string(apiclient.GroupObjstore), string(objstore.KindObject), objstore.Buckets_auditevents.String(), "", auth.Permission_Create.String()),
	)
	defer MustDeleteRole(ti.Apicl, "ArchivingPerms", globals.DefaultTenant)
	MustCreateRoleBinding(ti.Apicl, "ArchivingPermsRB", globals.DefaultTenant, "ArchivingPerms", []string{"tuser1"}, nil)
	defer MustDeleteRoleBinding(ti.Apicl, "ArchivingPermsRB", globals.DefaultTenant)
	_, err = CreateArchiveRequest(archUserCtx, ti.Restcl, req)
	AssertOk(t, err, "unexpected error in creating archive request")
	MustDeleteArchiveRequest(context.TODO(), ti.Apicl, &req.ObjectMeta)
}

func TestTenantScopedQueries(t *testing.T) {
	ti := TestInfo{Name: t.Name()}
	err := ti.SetupElastic()
	AssertOk(t, err, "setupElastic failed")
	defer ti.TeardownElastic()
	err = ti.StartAPIServer()
	AssertOk(t, err, "failed to start API server")
	defer ti.APIServer.Stop()
	err = ti.StartSpyglass()
	AssertOk(t, err, "failed to start spyglass")
	defer ti.Fdr.Stop()
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

	// create testtenant and admin user
	MustCreateTenant(ti.Apicl, testTenant)
	defer MustDeleteTenant(ti.Apicl, testTenant)
	MustCreateTestUser(ti.Apicl, testUser, testPassword, testTenant)
	defer MustDeleteUser(ti.Apicl, testUser, testTenant)
	MustUpdateRoleBinding(ti.Apicl, globals.AdminRoleBinding, testTenant, globals.AdminRole, []string{testUser}, nil)
	defer MustUpdateRoleBinding(ti.Apicl, globals.AdminRoleBinding, testTenant, globals.AdminRole, nil, nil)
	ctx, err := NewLoggedInContext(context.Background(), ti.APIGwAddr, &auth.PasswordCredential{Username: testUser, Password: testPassword, Tenant: testTenant})
	AssertOk(t, err, "error creating logged in context for testtenant admin user")
	var evts string
	AssertEventually(t, func() (bool, interface{}) {
		req := &monitoring.ArchiveRequest{
			TypeMeta: api.TypeMeta{Kind: string(monitoring.KindArchiveRequest)},
			ObjectMeta: api.ObjectMeta{
				Name:   fmt.Sprintf("test-arch1"),
				Tenant: testTenant,
			},
			Spec: monitoring.ArchiveRequestSpec{
				Type:  monitoring.ArchiveRequestSpec_AuditEvent.String(),
				Query: &monitoring.ArchiveQuery{},
			},
		}
		MustCreateArchiveRequest(ctx, ti.Restcl, req)
		defer MustDeleteArchiveRequest(context.TODO(), ti.Apicl, &req.ObjectMeta)
		if !CheckEventually(func() (bool, interface{}) {
			var fetchedReq *monitoring.ArchiveRequest
			fetchedReq, err = ti.Restcl.MonitoringV1().ArchiveRequest().Get(ctx, &req.ObjectMeta)
			if err != nil {
				return false, err
			}
			req = fetchedReq
			if fetchedReq.Status.Status != monitoring.ArchiveRequestStatus_Completed.String() {
				return false, fmt.Errorf("archive request not completed: %#v", *req)
			}

			return true, nil
		}, "1s", "30s") {
			return false, fmt.Errorf("archive request not completed: %v", err)
		}

		objname, err := ExtractObjectNameFromURI(req.Status.URI)
		if err != nil {
			return false, err
		}
		stats, err := ti.Objstorecl.StatObject(objname)
		if err != nil {
			return false, err
		}
		if stats.Size == 0 {
			return false, fmt.Errorf("object size is 0")
		}
		r, err := ti.Objstorecl.GetObject(ctx, objname)
		if err != nil {
			return false, err
		}
		evts, err = ExtractArchive(r)
		if err != nil {
			return false, err
		}
		if !strings.Contains(evts, fmt.Sprintf("tenant\":\"%v", testTenant)) || !strings.Contains(evts, "Login") {
			return false, fmt.Errorf("expected audit events to belong to testtenant: %v", evts)
		}
		if strings.Contains(evts, fmt.Sprintf("tenant\":\"%v", globals.DefaultTenant)) {
			return false, fmt.Errorf("contains audit events in default tenant: %v", evts)
		}
		t.Logf("testtenant admin audit archive:\n%v", evts)
		return true, nil
	}, fmt.Sprintf("testtenant admin archive doesn't contain expected audit events: %v", evts), "2s", "90s")
	// search across tenants
	superAdminCtx, err := NewLoggedInContext(context.TODO(), ti.APIGwAddr, adminCred)
	AssertOk(t, err, "error creating logged in context")
	AssertEventually(t, func() (bool, interface{}) {
		req := &monitoring.ArchiveRequest{
			TypeMeta: api.TypeMeta{Kind: string(monitoring.KindArchiveRequest)},
			ObjectMeta: api.ObjectMeta{
				Name:   fmt.Sprintf("test-arch2"),
				Tenant: globals.DefaultTenant,
			},
			Spec: monitoring.ArchiveRequestSpec{
				Type: monitoring.ArchiveRequestSpec_AuditEvent.String(),
				Query: &monitoring.ArchiveQuery{
					Tenants: []string{globals.DefaultTenant, testTenant},
				},
			},
		}
		MustCreateArchiveRequest(superAdminCtx, ti.Restcl, req)
		defer MustDeleteArchiveRequest(context.TODO(), ti.Apicl, &req.ObjectMeta)
		if !CheckEventually(func() (bool, interface{}) {
			var fetchedReq *monitoring.ArchiveRequest
			fetchedReq, err = ti.Restcl.MonitoringV1().ArchiveRequest().Get(superAdminCtx, &req.ObjectMeta)
			if err != nil {
				return false, err
			}
			req = fetchedReq
			if fetchedReq.Status.Status != monitoring.ArchiveRequestStatus_Completed.String() {
				return false, fmt.Errorf("archive request not completed: %#v", *req)
			}
			return true, nil
		}, "1s", "30s") {
			return false, fmt.Errorf("archive request not completed: %v", err)
		}

		objname, err := ExtractObjectNameFromURI(req.Status.URI)
		if err != nil {
			return false, err
		}
		stats, err := ti.Objstorecl.StatObject(objname)
		if err != nil {
			return false, err
		}
		if stats.Size == 0 {
			return false, fmt.Errorf("object size is 0")
		}
		r, err := ti.Objstorecl.GetObject(superAdminCtx, objname)
		if err != nil {
			return false, err
		}
		evts, err = ExtractArchive(r)
		if err != nil {
			return false, err
		}
		if !strings.Contains(evts, fmt.Sprintf("tenant\":\"%v", testTenant)) || !strings.Contains(evts, "Login") {
			return false, fmt.Errorf("expected audit events to belong to testtenant: %v", evts)
		}
		if !strings.Contains(evts, fmt.Sprintf("tenant\":\"%v", globals.DefaultTenant)) || !strings.Contains(evts, "Login") {
			return false, fmt.Errorf("expected audit events to belong to default tenant: %v", evts)
		}
		t.Logf("default tenant admin audit archive:\n%v", evts)
		return true, nil
	}, fmt.Sprintf("default tenant admin archive doesn't contain expected audit events: %v", evts), "2s", "90s")
}

func TestArchivalDuringUpgrade(t *testing.T) {
	ti := TestInfo{Name: t.Name()}
	err := ti.SetupElastic()
	AssertOk(t, err, "setupElastic failed")
	defer ti.TeardownElastic()
	err = ti.StartAPIServer()
	AssertOk(t, err, "failed to start API server")
	defer ti.APIServer.Stop()
	err = ti.StartSpyglass()
	AssertOk(t, err, "failed to start spyglass")
	defer ti.Fdr.Stop()
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

	verObj := &cluster.Version{}
	verObj.Defaults("all")
	verObj.Name = "VersionSingleton"
	verObj.Status.RolloutBuildVersion = "1.5"
	AssertEventually(t, func() (bool, interface{}) {
		fVerObj, err := ti.Apicl.ClusterV1().Version().Create(context.TODO(), verObj)
		if err != nil {
			return false, err
		}
		verObj = fVerObj
		return true, verObj
	}, "failed to create version object")
	defer ti.Apicl.ClusterV1().Version().Delete(context.TODO(), &verObj.ObjectMeta)
	superAdminCtx, err := NewLoggedInContext(context.TODO(), ti.APIGwAddr, adminCred)
	AssertOk(t, err, "error creating logged in context")
	req := &monitoring.ArchiveRequest{
		TypeMeta: api.TypeMeta{Kind: string(monitoring.KindArchiveRequest)},
		ObjectMeta: api.ObjectMeta{
			Name:   fmt.Sprintf("test-arch1"),
			Tenant: testTenant,
		},
		Spec: monitoring.ArchiveRequestSpec{
			Type:  monitoring.ArchiveRequestSpec_AuditEvent.String(),
			Query: &monitoring.ArchiveQuery{},
		},
	}
	_, err = ti.Restcl.MonitoringV1().ArchiveRequest().Create(superAdminCtx, req)
	Assert(t, err != nil, "expected error in creating archive request")
	Assert(t, strings.Contains(err.Error(), "rollout in progress"), fmt.Sprintf("unexpected error: %v", err))
}

func createArchiveRequests(ctx context.Context, t *testing.T, ti TestInfo, tData []testData) {
	for _, test := range tData {
		_, err := CreateArchiveRequest(ctx, ti.Restcl, test.req)
		AssertOk(t, err, "test [%s] failed, error creating archive request", test.name)
	}
}

func verifyArchiveRequests(ctx context.Context, t *testing.T, ti TestInfo, tData []testData) error {
	for _, test := range tData {
		req := test.req
		var err error
		if !CheckEventually(func() (bool, interface{}) {
			req, err = ti.Restcl.MonitoringV1().ArchiveRequest().Get(ctx, &req.ObjectMeta)
			if err != nil {
				return false, err
			}
			if req.Status.Status != test.status {
				err = fmt.Errorf("test [%s] failed, expected status [%s], got [%s]", test.name, test.status, req.Status.Status)
				return false, err
			}
			t.Logf("archive request: %v", req)
			var objname string
			objname, err = ExtractObjectNameFromURI(req.Status.URI)
			if err != nil {
				return false, err
			}
			var stats *objstore2.ObjectStats
			stats, err = ti.Objstorecl.StatObject(objname)
			if err != nil {
				return false, err
			}
			if !(stats.Size > 0) {
				err = fmt.Errorf("test [%s] failed, unexpected size [%d] for object [%s]", test.name, stats.Size, objname)
				return false, err
			}
			var r io.ReadCloser
			r, err = ti.Objstorecl.GetObject(ctx, objname)
			if err != nil {
				return false, err
			}
			var evts string
			evts, err = ExtractArchive(r)
			if err != nil {
				return false, err
			}
			if test.evtStr == "" && evts != test.evtStr {
				err = fmt.Errorf("test [%s] failed, expected archived events to be empty", test.name)
				return false, err
			}
			if !strings.Contains(evts, test.evtStr) {
				err = fmt.Errorf("test [%s] failed, expected archived events [%s] to contain [%s]", test.name, evts, test.evtStr)
				return false, err
			}
			t.Logf("test [%s], events archive [%s]", test.name, evts)
			return true, nil
		}, "100ms", "3s") {
			return fmt.Errorf("test [%s] failed, error : %v", test.name, err)
		}
	}
	return nil
}

func deleteArchiveRequests(ctx context.Context, t *testing.T, ti TestInfo, tData []testData) {
	for _, test := range tData {
		_, err := DeleteArchiveRequest(ctx, ti.Restcl, &test.req.ObjectMeta)
		AssertOk(t, err, "test [%s] failed, error deleting archive request", test.name)
	}
}

func createAuditLogs(t *testing.T, ti TestInfo, count int) int {
	auditor := auditmgr.WithAuditors(elasticauditor.NewSynchAuditor(ti.ElasticSearchAddr, ti.Rslvr, ti.Logger, elasticauditor.WithElasticClient(ti.ESClient)))
	err := auditor.Run()
	AssertOk(t, err, "error starting elastic auditor")
	var success int
	for i := 0; i < count; i++ {
		ts, _ := types.TimestampProto(time.Now())
		ts1 := api.Timestamp{Timestamp: *ts}
		evt := &auditapi.AuditEvent{
			TypeMeta:   api.TypeMeta{Kind: "AuditEvent"},
			ObjectMeta: api.ObjectMeta{Name: fmt.Sprintf("auditevent-%d", i), UUID: uuid.NewV4().String(), Tenant: "default", CreationTime: ts1, ModTime: ts1},
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
		err := auditor.ProcessEvents(evt)
		if err == nil {
			success++
		}
	}
	return success
}
