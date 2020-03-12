package audit

import (
	"github.com/pensando/sw/api/generated/audit"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/utils/memdb"
)

// Auditor represents an audit event processor that processes, stores and manages audit events. An Auditor implementation
// could store audit events in ElasticSearch, log file etc.
type Auditor interface {
	// ProcessEvents handles an audit event
	ProcessEvents(events ...*audit.AuditEvent) error

	// Run will initialize the backend. It must not block, but may run go routines in the background.
	// Run will be called before the first call to ProcessEvents.
	Run() error

	// Shutdown will synchronously shut down the backend while making sure that all pending
	// events are delivered.
	Shutdown()
}

// PolicyChecker checks audit policy to determine if audit event needs to be logged.
type PolicyChecker interface {
	// PopulateEvent modifies audit information from audit event based on policy and returns a copy. Returns true if audit event needs to be logged.
	// Second bool return value indicates if operation should be failed if auditing fails
	PopulateEvent(event *audit.AuditEvent, populators ...EventPopulator) (bool, bool, error)
}

// EventPopulator is an abstraction to populate audit event with information from request, response etc.
type EventPopulator func(*audit.AuditEvent) error

// PolicyGetter abstracts out retrieval of audit policy
type PolicyGetter interface {
	// GetAuditPolicy returns audit policy
	GetAuditPolicy(tenant string) (*monitoring.AuditPolicy, error)

	// ListAuditPolicies lists audit policies
	ListAuditPolicies() []*monitoring.AuditPolicy

	// WatchAuditPolicy establishes watch on audit policy
	WatchAuditPolicy() (*memdb.Watcher, error)

	// StopWatchAuditPolicy cleans up watcher
	StopWatchAuditPolicy(watcher *memdb.Watcher) error

	// Stop un-initializes PolicyGetter
	Stop()

	// Start re-initializes PolicyGetter. It blocks if PolicyGetter has not been un-initialized through Stop()
	Start()
}
