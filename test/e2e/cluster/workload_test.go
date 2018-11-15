// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package cluster

import (
	"fmt"
	"time"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	api "github.com/pensando/sw/api"

	workload "github.com/pensando/sw/api/generated/workload"
)

// workloadTestGroup contains the state common to workload tests
type workloadTestGroup struct {
	suite  *TestSuite
	wkldIf workload.WorkloadV1WorkloadInterface
}

var workloadTg = &workloadTestGroup{}

const (
	wkld1Name = "coke-vm-1"
	wkld2Name = "pepsi-vm-1"
	wkld3Name = "coke-vm-2"
	wkld4Name = "coke-vm-3"
)

// setupTest setup tests
func (wtg *workloadTestGroup) setupTest() {
	wtg.suite = ts
	wtg.wkldIf = ts.restSvc.WorkloadV1().Workload()
}

// workload CRUD operation test
//  Sample workload object in JSON format
//	{
//		"kind" : "Workload",
//		"api-version" : "v1",
//		"meta" : {
//			"name" : "coke-vm-1",
//			"tenant" : "default",
//			"resource-version" : "3123",
//			"uuid" : "485eb70a-1567-4f47-bf53-30b3766e53c4",
//		"labels" : {
//			"_category" : "default"
//		},
//		"creation-time" : "2018-07-11T23:45:28.522868995Z",
//		"mod-time" : "2018-07-11T23:45:28.522876286Z",
//		"self-link" : "/venice/workload/workloads/default/coke-vm-1"
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
func (wtg *workloadTestGroup) testWorkloadCrudops() {

	// workload-1 object
	wkld1 := workload.Workload{
		ObjectMeta: api.ObjectMeta{
			Name:      wkld1Name,
			Tenant:    "default",
			Namespace: "default",
		},
		Spec: workload.WorkloadSpec{
			HostName: "naples1-host",
			// Interfaces Spec keyed by MAC addr of Interface
			Interfaces: []workload.WorkloadIntfSpec{
				{
					MACAddress:   "00:50:56:00:00:01",
					MicroSegVlan: 101,
					ExternalVlan: 1001,
				},
				{
					MACAddress:   "00:50:56:00:00:02",
					MicroSegVlan: 102,
					ExternalVlan: 1002,
				},
			},
		},
	}

	// workload-2 object
	wkld2 := workload.Workload{
		ObjectMeta: api.ObjectMeta{
			Name:      wkld2Name,
			Tenant:    "default",
			Namespace: "default",
		},
		Spec: workload.WorkloadSpec{
			HostName: "naples2-host",
			// Interfaces Spec keyed by MAC addr of Interface
			Interfaces: []workload.WorkloadIntfSpec{
				{
					MACAddress:   "00:50:56:00:00:03",
					MicroSegVlan: 103,
					ExternalVlan: 1003,
				},
				{
					MACAddress:   "00:50:56:00:00:04",
					MicroSegVlan: 104,
					ExternalVlan: 1004,
				},
			},
		},
	}

	// Verify creation for workload-1 object
	Eventually(func() bool {
		w1, err := wtg.wkldIf.Create(wtg.suite.loggedInCtx, &wkld1)
		if err != nil || w1.Name != wkld1.Name {
			By(fmt.Sprintf("ts:%s Workload CREATE failed for [%s] err: %+v w1: %+v", time.Now().String(), wkld1Name, err, w1))
			return false
		}
		By(fmt.Sprintf("ts:%s Workload CREATE validated for [%s]", time.Now().String(), wkld1Name))
		return true
	}, 30, 1).Should(BeTrue(), fmt.Sprintf("Failed to create %s object", wkld1Name))

	// Verify creation for workload-2 object
	Eventually(func() bool {
		w2, err := wtg.wkldIf.Create(wtg.suite.loggedInCtx, &wkld2)
		if err != nil || w2.Name != wkld2.Name {
			By(fmt.Sprintf("ts:%s Workload CREATE failed for [%s] err: %+v w2: %+v", time.Now().String(), wkld2Name, err, w2))
			return false
		}
		By(fmt.Sprintf("ts:%s Workload CREATE validated for [%s]", time.Now().String(), wkld2Name))
		return true
	}, 30, 1).Should(BeTrue(), fmt.Sprintf("Failed to create %s object", wkld2Name))

	// Verify GET for workload-1 object
	Eventually(func() bool {
		obj := api.ObjectMeta{Name: wkld1Name, Tenant: "default"}
		w1, err := wtg.wkldIf.Get(wtg.suite.loggedInCtx, &obj)
		if err != nil || w1.Name != wkld1.Name {
			By(fmt.Sprintf("ts:%s Workload GET failed for [%s] err: %+v w1: %+v", time.Now().String(), wkld1Name, err, w1))
			return false
		}
		By(fmt.Sprintf("ts:%s Workload GET validated for [%s]", time.Now().String(), wkld1Name))
		return true
	}, 30, 1).Should(BeTrue(), fmt.Sprintf("Failed to get %s object", wkld1Name))

	// Verify GET for workload-2 object
	Eventually(func() bool {
		obj := api.ObjectMeta{Name: wkld2Name, Tenant: "default"}
		w2, err := wtg.wkldIf.Get(wtg.suite.loggedInCtx, &obj)
		if err != nil || w2.Name != wkld2.Name {
			By(fmt.Sprintf("ts:%s Workload GET failed for [%s] err: %+v w2: %+v", time.Now().String(), wkld2Name, err, w2))
			return false
		}
		By(fmt.Sprintf("ts:%s Workload GET validated for [%s]", time.Now().String(), wkld2Name))
		return true
	}, 30, 1).Should(BeTrue(), fmt.Sprintf("Failed to get %s object", wkld2Name))

	// Verify LIST of all workload objects (LIST)
	Eventually(func() bool {
		ometa := api.ObjectMeta{Tenant: "default"}
		wklds, err := wtg.wkldIf.List(wtg.suite.loggedInCtx, &api.ListWatchOptions{ObjectMeta: ometa})
		if err != nil || len(wklds) != 2 {
			By(fmt.Sprintf("ts:%s Workload LIST failed, err: %+v wklds: %+v", time.Now().String(), err, wklds))
			return false
		}
		for _, wkld := range wklds {
			if (wkld.Name != wkld1Name) && (wkld.Name != wkld2Name) {
				By(fmt.Sprintf("ts:%s Workload name mismatch, wklds: %+v", time.Now().String(), wklds))
				return false
			}
		}
		By(fmt.Sprintf("ts:%s Workload LIST validated: %+v", time.Now().String(), wklds))

		return true
	}, 30, 1).Should(BeTrue(), "Failed to get all workload objects")

	// verify agents have the endpoints for the workload
	Eventually(func() bool {
		for _, rclient := range ts.netagentClients {
			eplist, err := rclient.EndpointList()
			if err != nil {
				By(fmt.Sprintf("ts:%s endpoint list failed, err: %+v endpoints: %+v", time.Now().String(), err, eplist))
				return false
			}
			if len(eplist) < 2 {
				By(fmt.Sprintf("ts:%s endpoint list doesnt have enough items, endpoints: %+v", time.Now().String(), eplist))
				return false
			}
		}
		return true
	}, 30, 1).Should(BeTrue(), "Failed to get endpoints on netagent")
	// Verify update for workload-2 object
	wkld2.ObjectMeta.Labels = map[string]string{
		"Location": "us-west",
	}
	Eventually(func() bool {
		w2, err := wtg.wkldIf.Update(wtg.suite.loggedInCtx, &wkld2)
		if err != nil || w2.Name != wkld2.Name {
			return false
		}
		By(fmt.Sprintf("ts:%s Workload UPDATE validated for [%s]", time.Now().String(), wkld2Name))
		return true
	}, 30, 1).Should(BeTrue(), "Failed to update workload-2 object")

	// Verify GET on updated workload-2 object
	Eventually(func() bool {
		obj := api.ObjectMeta{Name: wkld2Name, Tenant: "default"}
		w2, err := wtg.wkldIf.Get(wtg.suite.loggedInCtx, &obj)
		if err != nil || w2.Name != wkld2.Name || w2.Labels["Location"] != "us-west" {
			return false
		}
		return true
	}, 30, 1).Should(BeTrue(), fmt.Sprintf("Failed to GET updated %s object", wkld2Name))

	// Verify delete on workload-1 object
	Eventually(func() bool {
		obj := api.ObjectMeta{Name: wkld1Name, Tenant: "default"}
		w1, err := wtg.wkldIf.Delete(wtg.suite.loggedInCtx, &obj)
		if err != nil || w1.Name != wkld1.Name {
			return false
		}
		By(fmt.Sprintf("ts:%s Workload DELETE validated for [%s]", time.Now().String(), wkld1Name))
		return true
	}, 30, 1).Should(BeTrue(), fmt.Sprintf("Failed to delete %s object", wkld1Name))

	// Verify delete on workload-2 object
	Eventually(func() bool {
		obj := api.ObjectMeta{Name: wkld2Name, Tenant: "default"}
		w2, err := wtg.wkldIf.Delete(wtg.suite.loggedInCtx, &obj)
		if err != nil || w2.Name != wkld2.Name {
			return false
		}
		By(fmt.Sprintf("ts:%s Workload DELETE validated for [%s]", time.Now().String(), wkld2Name))

		return true

	}, 30, 1).Should(BeTrue(), fmt.Sprintf("Failed to delete %s object", wkld2Name))

	// Verify GET for all workload objects (LIST) returns empty
	Eventually(func() bool {

		ometa := api.ObjectMeta{Tenant: "default"}
		wklds, err := wtg.wkldIf.List(wtg.suite.loggedInCtx, &api.ListWatchOptions{ObjectMeta: ometa})
		if err != nil || len(wklds) != 0 {
			By(fmt.Sprintf("ts:%s Workload LIST has unexpected objects, err: %+v wklds: %+v", time.Now().String(), err, wklds))
			return false
		}
		return true
	}, 30, 1).Should(BeTrue(), "Unexpected workload objects found")

}

