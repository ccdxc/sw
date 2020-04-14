// Package policyengine runs alert policies on objects coming from watcher and outputs match result on output channel.
package policyengine

import (
	"context"
	"fmt"
	"sync"

	"github.com/pensando/sw/api/fields"

	"github.com/pensando/sw/api/generated/monitoring"
	objectdb "github.com/pensando/sw/venice/ctrler/alertmgr/objdb"
	aeutils "github.com/pensando/sw/venice/utils/alertengine"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
)

// Interface for policyengine
type Interface interface {
	// Run policy engine in a separate go routine until a fatal error is encountered.
	// Each input watch event is processed to completion before the next one is picked up.
	Run(ctx context.Context, inchan <-chan *kvstore.WatchEvent) (<-chan *PEOutput, <-chan error, error)

	// Stop policy engine
	Stop()

	// GetRunningStatus of policy engine.
	GetRunningStatus() bool
}

type policyEngine struct {
	// Policy engine run context.
	ctx    context.Context
	cancel context.CancelFunc

	// AlertMgr logger.
	logger log.Logger

	// AlertMgr Object db.
	objdb objectdb.Interface

	// Incoming channel from watcher.
	inCh chan *kvstore.WatchEvent

	// output channel.
	// Output channel is closed when policy engine is explicitly stopped.
	outCh chan *PEOutput

	// Error channel.
	// AlertMgr must monitor the error channel and shutdown on error, allowing itself to be restarted.
	// Error channel is never closed (it will be garbage collected).
	errCh chan error

	// List of objects which should skip policy match execution.
	skipObjKinds []string

	// A cache of alert policies by object kind to speed up policy execution on incoming objects.
	// No mutex required as the incoming objects from watcher are processed to completion in order.
	policiesByKind map[string][]*monitoring.AlertPolicy

	// Running status of policy engine.
	// No mutex required as this is set and read only by the alertmgr run goroutine.
	running bool
}

// PEOutput is the output of policy engine.
// Alert policies are run on managed objects on two occasions:
//   1. when an object is created or updated,
//   2. when a new alert policy is created.
// In the first case, a list of all alert policies that match the managed object is sent out.
// The alert engine can use this list to open/clear/reopen alerts.
// In the second case, objects matching the new alert policy are sent out independently.
type PEOutput struct {
	// Managed object.
	Object interface{}

	// Operation on the object (C/U/D).
	Op kvstore.WatchEventType

	// Alert policies are not applied on certain object kinds, for eg, alert objects.
	// These objects are just passed on to the alert engine.
	WasPolicyApplied bool

	// List of alert policies that matched the object, after a create or update operation on the object.
	// Empty, if none matched.
	MatchingPolicies []MatchingPolicy

	// An object that matched a new alert policy created.
	// Nil, if none matched.
	MatchingObj MatchingObject
}

// MatchingPolicy gives details of an alert policy that matched the object.
type MatchingPolicy struct {
	// Alert policy.
	Policy *monitoring.AlertPolicy

	// Actual values of object matching policy requirements.
	Reqs []*monitoring.MatchedRequirement
}

// MatchingObject gives details of an object that matched the policy.
type MatchingObject struct {
	// Matching object.
	Obj interface{}

	// Actual values of object matching policy requirements.
	Reqs []*monitoring.MatchedRequirement
}

// New policy engine.
func New(objdb objectdb.Interface, logger log.Logger) (Interface, error) {
	// Fetch alert policies that are already there.
	policiesByKind := make(map[string][]*monitoring.AlertPolicy)
	for _, pol := range objdb.List("AlertPolicy") {
		res := pol.(*monitoring.AlertPolicy).Spec.Resource
		if res != "Event" {
			policiesByKind[res] = append(policiesByKind[res], pol.(*monitoring.AlertPolicy))
		}
	}

	pe := &policyEngine{
		logger:         logger,
		objdb:          objdb,
		policiesByKind: policiesByKind}

	logger.Infof("Created new policy engine")
	return pe, nil
}

func (pe *policyEngine) Run(ctx context.Context, inCh <-chan *kvstore.WatchEvent) (<-chan *PEOutput, <-chan error, error) {
	if pe.running {
		return nil, nil, fmt.Errorf("policy engine already running")
	}

	pe.ctx, pe.cancel = context.WithCancel(ctx)
	pe.outCh = make(chan *PEOutput)
	pe.errCh = make(chan error, 1)

	go func() {
		defer pe.cleanup()

		for wEvent := range inCh {
			result, err := pe.processWatchEvent(wEvent)
			if err != nil {
				if err != pe.ctx.Err() {
					pe.errCh <- err
				}
				return
			}

			if result == nil {
				// Nothing to output.
				continue
			}

			// Send result on output channel.
			select {
			case pe.outCh <- result:
			case <-pe.ctx.Done():
				pe.logger.Errorf("Context cancelled, exiting")
				return
			}
		}
	}()

	pe.running = true
	pe.logger.Infof("Running policy engine")
	return pe.outCh, pe.errCh, nil
}

func (pe *policyEngine) Stop() {
	pe.cancel()
}

