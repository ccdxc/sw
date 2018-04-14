// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package resource

import (
	"bytes"
	"context"
	"errors"
	"fmt"
	"reflect"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/resource/rproto"

	"github.com/willf/bitset"

	"github.com/pensando/sw/venice/utils/log"
)

// ------------------  wrappers for emulating runtime.Object interface -----------

// GetObjectKind returns the kind of an object.
func (provider *RsrcProvider) GetObjectKind() string {
	return "resource-provider"
}

// GetObjectAPIVersion returns the version of an object.
func (provider *RsrcProvider) GetObjectAPIVersion() string {
	return ""
}

// GetObjectMeta returns the ObjectMeta of an object.
func (provider *RsrcProvider) GetObjectMeta() *api.ObjectMeta {
	return &api.ObjectMeta{
		Name:      provider.ProviderID,
		Namespace: provider.Resource.ResourceType,
	}
}

// Clone clones the object into into
func (provider *RsrcProvider) Clone(into interface{}) (interface{}, error) {
	return nil, fmt.Errorf("not implemented")
}

// getProviderKey returns the kvstore key for the provider
func (provider *RsrcProvider) getProviderKey() string {
	return fmt.Sprintf("/venice/resource/%s/provider/%s", provider.Resource.ResourceType, provider.ProviderID)

}

// storeProvider stores the provider in kvstore
func (provider *RsrcProvider) storeProvider(create bool) error {
	// object at the key
	key := provider.getProviderKey()

	// see if this is create or update
	if create {
		return provider.Kvstore.Create(context.Background(), key, provider)
	}

	return provider.Kvstore.Update(context.Background(), key, provider)
}

// removeProvider removes the provider from kvstore
func (provider *RsrcProvider) removeProvider() error {
	// object at the key
	key := provider.getProviderKey()

	// delete from the store
	return provider.Kvstore.Delete(context.Background(), key, nil)
}

// hasConsumer checks if the provider has the consumer
func (provider *RsrcProvider) hasConsumer(consumerID string) bool {
	provider.Lock()
	defer provider.Unlock()
	_, ok := provider.Consumers[consumerID]
	return ok
}

// allocRangeResource allocates range resource
func (provider *RsrcProvider) allocRangeResource(req *rproto.ResourceRequest, consumer *rproto.ResourceConsumer) error {
	// convert bytes to bitset
	totalItems := provider.Resource.Range.End - provider.Resource.Range.Begin + 1
	buf := bytes.NewBuffer(provider.Resource.Range.Allocated)
	bs := bitset.New(uint(totalItems))
	_, _ = bs.ReadFrom(buf)

	// check the allocation type
	switch req.AllocType {
	case rproto.AllocType_Any:
		// loop till we allocate requested Quantity
		for i := uint64(0); i < req.Quantity; i++ {
			// find the next available
			avlbit, ok := bs.NextClear(0)
			if !ok || avlbit >= uint(totalItems) {
				log.Errorf("No available resource in the range")
				return errors.New("No available resource")
			}

			// set the bit
			bs.Set(avlbit)

			// calculate returning values
			val := provider.Resource.Range.Begin + uint64(avlbit)
			consumer.Values = append(consumer.Values, val)

		}

		// modify available resource count
		provider.Resource.Range.NumAvailable -= req.Quantity

	case rproto.AllocType_Specific:
		for _, val := range req.Values {
			// check the requested values
			if val < provider.Resource.Range.Begin || val > provider.Resource.Range.End {
				log.Errorf("Invalid values requested: %+v", req.Values)
				return errors.New("Invalid values requested")
			}

			// check if the bit is available
			reqBit := val - provider.Resource.Range.Begin
			if bs.Test(uint(reqBit)) {
				log.Errorf("Request value %v (bit %v)is not available", val, reqBit)
				return errors.New("Requested value not available")
			}

			// set the bit
			bs.Set(uint(reqBit))

			// calculate returning values
			val := provider.Resource.Range.Begin + reqBit
			consumer.Values = append(consumer.Values, val)
		}

		// modify available resource count
		provider.Resource.Range.NumAvailable -= uint64(len(req.Values))
	}

	// save the bitset back
	_, _ = bs.WriteTo(buf)
	provider.Resource.Range.Allocated = buf.Bytes()

	return nil
}

// getNextAvailableSetRsrc find next available resource
func (provider *RsrcProvider) getNextAvailableSetRsrc(tmpAllocations map[uint64]uint64) (uint64, error) {
	// find next available item
	for _, val := range provider.Resource.Set.Items {
		_, found := provider.Resource.Set.AllocatedItems[val]
		_, tmpFound := tmpAllocations[val]
		if !found && !tmpFound {
			return val, nil
		}
	}

	return 0, errors.New("No resource is available")
}

// allocSetResource allocates from set resource type
func (provider *RsrcProvider) allocSetResource(req *rproto.ResourceRequest, consumer *rproto.ResourceConsumer) error {
	tmpAllocations := make(map[uint64]uint64)
	// check the allocation type
	switch req.AllocType {
	case rproto.AllocType_Any:
		// loop for each item
		for i := uint64(0); i < req.Quantity; i++ {
			// find free resource
			val, err := provider.getNextAvailableSetRsrc(tmpAllocations)
			if err != nil {
				log.Errorf("Could not find any free resource. Err: %v", err)
				return err
			}

			// add it to the list
			consumer.Values = append(consumer.Values, val)
			tmpAllocations[val] = val

		}

		// modify available resource count
		provider.Resource.Set.NumAvailable -= req.Quantity

	case rproto.AllocType_Specific:
		// loop thru requested values
		for _, val := range req.Values {
			// make sure the value is not already allocated
			_, ok := provider.Resource.Set.AllocatedItems[val]
			if ok {
				log.Errorf("Requested value %v is already allocated", val)
				return errors.New("Requested value is not available")
			}

			// add it to returning values
			consumer.Values = append(consumer.Values, val)
			tmpAllocations[val] = val
		}

		// modify available resource count
		provider.Resource.Set.NumAvailable -= uint64(len(req.Values))
	}

	// mark the values as reserved
	for _, val := range consumer.Values {
		// mark it as used
		provider.Resource.Set.AllocatedItems[val] = val
	}

	return nil
}

