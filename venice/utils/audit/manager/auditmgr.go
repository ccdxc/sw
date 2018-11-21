package manager

import (
	"errors"

	k8serrors "k8s.io/apimachinery/pkg/util/errors"

	auditapi "github.com/pensando/sw/api/generated/audit"
	"github.com/pensando/sw/venice/utils/audit"
	"github.com/pensando/sw/venice/utils/audit/elastic"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

var (
	// ErrNoEventToProcess is thrown when nil audit events are passed to auditor
	ErrNoEventToProcess = errors.New("no audit events to process")
)

type auditManager struct {
	auditors []audit.Auditor
}

// NewAuditManager creates audit logs in configured auditor backends. Currently we only support auditor that synchronously
// logs to Elastic Server.
func NewAuditManager(rslver resolver.Interface, logger log.Logger) audit.Auditor {
	return &auditManager{
		auditors: []audit.Auditor{
			elastic.NewSynchAuditor("", rslver, logger),
		},
	}
}

// WithAuditors creates audit logs using passed in auditors
func WithAuditors(auditor ...audit.Auditor) audit.Auditor {
	auditMgr := &auditManager{}
	auditMgr.auditors = append(auditMgr.auditors, auditor...)
	return auditMgr
}

func (a *auditManager) ProcessEvents(events ...*auditapi.Event) error {
	// remove nil events
	var nEvents []*auditapi.Event
	for _, event := range events {
		if event != nil {
			nEvents = append(nEvents, event)
		}
	}
	if len(nEvents) == 0 {
		return ErrNoEventToProcess
	}
	var errlist []error
	for _, auditor := range a.auditors {
		if err := auditor.ProcessEvents(nEvents...); err != nil {
			errlist = append(errlist, err)
		}
	}
	return k8serrors.NewAggregate(errlist)
}

func (a *auditManager) Run(stopCh <-chan struct{}) error {
	var funcs []func() error
	for _, auditor := range a.auditors {
		funcs = append(funcs, func() error {
			return auditor.Run(stopCh)
		})
	}
	return k8serrors.AggregateGoroutines(funcs...)
}

func (a *auditManager) Shutdown() {
	for _, auditor := range a.auditors {
		auditor.Shutdown()
	}
}