func (pe *policyEngine) GetRunningStatus() bool {
	return pe.running
}

func (pe *policyEngine) processWatchEvent(wEvent *kvstore.WatchEvent) (*PEOutput, error) {
	// Skip if input is in skip list.
	obj := wEvent.Object
	kind := obj.GetObjectKind()
	for _, k := range pe.skipObjKinds {
		if kind == k {
			return nil, nil
		}
	}

	result := &PEOutput{Object: obj, Op: wEvent.Type}

	switch obj.(type) {
	case *monitoring.Alert:
		// Pass to output channel.

	case *monitoring.AlertPolicy:
		res := obj.(*monitoring.AlertPolicy).Spec.Resource
		if res != "Event" {
			// Update policyByKind cache.
			pe.updatePoliciesByKind(res, obj.(*monitoring.AlertPolicy), wEvent.Type)

			// If the policy just got created, run it on all objects of referenced kind.
			if wEvent.Type == kvstore.Created {
				pe.runPolicyOnObjects(obj.(*monitoring.AlertPolicy), res, result)
				return nil, nil
			}
		}

	default:
		// If a managed object just got created or updated, run all applicable policies on it.
		if (wEvent.Type == kvstore.Created) || (wEvent.Type == kvstore.Updated) {
			mpl := pe.runPoliciesOnObject(obj, kind, wEvent.Type)
			result.WasPolicyApplied = true
			result.MatchingPolicies = mpl
		}
	}

	return result, nil
}

func (pe *policyEngine) runPoliciesOnObject(obj interface{}, kind string, op kvstore.WatchEventType) []MatchingPolicy {
	numPols := len(pe.policiesByKind[kind])
	if numPols == 0 {
		return nil
	}

	runPolicies := func() <-chan MatchingPolicy {
		var wg sync.WaitGroup
		ch := make(chan MatchingPolicy, numPols)
		for _, policy := range pe.policiesByKind[kind] {
			wg.Add(1)
			go func(policy *monitoring.AlertPolicy) {
				defer wg.Done()
				var reqs []*fields.Requirement
				for _, t := range policy.Spec.GetRequirements() {
					r := *t
					reqs = append(reqs, &r)
				}
				match, req := aeutils.Match(reqs, obj.(runtime.Object))
				if match {
					ch <- MatchingPolicy{Policy: policy, Reqs: req}
				}
			}(policy)
		}
		go func() {
			wg.Wait()
			close(ch)
		}()
		return ch
	}

	var mpl []MatchingPolicy
	for mp := range runPolicies() {
		mpl = append(mpl, mp)
	}

	return mpl
}

func (pe *policyEngine) runPolicyOnObjects(pol *monitoring.AlertPolicy, kind string, result *PEOutput) {
	// First, pass on the alert policy.
	select {
	case pe.outCh <- result:
	case <-pe.ctx.Done():
		pe.logger.Errorf("Context cancelled, exiting")
		return
	}

	objs := pe.objdb.List(kind)
	if len(objs) == 0 {
		return
	}

	var wg sync.WaitGroup
	for _, obj := range objs {
		wg.Add(1)
		go func() {
			defer wg.Done()
			var reqs []*fields.Requirement
			for _, t := range pol.Spec.GetRequirements() {
				r := *t
				reqs = append(reqs, &r)
			}
			match, req := aeutils.Match(reqs, obj.(runtime.Object))
			if match {
				rslt := *result
				rslt.WasPolicyApplied = true
				rslt.MatchingObj = MatchingObject{Obj: obj, Reqs: req}
				select {
				case pe.outCh <- &rslt:
				case <-pe.ctx.Done():
					pe.logger.Errorf("Context cancelled, exiting")
					return
				}
			}
		}()
		wg.Wait()
	}
}

func (pe *policyEngine) updatePoliciesByKind(kind string, policy *monitoring.AlertPolicy, op kvstore.WatchEventType) {
	ometa, _ := runtime.GetObjectMeta(policy)
	uuid := ometa.GetUUID()

	found, pos := func() (bool, int) {
		pols, found := pe.policiesByKind[kind]
		if !found {
			return false, 0
		}
		for pos, pol := range pols {
			ometa, _ := runtime.GetObjectMeta(pol)
			if uuid == ometa.GetUUID() {
				return true, pos
			}
		}
		return false, 0
	}()

	if op == kvstore.Created {
		if found {
			// How did this happen?
			pe.logger.Infof("Create operation on policy %v that is already in cache", uuid)
		} else {
			pe.policiesByKind[kind] = append(pe.policiesByKind[kind], policy)
		}
	} else if op == kvstore.Deleted {
		if !found {
			// How did this happen?
			pe.logger.Infof("Delete operation on policy %v that does not exist in cache", uuid)
		} else {
			pe.policiesByKind[kind] = append(pe.policiesByKind[kind][:pos], pe.policiesByKind[kind][pos+1:]...)
		}
	}
}

func (pe *policyEngine) cleanup() {
	if pe.running {
		pe.running = false
		close(pe.errCh)
		close(pe.outCh)
	}
}
