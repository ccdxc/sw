// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package resource

import (
	"errors"
	"sync"

	"github.com/golang/protobuf/proto"

	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resource/rproto"
)

// RsrcProvider provides the instance
type RsrcProvider struct {
	rproto.ResourceProvider                   // persistable resource state
	sync.RWMutex                              // lock the provider
	Kvstore                 kvstore.Interface // kvstore interface

}

// RsrcList holds a list of providers for a resource type
type RsrcList struct {
	sync.RWMutex
	Providers map[string]*RsrcProvider
}

// SchedulerFunc is the scheduler function prototype
type SchedulerFunc func(*rproto.ResourceRequest, []*RsrcProvider) (*RsrcProvider, error)

// RsrcMgr holds the resource manager context
type RsrcMgr struct {
	sync.Mutex                          // lock for the resource manager
	Kvstore    kvstore.Interface        // kvstore interface
	schedulers map[string]SchedulerFunc // schedulers
	RsrcDB     map[string]*RsrcList     // resource database
}

// NewResourceMgr returns new instance of resource manager
func NewResourceMgr(kvstore kvstore.Interface) (*RsrcMgr, error) {
	rm := RsrcMgr{
		Kvstore:    kvstore,
		RsrcDB:     make(map[string]*RsrcList),
		schedulers: make(map[string]SchedulerFunc),
	}

	// setup scheduler functions
	rm.schedulers["leastUsed"] = leastUsedSchedFunc

	return &rm, nil
}

// AddProvider adds a resource provider
func (rm *RsrcMgr) AddProvider(provide *rproto.ResourceProvide) error {
	rm.Lock()
	// see if the resource list exists for this type
	rlist, ok := rm.RsrcDB[provide.Resource.ResourceType]
	if !ok {
		// create the resource list
		rlist = &RsrcList{
			Providers: make(map[string]*RsrcProvider),
		}

		rm.RsrcDB[provide.Resource.ResourceType] = rlist
	}
	rm.Unlock()

	// lock the rlist for modification
	rlist.Lock()
	defer rlist.Unlock()

	// check if the provider already exists
	_, ok = rlist.Providers[provide.ProviderID]
	if ok {
		// TODO: handle this gracefully
		log.Errorf("Provider %s for type: %s already exists", provide.ProviderID, provide.Resource.ResourceType)
		return errors.New("Provider Already exists")
	}

	// create a new provider
	provider := RsrcProvider{
		ResourceProvider: rproto.ResourceProvider{
			Resource:   proto.Clone(provide.Resource).(*rproto.Resource),
			ProviderID: provide.ProviderID,
			Consumers:  make(map[string]*rproto.ResourceConsumer),
		},
		Kvstore: rm.Kvstore,
	}

	// some default value assignments
	switch provider.Resource.ResourceKind {
	case rproto.ResourceKind_Scalar:
		provider.Resource.Scalar.AvailableResource = provider.Resource.Scalar.TotalResource
	case rproto.ResourceKind_Range:
		if provider.Resource.Range.End < provider.Resource.Range.Begin {
			log.Errorf("Invalid provider params. Begining higher than end: %+v", provide)
			return errors.New("Invalid provider params")
		}

		provider.Resource.Range.NumAvailable = provider.Resource.Range.End - provider.Resource.Range.Begin + 1
		provider.Resource.Range.Allocated = []byte{}
	case rproto.ResourceKind_Set:
		provider.Resource.Set.AllocatedItems = make(map[uint64]uint64)
		provider.Resource.Set.NumTotal = uint64(len(provider.Resource.Set.Items))
		provider.Resource.Set.NumAvailable = provider.Resource.Set.NumTotal
	}
	// save it in the DB
	rlist.Providers[provide.ProviderID] = &provider

	// persist to kv store
	return provider.storeProvider(true)
}

// DelProvider removes a provider
func (rm *RsrcMgr) DelProvider(provide *rproto.ResourceProvide) error {
	// see if the resource list exists for this type
	rlist, ok := rm.RsrcDB[provide.Resource.ResourceType]
	if !ok {
		log.Errorf("No providers for resource type %s", provide.Resource.ResourceType)
		return errors.New("No providers for resource type")
	}

	// lock the rlist
	rlist.Lock()
	defer rlist.Unlock()

	// check if the provider exists
	provider, ok := rlist.Providers[provide.ProviderID]
	if !ok {
		log.Errorf("Provider %s for type: %s does not exists", provide.ProviderID, provide.Resource.ResourceType)
		return errors.New("Provider does not exists")
	}

	// TODO: what should we do about existing consumers?
	// verify consumer list is empty
	if len(provider.Consumers) > 0 {
		log.Errorf("Provider %s/%s still has consumers", provide.Resource.ResourceType, provide.ProviderID)
		return errors.New("Provider still has consumers")
	}

	// remove the provider from the DB
	delete(rlist.Providers, provide.ProviderID)

	// persist to kv store
	return provider.removeProvider()
}