// testWorkloadHostname test workload with invalid host name
func (wtg *workloadTestGroup) testWorkloadHostname() {
	// workload-3 object
	wkld3 := workload.Workload{
		ObjectMeta: api.ObjectMeta{
			Name:      wkld3Name,
			Tenant:    "default",
			Namespace: "default",
		},
		Spec: workload.WorkloadSpec{
			HostName: "10.1.1.0/24",
			// Interfaces Spec keyed by MAC addr of Interface
			Interfaces: []workload.WorkloadIntfSpec{
				{
					MACAddress:   "00:50:56:00:00:01",
					MicroSegVlan: 101,
					ExternalVlan: 1001,
				},
			},
		},
	}

	// Verify creation for workload-3 object fails with invalid hostname
	Eventually(func() bool {
		_, err := wtg.wkldIf.Create(wtg.suite.loggedInCtx, &wkld3)
		By(fmt.Sprintf("ts:%s Workload CREATE status, [%s] err: %+v w3: %+v", time.Now().String(), wkld3Name, err, wkld3))
		if err != nil {
			By(fmt.Sprintf("ts:%s Workload CREATE failed due to invalid host[%s]", time.Now().String(), wkld3.Spec.HostName))
			return true
		}
		By(fmt.Sprintf("ts:%s Workload CREATE expected to fail, but succeeded for [%s] err: %+v w3: %+v", time.Now().String(), wkld3Name, err, wkld3))
		return false
	}, 30, 1).Should(BeTrue(), fmt.Sprintf("Object creation expected to fail, but succeeded for obj:%s host:%s", wkld3Name, wkld3.Spec.HostName))

	// Verify creation for workload-3 object succeeds with valid hostname
	wkld3.Spec.HostName = "naples3-host.local"
	Eventually(func() bool {
		w3, err := wtg.wkldIf.Create(wtg.suite.loggedInCtx, &wkld3)
		if err != nil || w3.Name != wkld3.Name || w3.Spec.HostName != wkld3.Spec.HostName {
			By(fmt.Sprintf("ts:%s Workload CREATE failed for [%s] err: %+v w3: %s", time.Now().String(), wkld3Name, err, w3))
			return false
		}
		By(fmt.Sprintf("ts:%s Workload CREATE validated for [%+v]", time.Now().String(), wkld3))
		return true
	}, 30, 1).Should(BeTrue(), fmt.Sprintf("Failed to create %s object", wkld3Name))

	// Verify update for workload-3 object fails for invalid hostname
	wkld3.Spec.HostName = ".testhost.local"
	Eventually(func() bool {
		_, err := wtg.wkldIf.Update(wtg.suite.loggedInCtx, &wkld3)
		By(fmt.Sprintf("ts:%s Workload UPDATE failed for [%s] err: %+v w3: %+v", time.Now().String(), wkld3Name, err, wkld3))
		if err != nil {
			By(fmt.Sprintf("ts:%s Workload UPDATE failed due to invalid host[%s]", time.Now().String(), wkld3.Spec.HostName))
			return true
		}
		By(fmt.Sprintf("ts:%s Workload UPDATE expected to fail, but succeeded for [%s] err: %+v w3: %+v", time.Now().String(), wkld3Name, err, wkld3))
		return false
	}, 30, 1).Should(BeTrue(), fmt.Sprintf("Object update expected to fail, but succeeded for obj:%s host:%s", wkld3Name, wkld3.Spec.HostName))

	// Verify update for workload-3 object suceeds for valid hostname
	wkld3.Spec.HostName = "naples31-host.local"
	Eventually(func() bool {
		w3, err := wtg.wkldIf.Update(wtg.suite.loggedInCtx, &wkld3)
		if err != nil || w3.Name != wkld3.Name || w3.Spec.HostName != wkld3.Spec.HostName {
			By(fmt.Sprintf("ts:%s Workload UPDATE failed for [%s] err: %+v w3: %+v", time.Now().String(), wkld3Name, err, w3))
			return false
		}
		By(fmt.Sprintf("ts:%s Workload UPDATE validated for [%+v]", time.Now().String(), wkld3))
		return true
	}, 30, 1).Should(BeTrue(), fmt.Sprintf("Failed to update %s object", wkld3Name))
}

