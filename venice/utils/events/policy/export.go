package policy

import (
	"context"
	"fmt"
	"strings"
	"sync"
	"time"

	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/ctrler/evtsmgr/rpcserver/protos"
	"github.com/pensando/sw/venice/evtsproxy"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/events"
	"github.com/pensando/sw/venice/utils/events/exporters"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/syslog"
)

// ExportMgr is the export manager for all the event policies.
// It keeps track of all the available exporters for each policy and it updates the list
// based on the CRUD operations on event policies.
type ExportMgr struct {
	sync.RWMutex
	evtsProxy  *evtsproxy.EventsProxy
	logger     log.Logger
	exporters  map[string]map[evtsproxy.ExporterType]interface{}
	cancelFunc context.CancelFunc
	wg         sync.WaitGroup
}

// syslogExporterInfo represents the syslog exporter info (writer and config)
type syslogExporterInfo struct {
	sync.Mutex
	writers  map[string]*syslogWriter
	exporter events.Exporter
}

// syslogWriter contains the config and the associated writer
type syslogWriter struct {
	format     monitoring.MonitoringExportFormat
	network    string
	remoteAddr string
	priority   syslog.Priority
	tag        string
	writer     syslog.Writer
}

// NewExportManager creates a new export manager with given params
func NewExportManager(proxy *evtsproxy.EventsProxy, logger log.Logger) (*ExportMgr, error) {
	ctx, cancelFunc := context.WithCancel(context.Background())
	m := &ExportMgr{
		evtsProxy:  proxy,
		logger:     logger,
		exporters:  make(map[string]map[evtsproxy.ExporterType]interface{}),
		cancelFunc: cancelFunc}

	m.wg.Add(1)
	go m.reconnect(ctx)
	return m, nil
}

// retries the failed connections to syslog server
func (m *ExportMgr) reconnect(ctx context.Context) {
	defer m.wg.Done()
	for {
		select {
		case <-ctx.Done():
			m.logger.Infof("exiting reconnect")
			return
		case <-time.After(time.Second):
			m.RWMutex.RLock()
			for pName, exporters := range m.exporters {
				if ctx.Err() != nil {
					m.logger.Infof("exiting reconnect")
					return
				}
				for exporterType, exporterInfo := range exporters {
					switch exporterType {
					case evtsproxy.Syslog:
						syslogExpInfo := exporterInfo.(*syslogExporterInfo)
						syslogExpInfo.Lock()
						for wKey, w := range syslogExpInfo.writers {
							if w.writer == nil { // try reconnecting
								m.logger.Debugf("reconnecting to syslog server, target: {%s - %v}", pName, wKey)
								if w.writer = m.createSyslogWriter(w.format, w.network, w.remoteAddr, w.tag, w.priority); w.writer != nil {
									m.logger.Infof("reconnected to syslog server {%s - %v/%v}", pName, w.network, w.remoteAddr)
									// these new writers will start receiving messages from this point on.
									// old messages will not be delivered to them.
									syslogExpInfo.exporter.AddWriter(w.writer)
								}
							}
						}
						syslogExpInfo.Unlock()
					default:
						m.logger.Infof("invalid exporter type: %v", exporterType)
					}
				}
			}
			m.RWMutex.RUnlock()
		}
	}
}

// Create creates the list of required syslog writers for the given event policy
func (m *ExportMgr) Create(policy *evtsmgrprotos.EventPolicy) error {
	if policy != nil {
		m.Lock()
		defer m.Unlock()
		return m.create(policy)
	}
	return fmt.Errorf("nil event policy")
}

// Delete stops and deletes all the associated writers for the given event policy
func (m *ExportMgr) Delete(policy *evtsmgrprotos.EventPolicy) error {
	if policy != nil {
		m.Lock()
		defer m.Unlock()
		return m.delete(policy)
	}
	return fmt.Errorf("nil event policy")
}

// Update updates the existing exporter with new set of writers based on the updated policy.
// TODO: compare and delete/update the existing writers. It need not delete and re-create everything
func (m *ExportMgr) Update(policy *evtsmgrprotos.EventPolicy) error {
	if policy != nil {
		m.Lock()
		defer m.Unlock()

		// delete all the old writers
		if err := m.delete(policy); err != nil {
			return err
		}

		// create new writers based on the updated policy
		return m.create(policy)
	}
	return fmt.Errorf("nil event policy")
}

// ExporterExists returns true if the exporter exists for the given policy name; used only in tests
func (m *ExportMgr) ExporterExists(name string) bool {
	m.Lock()
	defer m.Unlock()

	_, found := m.exporters[name]
	return found
}

// TargetExists returns true if the given target exists for the given policy; used only in tests
func (m *ExportMgr) TargetExists(name string, target *monitoring.ExportConfig) bool {
	m.Lock()
	defer m.Unlock()

	if target != nil {
		for exporterType, exporterInfo := range m.exporters[name] {
			switch exporterType {
			case evtsproxy.Syslog:
				syslogExpInfo := exporterInfo.(*syslogExporterInfo)
				syslogExpInfo.Lock()
				for writerKey := range syslogExpInfo.writers {
					if writerKey == m.getWriterKey(*target) {
						syslogExpInfo.Unlock()
						return true
					}
				}
				syslogExpInfo.Unlock()
			}
		}
	}

	return false
}

