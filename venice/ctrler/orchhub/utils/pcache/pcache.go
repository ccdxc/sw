package pcache

import (
	"context"
	"fmt"
	"reflect"
	"sync"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/ctkit"
	"github.com/pensando/sw/api/generated/workload"
	"github.com/pensando/sw/venice/ctrler/orchhub/statemgr"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
)

/**
* PCache (partial cache) is an object cache that is used to store
* partially complete objects. Since an object's data may be
* coming for multiple sources, this cache will store the incomplete
* objects and write it to statemgr once the object meets the
* given validation requirements.
*
* Current use cases for this are:
*  - we get a VM create event before we get the corresponding host event,
*    preventing a workload being written to APIserver
*  - Prevent workload write until it's vnics have been moved from pvlan
     into its own useg vlan
*  - happen to get tag events before we get a VM create event.
*  - tag events for a VM that isn't on a pensando host yet, but then moves to one.
*
* supported kinds - Workload
* To add a kind, add to the switch case in the writeStatemgr, and deleteFromStateMgr methods
*/

const (
	// PCacheRetryInterval is the cadence of retry for pcache retries
	PCacheRetryInterval = 1
)

type kindEntry struct {
	sync.Mutex
	entries map[string]interface{}
}

// PCache is the structure for pcache
type PCache struct {
	sync.RWMutex
	Log        log.Logger
	stateMgr   *statemgr.Statemgr
	kinds      map[string]*kindEntry
	validators map[string]func(interface{}) bool
	waitGrp    sync.WaitGroup
	ctx        context.Context
	cancel     context.CancelFunc
}

// NewPCache creates a new instance of pcache
func NewPCache(stateMgr *statemgr.Statemgr, logger log.Logger) *PCache {
	logger.Debugf("Creating pcache")
	ctx, cancel := context.WithCancel(context.Background())
	return &PCache{
		stateMgr:   stateMgr,
		Log:        logger,
		kinds:      make(map[string]*kindEntry),
		validators: make(map[string]func(interface{}) bool),
		ctx:        ctx,
		cancel:     cancel,
	}
}

// SetValidator sets the validator for the given kind. If an object fails the validator, it won't be written to stateMgr
func (p *PCache) SetValidator(kind string, validator func(interface{}) bool) {
	p.Log.Infof("validator set for kind %s", kind)
	p.Lock()
	p.validators[kind] = validator
	p.Unlock()
	return
}

// GetWorkload Retrieves a workload
func (p *PCache) GetWorkload(meta *api.ObjectMeta) *workload.Workload {
	obj := p.Get("Workload", meta)
	if obj == nil {
		return nil
	}
	switch ret := obj.(type) {
	case *ctkit.Workload:
		return &ret.Workload
	case *workload.Workload:
		return ret
	default:
		p.Log.Errorf("Workload returned wasn't the expected type %v", ret)
	}
	return nil
}

// Get retrieves the object from either the PCache (apiserver object) or from statemgr (ctkit object)
func (p *PCache) Get(kind string, meta *api.ObjectMeta) interface{} {
	p.RLock()
	kindMap := p.kinds[kind]
	p.RUnlock()
	// Check if it is in our map first. If not, then we check in statemgr.

	if kindMap != nil {
		kindMap.Lock()
		defer kindMap.Unlock()
		if ret, ok := kindMap.entries[meta.GetKey()]; ok {
			return ret
		}
	}

	// Return stateMgr object
	obj, err := p.stateMgr.Controller().FindObject(kind, meta)
	if err != nil {
		return nil
	}
	return obj
}

// validateAndPush validates the object and pushes it to API server or adds it to kindmap as necessary.
// This is not thread safe, and the calling function is expected to hold Write lock on kindMap
func (p *PCache) validateAndPush(kindMap *kindEntry, in interface{}, validateFn func(interface{}) bool) error {
	valid := true
	if validateFn != nil {
		valid = validateFn(in)
	}

	objMeta, err := runtime.GetObjectMeta(in)
	if err != nil {
		return fmt.Errorf("Failed to get object meta, %v", err)
	}
	key := objMeta.GetKey()
	p.Log.Debugf("set for %s", key)

	if valid {
		// Try to write to statemgr
		p.Log.Debugf("%s object passed validation, writing to stateMgr", key)
		err := p.writeStateMgr(in)
		if err != nil {
			// TODO: Look at the err reason to determine what to do.
			// Possible error causes:
			//   If its 4xx, we should save in map and do nothing
			//   5xx should retry?
			//   Unsupported object error
			// Update in map for now
			p.Log.Errorf("%s write to stateMgr failed, err:%s", key, err.Error())
			kindMap.entries[key] = in
		} else if _, ok := kindMap.entries[key]; ok {
			p.Log.Debugf("%s write successful, removing from cache", key)
			delete(kindMap.entries, key)
		}
	} else {
		p.Log.Debugf("%s object failed validation, putting in cache", key)
		// Update in map
		kindMap.entries[key] = in
	}

	return nil
}

