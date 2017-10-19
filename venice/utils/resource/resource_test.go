package resource

import (
	"fmt"
	"os"
	"runtime"
	"strings"
	"testing"

	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resource/rproto"
	kvapi "github.com/pensando/sw/venice/utils/runtime"
	. "github.com/pensando/sw/venice/utils/testutils"
)

// create a new resource manager instance with memkv as the backend
func newRsrcmgr(t *testing.T, kvstoreURL string) *RsrcMgr {
	// memkv config
	s := kvapi.NewScheme()
	config := store.Config{Type: store.KVStoreTypeMemkv, Servers: strings.Split(kvstoreURL, ","), Codec: kvapi.NewJSONCodec(s)}

	// create a new memkv store
	kv, err := store.New(config)
	if err != nil {
		log.Fatalf("Failed to create store with error: %v", err)
	}

	// create a resource manager instance
	rm, err := NewResourceMgr(kv)
	AssertOk(t, err, "Resourcemgr init failed")

	return rm
}

func TestMain(m *testing.M) {
	lcfg := log.GetDefaultConfig("resource_test")
	log.SetConfig(lcfg)
	os.Exit(m.Run())
}

// test least used scheduler
func TestLeastUsedSched(t *testing.T) {
	// create a resource manager instance
	rm := newRsrcmgr(t, t.Name())

	rsrc := &rproto.Resource{
		ResourceType: "type1",
		ResourceKind: rproto.ResourceKind_Scalar,
		Scalar: &rproto.ScalarResource{
			TotalResource:     10,
			AvailableResource: 10,
		},
	}

	// add some providers
	err := rm.AddProvider(&rproto.ResourceProvide{Resource: rsrc, ProviderID: "prov1"})
	AssertOk(t, err, "AddProvider failed")
	err = rm.AddProvider(&rproto.ResourceProvide{Resource: rsrc, ProviderID: "prov2"})
	AssertOk(t, err, "AddProvider failed")
	err = rm.AddProvider(&rproto.ResourceProvide{Resource: rsrc, ProviderID: "prov3"})
	AssertOk(t, err, "AddProvider failed")

	// resource request
	req := &rproto.ResourceRequest{
		ResourceType: "type1",
		AllocType:    rproto.AllocType_Any,
		Scheduler:    "leastUsed",
		Quantity:     4,
		ConsumerID:   "consumer1",
	}

	// request some resources
	c1, err := rm.RequestResource(req)
	AssertOk(t, err, "RequestResource failed to get a resource for c1")
	Assert(t, c1.ResourceType == "type1", "Returned resource type did not match", c1)
	Assert(t, c1.Values[0] == 4, "Returned resource value was incorrect", c1)
	Assert(t, strings.Contains(c1.ProviderID, "prov"), "Returned provider was incorrect", c1)

	// request some more resources and verify they come from different providers
	req.ConsumerID = "consumer2"
	c2, err := rm.RequestResource(req)
	AssertOk(t, err, "RequestResource failed to get a resource for c2")
	Assert(t, c1.ProviderID != c2.ProviderID, "did not get least used resource", []*rproto.ResourceConsumer{c1, c2})
	req.ConsumerID = "consumer3"
	c3, err := rm.RequestResource(req)
	AssertOk(t, err, "RequestResource failed to get a resource for c3")
	Assert(t, (c1.ProviderID != c3.ProviderID && c2.ProviderID != c3.ProviderID), "did not get least used resource", []*rproto.ResourceConsumer{c1, c2, c3})

	// verify we request more resource than available, we get an error
	req.ConsumerID = "consumer4"
	req.Quantity = 8
	c4, err := rm.RequestResource(req)
	Assert(t, err != nil, "Resource allocation succeeded while expecting it to fail", c4)

	// try deleting a provider and verify it fails
	err = rm.DelProvider(&rproto.ResourceProvide{Resource: rsrc, ProviderID: "prov1"})
	Assert(t, err != nil, "Provider delete succeeded while expecting it to fail", err)

	// release one of the resources
	err = rm.ReleaseResource(c1)
	AssertOk(t, err, "ReleaseResource failed")

	// retry allocating resource which should go thru
	c4, err = rm.RequestResource(req)
	AssertOk(t, err, "RequestResource failed to get a resource for c4")

	// release all resources
	err = rm.ReleaseResource(c2)
	AssertOk(t, err, "ReleaseResource failed")
	err = rm.ReleaseResource(c3)
	AssertOk(t, err, "ReleaseResource failed")
	err = rm.ReleaseResource(c4)
	AssertOk(t, err, "ReleaseResource failed")

	// delete all providers
	err = rm.DelProvider(&rproto.ResourceProvide{Resource: rsrc, ProviderID: "prov1"})
	AssertOk(t, err, "DelProvider failed")
	err = rm.DelProvider(&rproto.ResourceProvide{Resource: rsrc, ProviderID: "prov2"})
	AssertOk(t, err, "DelProvider failed")
	err = rm.DelProvider(&rproto.ResourceProvide{Resource: rsrc, ProviderID: "prov3"})
	AssertOk(t, err, "DelProvider failed")
}