// testWorkloadMacaddr tests workloads with invalid mac address
func (wtg *workloadTestGroup) testWorkloadMacaddr() {
	// workload-4 object
	wkld4 := workload.Workload{
		ObjectMeta: api.ObjectMeta{
			Name:      wkld4Name,
			Tenant:    "default",
			Namespace: "default",
		},
		Spec: workload.WorkloadSpec{
			HostName: "naples4-host.local",
			// Interfaces Spec keyed by MAC addr of Interface
			Interfaces: []workload.WorkloadIntfSpec{
				{
					MACAddress:   "aaBB.ccDD.00.00",
					MicroSegVlan: 101,
					ExternalVlan: 1001,
				},
			},
		},
	}

	// Verify creation for workload-4 object fails with invalid interface mac-address
	Eventually(func() bool {
		_, err := wtg.wkldIf.Create(wtg.suite.loggedInCtx, &wkld4)
		By(fmt.Sprintf("ts:%s Workload CREATE status, [%s] err: %+v w4: %+v", time.Now().String(), wkld4Name, err, wkld4))
		if err != nil {
			By(fmt.Sprintf("ts:%s Workload CREATE failed due to invalid interface-mac[%+v]", time.Now().String(), wkld4))
			return true
		}
		By(fmt.Sprintf("ts:%s Workload CREATE expected to fail, but succeeded for [%s] err: %+v w4: %+v", time.Now().String(), wkld4Name, err, wkld4))
		return false
	}, 30, 1).Should(BeTrue(), fmt.Sprintf("Object creation expected to fail, but succeeded for obj:%s with invalid mac", wkld4Name))

	// Verify creation for workload-4 object succeeds with valid interface mac-address
	wkld4.Spec.Interfaces = []workload.WorkloadIntfSpec{
		{
			MACAddress:   "00:50:56:00:00:01",
			MicroSegVlan: 101,
			ExternalVlan: 1001,
		},
	}
	Eventually(func() bool {
		w4, err := wtg.wkldIf.Create(wtg.suite.loggedInCtx, &wkld4)
		if err != nil || w4.Name != wkld4.Name || w4.Spec.HostName != wkld4.Spec.HostName {
			By(fmt.Sprintf("ts:%s Workload CREATE failed for [%s] err: %+v w4: %+v", time.Now().String(), wkld4Name, err, w4))
			return false
		}
		By(fmt.Sprintf("ts:%s Workload CREATE validated for [%+v]", time.Now().String(), wkld4))
		return true
	}, 30, 1).Should(BeTrue(), fmt.Sprintf("Failed to create %s object", wkld4Name))
}

