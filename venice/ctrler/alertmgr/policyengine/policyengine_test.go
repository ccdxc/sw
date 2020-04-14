package policyengine

import (
	"context"
	"fmt"
	"sync"
	"testing"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/fields"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/events/generated/eventattrs"
	objectdb "github.com/pensando/sw/venice/ctrler/alertmgr/objdb"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/testutils/policygen"
)

var (
	logConfig = log.GetDefaultConfig(fmt.Sprintf("%s.%s", globals.AlertMgr, "test"))
	logger    = log.SetConfig(logConfig)
	inCh      chan *kvstore.WatchEvent
	pe        *policyEngine
)

func setup(t *testing.T) {
	// Create logger, object db, context.
	logger := logger.WithContext("t_name", t.Name())
	objdb := objectdb.New()

	// Create new policy engine instance.
	pi, err := New(objdb, logger)
	AssertOk(t, err, "Failed to create new policy engine")
	pe = pi.(*policyEngine)

	// Create input channel.
	inCh = make(chan *kvstore.WatchEvent)
	AssertOk(t, err, "Failed to create inCh")
}

func teardown() {
	defer logger.Close()
}

func TestRun(t *testing.T) {
	setup(t)
	Assert(t, pe != nil, "policyengine nil")
	pe.ctx, pe.cancel = context.WithCancel(context.Background())

	defer func() {
		close(inCh)
		pe.cancel()
		time.Sleep(100 * time.Millisecond)
		Assert(t, !pe.GetRunningStatus(), "running flag still set")
	}()

	outCh, errCh, err := pe.Run(pe.ctx, inCh)
	AssertOk(t, err, "Error running policyengine")
	Assert(t, outCh != nil, "out channel nil")
	Assert(t, errCh != nil, "error channel nil")
	Assert(t, pe.GetRunningStatus(), "running flag not set")

	time.Sleep(1 * time.Second)
}

func TestHandleWatchEvent(t *testing.T) {
	setup(t)
	Assert(t, pe != nil, "policyengine nil")
	pe.ctx, pe.cancel = context.WithCancel(context.Background())
	defer func() {
		close(inCh)
		pe.cancel()
		time.Sleep(100 * time.Millisecond)
		Assert(t, !pe.GetRunningStatus(), "running flag still set")
	}()

	outCh, _, err := pe.Run(pe.ctx, inCh)
	AssertOk(t, err, "error running policyengine")

	/* Alert object. */
	alrt := &monitoring.Alert{
		TypeMeta:   api.TypeMeta{Kind: "Alert"},
		ObjectMeta: api.ObjectMeta{Name: "test-alert", Tenant: globals.DefaultTenant},
		Spec:       monitoring.AlertSpec{},
	}

	/* Alert policy object. */
	req := []*fields.Requirement{&fields.Requirement{Key: "status.primary-mac", Operator: "in", Values: []string{"00ae.cd00.1142"}}}
	pol := policygen.CreateAlertPolicyObj(globals.DefaultTenant, "", CreateAlphabetString(5), "DistributedServiceCard", eventattrs.Severity_INFO, "DSC mac check", req, []string{})

	/* DSC object. */
	dsc := policygen.CreateSmartNIC("00ae.cd00.1142", "admitted", "naples-1", &cluster.DSCCondition{Type: "healthy", Status: "true", LastTransitionTime: ""})

	/* Send policy first, then alert, then DSC */
	wEvent1 := &kvstore.WatchEvent{Type: "Created", Key: "Unused", Object: pol}
	wEvent2 := &kvstore.WatchEvent{Type: "Created", Key: "Unused", Object: alrt}
	wEvent3 := &kvstore.WatchEvent{Type: "Created", Key: "Unused", Object: dsc}
	wEvent := []*kvstore.WatchEvent{wEvent1, wEvent2, wEvent3}

	var wg sync.WaitGroup
	var in int
	var out int

	wg.Add(1)

	go func() {
		defer wg.Done()
		for _, we := range wEvent {
			select {
			case inCh <- we:
				in++
				logger.Infof("sent %v", we.Object.GetObjectKind())
				time.Sleep(100 * time.Millisecond)
			case <-time.After(5 * time.Second):
				err := fmt.Errorf("input channel timeout")
				AssertOk(t, err, "Input channel timeout")
			}
		}
	}()

	wg.Add(1)
	go func() {
		defer wg.Done()
		for {
			select {
			case peOutput, ok := <-outCh:
				if !ok {
					return
				}
				kind := peOutput.Object.(runtime.Object).GetObjectKind()
				logger.Infof("received obj %v", kind)
				out++
				if kind == "DistributedServiceCard" {
					Assert(t, peOutput.WasPolicyApplied == true, "policy was not applied")
					Assert(t, len(peOutput.MatchingPolicies) == 1, "%v matching policies found, dsc mac %v", len(peOutput.MatchingPolicies), peOutput.Object.(*cluster.DistributedServiceCard).Status.PrimaryMAC)
					return
				}
			case <-time.After(5 * time.Second):
				err := fmt.Errorf("output channel timeout")
				AssertOk(t, err, "Output channel timeout")
			}
		}
	}()
	wg.Wait()
	Assert(t, in == 3 && out == 3, "in %v, out %v", in, out)
}

