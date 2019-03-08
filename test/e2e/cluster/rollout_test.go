// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package cluster

import (
	"context"
	"fmt"
	"io/ioutil"
	"time"

	"github.com/gogo/protobuf/types"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	api "github.com/pensando/sw/api"

	rollout "github.com/pensando/sw/api/generated/rollout"
)

const (
	rolloutName        = "e2e_rollout"
	rolloutSuspendName = "e2e_rollout_suspend"
)

// run the tests
var _ = Describe("Rollout object tests", func() {

	Context("Rollout tests", func() {

		// setup
		It("Rollout setup: upload image should succeed", func() {
			Skip(fmt.Sprintf("Skipping upload venice image test"))
			//TODO integrate image pickup with the hourly build
			upgImageName := "/import/src/github.com/pensando/sw/bin/venice.tgz"

			// location of the objstore.
			err := ts.tu.SetupObjstoreClient()
			filename := "venice.tgz"
			metadata := map[string]string{
				"Version":     "v1.3.2",
				"Environment": "production",
				"Description": "E2E test Image upload",
				"Releasedate": "May2018",
			}
			buf, _ := ioutil.ReadFile(upgImageName)
			ctx := ts.tu.NewLoggedInContext(context.Background())
			_, err = uploadFile(ctx, filename, metadata, buf)
			Expect(err).Should(BeNil(), "Failed to upload file")
		})

		// run tests
		It("Rollout operations should succeed", func() {
			Skip(fmt.Sprintf("Skipping rollout tests"))
			seconds := time.Now().Unix()
			scheduledStartTime := &api.Timestamp{
				Timestamp: types.Timestamp{
					Seconds: seconds + 30, //Add a scheduled rollout with 30 second delay
				},
			}

			rollout := rollout.Rollout{
				TypeMeta: api.TypeMeta{
					Kind: "Rollout",
				},
				ObjectMeta: api.ObjectMeta{
					Name:      rolloutName,
					Namespace: "default",
				},
				Spec: rollout.RolloutSpec{
					Version:                     "2.8",
					ScheduledStartTime:          scheduledStartTime,
					Duration:                    "",
					Strategy:                    "LINEAR",
					MaxParallel:                 0,
					MaxNICFailuresBeforeAbort:   0,
					OrderConstraints:            nil,
					Suspend:                     false,
					SmartNICsOnly:               false,
					SmartNICMustMatchConstraint: true, // hence venice upgrade only
					UpgradeType:                 "Disruptive",
				},
			}

			// Verify creation for rollout object
			Eventually(func() bool {

				r1, err := ts.restSvc.RolloutV1().Rollout().Create(ts.loggedInCtx, &rollout)
				if err != nil || r1.Name != rolloutName {
					By(fmt.Sprintf("ts:%s Rollout CREATE failed for [%s] err: %+v r1: %+v", time.Now().String(), rolloutName, err, r1))
					return false
				}
				By(fmt.Sprintf("ts:%s Rollout created at [%s] and scheduled at [%+v] for [%s]", time.Now().String(), time.Unix(r1.CreationTime.Seconds, 0), time.Unix(r1.Spec.ScheduledStartTime.Seconds, 0), rolloutName))
				return true
			}).Should(BeTrue(), fmt.Sprintf("Failed to create %s object", rolloutName))

			// Verify GET for rollout object
			Eventually(func() bool {
				obj := api.ObjectMeta{Name: rolloutName, Tenant: "default"}
				r1, err := ts.restSvc.RolloutV1().Rollout().Get(ts.loggedInCtx, &obj)
				if err != nil || r1.Name != rolloutName {
					By(fmt.Sprintf("ts:%s Rollout GET failed for [%s] err: %+v r1: %+v", time.Now().String(), rolloutName, err, r1))
					return false
				}
				By(fmt.Sprintf("ts:%s Rollout GET validated for [%s]", time.Now().String(), rolloutName))
				return true
			}, 30, 1).Should(BeTrue(), fmt.Sprintf("Failed to get %s object", rolloutName))

			// Verify Start Time of rollout object
			Eventually(func() bool {
				obj := api.ObjectMeta{Name: rolloutName, Tenant: "default"}
				r1, err := ts.restSvc.RolloutV1().Rollout().Get(ts.loggedInCtx, &obj)
				if err != nil || r1.Name != rolloutName {
					By(fmt.Sprintf("ts:%s Rollout GET failed for [%s] err: %+v r1: %+v", time.Now().String(), rolloutName, err, r1))
					return false
				}
				if r1.Status.StartTime == nil || r1.Spec.ScheduledStartTime.Seconds > r1.Status.StartTime.Seconds {
					By(fmt.Sprintf("ts:%s Waiting to schedule rollout : %s", time.Now().String(), rolloutName))
					return false
				}
				By(fmt.Sprintf("ts:%s Rollout successfully started at [%+v] for [%s]", time.Now().String(), time.Unix(r1.Status.StartTime.Seconds, 0), rolloutName))
				return true
			}, 300, 5).Should(BeTrue(), fmt.Sprintf("Failed to validate scheduled rollout for object %s", rolloutName))

			// Verify rollout Node status
			Eventually(func() bool {
				obj := api.ObjectMeta{Name: rolloutName, Tenant: "default"}
				rollout, err := ts.restSvc.RolloutV1().Rollout().Get(ts.loggedInCtx, &obj)
				if err != nil {
					By(fmt.Sprintf("ts:%s Rollout GET failed for status check, err: %+v rollouts: %+v", time.Now().String(), err, rollout))
					return false
				}

				status := rollout.Status.GetControllerNodesStatus()
				if len(status) == 0 {
					By(fmt.Sprintf("ts:%s Rollout controller node status: not found", time.Now().String()))
					return false
				}
				if status[0].Phase != "COMPLETE" {
					By(fmt.Sprintf("ts:%s Rollout controller node status: %+v", time.Now().String(), status[0].Phase))
					return false
				}

				By(fmt.Sprintf("ts:%s Rollout Node Status: Complete", time.Now().String()))

				return true
			}, 300, 5).Should(BeTrue(), "Failed to finish rollout controller node")

			// Verify rollout service status
			Eventually(func() bool {
				obj := api.ObjectMeta{Name: rolloutName, Tenant: "default"}
				rollout, err := ts.restSvc.RolloutV1().Rollout().Get(ts.loggedInCtx, &obj)
				if err != nil {
					By(fmt.Sprintf("ts:%s Rollout LIST failed for status check, err: %+v rollouts: %+v", time.Now().String(), err, rollout))
					return false
				}
				status := rollout.Status.GetControllerServicesStatus()
				if len(status) == 0 {
					By(fmt.Sprintf("ts:%s Rollout controller services status: Not Found", time.Now().String()))
					return false
				}
				if status[0].Phase != "COMPLETE" {
					By(fmt.Sprintf("ts:%s Rollout controller services status: : %+v", time.Now().String(), status[0].Phase))
					return false
				}

				By(fmt.Sprintf("ts:%s Rollout Controller Services Status: Complete", time.Now().String()))

				return true
			}, 300, 5).Should(BeTrue(), "Failed to finish rollout for controller services")

			// Verify delete on rollout object
			Eventually(func() bool {
				obj := api.ObjectMeta{Name: rolloutName, Tenant: "default"}
				r1, err := ts.restSvc.RolloutV1().Rollout().Delete(ts.loggedInCtx, &obj)
				if err != nil || r1.Name != rolloutName {
					return false
				}
				By(fmt.Sprintf("ts:%s Rollout DELETE validated for [%s]", time.Now().String(), rolloutName))
				return true
			}, 30, 1).Should(BeTrue(), fmt.Sprintf("Failed to delete %s object", rolloutName))

			// Verify GET for all rollout objects (LIST) returns empty
			Eventually(func() bool {

				ometa := api.ObjectMeta{Tenant: "default"}
				rollouts, err := ts.restSvc.RolloutV1().Rollout().List(ts.loggedInCtx, &api.ListWatchOptions{ObjectMeta: ometa})
				if err != nil || len(rollouts) != 0 {
					By(fmt.Sprintf("ts:%s Rollout LIST has unexpected objects, err: %+v rollouts: %+v", time.Now().String(), err, rollouts))
					return false
				}
				return true
			}, 30, 1).Should(BeTrue(), "Unexpected rollout objects found")

		})

		It("Rollout suspend operations should succeed", func() {
			Skip(fmt.Sprintf("Skipping suspend rollout tests"))
			if ts.tu.NumQuorumNodes < 2 {
				Skip(fmt.Sprintf("Skipping suspend rollout tests :%d quorum nodes found, need >= 2", ts.tu.NumQuorumNodes))
			}

			rollout := rollout.Rollout{
				TypeMeta: api.TypeMeta{
					Kind: "Rollout",
				},
				ObjectMeta: api.ObjectMeta{
					Name:      rolloutSuspendName,
					Namespace: "default",
				},
				Spec: rollout.RolloutSpec{
					Version:                     "2.8",
					ScheduledStartTime:          nil,
					Duration:                    "",
					Strategy:                    "LINEAR",
					MaxParallel:                 0,
					MaxNICFailuresBeforeAbort:   0,
					OrderConstraints:            nil,
					Suspend:                     false,
					SmartNICsOnly:               false,
					SmartNICMustMatchConstraint: true, // hence venice upgrade only
					UpgradeType:                 "Disruptive",
				},
			}

			// Verify creation for rollout object
			Eventually(func() bool {
				r1, err := ts.restSvc.RolloutV1().Rollout().Create(ts.loggedInCtx, &rollout)
				if err != nil || r1.Name != rolloutSuspendName {
					By(fmt.Sprintf("ts:%s Rollout CREATE failed for [%s] err: %+v r1: %+v", time.Now().String(), rolloutSuspendName, err, r1))
					return false
				}
				By(fmt.Sprintf("ts:%s Rollout created for [%s]", time.Now().String(), rolloutSuspendName))
				return true
			}).Should(BeTrue(), fmt.Sprintf("Failed to create %s object", rolloutSuspendName))

			// Verify GET for rollout object
			Eventually(func() bool {
				obj := api.ObjectMeta{Name: rolloutSuspendName, Tenant: "default"}
				r1, err := ts.restSvc.RolloutV1().Rollout().Get(ts.loggedInCtx, &obj)
				if err != nil || r1.Name != rolloutSuspendName {
					By(fmt.Sprintf("ts:%s Rollout GET failed for [%s] err: %+v r1: %+v", time.Now().String(), rolloutSuspendName, err, r1))
					return false
				}
				By(fmt.Sprintf("ts:%s Rollout GET validated for [%s]", time.Now().String(), rolloutSuspendName))
				return true
			}, 30, 1).Should(BeTrue(), fmt.Sprintf("Failed to get %s object", rolloutSuspendName))

			// Verify suspend for rollout object
			Eventually(func() bool {
				//Sleep 20 seconds
				time.Sleep(20 * time.Second)
				//Now Suspend rollout
				rollout.Spec.Suspend = true
				r1, err := ts.restSvc.RolloutV1().Rollout().Update(ts.loggedInCtx, &rollout)
				if err != nil || r1.Name != rolloutSuspendName {
					By(fmt.Sprintf("ts:%s Rollout Update failed for [%s] err: %+v r1: %+v", time.Now().String(), rolloutSuspendName, err, r1))
					return false
				}
				if r1.Spec.GetSuspend() {
					By(fmt.Sprintf("ts:%s Rollout suspended for [%s]", time.Now().String(), rolloutSuspendName))
					return true
				}
				return false
			}).Should(BeTrue(), fmt.Sprintf("Failed to suspend rollout %s object", rolloutSuspendName))

			// Verify Status of rollout
			Eventually(func() bool {
				obj := api.ObjectMeta{Name: rolloutSuspendName, Tenant: "default"}
				r1, err := ts.restSvc.RolloutV1().Rollout().Get(ts.loggedInCtx, &obj)
				if err != nil || r1.Name != rolloutSuspendName {
					By(fmt.Sprintf("ts:%s Rollout GET failed for [%s] err: %+v r1: %+v", time.Now().String(), rolloutSuspendName, err, r1))
					return false
				}

				time.Sleep(20 * time.Second)
				count := 0

				r2, err := ts.restSvc.RolloutV1().Rollout().Get(ts.loggedInCtx, &obj)
				if err != nil || r2.Name != rolloutSuspendName {
					By(fmt.Sprintf("ts:%s Rollout GET failed for [%s] err: %+v r2: %+v", time.Now().String(), rolloutSuspendName, err, r2))
					return false
				}

				for _, r1Status := range r1.Status.ControllerNodesStatus {
					for _, r2Status := range r2.Status.ControllerNodesStatus {
						if r1Status.Name == r2Status.Name && r1Status.Phase != r2Status.Phase {
							count++
						}
					}
				}

				//LINEAR Rollout Scheme

				if count > 1 {
					By(fmt.Sprintf("ts:%s More than one rollout object changed status : %v", time.Now().String(), count))
					return false
				}

				By(fmt.Sprintf("ts:%s Rollout successfully suspended for [%s]", time.Now().String(), rolloutSuspendName))
				return true
			}).Should(BeTrue(), fmt.Sprintf("Failed to suspend rollout for object %s", rolloutSuspendName))
		})
		// cleanup
		AfterEach(func() {
			// Cleanup rollout objects regardless of test outcome
			By(fmt.Sprintf("ts:%s Test completed cleaning up rollout objects if any", time.Now().String()))
			obj1 := api.ObjectMeta{Name: rolloutName, Tenant: "default"}
			ts.restSvc.RolloutV1().Rollout().Delete(ts.loggedInCtx, &obj1)
		})
	})
})