// testdownTest cleans up the test
func (wtg *workloadTestGroup) testdownTest() {
	// Cleanup workload objects regardless of test outcome
	By(fmt.Sprintf("ts:%s Test completed cleaning up workload objects if any", time.Now().String()))
	obj1 := api.ObjectMeta{Name: wkld1Name, Tenant: "default"}
	wtg.wkldIf.Delete(wtg.suite.loggedInCtx, &obj1)

	obj2 := api.ObjectMeta{Name: wkld2Name, Tenant: "default"}
	wtg.wkldIf.Delete(wtg.suite.loggedInCtx, &obj2)

	obj3 := api.ObjectMeta{Name: wkld3Name, Tenant: "default"}
	wtg.wkldIf.Delete(wtg.suite.loggedInCtx, &obj3)

	obj4 := api.ObjectMeta{Name: wkld4Name, Tenant: "default"}
	wtg.wkldIf.Delete(wtg.suite.loggedInCtx, &obj4)
}

/*
Commenting till https://github.com/pensando/sw/issues/7261 is fixed

// run the tests
var _ = Describe("Workload object tests", func() {

	Context("Workload CRUD tests", func() {

		// setup
		BeforeEach(workloadTg.setupTest)

		// run tests
		It("Workload CRUD operations should succeed", workloadTg.testWorkloadCrudops)

		It("Creation or Update of workload object with invalid hostname should fail", workloadTg.testWorkloadHostname)

		It("Creation of workload object with invalid interface mac-address should fail ", workloadTg.testWorkloadMacaddr)

		// cleanup
		AfterEach(workloadTg.testdownTest)
	})
})
*/
