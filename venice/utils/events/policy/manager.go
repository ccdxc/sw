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

// Policy manager is the central entity managing the CRUD operations on the event policy.
// It is responsible for creating/updating/deleting syslog writers based on the policy changes.
// It will be used on both network (will be plugged with the policy watcher) and host (will be plugged with the REST server) modes.

// How the export works?
//
//  -------------------------------             --------------------               -----------------------
// | Policy Watcher or REST server |  -------> | Manager            | ----------> | Evtsproxy             |
// |                               |   policy  |  - create exporter |  exporter   |  - Register exporter  |
//  -------------------------------             --------------------               -----------------------
//

// Manager represents the event policy manager
type Manager struct {
	sync.Mutex
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

// NewManager creates a new manager with given params
func NewManager(proxy *evtsproxy.EventsProxy, logger log.Logger) (*Manager, error) {
	ctx, cancelFunc := context.WithCancel(context.Background())
	m := &Manager{
		evtsProxy:  proxy,
		logger:     logger,
		exporters:  make(map[string]map[evtsproxy.ExporterType]interface{}),
		cancelFunc: cancelFunc}

	m.wg.Add(1)
	go m.reconnect(ctx)
	return m, nil
}

// retries the failed connections to syslog server
func (m *Manager) reconnect(ctx context.Context) {
	defer m.wg.Done()
	for {
		select {
		case <-ctx.Done():
			m.logger.Infof("exiting reconnect")
			return
		case <-time.After(time.Second):
			for _, exporters := range m.exporters {
				for exporterType, exporterInfo := range exporters {
					switch exporterType {
					case evtsproxy.Syslog:
						syslogExpInfo := exporterInfo.(*syslogExporterInfo)
						syslogExpInfo.Lock()
						for wKey, w := range syslogExpInfo.writers {
							if w.writer == nil { // try reconnecting
								m.logger.Debugf("reconnecting to syslog server, target: {%v}", wKey)
								if w.writer = m.createSyslogWriter(w.format, w.network, w.remoteAddr, w.tag, w.priority); w.writer != nil {
									m.logger.Infof("reconnected to syslog server{%v/%v}", w.network, w.remoteAddr)
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
		}
	}
}

// Create creates the list of required syslog writers for the given event policy
func (m *Manager) Create(policy *evtsmgrprotos.EventPolicy) error {
	if policy != nil {
		m.Lock()
		defer m.Unlock()
		return m.create(policy)
	}
	return fmt.Errorf("nil event policy")
}

// Delete deletes the policy; stops and deletes all the associated writers
func (m *Manager) Delete(policy *evtsmgrprotos.EventPolicy) error {
	if policy != nil {
		m.Lock()
		defer m.Unlock()
		return m.delete(policy)
	}
	return fmt.Errorf("nil event policy")
}

// Update updates the existing exporter with new set of writers based on the updated policy.
// TODO: compare and delete/update the existing writers. It need not delete and re-create everything
func (m *Manager) Update(policy *evtsmgrprotos.EventPolicy) error {
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

// Stop stops the policy manager
func (m *Manager) Stop() {
	m.cancelFunc()
	m.wg.Wait()
	m.exporters = make(map[string]map[evtsproxy.ExporterType]interface{})
}

// helper function to delete the given event policy
func (m *Manager) delete(policy *evtsmgrprotos.EventPolicy) error {
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

// helper function to create writers and associations for the given event policy
func (m *Manager) create(policy *evtsmgrprotos.EventPolicy) error {
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
func (m *Manager) createWriters(policy *evtsmgrprotos.EventPolicy) {
	syslogWriters := m.createSyslogWriters(policy.Spec.GetSyslogConfig(), policy.Spec.Format, policy.Spec.GetTargets())
	m.exporters[policy.GetName()] = map[evtsproxy.ExporterType]interface{}{
		evtsproxy.Syslog: &syslogExporterInfo{writers: syslogWriters},
	}
}

// helper function to create all the required syslog writers from the config
func (m *Manager) createSyslogWriters(syslogExportCfg *monitoring.SyslogExportConfig, format string,
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
func (m *Manager) createSyslogWriter(format monitoring.MonitoringExportFormat, network, remoteAddr, tag string, priority syslog.Priority) syslog.Writer {
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
func (m *Manager) getWriterKey(target monitoring.ExportConfig) string {
	return fmt.Sprintf("%s:%s", target.GetDestination(), target.GetTransport())
}
