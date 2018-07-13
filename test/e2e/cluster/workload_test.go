package cluster

import (
	"context"
	"fmt"
	"time"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	api "github.com/pensando/sw/api"

	apiclient "github.com/pensando/sw/api/generated/apiclient"
	workload "github.com/pensando/sw/api/generated/workload"
	"github.com/pensando/sw/venice/globals"
)

var _ = Describe("Workload CRUD tests", func() {

	Context("Workload CRUD tests", func() {
		var (
			wkldIf    workload.WorkloadV1WorkloadInterface
			wkld1Name = "coke-vm-1"
			wkld2Name = "pepsi-vm-1"
		)
		BeforeEach(func() {
			apiGwAddr := ts.tu.ClusterVIP + ":" + globals.APIGwRESTPort
			restSvc, err := apiclient.NewRestAPIClient(apiGwAddr)
			if err == nil {
				wkldIf = restSvc.WorkloadV1().Workload()
			}
			Expect(err).ShouldNot(HaveOccurred())
			Expect(wkldIf).ShouldNot(Equal(nil))
		})

		It("Workload CRUD operations should succeed", func() {

			// workload-1 object

			wkld1 := workload.Workload{
				ObjectMeta: api.ObjectMeta{
					Name:   wkld1Name,
					Tenant: "Coke",
				},
				Spec: workload.WorkloadSpec{
					HostName: "naples1-host",
					// Interfaces Spec keyed by MAC addr of Interface
					Interfaces: map[string]workload.WorkloadIntfSpec{
						"00:50:56:00:00:01": workload.WorkloadIntfSpec{
							MicroSegVlan: 101,
							ExternalVlan: 1001,
						},
						"00:50:56:00:00:02": workload.WorkloadIntfSpec{
							MicroSegVlan: 102,
							ExternalVlan: 1002,
						},
					},
				},
			}

			// workload-2 object
			wkld2 := workload.Workload{
				ObjectMeta: api.ObjectMeta{
					Name:   wkld2Name,
					Tenant: "Pepsi",
				},
				Spec: workload.WorkloadSpec{
					HostName: "naples2-host",
					// Interfaces Spec keyed by MAC addr of Interface
					Interfaces: map[string]workload.WorkloadIntfSpec{
						"00:50:56:00:00:03": workload.WorkloadIntfSpec{
							MicroSegVlan: 103,
							ExternalVlan: 1003,
						},
						"00:50:56:00:00:04": workload.WorkloadIntfSpec{
							MicroSegVlan: 104,
							ExternalVlan: 1004,
						},
					},
				},
			}

			// Verify creation for workload-1 object
			Eventually(func() bool {
				w1, err := wkldIf.Create(ts.tu.NewLoggedInContext(context.Background()), &wkld1)
				if err != nil || w1.Name != wkld1.Name {
					By(fmt.Sprintf("ts:%s Workload CREATE failed for [%s] err: %+v w1: %s", time.Now().String(), wkld1Name, err, w1))
					return false
				}
				By(fmt.Sprintf("ts:%s Workload CREATE validated for [%s]", time.Now().String(), wkld1Name))
				return true
			}, 30, 1).Should(BeTrue(), fmt.Sprintf("Failed to create %s object", wkld1Name))

			// Verify creation for workload-2 object
			Eventually(func() bool {
				w2, err := wkldIf.Create(ts.tu.NewLoggedInContext(context.Background()), &wkld2)
				if err != nil || w2.Name != wkld2.Name {
					By(fmt.Sprintf("ts:%s Workload CREATE failed for [%s] err: %+v w2: %s", time.Now().String(), wkld2Name, err, w2))
					return false
				}
				By(fmt.Sprintf("ts:%s Workload CREATE validated for [%s]", time.Now().String(), wkld2Name))
				return true
			}, 30, 1).Should(BeTrue(), fmt.Sprintf("Failed to create %s object", wkld2Name))

			// Verify GET for workload-1 object
			Eventually(func() bool {
				obj := api.ObjectMeta{Name: wkld1Name, Tenant: "Coke"}
				w1, err := wkldIf.Get(ts.tu.NewLoggedInContext(context.Background()), &obj)
				if err != nil || w1.Name != wkld1.Name {
					By(fmt.Sprintf("ts:%s Workload GET failed for [%s] err: %+v w1: %s", time.Now().String(), wkld1Name, err, w1))
					return false
				}
				By(fmt.Sprintf("ts:%s Workload GET validated for [%s]", time.Now().String(), wkld1Name))
				return true
			}, 30, 1).Should(BeTrue(), fmt.Sprintf("Failed to get %s object", wkld1Name))

			//  Sample workload object in JSON format
			//	{
			//		"kind" : "Workload",
			//		"api-version" : "v1",
			//		"meta" : {
			//			"name" : "coke-vm-1",
			//			"tenant" : "Coke",
			//			"resource-version" : "3123",
			//			"uuid" : "485eb70a-1567-4f47-bf53-30b3766e53c4",
			//		"labels" : {
			//			"_category" : "default"
			//		},
			//		"creation-time" : "2018-07-11T23:45:28.522868995Z",
			//		"mod-time" : "2018-07-11T23:45:28.522876286Z",
			//		"self-link" : "/venice/workload/workloads/Coke/coke-vm-1"
			//		},
			//		"spec" : {
			//			"host-name" : "naples1-host",
			//			"interfaces" : {
			//				"00:50:56:00:00:01" : {
			//					"micro-seg-vlan" : 101,
			//					"external-vlan" : 1001
			//				},
			//				"00:50:56:00:00:02" : {
			//					"micro-seg-vlan" : 102,
			//					"external-vlan" : 1002
			//				}
			//			}
			//		},
			//		"status" : { }
			//	}

			// Verify GET for workload-2 object
			Eventually(func() bool {
				obj := api.ObjectMeta{Name: wkld2Name, Tenant: "Pepsi"}
				w2, err := wkldIf.Get(ts.tu.NewLoggedInContext(context.Background()), &obj)
				if err != nil || w2.Name != wkld2.Name {
					By(fmt.Sprintf("ts:%s Workload GET failed for [%s] err: %+v w2: %s", time.Now().String(), wkld2Name, err, w2))
					return false
				}
				By(fmt.Sprintf("ts:%s Workload GET validated for [%s]", time.Now().String(), wkld2Name))
				return true
			}, 30, 1).Should(BeTrue(), fmt.Sprintf("Failed to get %s object", wkld2Name))

			// Verify LIST of all workload objects (LIST)
			Eventually(func() bool {
				ometa := api.ObjectMeta{Tenant: "Coke"}
				wklds, err := wkldIf.List(ts.tu.NewLoggedInContext(context.Background()), &api.ListWatchOptions{ObjectMeta: ometa})
				if err != nil || len(wklds) != 1 {
					By(fmt.Sprintf("ts:%s Workload LIST failed for Coke, err: %+v wklds: %+v", time.Now().String(), err, wklds))
					return false
				}
				if wklds[0].Name != wkld1Name {
					By(fmt.Sprintf("ts:%s Workload name mismatch for Coke, wklds: %+v", time.Now().String(), wklds))
					return false
				}
				By(fmt.Sprintf("ts:%s Workload LIST validated for Coke Tenant: %+v", time.Now().String(), wklds))

				ometa = api.ObjectMeta{Tenant: "Pepsi"}
				wklds, err = wkldIf.List(ts.tu.NewLoggedInContext(context.Background()), &api.ListWatchOptions{ObjectMeta: ometa})
				if err != nil || len(wklds) != 1 {
					By(fmt.Sprintf("ts:%s Workload LIST failed for Pepsi, err: %+v wklds: %+v", time.Now().String(), err, wklds))
					return false
				}
				if wklds[0].Name != wkld2Name {
					By(fmt.Sprintf("ts:%s Workload name mismatch for Pepsi, wklds: %+v", time.Now().String(), wklds))
					return false
				}
				By(fmt.Sprintf("ts:%s Workload LIST validated for Pepsi Tenant, wklds: %+v", time.Now().String(), wklds))
				return true
			}, 30, 1).Should(BeTrue(), "Failed to get all workload objects")

			// Verify update for workload-2 object
			wkld2.ObjectMeta.Labels = map[string]string{
				"Location": "us-west",
			}
			Eventually(func() bool {
				w2, err := wkldIf.Update(ts.tu.NewLoggedInContext(context.Background()), &wkld2)
				if err != nil || w2.Name != wkld2.Name {
					return false
				}
				By(fmt.Sprintf("ts:%s Workload UPDATE validated for [%s]", time.Now().String(), wkld2Name))
				return true
			}, 30, 1).Should(BeTrue(), "Failed to update workload-2 object")

			// Verify GET on updated workload-2 object
			Eventually(func() bool {
				obj := api.ObjectMeta{Name: wkld2Name, Tenant: "Pepsi"}
				w2, err := wkldIf.Get(ts.tu.NewLoggedInContext(context.Background()), &obj)
				if err != nil || w2.Name != wkld2.Name || w2.Labels["Location"] != "us-west" {
					return false
				}
				return true
			}, 30, 1).Should(BeTrue(), fmt.Sprintf("Failed to GET updated %s object", wkld2Name))

			// Verify delete on workload-1 object
			Eventually(func() bool {
				obj := api.ObjectMeta{Name: wkld1Name, Tenant: "Coke"}
				w1, err := wkldIf.Delete(ts.tu.NewLoggedInContext(context.Background()), &obj)
				if err != nil || w1.Name != wkld1.Name {
					return false
				}
				By(fmt.Sprintf("ts:%s Workload DELETE validated for [%s]", time.Now().String(), wkld1Name))
				return true
			}, 30, 1).Should(BeTrue(), fmt.Sprintf("Failed to delete %s object", wkld1Name))

			// Verify delete on workload-2 object
			Eventually(func() bool {
				obj := api.ObjectMeta{Name: wkld2Name, Tenant: "Pepsi"}
				w2, err := wkldIf.Delete(ts.tu.NewLoggedInContext(context.Background()), &obj)
				if err != nil || w2.Name != wkld2.Name {
					return false
				}
				By(fmt.Sprintf("ts:%s Workload DELETE validated for [%s]", time.Now().String(), wkld2Name))

				return true

			}, 30, 1).Should(BeTrue(), fmt.Sprintf("Failed to delete %s object", wkld2Name))

			// Verify GET for all workload objects (LIST) returns empty
			Eventually(func() bool {

				ometa := api.ObjectMeta{Tenant: "Coke"}
				wklds, err := wkldIf.List(ts.tu.NewLoggedInContext(context.Background()), &api.ListWatchOptions{ObjectMeta: ometa})
				if err != nil || len(wklds) != 0 {
					By(fmt.Sprintf("ts:%s Workload LIST has unexpected objects for Coke, err: %+v wklds: %+v", time.Now().String(), err, wklds))
					return false
				}
				ometa = api.ObjectMeta{Tenant: "Pepsi"}
				wklds, err = wkldIf.List(ts.tu.NewLoggedInContext(context.Background()), &api.ListWatchOptions{ObjectMeta: ometa})
				if err != nil || len(wklds) != 0 {
					By(fmt.Sprintf("ts:%s Workload LIST has unexpected objects for Pepsi, err: %+v wklds: %+v", time.Now().String(), err, wklds))
					return false
				}
				return true
			}, 30, 1).Should(BeTrue(), "Unexpected workload objects found")

		})

		AfterEach(func() {
			// Cleanup workload objects regardless of test outcome
			By(fmt.Sprintf("ts:%s Test completed cleaning up workload objects if any", time.Now().String()))
			obj1 := api.ObjectMeta{Name: wkld1Name, Tenant: "Coke"}
			wkldIf.Delete(ts.tu.NewLoggedInContext(context.Background()), &obj1)

			obj2 := api.ObjectMeta{Name: wkld2Name, Tenant: "Pepsi"}
			wkldIf.Delete(ts.tu.NewLoggedInContext(context.Background()), &obj2)
		})
	})
})
