package policy

import (
	"strings"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	evtsmgrprotos "github.com/pensando/sw/nic/agent/protos/evtprotos"
	"github.com/pensando/sw/venice/utils/emstore"
	"github.com/pensando/sw/venice/utils/events"
	"github.com/pensando/sw/venice/utils/log"
)

// Policy manager is the central entity managing the CRUD operations on the event policy.
// It is responsible for
// - storing the policy/config in a bolt DB store (optional)
// - creating/updating/deleting syslog writers based on the policy changes.
// It will be used on both network (will be plugged with the policy watcher) and host (will be plugged with the REST server) modes.

// How the export works?
//
//  -------------------------------             --------------------------------               -----------------------
// | Policy Watcher or REST server |  -------> | ExportMgr                      | ----------> | Dispatcher            |
// |                               |   policy  |  - store policy (optional)     |  exporter   | - Register exporter   |
// |                               |           |  - create exporter             |             |                       |
// |                               |           |    -- (through export manager) |             |                       |
//  -------------------------------             --------------------------------               -----------------------
//

// Manager represents the event policy manager
type Manager struct {
	hostname   string // user friendly hostname
	dispatcher events.Dispatcher
	expMgr     *ExportMgr
	store      emstore.Emstore
	logger     log.Logger
}

// MOption fills the optional params for policy manager
type MOption func(*Manager)

// WithStore passes a store for the manager
func WithStore(store emstore.Emstore) MOption {
	return func(m *Manager) {
		m.store = store
	}
}

// NewManager creates a new policy manager with the given params
func NewManager(hostname string, dispatcher events.Dispatcher, logger log.Logger, opts ...MOption) (*Manager, error) {
	expMgr, err := NewExportManager(hostname, dispatcher, logger)
	if err != nil {
		logger.Errorf("failed to create policy manager, err: %v", err)
		return nil, err
	}

	m := &Manager{
		hostname:   hostname,
		dispatcher: dispatcher,
		expMgr:     expMgr,
		logger:     logger,
	}

	for _, opt := range opts {
		if opt != nil {
			opt(m)
		}
	}

	// read existing policies from the store and create exporters
	m.readExistingPoliciesFromStore()

	return m, nil
}

// Create creates the policy manager by storing if agent store if available
// and creates all the required exporters using export manager.
func (m *Manager) Create(policy *evtsmgrprotos.EventPolicy) error {
	m.logger.Infof("creating policy {%s:%s}", policy.GetKind(), policy.GetName())
	if m.store != nil {
		_, err := m.store.Read(policy)
		if err == nil {
			return ErrPolicyExistsAlready
		}
	}

	if err := m.expMgr.Create(policy); err != nil {
		m.logger.Errorf("failed to create exporters for policy {%s}, err: %v", policy.GetName(), err)
		return ErrFailedToCreatePolicy
	}

	if m.store != nil {
		if err := m.store.Write(policy); err != nil {
			m.logger.Errorf("failed to write policy {%s} to agent store, err: %v", policy.GetName(), err)
			m.expMgr.Delete(policy)
			return ErrFailedToCreatePolicy
		}
	}

	return nil
}

// Get returns the policy identified from the given policy meta.
// returns nil if the store is not configured/provided.
func (m *Manager) Get(policy *evtsmgrprotos.EventPolicy) (*evtsmgrprotos.EventPolicy, error) {
	m.logger.Infof("get policy {%s:%s}", policy.GetKind(), policy.GetName())
	policy.Spec = monitoring.EventPolicySpec{}
	if m.store != nil {
		obj, err := m.store.Read(policy)
		if err != nil {
			m.logger.Errorf("failed to read policy {%s} from agent store, err: %v", policy.GetName(), err)
			return nil, ErrFailedToGetPolicy
		}
		if pol, ok := obj.(*evtsmgrprotos.EventPolicy); ok {
			return pol, nil
		}
		return nil, ErrFailedToGetPolicy
	}

	return nil, nil
}

