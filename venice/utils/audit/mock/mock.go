package mock

import (
	"fmt"
	"sync"
	"time"

	uuid "github.com/satori/go.uuid"

	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/utils/audit"
	"github.com/pensando/sw/venice/utils/memdb"
)

const (
	// ErrorTenant to simulate error in GetAuditPolicy()
	ErrorTenant = "errortenant"
)

type policyGetter struct {
	sync.RWMutex
	policies      map[string]*monitoring.AuditPolicy
	watcher       *memdb.Watcher
	stopCh        chan error
	simulateError bool
}

func (pg *policyGetter) GetAuditPolicy(tenant string) (*monitoring.AuditPolicy, error) {
	if tenant == ErrorTenant {
		return nil, fmt.Errorf("simulate error in GetAuditPolicy")
	}
	return pg.policies[tenant], nil
}

// ListAuditPolicies lists audit policies
func (pg *policyGetter) ListAuditPolicies() []*monitoring.AuditPolicy {
	var policies []*monitoring.AuditPolicy
	for _, policy := range pg.policies {
		policies = append(policies, policy)
	}
	return policies
}

// WatchAuditPolicy establishes watch on audit policy
func (pg *policyGetter) WatchAuditPolicy() (*memdb.Watcher, error) {
	defer pg.Unlock()
	pg.Lock()
	if pg.simulateError {
		pg.simulateError = false
		return nil, fmt.Errorf("simulate error in WatchAuditPolicy")
	}
	pg.simulateError = true
	return pg.watcher, nil
}

// StopWatchAuditPolicy cleans up watcher
func (pg *policyGetter) StopWatchAuditPolicy(watcher *memdb.Watcher) error {
	return nil
}

// Stop un-initializes PolicyGetter
func (pg *policyGetter) Stop() {
	pg.stopCh <- fmt.Errorf("shutting down mock policy getter")
}

// Start re-initializes PolicyGetter. It blocks if PolicyGetter has not been un-initialized through Stop()
func (pg *policyGetter) Start() {
	go pg.sendWatchEvents()
}

func (pg *policyGetter) sendWatchEvents() {
	for {
		select {
		case <-pg.stopCh:
			return
		case <-time.After(3 * time.Second):
			for _, policy := range pg.policies {
				pg.watcher.Channel <- memdb.Event{
					EventType: memdb.UpdateEvent,
					Obj:       policy,
				}
			}
		case <-time.After(2 * time.Second):
			for _, policy := range pg.policies {
				if policy.Tenant == ErrorTenant {
					pg.watcher.Channel <- memdb.Event{
						EventType: memdb.DeleteEvent,
						Obj:       policy,
					}
					pg.watcher.Channel <- memdb.Event{
						EventType: memdb.CreateEvent,
						Obj:       policy,
					}
				}

			}
		}
	}
}

// NewPolicyGetter returns a mock audit policy getter
func NewPolicyGetter(policies []*monitoring.AuditPolicy) audit.PolicyGetter {
	policyMap := make(map[string]*monitoring.AuditPolicy)
	for _, policy := range policies {
		policyMap[policy.Tenant] = policy
	}
	getter := &policyGetter{
		policies: policyMap,
		watcher: &memdb.Watcher{
			Name:    fmt.Sprintf("%s-%s", "AuditPolicy", uuid.NewV4().String()),
			Channel: make(chan memdb.Event, memdb.WatchLen),
		},
		stopCh:        make(chan error),
		simulateError: true,
	}
	go getter.sendWatchEvents()
	return getter
}