func TestSchedConstraints(t *testing.T) {
	// create a resource manager instance
	rm := newRsrcmgr(t, t.Name())

	rsrc := &rproto.Resource{
		ResourceType: "type1",
		ResourceKind: rproto.ResourceKind_Scalar,
		Scalar: &rproto.ScalarResource{
			TotalResource:     10,
			AvailableResource: 10,
		},
		Attributes: &rproto.ResourceAttributes{
			Attributes: make(map[string]string),
		},
	}

	// add some providers
	rsrc.Attributes.Attributes["attr1"] = "attrVal1"
	err := rm.AddProvider(&rproto.ResourceProvide{Resource: rsrc, ProviderID: "prov1"})
	AssertOk(t, err, "AddProvider failed")
	rsrc.Attributes.Attributes["attr1"] = "attrVal2"
	err = rm.AddProvider(&rproto.ResourceProvide{Resource: rsrc, ProviderID: "prov2"})
	AssertOk(t, err, "AddProvider failed")
	rsrc.Attributes.Attributes["attr1"] = "attrVal3"
	err = rm.AddProvider(&rproto.ResourceProvide{Resource: rsrc, ProviderID: "prov3"})
	AssertOk(t, err, "AddProvider failed")

	// resource request
	req := &rproto.ResourceRequest{
		ResourceType: "type1",
		AllocType:    rproto.AllocType_Any,
		Scheduler:    "leastUsed",
		Quantity:     4,
		ConsumerID:   "consumer1",
		Constraints: &rproto.ResourceConstraints{
			Attributes: &rproto.ResourceAttributes{
				Attributes: make(map[string]string),
			},
		},
	}

	// request some resources from a specific provider
	req.Constraints.ProviderID = "prov1"
	c1, err := rm.RequestResource(req)
	AssertOk(t, err, "RequestResource failed to get a resource for c1")
	Assert(t, (c1.ProviderID == "prov1"), "Returned provider was incorrect", c1)
	req.Constraints.ProviderID = ""

	// request some resource with specific attributes
	req.Constraints.Attributes.Attributes["attr1"] = "attrVal2"
	req.ConsumerID = "consumer2"
	c2, err := rm.RequestResource(req)
	AssertOk(t, err, "RequestResource failed to get a resource for c2")
	Assert(t, (c2.ProviderID == "prov2"), "Returned provider was incorrect", c2)

	// verify incorrect constraits dont match anything
	req.Constraints.Attributes.Attributes["attr1"] = "attrVal5"
	req.ConsumerID = "consumer3"
	c3, err := rm.RequestResource(req)
	Assert(t, err != nil, "resource req with incorrect constraints succeeded while expecting it to fail", c3)
	req.Constraints.Attributes.Attributes["attr1"] = "attrVal2"
	req.Constraints.Attributes.Attributes["attr2"] = "attrVal2"
	c3, err = rm.RequestResource(req)
	Assert(t, err != nil, "resource req with incorrect constraints succeeded while expecting it to fail", c3)
	delete(req.Constraints.Attributes.Attributes, "attr2")

	// verify if we run out of available resources on a specific provider, we return error
	req.Constraints.ProviderID = "prov1"
	req.Quantity = 8
	c3, err = rm.RequestResource(req)
	Assert(t, err != nil, "resource req succeeded while expecting it to fail", c3)
}