func TestRunPolicyOnObjects(t *testing.T) {
	setup(t)
	Assert(t, pe != nil, "policyengine nil")
	pe.ctx, pe.cancel = context.WithCancel(context.Background())

	defer func() {
		close(inCh)
		pe.cancel()
		time.Sleep(100 * time.Millisecond)
		Assert(t, !pe.GetRunningStatus(), "running flag still set")
	}()

	outCh, _, err := pe.Run(pe.ctx, inCh)
	AssertOk(t, err, "error running policyengine")

	/* Alert object. */
	alrt := &monitoring.Alert{
		TypeMeta:   api.TypeMeta{Kind: "Alert"},
		ObjectMeta: api.ObjectMeta{Name: "test-alert", Tenant: globals.DefaultTenant},
		Spec:       monitoring.AlertSpec{},
	}

	/* Alert policy object. */
	req := []*fields.Requirement{&fields.Requirement{Key: "status.primary-mac", Operator: "in", Values: []string{"00ae.cd00.1142"}}}
	pol := policygen.CreateAlertPolicyObj(globals.DefaultTenant, "", CreateAlphabetString(5), "DistributedServiceCard", eventattrs.Severity_INFO, "DSC mac check", req, []string{})

	/* DSC object. */
	dsc := policygen.CreateSmartNIC("00ae.cd00.1142", "admitted", "naples-1", &cluster.DSCCondition{Type: "healthy", Status: "true", LastTransitionTime: ""})

	/* Send alert first, then DSC, then policy. */
	wEvent1 := &kvstore.WatchEvent{Type: "Created", Key: "Unused", Object: alrt}
	wEvent2 := &kvstore.WatchEvent{Type: "Created", Key: "Unused", Object: dsc}
	wEvent3 := &kvstore.WatchEvent{Type: "Created", Key: "Unused", Object: pol}
	wEvent := []*kvstore.WatchEvent{wEvent1, wEvent2, wEvent3}

	var wg sync.WaitGroup
	var in int
	var out int

	wg.Add(1)

	go func() {
		defer wg.Done()
		for _, we := range wEvent {
			select {
			case inCh <- we:
				in++
				logger.Infof("sent %v", we.Object.GetObjectKind())
				time.Sleep(100 * time.Millisecond)
			case <-time.After(5 * time.Second):
				err := fmt.Errorf("input channel timeout")
				AssertOk(t, err, "Input channel timeout")
			}
		}
	}()

	wg.Add(1)
	go func() {
		defer wg.Done()
		for {
			select {
			case peOutput, ok := <-outCh:
				if !ok {
					return
				}
				pe.objdb.Add(peOutput.Object.(objectdb.Object))
				kind := peOutput.Object.(runtime.Object).GetObjectKind()
				logger.Errorf("received obj %v", kind)
				out++
				if kind == "AlertPolicy" && out == 4 {
					Assert(t, peOutput.WasPolicyApplied == true, "policy was not applied")
					matchKind := peOutput.MatchingObj.Obj.(runtime.Object).GetObjectKind()
					Assert(t, matchKind == "DistributedServiceCard", "expected DSC kind, received %v", matchKind)
					return
				}
			case <-time.After(5 * time.Second):
				err := fmt.Errorf("output channel timeout")
				AssertOk(t, err, "Output channel timeout")
			}
		}
	}()
	wg.Wait()
	Assert(t, in == 3 && out == 4, "in %v, out %v", in, out)
}
