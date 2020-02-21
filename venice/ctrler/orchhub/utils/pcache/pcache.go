package pcache

import (
	"context"
	"fmt"
	"reflect"
	"strings"
	"sync"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/ctkit"
	"github.com/pensando/sw/api/generated/workload"
	"github.com/pensando/sw/venice/ctrler/orchhub/statemgr"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/ref"
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
* To add a kind, add to the switch case in the writeStatemgr, and deleteStatemgr methods
*/

const (
	// PCacheRetryInterval is the cadence of retry for pcache retries
	PCacheRetryInterval = 10 * time.Second
)

type kindEntry struct {
	sync.Mutex
	entries map[string]interface{}
}

// ValidatorFn is the signature of object validation functions
type ValidatorFn func(in interface{}) (isValid bool, commitIfValid bool)

// PCache is the structure for pcache
type PCache struct {
	sync.RWMutex
	Log           log.Logger
	stateMgr      *statemgr.Statemgr
	kinds         map[string]*kindEntry
	validators    map[string]ValidatorFn
	retryInterval time.Duration
}

// NewPCache creates a new instance of pcache
func NewPCache(stateMgr *statemgr.Statemgr, logger log.Logger) *PCache {
	logger.Debugf("Creating pcache")
	return &PCache{
		stateMgr:      stateMgr,
		Log:           logger,
		kinds:         make(map[string]*kindEntry),
		validators:    make(map[string]ValidatorFn),
		retryInterval: PCacheRetryInterval,
	}
}

// SetValidator sets the validator for the given kind. If an object fails the validator, it won't be written to stateMgr
// Validator returns
func (p *PCache) SetValidator(kind string, validator ValidatorFn) {
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

// GetWorkloadByName Retrieves a workload
func (p *PCache) GetWorkloadByName(wlName string) *workload.Workload {
	meta := &api.ObjectMeta{
		Name: wlName,
		// TODO: Don't use default tenant
		Tenant:    globals.DefaultTenant,
		Namespace: globals.DefaultNamespace,
	}
	return p.GetWorkload(meta)
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
func (p *PCache) validateAndPush(kindMap *kindEntry, in interface{}, validateFn ValidatorFn) error {
	valid := true
	shouldCommit := true
	if validateFn != nil {
		valid, shouldCommit = validateFn(in)
	}

	objMeta, err := runtime.GetObjectMeta(in)
	if err != nil {
		return fmt.Errorf("Failed to get object meta, %v", err)
	}
	key := objMeta.GetKey()
	p.Log.Debugf("validateAndPush for %s", key)

	if valid && shouldCommit {
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
		if valid {
			p.Log.Debugf("%s object passed validation but shouldCommit was false, putting in cache", key)
		} else if shouldCommit {
			// not valid but commit, means remove from apiserver if previously committed
			p.Log.Debugf("%s object failed  validation but shouldCommit was true, remove from stateMgr and put it in cache", key)
			p.deleteStatemgr(in)
		} else {
			p.Log.Debugf("%s object failed validation, putting in cache", key)
		}
		// Update in map
		kindMap.entries[key] = in
	}

	return nil
}

// IsValid returns true if the item exists and passes validation
func (p *PCache) IsValid(kind string, meta *api.ObjectMeta) bool {
	if obj := p.Get(kind, meta); obj != nil {
		p.RLock()
		validateFn := p.validators[kind]
		p.RUnlock()
		valid, _ := validateFn(obj)
		return valid
	}
	return false
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
			// Merge user labels
			labels := map[string]string{}
			for k, v := range currObj.Labels {
				if !strings.HasPrefix(k, globals.SystemLabelPrefix) {
					// Only take user labels from existing object
					labels[k] = v
				}
			}
			for k, v := range obj.Labels {
				if strings.HasPrefix(k, globals.SystemLabelPrefix) {
					// Only take system labels from new object
					labels[k] = v
				}
			}
			obj.Labels = labels
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
			p.Log.Debugf("%s %s deleting from statemgr returned %v", "Workload", meta.GetKey(), writeErr)
		}
		return writeErr
	}
	// Unsupported object, we only write it to cache
	p.Log.Errorf("deleteStatemgr called on unsupported object %+v", in)
	return nil
}

// RevalidateKind re-runs the validator for all objects in the cache of the given kind
func (p *PCache) RevalidateKind(kind string) {
	p.RLock()
	kindMap, ok := p.kinds[kind]
	validateFn := p.validators[kind]
	p.RUnlock()
	if !ok {
		return
	}
	kindMap.Lock()
	for _, in := range kindMap.entries {
		err := p.validateAndPush(kindMap, in, validateFn)
		if err != nil {
			p.Log.Errorf("Validate and Push of object failed. Err : %v", err)
		}
	}
	kindMap.Unlock()
}

// Run runs loop to periodically push pending objects to apiserver
func (p *PCache) Run(ctx context.Context, wg *sync.WaitGroup) {
	ticker := time.NewTicker(p.retryInterval)
	inProgress := false
	defer wg.Done()

	for {
		select {
		case <-ctx.Done():
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
}

// Debug returns the pcache entries
func (p *PCache) Debug(params map[string]string) (interface{}, error) {

	addKind := func(kind string, debugMap map[string]interface{}) error {
		kindEntry, ok := p.kinds[kind]
		if !ok {
			return fmt.Errorf("Kind %s is not a known kind", kind)
		}
		retMap := map[string]interface{}{}
		kindEntry.Lock()
		defer kindEntry.Unlock()
		for key, in := range kindEntry.entries {
			retMap[key] = ref.DeepCopy(in)
		}
		debugMap[kind] = retMap
		return nil
	}

	p.RLock()
	defer p.RUnlock()

	debugMap := map[string]interface{}{}
	if kind, ok := params["kind"]; ok {
		err := addKind(kind, debugMap)
		if err != nil {
			return nil, err
		}
		return debugMap, nil
	}

	for kind := range p.kinds {
		err := addKind(kind, debugMap)
		if err != nil {
			return nil, err
		}
	}
	return debugMap, nil
}