// Set adds the object to stateMgr if the object is valid and changed, and stores it in the cache otherwise
func (p *PCache) Set(kind string, in interface{}) error {
	p.Log.Debugf("Set called for kind : %v and Object : %v", kind, in)

	// Get entries for the kind
	p.RLock()
	kindMap, ok := p.kinds[kind]
	validateFn := p.validators[kind]
	p.RUnlock()
	if !ok {
		kindMap = &kindEntry{
			entries: make(map[string]interface{}),
		}
		p.Lock()
		p.kinds[kind] = kindMap
		p.Unlock()
	}

	kindMap.Lock()
	defer kindMap.Unlock()

	return p.validateAndPush(kindMap, in, validateFn)
}

// Delete deletes the given object from pcache and from statemgr
func (p *PCache) Delete(kind string, in interface{}) error {
	// Deletes from cache and statemgr
	obj, err := runtime.GetObjectMeta(in)
	if err != nil {
		p.Log.Errorf("Delete called on a non apiserver object, kind %s", kind)
		return fmt.Errorf(("Object is not an apiserver object"))
	}
	key := obj.GetKey()
	p.Log.Debugf("delete for %s %s", kind, key)

	// Get entries for the kind
	p.RLock()
	kindMap, ok := p.kinds[kind]
	p.RUnlock()
	if ok {
		kindMap.Lock()
		defer kindMap.Unlock()
		if _, ok := kindMap.entries[key]; ok {
			delete(kindMap.entries, key)
			p.Log.Debugf("%s %s deleted from cache", kind, key)
		}
	}

	p.Log.Debugf("%s %s attempting to delete from statemgr", kind, key)
	return p.deleteStatemgr(in)
}

func (p *PCache) writeStateMgr(in interface{}) error {
	ctrler := p.stateMgr.Controller()
	switch obj := in.(type) {
	case *workload.Workload:
		var writeErr error
		meta := obj.GetObjectMeta()
		if currObj, err := ctrler.Workload().Find(meta); err == nil {
			// Object exists and is changed
			if !reflect.DeepEqual(&currObj.Workload, obj) {
				p.Log.Debugf("%s %s statemgr update called", "Workload", meta.GetKey())
				writeErr = ctrler.Workload().Update(obj)
			}
		} else {
			p.Log.Debugf("%s %s statemgr create called", "Workload", meta.GetKey())
			writeErr = ctrler.Workload().Create(obj)
		}
		p.Log.Debugf("%s %s write to statemgr returned %v", "Workload", meta.GetKey(), writeErr)
		return writeErr
	}
	// Unsupported object, we only write it to cache
	p.Log.Errorf("writeStatemgr called on unsupported object %+v", in)
	return fmt.Errorf("Unsupported object")
}

func (p *PCache) deleteStatemgr(in interface{}) error {
	ctrler := p.stateMgr.Controller()
	switch obj := in.(type) {
	case *workload.Workload:
		var writeErr error
		meta := obj.GetObjectMeta()
		if _, err := ctrler.Workload().Find(meta); err == nil {
			// Object exists
			p.Log.Debugf("%s %s deleting from statemgr", "Workload", meta.GetKey())
			writeErr = ctrler.Workload().Delete(obj)
		}
		p.Log.Debugf("%s %s deleting from statemgr returned %v", "Workload", meta.GetKey(), writeErr)
		return writeErr
	}
	// Unsupported object, we only write it to cache
	p.Log.Errorf("deleteStatemgr called on unsupported object %+v", in)
	return fmt.Errorf("Unsupported object")
}

// Run runs loop to periodically push pending objects to apiserver
func (p *PCache) Run() {
	p.waitGrp.Add(1)
	go func() {
		defer p.waitGrp.Done()
		ticker := time.NewTicker(PCacheRetryInterval * time.Second)
		inProgress := false

		for {
			select {
			case <-p.ctx.Done():
				return
			case <-ticker.C:
				if !inProgress {
					inProgress = true

					p.RLock()
					for kind, m := range p.kinds {
						validateFn := p.validators[kind]
						m.Lock()
						for _, in := range m.entries {
							err := p.validateAndPush(m, in, validateFn)
							if err != nil {
								p.Log.Errorf("Validate and Push of object failed. Err : %v", err)
							}
						}
						m.Unlock()
					}
					p.RUnlock()

					inProgress = false
				}
			}
		}
	}()
}

// Stop stops pcache goroutines
func (p *PCache) Stop() {
	p.Lock()
	defer p.Unlock()

	p.cancel()
	p.waitGrp.Wait()
}
