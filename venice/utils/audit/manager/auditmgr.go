package manager

import (
	"context"
	"fmt"
	"path/filepath"

	k8serrors "k8s.io/apimachinery/pkg/util/errors"

	auditapi "github.com/pensando/sw/api/generated/audit"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/audit"
	"github.com/pensando/sw/venice/utils/audit/elastic"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

type auditManager struct {
	auditors          []audit.Auditor
	logger            log.Logger
	techsupportLogger log.Logger
}

// NewAuditManager creates audit logs in configured auditor backends. Currently we support auditors that synchronously
// logs to Elastic Server and file
func NewAuditManager(rslver resolver.Interface, logger log.Logger) audit.Auditor {
	var tLogger log.Logger // audit logger for tech support collection
	{
		techsupportLogConfig := &log.Config{
			Module:      "audit",
			Format:      log.JSONFmt,
			Filter:      log.AllowAllFilter,
			Debug:       false,
			CtxSelector: log.ContextAll,
			LogToStdout: false,
			LogToFile:   true,
			FileCfg: log.FileConfig{
				Filename:   fmt.Sprintf("%s.log", filepath.Join(globals.LogDir, "audit")),
				MaxSize:    30,
				MaxBackups: 10,
				MaxAge:     15,
			},
		}
		tLogger = log.GetNewLogger(techsupportLogConfig)
	}
	return &auditManager{
		auditors: []audit.Auditor{
			elastic.NewSynchAuditor("", rslver, logger),
			NewLogAuditor(context.Background(), tLogger),
		},
		logger:            logger,
		techsupportLogger: tLogger,
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
		return nil
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
	for _, auditor := range a.auditors {
		if err := auditor.Run(stopCh); err != nil {
			return err
		}
	}
	return nil
}

func (a *auditManager) Shutdown() {
	for _, auditor := range a.auditors {
		auditor.Shutdown()
	}
	if a.techsupportLogger != nil {
		a.techsupportLogger.Close()
	}
}
