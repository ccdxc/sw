package pcache

import (
	"fmt"
	"reflect"
	"sync"

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
}

// NewPCache creates a new instance of pcache
func NewPCache(stateMgr *statemgr.Statemgr, logger log.Logger) *PCache {
	logger.Infof("Creating pcache")
	return &PCache{
		stateMgr:   stateMgr,
		Log:        logger,
		kinds:      make(map[string]*kindEntry),
		validators: make(map[string]func(interface{}) bool),
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

// Set adds the object to stateMgr if the object is valid and changed, and stores it in the cache otherwise
func (p *PCache) Set(kind string, in interface{}) error {
	objMeta, err := runtime.GetObjectMeta(in)
	if err != nil {
		p.Log.Errorf("Set called on a non apiserver object, kind %s", kind)
		return fmt.Errorf(("Object is not an apiserver object"))
	}
	key := objMeta.GetKey()
	p.Log.Debugf("set for %s %s", kind, key)

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

	valid := true
	if validateFn != nil {
		valid = validateFn(in)
	}

	kindMap.Lock()
	defer kindMap.Unlock()

	if valid {
		// Try to write to statemgr
		p.Log.Debugf("%s %s object passed validation, writing to stateMgr", kind, key)
		err := p.writeStateMgr(in)
		if err != nil {
			// TODO: Look at the err reason to determine what to do.
			// Possible error causes:
			//   If its 4xx, we should save in map and do nothing
			//   5xx should retry?
			//   Unsupported object error
			// Update in map for now
			p.Log.Errorf("%s %s write to stateMgr failed, err:%s", kind, key, err.Error())
			kindMap.entries[key] = in
		} else if _, ok := kindMap.entries[key]; ok {
			p.Log.Debugf("%s %s write successful, removing from cache", kind, key)
			delete(kindMap.entries, key)
		}
	} else {
		p.Log.Debugf("%s %s object failed validation, putting in cache", kind, key)
		// Update in map
		kindMap.entries[key] = in
	}
	return nil
}

// Delete deletes the given object from pcache and from statemgr
func (p *PCache) Delete(kind string, in interface{}) error {
	// Deletes from cache and statemgr
	obj, err := runtime.GetObjectMeta(in)
	if err != nil {
		p.Log.Errorf("Set called on a non apiserver object, kind %s", kind)
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
			return nil
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
