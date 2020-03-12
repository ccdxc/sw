package syslog

import (
	"context"
	"errors"
	"fmt"
	"sync"
	"time"

	k8serrors "k8s.io/apimachinery/pkg/util/errors"

	auditapi "github.com/pensando/sw/api/generated/audit"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/audit"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/memdb"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/syslog"
)

type synchAuditor struct {
	sync.RWMutex
	rslver       resolver.Interface
	logger       log.Logger
	policyGetter audit.PolicyGetter
	writers      map[string]map[string]*syslog.WriterConfig // map tenant->writers, nil writer indicates failed initialization of a writer, re-connection will be attempted periodically
	stopCh       chan error
	reportError  bool
}

// Option fills the optional params for syslog-based synchronous auditor
type Option func(*synchAuditor)

// WithPolicyGetter passes in an audit policy getter to the syslog auditor
func WithPolicyGetter(policyGetter audit.PolicyGetter) Option {
	return func(auditor *synchAuditor) {
		auditor.policyGetter = policyGetter
	}
}

// WithReportError sets a flag to indicate if syslog auditor should report error to caller of ProcessEvents()
func WithReportError(reportError bool) Option {
	return func(auditor *synchAuditor) {
		auditor.reportError = reportError
	}
}

// NewSynchAuditor creates an auditor to save audit events synchronously to syslog server
func NewSynchAuditor(rslver resolver.Interface, logger log.Logger, opts ...Option) audit.Auditor {
	auditor := &synchAuditor{
		rslver:  rslver,
		logger:  logger,
		writers: make(map[string]map[string]*syslog.WriterConfig),
		stopCh:  make(chan error),
	}
	for _, opt := range opts {
		if opt != nil {
			opt(auditor)
		}
	}
	return auditor
}

func (a *synchAuditor) ProcessEvents(events ...*auditapi.AuditEvent) error {
	if a.policyGetter == nil {
		return errors.New("audit policy cache not initialized")
	}
	var errList []error
	for _, evt := range events {
		policy, err := a.policyGetter.GetAuditPolicy(evt.Tenant)
		if err != nil {
			a.logger.ErrorLog("method", "ProcessEvents", "msg", fmt.Sprintf("unable to get audit policy for tenant %s", evt.Tenant), "error", err)
			errList = append(errList, err)
			continue
		}
		if policy.Spec.Syslog == nil || !policy.Spec.Syslog.Enabled {
			continue
		}
		a.RLock()
		writers := a.writers[evt.Tenant]
		for _, w := range writers {
			if w.Writer != nil {
				if err := w.Writer.Info(a.generateSyslogMessage(evt)); err != nil {
					errList = append(errList, err)
				}
			}
		}
		a.RUnlock()
	}
	if a.reportError {
		return k8serrors.NewAggregate(errList)
	}
	if len(errList) > 0 {
		a.logger.DebugLog("method", "ProcessEvents", "msg", "error in sending audit events to syslog", "error", k8serrors.NewAggregate(errList))
	}
	return nil
}

func (a *synchAuditor) Run() error {
	if a.policyGetter == nil {
		return errors.New("audit policy cache not initialized")
	}
	a.policyGetter.Start()
	go a.createWriters()
	return nil
}

func (a *synchAuditor) Shutdown() {
	a.stopCh <- errors.New("shutdown on syslog auditor called")
}