// Stop stops the export manager
func (m *ExportMgr) Stop() {
	m.cancelFunc()
	m.wg.Wait()
	m.exporters = make(map[string]map[evtsproxy.ExporterType]interface{})
}

// helper function to delete the exporters and configs associated with the given policy
func (m *ExportMgr) delete(policy *evtsmgrprotos.EventPolicy) error {
	if _, found := m.exporters[policy.GetName()]; !found {
		m.logger.Debugf("exporter does not exist for the policy {%v}", policy.GetName())
		return nil
	}

	m.evtsProxy.UnregisterEventsExporter(policy.GetName())
	for t, exporterInfo := range m.exporters[policy.GetName()] {
		switch t {
		case evtsproxy.Syslog:
			syslogExpInfo := exporterInfo.(*syslogExporterInfo)
			syslogExpInfo.Lock()
			syslogExpInfo.exporter.Stop()
			syslogExpInfo.writers = make(map[string]*syslogWriter)
			syslogExpInfo.Unlock()
		default:
			m.logger.Infof("invalid exporter type: %v", t)
		}
	}
	delete(m.exporters, policy.GetName())

	return nil
}

// helper function to create writers for the given event policy
func (m *ExportMgr) create(policy *evtsmgrprotos.EventPolicy) error {
	var err error

	m.createWriters(policy)

	for exporterType, exporterInfo := range m.exporters[policy.GetName()] {
		switch exporterType {
		case evtsproxy.Syslog:
			syslogExporterInfo := exporterInfo.(*syslogExporterInfo)

			var ws []syslog.Writer
			for _, sw := range syslogExporterInfo.writers {
				if sw.writer != nil {
					ws = append(ws, sw.writer)
				}
			}

			syslogExporterInfo.exporter, err = m.evtsProxy.RegisterEventsExporter(evtsproxy.Syslog,
				&exporters.SyslogExporterConfig{Name: policy.GetName(), Writers: ws})
			if err != nil {
				return err
			}
		}
	}

	return nil
}

// helper function to create syslog writers for all the targets given on event policy
func (m *ExportMgr) createWriters(policy *evtsmgrprotos.EventPolicy) {
	syslogWriters := m.createSyslogWriters(policy.Spec.GetSyslogConfig(), policy.Spec.Format, policy.Spec.GetTargets())
	m.exporters[policy.GetName()] = map[evtsproxy.ExporterType]interface{}{
		evtsproxy.Syslog: &syslogExporterInfo{writers: syslogWriters},
	}
}

// helper function to create all the required syslog writers from the config
func (m *ExportMgr) createSyslogWriters(syslogExportCfg *monitoring.SyslogExportConfig, format string,
	targets []*monitoring.ExportConfig) map[string]*syslogWriter {
	priority := syslog.LogUser // is a combination of facility and priority
	if !utils.IsEmpty(syslogExportCfg.GetFacilityOverride()) {
		priority = syslog.Priority(monitoring.SyslogFacility_value[syslogExportCfg.GetFacilityOverride()])
	}

	tag := "pen-events" // a.k.a prefix tag
	if !utils.IsEmpty(syslogExportCfg.GetPrefix()) {
		tag = syslogExportCfg.GetPrefix()
	}

	writers := make(map[string]*syslogWriter)
	for _, target := range targets {
		tmp := strings.Split(target.GetTransport(), "/")                                     // e.g. transport = tcp/514
		network, remoteAddr := tmp[0], fmt.Sprintf("%s:%s", target.GetDestination(), tmp[1]) // {tcp, udp, etc.}, <remote_addr>:<port>
		writer := m.createSyslogWriter(monitoring.MonitoringExportFormat(monitoring.MonitoringExportFormat_value[format]), network, remoteAddr, tag, priority)
		writerKey := m.getWriterKey(*target)
		writers[writerKey] = &syslogWriter{
			format:     monitoring.MonitoringExportFormat(monitoring.MonitoringExportFormat_value[format]),
			network:    network,
			remoteAddr: remoteAddr,
			tag:        tag,
			priority:   priority,
			writer:     writer}
	}

	return writers
}

// helper function to create a syslog writer using the given params
func (m *ExportMgr) createSyslogWriter(format monitoring.MonitoringExportFormat, network, remoteAddr, tag string, priority syslog.Priority) syslog.Writer {
	var writer syslog.Writer
	var err error

	switch format {
	case monitoring.MonitoringExportFormat_SYSLOG_BSD:
		writer, err = syslog.NewBsd(network, remoteAddr, priority, tag)
		if err != nil {
			m.logger.Errorf("failed to create syslog BSD writer (will try reconnecting in few secs), err: %v", err)
		}
	case monitoring.MonitoringExportFormat_SYSLOG_RFC5424:
		writer, err = syslog.NewRfc5424(network, remoteAddr, priority, utils.GetHostname(), tag)
		if err != nil {
			m.logger.Errorf("failed to create syslog RFC5424 writer (will try reconnecting in few secs), err: %v", err)
		}
	}

	return writer
}

// getWriterKey constructs and returns the writer key from given export config
func (m *ExportMgr) getWriterKey(target monitoring.ExportConfig) string {
	return fmt.Sprintf("%s:%s", target.GetDestination(), target.GetTransport())
}