// test resource requests in parallel
func TestSchedConcurrency(t *testing.T) {
	// concurrency parameters
	const concurrency = 1000
	const rsrcCount = 10
	runtime.GOMAXPROCS(32)
	errChan := make(chan error, (concurrency + 1))

	// create a resource manager instance
	rm := newRsrcmgr(t, t.Name())

	// create providers in parallel
	for i := 0; i < concurrency/rsrcCount; i++ {
		go func(provId int) {
			provide := &rproto.ResourceProvide{
				ProviderID: fmt.Sprintf("prov%d", provId),
				Resource: &rproto.Resource{
					ResourceType: "type1",
					ResourceKind: rproto.ResourceKind_Scalar,
					Scalar: &rproto.ScalarResource{
						TotalResource:     rsrcCount,
						AvailableResource: rsrcCount,
					},
				},
			}

			// add the providers
			err := rm.AddProvider(provide)
			errChan <- err
		}(i)
	}

	// wait for adds to complete
	for i := 0; i < concurrency/rsrcCount; i++ {
		err := <-errChan
		AssertOk(t, err, "AddProvider failed")
	}

	// request resources in parallel
	var consumerList [concurrency]*rproto.ResourceConsumer
	for i := 0; i < concurrency; i++ {
		go func(reqID int) {
			// resource request
			req := &rproto.ResourceRequest{
				ResourceType: "type1",
				AllocType:    rproto.AllocType_Any,
				Scheduler:    "leastUsed",
				Quantity:     1,
				ConsumerID:   fmt.Sprintf("consumer%d", reqID),
			}

			// request some resources
			c, err := rm.RequestResource(req)
			consumerList[reqID] = c
			errChan <- err

		}(i)
	}

	// wait for allocations to complete
	for i := 0; i < concurrency; i++ {
		err := <-errChan
		AssertOk(t, err, "RequestResource failed to get a resource")
	}

	// free resources in parallel
	for i := 0; i < concurrency; i++ {
		go func(reqID int) {
			err := rm.ReleaseResource(consumerList[reqID])
			errChan <- err
		}(i)
	}

	// wait for releases to complete
	for i := 0; i < concurrency; i++ {
		err := <-errChan
		AssertOk(t, err, "ReleaseResource failed")
	}
}