// consumeRsrc consumes the resource(i.e allocates the resource)
func (provider *RsrcProvider) consumeRsrc(req *rproto.ResourceRequest) (*rproto.ResourceConsumer, error) {
	// see if this consumer has already consumed any resource
	prevConsumer, ok := provider.Consumers[req.ConsumerID]
	if ok {
		// just return previously consumed resource
		consumer := rproto.ResourceConsumer{
			ResourceType: provider.Resource.ResourceType,
			ProviderID:   provider.ProviderID,
			ConsumerID:   req.ConsumerID,
			Values:       make([]uint64, len(prevConsumer.Values)),
		}
		reflect.Copy(reflect.ValueOf(consumer.Values), reflect.ValueOf(prevConsumer.Values))

		return &consumer, nil
	}

	// verify we have enough resources
	if provider.availableRsrc() < req.Quantity {
		log.Errorf("Not enough resources in provider %s. Available: %d req: %d",
			provider.ProviderID, provider.availableRsrc(), req.Quantity)
		return nil, errors.New("Not enough resources")
	}

	// lock the provider
	provider.Lock()
	defer provider.Unlock()

	// create the consumer
	consumer := rproto.ResourceConsumer{
		ResourceType: provider.Resource.ResourceType,
		ProviderID:   provider.ProviderID,
		ConsumerID:   req.ConsumerID,
		Values:       []uint64{},
	}

	// handle based on resource kind
	switch provider.Resource.ResourceKind {
	case rproto.ResourceKind_Scalar:
		// consume the resource
		provider.Resource.Scalar.AvailableResource -= req.Quantity

		// return values
		consumer.Values = append(consumer.Values, req.Quantity)
	case rproto.ResourceKind_Range:
		// allocate a range resource
		err := provider.allocRangeResource(req, &consumer)
		if err != nil {
			log.Errorf("Error allocating range resource. Err: %v", err)
			return nil, err
		}
	case rproto.ResourceKind_Set:
		// allocate a set resource
		err := provider.allocSetResource(req, &consumer)
		if err != nil {
			log.Errorf("Error allocating set resource. Err: %v", err)
			return nil, err
		}

	}

	// save the consumer
	provider.Consumers[req.ConsumerID] = &consumer

	// persist this
	err := provider.storeProvider(false)
	if err != nil {
		log.Errorf("Error storing the provider. Err: %v", err)
		return nil, err
	}

	// done
	return &consumer, nil
}

// freeRsrc frees resource consumed by a resource
func (provider *RsrcProvider) freeRsrc(consumer *rproto.ResourceConsumer) error {
	// lock the provider
	provider.Lock()
	defer provider.Unlock()

	// handle based on resource kind
	switch provider.Resource.ResourceKind {
	case rproto.ResourceKind_Scalar:
		// release the resource
		provider.Resource.Scalar.AvailableResource += consumer.Values[0]

	case rproto.ResourceKind_Range:
		// convert bytes to bitset
		totalItems := provider.Resource.Range.End - provider.Resource.Range.Begin + 1
		buf := bytes.NewBuffer(provider.Resource.Range.Allocated)
		bs := bitset.New(uint(totalItems))
		_, _ = bs.ReadFrom(buf)

		// free each resource
		for _, val := range consumer.Values {
			// verify the bit was allocated
			reqBit := val - provider.Resource.Range.Begin
			if !bs.Test(uint(reqBit)) {
				log.Warnf("Request value %v(bit %v) was not allocated", val, reqBit)
			}

			// clear the bit
			bs.Clear(uint(reqBit))
		}

		// modify available resource count
		provider.Resource.Range.NumAvailable += uint64(len(consumer.Values))

		// save the bitset back
		_, _ = bs.WriteTo(buf)
		provider.Resource.Range.Allocated = buf.Bytes()

	case rproto.ResourceKind_Set:
		// free each value
		for _, val := range consumer.Values {
			_, ok := provider.Resource.Set.AllocatedItems[val]
			if !ok {
				log.Warnf("Value %v was never allocated", val)
			}
			delete(provider.Resource.Set.AllocatedItems, val)
		}

		// modify available resource count
		provider.Resource.Set.NumAvailable += uint64(len(consumer.Values))
	}

	// remove the consumer from DB
	delete(provider.Consumers, consumer.ConsumerID)

	// persist this
	return provider.storeProvider(false)
}

// availableRsrc returns available number of resource on a provider
func (provider *RsrcProvider) availableRsrc() uint64 {
	switch provider.Resource.ResourceKind {
	case rproto.ResourceKind_Scalar:
		return provider.Resource.Scalar.AvailableResource

	case rproto.ResourceKind_Range:
		return provider.Resource.Range.NumAvailable

	case rproto.ResourceKind_Set:
		return provider.Resource.Set.NumAvailable
	}

	return 0
}