// Delete deletes the given policy from store and the associated exporters.
func (m *Manager) Delete(policy *evtsmgrprotos.EventPolicy) error {
	m.logger.Infof("deleting policy {%s:%s}", policy.GetKind(), policy.GetName())
	if m.store != nil {
		if _, err := m.store.Read(policy); err != nil {
			if strings.Contains(err.Error(), "Object not found") {
				return ErrPolicyDoesNotExists
			}
			m.logger.Errorf("failed to read policy {%s} from agent store, err: %v", policy.GetName(), err)
			return ErrFailedToDeletePolicy
		}
	}

	if err := m.expMgr.Delete(policy); err != nil {
		m.logger.Errorf("failed to delete associated exporters for the policy {%s}, err: %v", policy.GetName(), err)
		return ErrFailedToDeletePolicy
	}

	if m.store != nil {
		if err := m.store.Delete(policy); err != nil {
			m.logger.Errorf("failed to delete policy {%s} from agent store, err: %v", policy.GetName(), err)
			return ErrFailedToDeletePolicy
		}
	}

	return nil
}

// Update updates the given policy on the store and updates the exporters accordingly.
func (m *Manager) Update(policy *evtsmgrprotos.EventPolicy) error {
	m.logger.Infof("updating policy {%s:%s}", policy.GetKind(), policy.GetName())
	if m.store != nil {
		p := &evtsmgrprotos.EventPolicy{TypeMeta: policy.TypeMeta, ObjectMeta: policy.ObjectMeta}
		if _, err := m.store.Read(p); err != nil {
			if strings.Contains(err.Error(), "Object not found") {
				return ErrPolicyDoesNotExists
			}
			m.logger.Errorf("failed to read policy {%s} from agent store, err: %v", policy.GetName(), err)
			return ErrFailedToUpdatePolicy
		}
	}

	if err := m.expMgr.Update(policy); err != nil {
		m.logger.Errorf("failed to update exporters for the policy {%s}, err: %v", policy.GetName(), err)
		return ErrFailedToUpdatePolicy
	}

	if m.store != nil {
		if err := m.store.Write(policy); err != nil {
			m.logger.Errorf("failed to update policy {%s} in the agent store, err: %v", policy.GetName(), err)
			m.expMgr.Delete(policy)
			return ErrFailedToUpdatePolicy
		}
	}

	return nil
}

// List lists all the available event policies from the store.
func (m *Manager) List() ([]*evtsmgrprotos.EventPolicy, error) {
	var eventPolicies []*evtsmgrprotos.EventPolicy

	if m.store != nil {
		objList, err := m.store.List(&evtsmgrprotos.EventPolicy{
			TypeMeta: api.TypeMeta{
				Kind: "EventPolicy",
			},
		})
		if err != nil && err != emstore.ErrTableNotFound {
			m.logger.Errorf("failed to list policies, err: %v", err)
			return []*evtsmgrprotos.EventPolicy{}, ErrFailedToListPolicies
		}

		for _, obj := range objList {
			pol, ok := obj.(*evtsmgrprotos.EventPolicy)
			if !ok {
				return []*evtsmgrprotos.EventPolicy{}, ErrFailedToListPolicies
			}
			eventPolicies = append(eventPolicies, pol)
		}
	}

	return eventPolicies, nil
}

// GetExportManager returns the export manager; this is used only in the tests.
func (m *Manager) GetExportManager() *ExportMgr {
	return m.expMgr
}

// Reset deletes all the existing event policies.
func (m *Manager) Reset() error {
	// delete all the existing event policies that are stored in file (agent store)
	eventPolicies, err := m.List()
	if err != nil {
		return err
	}
	for _, policy := range eventPolicies {
		m.Delete(policy)
	}

	return nil
}

// Stop stops the policy manager.
func (m *Manager) Stop() {
	m.expMgr.Stop()
}

// helper function to read existing policies from the store and create exporters
// when a process restarts or when venice is down, agent will continue to function with the exiting policies.
func (m *Manager) readExistingPoliciesFromStore() {
	if m.store != nil {
		m.logger.Infof("reading existing policies from the store")
		policies, err := m.List()
		if err != nil {
			return
		}

		for _, policy := range policies {
			if err := m.expMgr.Create(policy); err != nil {
				m.logger.Debugf("failed to create exporters for policy {%s}, err: %v", policy.GetName(), err)
			}
		}
		m.logger.Infof("finished reading existing policies from the store")
	}
}