func (a *synchAuditor) createWriters() {

Loop:
	for {
		// list and initialize writers first
		policies := a.policyGetter.ListAuditPolicies()
		for _, policy := range policies {
			a.createSyslogWriters(context.Background(), policy)
		}
		watcher, err := a.policyGetter.WatchAuditPolicy() // currently watch on memcache never returns any error
		if err != nil {
			a.logger.ErrorLog("method", "createWriters", "msg", "unable to create audit policy watcher", "error", err)
			time.Sleep(time.Second)
			continue Loop
		}
		for {
			select {
			case <-time.After(6 * time.Second):
				a.reconnectWriters(context.Background())
			case <-a.stopCh:
				a.policyGetter.StopWatchAuditPolicy(watcher)
				a.policyGetter.Stop()
				return
			case evt, ok := <-watcher.Channel:
				if !ok {
					a.logger.ErrorLog("method", "createWriters", "msg", "unable to read from audit policy watcher channel")
					a.policyGetter.StopWatchAuditPolicy(watcher)
					continue Loop
				}
				policy := evt.Obj.(*monitoring.AuditPolicy)
				switch evt.EventType {
				case memdb.UpdateEvent, memdb.CreateEvent:
					a.createSyslogWriters(context.Background(), policy)
				case memdb.DeleteEvent:
					a.Lock()
					writerMap := a.writers[policy.Tenant]
					for _, writerConfig := range writerMap {
						writerConfig.Writer.Close()
					}
					delete(a.writers, policy.Tenant)
					a.Unlock()
				}
			}
		}
	}
}

func (a *synchAuditor) createSyslogWriters(ctx context.Context, policy *monitoring.AuditPolicy) error {
	a.Lock() // remove existing writers in case when syslog config in audit policy was deleted
	writerMap := a.writers[policy.Tenant]
	for _, writerConfig := range writerMap {
		writerConfig.Writer.Close()
	}
	delete(a.writers, policy.Tenant)
	a.Unlock()
	auditor := policy.Spec.Syslog
	if auditor != nil && len(auditor.Targets) > 0 && auditor.SyslogConfig != nil {
		config := auditor.SyslogConfig
		writerMap, err := syslog.CreateSyslogWriters(ctx, "pen-audit-events", "", config, auditor.Format, auditor.Targets)
		if err != nil {
			a.logger.ErrorLog("method", "createSyslogWriters", "msg", "failed to create syslog writers", "tenant", policy.Tenant, "error", err)
			// TODO: throw an event
			return err
		}
		a.Lock()
		a.writers[policy.Tenant] = writerMap
		a.Unlock()
	}
	return nil
}

func (a *synchAuditor) reconnectWriters(ctx context.Context) {
	a.Lock()
	defer a.Unlock()
	for tenant, writersMap := range a.writers {
		for key, w := range writersMap {
			if w.Writer == nil {
				writer, err := syslog.CreateSyslogWriter(ctx, "", monitoring.MonitoringExportFormat(monitoring.MonitoringExportFormat_vvalue[w.Format]), w.Network, w.RemoteAddr, w.Tag, w.Priority)
				if err != nil || writer == nil {
					continue
				}
				a.logger.InfoLog("method", "reconnectWriters", "msg", "reconnected to syslog server {%v/%v} for tenant {%v}", w.Network, w.RemoteAddr, tenant)
				a.writers[tenant][key].Writer = writer
			}
		}
	}
}

// generateSyslogMessage helper function to generate syslog message from the given event
func (a *synchAuditor) generateSyslogMessage(evt *auditapi.AuditEvent) *syslog.Message {
	crTime, _ := evt.CreationTime.Time()
	strData := syslog.StrData{}
	m := map[string]string{
		"id":            evt.GetUUID(),
		"level":         evt.Level,
		"stage":         evt.Stage,
		"user":          evt.User.Name,
		"user-tenant":   evt.User.Tenant,
		"res-tenant":    evt.Resource.Tenant,
		"res-namespace": evt.Resource.Namespace,
		"res-kind":      evt.Resource.Kind,
		"res-name":      evt.Resource.Name,
		"action":        evt.Action,
		"outcome":       evt.Outcome,
		"error":         evt.Data[audit.APIErrorKey],
		"request-uri":   evt.RequestURI,
		"client-ips":    fmt.Sprintf("%v", evt.ClientIPs),
		"gateway-node":  evt.GatewayNode,
		"gateway-ip":    evt.GatewayIP,
		"service-name":  evt.ServiceName,
		"creation-time": crTime.String(),
	}

	strData[fmt.Sprintf("auditevent@%d", globals.PensandoPEN)] = m

	return &syslog.Message{MsgID: "AuditEvent", Msg: evt.Name, StructuredData: strData}
}
