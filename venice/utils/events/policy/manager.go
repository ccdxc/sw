package policy

import (
	"fmt"
	"strings"
	"sync"

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
	evtsProxy *evtsproxy.EventsProxy
	logger    log.Logger
	exporters map[string]map[evtsproxy.ExporterType]events.Exporter
}

// NewManager creates a new manager with given params
func NewManager(proxy *evtsproxy.EventsProxy, logger log.Logger) (*Manager, error) {
	return &Manager{
		evtsProxy: proxy,
		logger:    logger,
		exporters: make(map[string]map[evtsproxy.ExporterType]events.Exporter)}, nil
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
	m.exporters = make(map[string]map[evtsproxy.ExporterType]events.Exporter)
}

// helper function to delete the given event policy
func (m *Manager) delete(policy *evtsmgrprotos.EventPolicy) error {
	m.evtsProxy.UnregisterEventsExporter(policy.GetName())
	for _, writer := range m.exporters[policy.GetName()] {
		writer.Stop()
	}
	delete(m.exporters, policy.GetName())

	return nil
}

// helper function to create writers and associations for the given event policy
func (m *Manager) create(policy *evtsmgrprotos.EventPolicy) error {
	writers, err := m.createWriters(policy)
	if err != nil {
		return err
	}

	syslogExporter, err := m.evtsProxy.RegisterEventsExporter(evtsproxy.Syslog,
		&exporters.SyslogExporterConfig{Name: policy.GetName(), Writers: writers})
	if err != nil {
		return err
	}

	m.exporters[policy.GetName()] = map[evtsproxy.ExporterType]events.Exporter{
		evtsproxy.Syslog: syslogExporter,
	}

	return nil
}

// helper function to create syslog writers for all the targets given on event policy
func (m *Manager) createWriters(policy *evtsmgrprotos.EventPolicy) ([]syslog.Writer, error) {
	priority := syslog.LogUser // is a combination of facility and priority
	if !utils.IsEmpty(policy.Spec.GetSyslogConfig().GetFacilityOverride()) {
		priority = syslog.Priority(monitoring.SyslogFacility_value[policy.Spec.GetSyslogConfig().GetFacilityOverride()])
	}

	tag := "pen-events" // a.k.a prefix tag
	if !utils.IsEmpty(policy.Spec.GetSyslogConfig().GetPrefix()) {
		tag = policy.Spec.GetSyslogConfig().GetPrefix()
	}

	var err error
	var writers []syslog.Writer

	for _, target := range policy.Spec.Targets {
		var writer syslog.Writer

		tmp := strings.Split(target.GetTransport(), "/")                                     // e.g. transport = tcp/514
		network, remoteAddr := tmp[0], fmt.Sprintf("%s:%s", target.GetDestination(), tmp[1]) // {tcp, udp, etc.}, <remote_addr>:<port>

		switch monitoring.MonitoringExportFormat(monitoring.MonitoringExportFormat_value[policy.Spec.GetFormat()]) {
		case monitoring.MonitoringExportFormat_SYSLOG_BSD:
			// TODO: store the config and retry connecting in the background upon failure
			writer, err = syslog.NewBsd(network, remoteAddr, priority, tag)
			if err != nil {
				m.logger.Errorf("failed to create syslog BSD writer, err: %v", err)
			}
		case monitoring.MonitoringExportFormat_SYSLOG_RFC5424:
			m.logger.Infof("creating RFC format writer")
			writer, err = syslog.NewRfc5424(network, remoteAddr, priority, utils.GetHostname(), tag)
			if err != nil {
				m.logger.Errorf("failed to create syslog RFC5424 writer, err: %v", err)
			}
		default:
			return nil, fmt.Errorf("syslog format not supported: %v", policy.Spec.GetFormat())
		}

		writers = append(writers, writer)
	}

	return writers, nil
}