// test error conditions in add/delete provider
func TestAddDelProvider(t *testing.T) {
	// create a resource manager instance
	rm := newRsrcmgr(t, t.Name())

	rsrc := &rproto.Resource{
		ResourceType: "type1",
		ResourceKind: rproto.ResourceKind_Scalar,
		Scalar: &rproto.ScalarResource{
			TotalResource:     10,
			AvailableResource: 10,
		},
	}

	// add some providers
	err := rm.AddProvider(&rproto.ResourceProvide{Resource: rsrc, ProviderID: "prov1"})
	AssertOk(t, err, "AddProvider failed")

	// add duplicate provider and verify it fails
	err = rm.AddProvider(&rproto.ResourceProvide{Resource: rsrc, ProviderID: "prov1"})
	Assert(t, (err != nil), "Duplicate provider add succeeded", err)

	// add invalid range params
	prov2 := &rproto.ResourceProvide{
		ProviderID: "prov2",
		Resource: &rproto.Resource{
			ResourceType: "type1",
			ResourceKind: rproto.ResourceKind_Range,
			Range: &rproto.RangeResource{
				Begin: 21,
				End:   10,
			},
		},
	}

	err = rm.AddProvider(prov2)
	Assert(t, (err != nil), "Invalid provider params add succeeded", err)

	// try to delete invalid resource type
	rsrc.ResourceType = "type2"
	err = rm.DelProvider(&rproto.ResourceProvide{Resource: rsrc, ProviderID: "prov1"})
	Assert(t, (err != nil), "Invalid resource type delete succeeded", err)
	rsrc.ResourceType = "type1"

	// try deleting non-existing provider id
	err = rm.DelProvider(&rproto.ResourceProvide{Resource: rsrc, ProviderID: "prov2"})
	Assert(t, (err != nil), "Invalid resource type delete succeeded", err)

	// delete the provider
	err = rm.DelProvider(&rproto.ResourceProvide{Resource: rsrc, ProviderID: "prov1"})
	AssertOk(t, err, "DelProvider failed")
}

// test failure conditions in resource allocation
func TestResourceReqFailures(t *testing.T) {
	// create a resource manager instance
	rm := newRsrcmgr(t, t.Name())

	rsrc := &rproto.Resource{
		ResourceType: "type1",
		ResourceKind: rproto.ResourceKind_Scalar,
		Scalar: &rproto.ScalarResource{
			TotalResource:     10,
			AvailableResource: 10,
		},
	}

	// add some providers
	err := rm.AddProvider(&rproto.ResourceProvide{Resource: rsrc, ProviderID: "prov1"})
	AssertOk(t, err, "AddProvider failed")
	err = rm.AddProvider(&rproto.ResourceProvide{Resource: rsrc, ProviderID: "prov2"})
	AssertOk(t, err, "AddProvider failed")

	// try to allocate more resource than any provider can provide
	req := &rproto.ResourceRequest{
		ResourceType: "type1",
		AllocType:    rproto.AllocType_Any,
		Scheduler:    "leastUsed",
		Quantity:     11,
		ConsumerID:   "consumer1",
		Constraints:  &rproto.ResourceConstraints{},
	}
	_, err = rm.RequestResource(req)
	Assert(t, (err != nil), "resource req suceeded while expecting to fail", err)
	req.Quantity = 4

	// try to allocate an invalid resource type
	req.ResourceType = "type2"
	_, err = rm.RequestResource(req)
	Assert(t, (err != nil), "invalid resource type resource req suceeded while expecting to fail", err)
	req.ResourceType = "type1"

	consumer := rproto.ResourceConsumer{
		ConsumerID:   "consumer1",
		ProviderID:   "prov1",
		ResourceType: "type1",
		Values:       []uint64{1},
	}
	// try some invalid resource releases
	err = rm.ReleaseResource(nil)
	Assert(t, (err != nil), "resource release suceeded while expecting to fail", err)
	consumer.ResourceType = "type2"
	err = rm.ReleaseResource(&consumer)
	Assert(t, (err != nil), "resource release suceeded while expecting to fail", err)
	consumer.ResourceType = "type1"
	consumer.ProviderID = "prov5"
	err = rm.ReleaseResource(&consumer)
	Assert(t, (err != nil), "resource release suceeded while expecting to fail", err)
	consumer.ProviderID = "prov1"

	// test idempotent release requests
	err = rm.ReleaseResource(&consumer)
	AssertOk(t, err, "Idempotent release request failed")

	// test a request with empty constraints
	_, err = rm.RequestResource(req)
	AssertOk(t, err, "request with empty constraints failed")

}

