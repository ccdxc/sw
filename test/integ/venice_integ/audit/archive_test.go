package audit

import (
	"context"
	"fmt"
	"strings"
	"sync"
	"testing"
	"time"

	"github.com/gogo/protobuf/types"
	uuid "github.com/satori/go.uuid"

	"github.com/pensando/sw/api"
	auditapi "github.com/pensando/sw/api/generated/audit"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/monitoring"
	. "github.com/pensando/sw/test/utils"
	"github.com/pensando/sw/venice/globals"
	. "github.com/pensando/sw/venice/utils/archive/testutils"
	elasticauditor "github.com/pensando/sw/venice/utils/audit/elastic"
	auditmgr "github.com/pensando/sw/venice/utils/audit/manager"
	. "github.com/pensando/sw/venice/utils/authn/testutils"
	. "github.com/pensando/sw/venice/utils/testutils"
)

type testData struct {
	name   string
	req    *monitoring.ArchiveRequest
	status string
}

func TestAuditLogArchive(t *testing.T) {
	ti := tInfo{}
	err := ti.setupElastic()
	AssertOk(t, err, "setupElastic failed")
	defer ti.teardownElastic()
	err = ti.startAPIServer()
	AssertOk(t, err, "failed to start API server")
	defer ti.apiServer.Stop()
	err = ti.startSpyglass()
	AssertOk(t, err, "failed to start spyglass")
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
		},
	}
	createArchiveRequests(superAdminCtx, t, ti, tests)
	verifyArchiveRequests(superAdminCtx, t, ti, tests)
	deleteArchiveRequests(superAdminCtx, t, ti, tests)
	// stop spyglass
	ti.fdr.Stop()
	// create archive requests
	createArchiveRequests(superAdminCtx, t, ti, tests)
	// start spyglass
	ti.startSpyglass()
	defer ti.fdr.Stop()
	verifyArchiveRequests(superAdminCtx, t, ti, tests)
	deleteArchiveRequests(superAdminCtx, t, ti, tests)
}

func TestDuplicateArchiveRequests(t *testing.T) {
	ti := tInfo{}
	err := ti.setupElastic()
	AssertOk(t, err, "setupElastic failed")
	defer ti.teardownElastic()
	err = ti.startAPIServer()
	AssertOk(t, err, "failed to start API server")
	defer ti.apiServer.Stop()
	err = ti.startSpyglass()
	AssertOk(t, err, "failed to start spyglass")
	defer ti.fdr.Stop()
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
	var waitgrp sync.WaitGroup
	numReqs := 50
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
			MustCreateArchiveRequest(superAdminCtx, ti.restcl, req)
		}(i)
	}
	waitgrp.Wait()
	var mutex sync.RWMutex
	var failedCount, successCount int
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
				fetchedReq, err = ti.restcl.MonitoringV1().ArchiveRequest().Get(superAdminCtx, &req.ObjectMeta)
				if err != nil {
					return false, err
				}
				if fetchedReq.Status.Status == monitoring.ArchiveRequestStatus_Failed.String() && strings.Contains(fetchedReq.Status.Reason, "already running for log type") {
					mutex.Lock()
					failedCount++
					mutex.Unlock()
					return true, fetchedReq
				}
				if fetchedReq.Status.Status == monitoring.ArchiveRequestStatus_Completed.String() {
					mutex.Lock()
					successCount++
					mutex.Unlock()
					return true, fetchedReq
				}
				return false, fetchedReq
			}, "1s", "30s") {
				t.Logf("unexpected archive request status for %v", fetchedReq)
			}
		}(i)
	}
	waitgrp.Wait()
	Assert(t, failedCount > 0, fmt.Sprintf("unexpected failed count %d", failedCount))
	Assert(t, successCount > 0, fmt.Sprintf("unexpected success count %d", successCount))
	Assert(t, failedCount+successCount == numReqs, fmt.Sprintf("unexpected total of failed [%d] and success [%d] count", failedCount, successCount))
	t.Logf("failed count %d, success count %d", failedCount, successCount)
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
		MustDeleteArchiveRequest(superAdminCtx, ti.restcl, &req.ObjectMeta)
	}
}

