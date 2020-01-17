// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

// Resource ID Generator.

package emstore

import (
	"sync"
	"sync/atomic"

	"github.com/pensando/sw/venice/utils/log"
)

// uInt64 is a 64 bit unsigned integer to be used as interface in sync.Map
type uInt64 struct {
	i uint64
}

type resourceIDAllocator struct {
	sync.Mutex
	id sync.Map
}

// ResourceID is the type of resource

func (r *resourceIDAllocator) getNextID(resource ResourceType, offset int) (nextID uint64, err error) {
	r.Lock()
	defer r.Unlock()
	curID, loaded := r.id.LoadOrStore(resource, new(uInt64))
	if !loaded {
		log.Infof("New ID generated for resource type %v", resource)
	}
	id, ok := curID.(*uInt64)
	if ok {
		id.i = id.inc()
		nextID = id.i + uint64(offset)
	}
	r.id.Store(resource, id)
	return
}

func (v *uInt64) inc() uint64 {
	return atomic.AddUint64(&v.i, 1)
}