func TestRangeResource(t *testing.T) {
	// create a resource manager instance
	rm := newRsrcmgr(t, t.Name())

	rsrc := &rproto.Resource{
		ResourceType: "type1",
		ResourceKind: rproto.ResourceKind_Range,
		Range: &rproto.RangeResource{
			Begin:        11,
			End:          20,
			NumAvailable: 10,
			Allocated:    []byte{},
		},
	}

	// add some providers
	err := rm.AddProvider(&rproto.ResourceProvide{Resource: rsrc, ProviderID: "prov1"})
	AssertOk(t, err, "AddProvider failed")
	err = rm.AddProvider(&rproto.ResourceProvide{Resource: rsrc, ProviderID: "prov2"})
	AssertOk(t, err, "AddProvider failed")

	// resource request
	req := &rproto.ResourceRequest{
		ResourceType: "type1",
		AllocType:    rproto.AllocType_Any,
		Scheduler:    "leastUsed",
		Quantity:     2,
		ConsumerID:   "consumer1",
	}

	// request some resources
	c1, err := rm.RequestResource(req)
	AssertOk(t, err, "RequestResource failed to get a resource for c1")
	Assert(t, len(c1.Values) == 2, "Returned resource value was incorrect", c1)
	Assert(t, c1.Values[0] == 11, "Returned resource value was incorrect", c1)
	Assert(t, c1.Values[1] == 12, "Returned resource value was incorrect", c1)

	// request a specific value from specific provider
	req.ConsumerID = "consumer2"
	req.AllocType = rproto.AllocType_Specific
	req.Values = []uint64{13, 14}
	req.Constraints = &rproto.ResourceConstraints{ProviderID: "prov1"}
	c2, err := rm.RequestResource(req)
	AssertOk(t, err, "RequestResource failed to get a resource for c2")
	Assert(t, len(c2.Values) == 2, "Returned resource value was incorrect", c2)
	Assert(t, c2.Values[0] == 13, "Returned resource value was incorrect", c2)
	Assert(t, c2.Values[1] == 14, "Returned resource value was incorrect", c2)

	// issue duplicate request and verify its idempotent
	c2, err = rm.RequestResource(req)
	AssertOk(t, err, "RequestResource failed to get a resource for c2")

	// request an already allocated value
	req.ConsumerID = "consumer3"
	_, err = rm.RequestResource(req)
	Assert(t, (err != nil), "Was able to allocate unavailable resource", err)
	req.Constraints = nil

	// request out of range resource
	req.Values = []uint64{20, 21}
	_, err = rm.RequestResource(req)
	Assert(t, (err != nil), "Was able to allocate unavailable resource", err)

	// request more resource than available
	req.AllocType = rproto.AllocType_Any
	req.Quantity = 12
	_, err = rm.RequestResource(req)
	Assert(t, (err != nil), "Was able to allocate more resource than available", err)

	// try to free an unallocated value
	c3 := *c2
	c3.Values = []uint64{15}
	_ = rm.ReleaseResource(&c3)

	// free allocated resources
	err = rm.ReleaseResource(c1)
	AssertOk(t, err, "Error freeing resource c1")
	err = rm.ReleaseResource(c2)
	AssertOk(t, err, "Error freeing resource c2")
}