// RequestResource requests a resource
// TODO: make sure this is idempotent. duplicate requests from same consumer should
//       not create duplicate reservations
func (rm *RsrcMgr) RequestResource(req *rproto.ResourceRequest) (*rproto.ResourceConsumer, error) {
	// find the resource list from type
	rlist, ok := rm.RsrcDB[req.ResourceType]
	if !ok {
		log.Errorf("No providers for resource type %s", req.ResourceType)
		return nil, errors.New("No providers for resource type")
	}

	// read lock the rlist
	rlist.Lock()
	defer rlist.Unlock()

	// apply constraints and make a short list of providers
	matchedProviders, err := rm.applyConstraints(rlist, req)
	if err != nil {
		log.Errorf("Error applying constraints from req{%+v}. Err: %v", req, err)
		return nil, err
	}

	log.Debugf("Providers %+v matched constraits for req: %+v", matchedProviders, req)

	// find the scheduling algorithm to run
	schedFunc := rm.schedulers[req.Scheduler]

	// run scheduling algorithm on matched providers
	provider, err := schedFunc(req, matchedProviders)
	if err != nil {
		log.Errorf("Scheduler %s returned error: %v", req.Scheduler, err)
		return nil, err
	}

	// consume resource from the provider(i.e, atomically allocate)
	consumer, err := provider.consumeRsrc(req)
	if err != nil {
		log.Errorf("Failed to consume resource from provider: %s. Err: %v", provider.ProviderID, err)
		return nil, err
	}

	return consumer, nil

}

// ReleaseResource releases a consumed resource
func (rm *RsrcMgr) ReleaseResource(consumer *rproto.ResourceConsumer) error {
	// some error checking
	if consumer == nil {
		return errors.New("Invalid consumer")
	}

	// find the resource list from type
	rlist, ok := rm.RsrcDB[consumer.ResourceType]
	if !ok {
		log.Errorf("No providers for resource type %s", consumer.ResourceType)
		return errors.New("No providers for resource type")
	}

	// read lock the rlist
	rlist.Lock()
	defer rlist.Unlock()

	// find the provider
	provider, ok := rlist.Providers[consumer.ProviderID]
	if !ok {
		log.Errorf("Could not find the provider %s", consumer.ProviderID)
		return errors.New("Could not find the provider")
	}

	// make sure the consumer exists, if not ignore the release message to be idempotent
	if !provider.hasConsumer(consumer.ConsumerID) {
		log.Warnf("Trying to release consumer %s for provider %s", consumer.ConsumerID, consumer.ProviderID)
		return nil
	}

	// free the resouece
	return provider.freeRsrc(consumer)
}

// ------------------- private functions ------------------------------------

// matchAttributes match attributes against a set of constraints
func (rm *RsrcMgr) matchConstraints(attrs, constraints *rproto.ResourceAttributes) bool {
	// some error checking
	if constraints == nil {
		return true
	} else if attrs == nil {
		return false
	}

	// loop thru all constraints
	for key, value := range constraints.Attributes {
		attrVal, ok := attrs.Attributes[key]
		if !ok || attrVal != value {
			return false
		}
	}

	return true
}

// applyConstraints applies specified constraints
func (rm *RsrcMgr) applyConstraints(rlist *RsrcList, req *rproto.ResourceRequest) ([]*RsrcProvider, error) {
	var matchedProviders []*RsrcProvider
	if req.Constraints != nil {
		// check if this request was for a specific provider
		if req.Constraints.ProviderID != "" {
			prvdr, ok := rlist.Providers[req.Constraints.ProviderID]
			if ok {
				matchedProviders = append(matchedProviders, prvdr)
			}
		} else {
			// walk all the providers and find the match
			for _, prvdr := range rlist.Providers {
				// if the provider matches the Constraints, add him to provider list
				if rm.matchConstraints(prvdr.Resource.Attributes, req.Constraints.Attributes) {
					matchedProviders = append(matchedProviders, prvdr)
				}
			}
		}
	} else {
		// walk all the providers and add them to the list
		for _, prvdr := range rlist.Providers {
			matchedProviders = append(matchedProviders, prvdr)
		}
	}

	// if no provider matched the constraits return
	if len(matchedProviders) == 0 {
		log.Infof("No provider matched the constraints of req: %+v", req)
		return nil, errors.New("No provider matched the constraints")
	}

	return matchedProviders, nil
}

// ------------------------ Scheduling functions -------------------------
// leastUsedScheduler implements least used Scheduling function
func leastUsedSchedFunc(req *rproto.ResourceRequest, providers []*RsrcProvider) (*RsrcProvider, error) {
	var luProvider *RsrcProvider

	// walk thru all providers
	for _, provdr := range providers {
		// check if this provider has enough resources
		if provdr.availableRsrc() >= req.Quantity {
			// find the least used
			if luProvider == nil {
				luProvider = provdr
			} else {
				if luProvider.availableRsrc() < provdr.availableRsrc() {
					luProvider = provdr
				}
			}
		}
	}

	// check if we found anything
	if luProvider == nil {
		return nil, errors.New("No providers with enough resource")
	}

	log.Debugf("leastUsed scheduler assigned provider %s for consumer %s", luProvider.ProviderID, req.ConsumerID)

	return luProvider, nil
}