func TestCancelArchiveRequests(t *testing.T) {
	ti := tInfo{}
	err := ti.setupElastic()
	AssertOk(t, err, "setupElastic failed")
	defer ti.teardownElastic()
	err = ti.startAPIServer()
	AssertOk(t, err, "failed to start API server")
	defer ti.apiServer.Stop()
	err = ti.startSpyglass()
	AssertOk(t, err, "failed to start spyglass")
	defer ti.fdr.Stop()
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
		MustCreateArchiveRequest(superAdminCtx, ti.restcl, req)
		defer MustDeleteArchiveRequest(superAdminCtx, ti.restcl, &req.ObjectMeta)
		cancelReq := &monitoring.CancelArchiveRequest{
			TypeMeta:   api.TypeMeta{Kind: string(monitoring.KindArchiveRequest)},
			ObjectMeta: req.ObjectMeta,
		}
		_, err := ti.restcl.MonitoringV1().ArchiveRequest().Cancel(superAdminCtx, cancelReq)
		if err != nil {
			return false, err
		}
		if !CheckEventually(func() (bool, interface{}) {
			fetchedReq, err = ti.restcl.MonitoringV1().ArchiveRequest().Get(superAdminCtx, &req.ObjectMeta)
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

func createArchiveRequests(ctx context.Context, t *testing.T, ti tInfo, tData []testData) {
	for _, test := range tData {
		_, err := CreateArchiveRequest(ctx, ti.restcl, test.req)
		AssertOk(t, err, "test [%s] failed, error creating archive request", test.name)
	}
}

func verifyArchiveRequests(ctx context.Context, t *testing.T, ti tInfo, tData []testData) {
	for _, test := range tData {
		req := test.req
		var err error
		AssertEventually(t, func() (bool, interface{}) {
			req, err = ti.restcl.MonitoringV1().ArchiveRequest().Get(ctx, &req.ObjectMeta)
			if err != nil {
				return false, err
			}
			if req.Status.Status != test.status {
				return false, req
			}
			t.Logf("archive request: %v", req)
			return true, req
		}, fmt.Sprintf("test [%s] failed, expected status [%s], got [%s]", test.name, test.status, req.Status.Status))
		objname, err := ExtractObjectNameFromURI(req.Status.URI)
		AssertOk(t, err, "extracting object name from URI %s failed", req.Status.URI)
		stats, err := ti.objstorecl.StatObject(objname)
		AssertOk(t, err, fmt.Sprintf("test [%s] failed, error stating object [%s]", test.name, objname))
		Assert(t, stats.Size > 0, fmt.Sprintf("test [%s] failed, unexpected size [%d] for object [%s]", test.name, stats.Size, objname))
		r, err := ti.objstorecl.GetObject(ctx, objname)
		AssertOk(t, err, fmt.Sprintf("test [%s] failed, error reading object [%s]", test.name, objname))
		_, err = ExtractArchive(r)
		AssertOk(t, err, fmt.Sprintf("test [%s] failed, error extracting archive for object [%s]", test.name, objname))
	}
}

func deleteArchiveRequests(ctx context.Context, t *testing.T, ti tInfo, tData []testData) {
	for _, test := range tData {
		_, err := DeleteArchiveRequest(ctx, ti.restcl, &test.req.ObjectMeta)
		AssertOk(t, err, "test [%s] failed, error deleting archive request", test.name)
	}
}

func createAuditLogs(t *testing.T, ti tInfo, count int) int {
	auditor := auditmgr.WithAuditors(elasticauditor.NewSynchAuditor(ti.elasticSearchAddr, ti.rslvr, ti.logger, elasticauditor.WithElasticClient(ti.esClient)))
	err := auditor.Run(make(<-chan struct{}))
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