func TestSetResource(t *testing.T) {
	// create a resource manager instance
	rm := newRsrcmgr(t, t.Name())

	rsrc := &rproto.Resource{
		ResourceType: "type1",
		ResourceKind: rproto.ResourceKind_Set,
		Set: &rproto.SetResource{
			NumTotal:       5,
			NumAvailable:   5,
			Items:          []uint64{11, 12, 13, 14, 15, 16},
			AllocatedItems: map[uint64]uint64{},
		},
	}

	// add some providers
	err := rm.AddProvider(&rproto.ResourceProvide{Resource: rsrc, ProviderID: "prov1"})
	AssertOk(t, err, "AddProvider failed")
	err = rm.AddProvider(&rproto.ResourceProvide{Resource: rsrc, ProviderID: "prov2"})
	AssertOk(t, err, "AddProvider failed")

	// resource request
	req := &rproto.ResourceRequest{
		ResourceType: "type1",
		AllocType:    rproto.AllocType_Any,
		Scheduler:    "leastUsed",
		Quantity:     2,
		ConsumerID:   "consumer1",
	}

	// request some resources
	c1, err := rm.RequestResource(req)
	AssertOk(t, err, "RequestResource failed to get a resource for c1")
	Assert(t, len(c1.Values) == 2, "Returned resource value was incorrect", c1)
	Assert(t, c1.Values[0] == 11, "Returned resource value was incorrect", c1)
	Assert(t, c1.Values[1] == 12, "Returned resource value was incorrect", c1)

	// request a specific value from specific provider
	req.ConsumerID = "consumer2"
	req.AllocType = rproto.AllocType_Specific
	req.Values = []uint64{13, 14}
	req.Constraints = &rproto.ResourceConstraints{ProviderID: "prov1"}
	c2, err := rm.RequestResource(req)
	AssertOk(t, err, "RequestResource failed to get a resource for c2")
	Assert(t, len(c2.Values) == 2, "Returned resource value was incorrect", c2)
	Assert(t, c2.Values[0] == 13, "Returned resource value was incorrect", c2)
	Assert(t, c2.Values[1] == 14, "Returned resource value was incorrect", c2)

	// request an already allocated value
	req.ConsumerID = "consumer3"
	req.Values = []uint64{13}
	_, err = rm.RequestResource(req)
	Assert(t, (err != nil), "Was able to allocate unavailable resource", err)
	req.Constraints = nil

	// try to free an unallocated value
	c3 := *c2
	c3.Values = []uint64{15}
	_ = rm.ReleaseResource(&c3)

	// free allocated resources
	err = rm.ReleaseResource(c1)
	AssertOk(t, err, "Error freeing resource c1")
	err = rm.ReleaseResource(c2)
	AssertOk(t, err, "Error freeing resource c2")

}

func TestResourcePartialFailure(t *testing.T) {
	// create a resource manager instance
	rm := newRsrcmgr(t, t.Name())

	rsrc := &rproto.Resource{
		ResourceType: "type1",
		ResourceKind: rproto.ResourceKind_Range,
		Range: &rproto.RangeResource{
			Begin:        11,
			End:          16,
			NumAvailable: 6,
			Allocated:    []byte{},
		},
	}

	// add some providers
	err := rm.AddProvider(&rproto.ResourceProvide{Resource: rsrc, ProviderID: "prov1"})
	AssertOk(t, err, "AddProvider failed")
	err = rm.AddProvider(&rproto.ResourceProvide{Resource: rsrc, ProviderID: "prov2"})
	AssertOk(t, err, "AddProvider failed")

	// resource request
	req := &rproto.ResourceRequest{
		ResourceType: "type1",
		AllocType:    rproto.AllocType_Specific,
		Scheduler:    "leastUsed",
		Quantity:     3,
		Values:       []uint64{11, 12, 13},
		ConsumerID:   "consumer1",
		Constraints: &rproto.ResourceConstraints{
			ProviderID: "prov1",
		},
	}

	// request some resources
	c1, err := rm.RequestResource(req)
	AssertOk(t, err, "RequestResource failed to get a resource for c1")
	Assert(t, len(c1.Values) == 3, "Returned resource value was incorrect", c1)

	// request values that will result in partial failure
	req.Values = []uint64{14, 15, 13}
	req.ConsumerID = "consumer2"
	_, err = rm.RequestResource(req)
	Assert(t, (err != nil), "Was able to allocate conflicting resources", err)

	// re-request with correct values and make sure it goes thru.
	req.Values = []uint64{14, 15, 16}
	req.ConsumerID = "consumer3"
	c3, err := rm.RequestResource(req)
	AssertOk(t, err, "RequestResource failed to get a resource for c3. Partial failure wasnt handled")
	Assert(t, len(c3.Values) == 3, "Returned resource value was incorrect", c3)
}
