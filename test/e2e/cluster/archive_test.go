package cluster

import (
	"fmt"
	"strings"
	"sync"
	"time"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/api/generated/objstore"
	"github.com/pensando/sw/api/login"
	"github.com/pensando/sw/venice/globals"

	. "github.com/pensando/sw/venice/utils/archive/testutils"
)

var _ = Describe("archive tests", func() {
	Context("archive audit logs", func() {
		var rb *auth.RoleBinding
		var err error
		BeforeEach(func() {
			Eventually(func() error {
				rb, err = ts.restSvc.AuthV1().RoleBinding().Create(ts.loggedInCtx,
					login.NewRoleBinding("ArchiveAdminRoleBinding", globals.DefaultTenant, globals.AdminRole, "", "archiveUserGroup"))
				return err
			}, 10, 1).Should(BeNil())
		})
		It("archive 20k audit logs", func() {
			var waitgrp sync.WaitGroup
			for i := 0; i < 100; i++ {
				waitgrp.Add(1)
				go func(j int) {
					defer waitgrp.Done()
					for k := 0; k < 100; k++ {
						ts.restSvc.AuthV1().RoleBinding().Update(ts.loggedInCtx, rb)
					}
				}(i)
			}
			waitgrp.Wait()
			time.Sleep(6 * time.Second)
			var req *monitoring.ArchiveRequest
			var err error
			Eventually(func() error {
				req, err = ts.restSvc.MonitoringV1().ArchiveRequest().Create(ts.loggedInCtx, &monitoring.ArchiveRequest{
					TypeMeta: api.TypeMeta{Kind: string(monitoring.KindArchiveRequest)},
					ObjectMeta: api.ObjectMeta{
						Name:   "test20kAuditArchive",
						Tenant: globals.DefaultTenant,
					},
					Spec: monitoring.ArchiveRequestSpec{
						Type:  monitoring.ArchiveRequestSpec_AuditEvent.String(),
						Query: &monitoring.ArchiveQuery{},
					},
				})
				return err
			}, 10, 1).Should(BeNil())
			var createdReq *monitoring.ArchiveRequest
			Eventually(func() error {
				createdReq, err = ts.restSvc.MonitoringV1().ArchiveRequest().Get(ts.loggedInCtx, &req.ObjectMeta)
				if err != nil {
					return err
				}
				if createdReq.Status.Status != monitoring.ArchiveRequestStatus_Completed.String() {
					return fmt.Errorf("archive request not completed, status: %s, reason: %s", createdReq.Status.Status, createdReq.Status.Reason)
				}
				return err
			}, 180, 1).Should(BeNil())
			Eventually(func() error {
				_, err = ts.restSvc.MonitoringV1().ArchiveRequest().Delete(ts.loggedInCtx, &req.ObjectMeta)
				return err
			}, 10, 1).Should(BeNil())
			objname, err := ExtractObjectNameFromURI(createdReq.Status.URI)
			Expect(err).Should(BeNil())
			r, err := ts.tu.AuditVOSClient.GetObject(ts.loggedInCtx, objname)
			Expect(err).Should(BeNil())
			logstr, err := ExtractArchive(r)
			r.Close()
			Expect(err).Should(BeNil())
			auditevents := strings.Split(logstr, "\n")
			Expect(len(auditevents)).Should(BeNumerically(">", 20000))
			objs, err := ts.restSvc.ObjstoreV1().Object().List(ts.loggedInCtx, &api.ListWatchOptions{
				ObjectMeta: api.ObjectMeta{
					Tenant:    globals.DefaultTenant,
					Namespace: objstore.Buckets_auditevents.String(),
				},
			})
			Expect(err).Should(BeNil())
			Expect(len(objs)).Should(BeNumerically(">", 0))
			for _, obj := range objs {
				// tenant and namespace in object meta are  not set in results returned from List
				obj.Namespace = objstore.Buckets_auditevents.String()
				obj.Tenant = globals.DefaultTenant
				Eventually(func() error {
					_, err = ts.restSvc.ObjstoreV1().Object().Delete(ts.loggedInCtx, &obj.ObjectMeta)
					return err
				}, 10, 1).Should(BeNil())
			}
		})
		AfterEach(func() {
			Eventually(func() error {
				_, err = ts.restSvc.AuthV1().RoleBinding().Delete(ts.loggedInCtx, &rb.ObjectMeta)
				return err
			}, 10, 1).Should(BeNil())
		})
	})
	Context("archive events", func() {
		It("archive events", func() {
			var req *monitoring.ArchiveRequest
			var err error
			Eventually(func() error {
				req, err = ts.restSvc.MonitoringV1().ArchiveRequest().Create(ts.loggedInCtx, &monitoring.ArchiveRequest{
					TypeMeta: api.TypeMeta{Kind: string(monitoring.KindArchiveRequest)},
					ObjectMeta: api.ObjectMeta{
						Name:   "testEventArchive",
						Tenant: globals.DefaultTenant,
					},
					Spec: monitoring.ArchiveRequestSpec{
						Type:  monitoring.ArchiveRequestSpec_Event.String(),
						Query: &monitoring.ArchiveQuery{},
					},
				})
				return err
			}, 10, 1).Should(BeNil())
			var createdReq *monitoring.ArchiveRequest
			Eventually(func() error {
				createdReq, err = ts.restSvc.MonitoringV1().ArchiveRequest().Get(ts.loggedInCtx, &req.ObjectMeta)
				if err != nil {
					return err
				}
				if createdReq.Status.Status != monitoring.ArchiveRequestStatus_Completed.String() {
					return fmt.Errorf("archive request not completed, status: %s, reason: %s", createdReq.Status.Status, createdReq.Status.Reason)
				}
				return err
			}, 180, 1).Should(BeNil())
			Eventually(func() error {
				_, err = ts.restSvc.MonitoringV1().ArchiveRequest().Delete(ts.loggedInCtx, &req.ObjectMeta)
				return err
			}, 10, 1).Should(BeNil())
			objname, err := ExtractObjectNameFromURI(createdReq.Status.URI)
			Expect(err).Should(BeNil())
			r, err := ts.tu.EventVOSClient.GetObject(ts.loggedInCtx, objname)
			Expect(err).Should(BeNil())
			logstr, err := ExtractArchive(r)
			r.Close()
			Expect(err).Should(BeNil())
			events := strings.Split(logstr, "\n")
			Expect(len(events)).Should(BeNumerically(">", 0))
		})
	})
})
