package policy

import (
	"context"
	"fmt"
	"strings"
	"sync"
	"time"

	"github.com/pensando/sw/api/generated/monitoring"
	evtsmgrprotos "github.com/pensando/sw/nic/agent/protos/evtprotos"
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
	hostname   string
	dispatcher events.Dispatcher
	logger     log.Logger
	exporters  map[string]map[exporters.Type]interface{}
	ctx        context.Context
	cancelFunc context.CancelFunc
	wg         sync.WaitGroup
}

// syslogExporterInfo represents the syslog exporter info (writer and config)
type syslogExporterInfo struct {
	sync.Mutex
	writers    map[string]*syslogWriter
	exporter   events.Exporter
	cancelFunc context.CancelFunc
	ctx        context.Context
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
func NewExportManager(hostname string, dispatcher events.Dispatcher, logger log.Logger) (*ExportMgr, error) {
	ctx, cancelFunc := context.WithCancel(context.Background())
	m := &ExportMgr{
		hostname:   hostname,
		dispatcher: dispatcher,
		logger:     logger,
		exporters:  make(map[string]map[exporters.Type]interface{}),
		ctx:        ctx,
		cancelFunc: cancelFunc}

	m.wg.Add(1)
	go m.reconnect()
	return m, nil
}

// retries the failed connections to syslog server
func (m *ExportMgr) reconnect() {
	defer m.wg.Done()
	for {
		select {
		case <-m.ctx.Done():
			m.logger.Infof("exiting reconnect")
			return
		case <-time.After(time.Second):
			var wg sync.WaitGroup
			m.RLock()
			for pName, exprs := range m.exporters {
				if m.ctx.Err() != nil {
					m.logger.Infof("exiting reconnect")
					m.RUnlock()
					return
				}
				for exporterType, exporterInfo := range exprs {
					switch exporterType {
					case exporters.Syslog:
						syslogExpInfo := exporterInfo.(*syslogExporterInfo)
						syslogExpInfo.Lock()
						for wKey, w := range syslogExpInfo.writers {
							if w.writer == nil { // try reconnecting

								wg.Add(1)
								go func(pName, wKey string, w *syslogWriter, syslogExpInfo *syslogExporterInfo) {
									defer wg.Done()
									m.logger.Debugf("reconnecting to syslog server, target: {%s - %v}", pName, wKey)

									writer, err := m.createSyslogWriter(syslogExpInfo.ctx, w.format, w.network, w.remoteAddr, w.tag, w.priority)
									if err != nil || writer == nil {
										return
									}

									m.logger.Infof("reconnected to syslog server {%s - %v/%v}", pName, w.network, w.remoteAddr)
									if syslogExpInfo.ctx.Err() != nil {
										return
									}

									syslogExpInfo.Lock()
									syslogExpInfo.writers[wKey].writer = writer
									// these new writers will start receiving messages from this point on.
									// old messages will not be delivered to them.
									syslogExpInfo.exporter.AddWriter(writer)
									syslogExpInfo.Unlock()
								}(pName, wKey, w, syslogExpInfo)

							}
						}
						syslogExpInfo.Unlock()
					default:
						m.logger.Infof("invalid exporter type: %v", exporterType)
					}
				}
			}
			m.RUnlock()
			wg.Wait()
		}
	}
}

// Create creates the list of required syslog writers for the given event policy
func (m *ExportMgr) Create(policy *evtsmgrprotos.EventPolicy) error {
	if policy != nil {
		return m.create(policy)
	}
	return fmt.Errorf("nil event policy")
}

// Delete stops and deletes all the associated writers for the given event policy
func (m *ExportMgr) Delete(policy *evtsmgrprotos.EventPolicy) error {
	if policy != nil {
		return m.delete(policy, false)
	}
	return fmt.Errorf("nil event policy")
}

// Update updates the existing exporter with new set of writers based on the updated policy.
// TODO: compare and delete/update the existing writers. It need not delete and re-create everything
func (m *ExportMgr) Update(policy *evtsmgrprotos.EventPolicy) error {
	if policy != nil {
		// delete all the old writers
		if err := m.delete(policy, true); err != nil {
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
			case exporters.Syslog:
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
	m.exporters = make(map[string]map[exporters.Type]interface{})
}

// helper function to delete the exporters and configs associated with the given policy
func (m *ExportMgr) delete(policy *evtsmgrprotos.EventPolicy, update bool) error {
	m.RLock()
	exportersInfo, found := m.exporters[policy.GetName()]
	if !found {
		m.logger.Debugf("exporter does not exist for the policy {%v}", policy.GetName())
		m.RUnlock()
		return nil
	}
	m.RUnlock()

	// TODO: rewrite the code to handle it better. Update should not be calling delete, writers should be updated inline.
	if update {
		m.dispatcher.UnregisterExporter(policy.GetName())
	} else {
		m.dispatcher.DeleteExporter(policy.GetName())
	}
	for t, exporterInfo := range exportersInfo {
		switch t {
		case exporters.Syslog:
			syslogExpInfo := exporterInfo.(*syslogExporterInfo)
			syslogExpInfo.Lock()
			syslogExpInfo.exporter.Stop()
			syslogExpInfo.cancelFunc() // this will cancel any reconnects that are happening for any of the writers belonging to this policy
			syslogExpInfo.writers = make(map[string]*syslogWriter)
			syslogExpInfo.Unlock()
		default:
			m.logger.Infof("invalid exporter type: %v", t)
		}
	}

	m.Lock()
	delete(m.exporters, policy.GetName())
	m.Unlock()

	return nil
}

// helper function to create writers for the given event policy
func (m *ExportMgr) create(policy *evtsmgrprotos.EventPolicy) error {
	ctx, cancelFunc := context.WithCancel(context.Background())
	syslogWriters, err := m.createSyslogWriters(ctx, policy.Spec.GetSyslogConfig(), policy.Spec.Format, policy.Spec.GetTargets())
	if err != nil {
		cancelFunc()
		return err
	}

	// create other writers once we start supporting (email, etc.)

	// create a syslog exporter with the available writers
	if len(syslogWriters) > 0 {
		var ws []syslog.Writer
		for _, sw := range syslogWriters {
			if sw.writer != nil {
				ws = append(ws, sw.writer)
			}
		}

		// register the exporter with dispatcher (so that it can start receiving events and write it to the desired destination)
		syslogExporter, err := m.startExporter(policy.GetName(), ws)
		if err != nil {
			cancelFunc()
			return err
		}

		m.Lock() // protect the exporters map
		m.exporters[policy.GetName()] = map[exporters.Type]interface{}{
			exporters.Syslog: &syslogExporterInfo{
				writers:    syslogWriters,
				exporter:   syslogExporter,
				cancelFunc: cancelFunc,
				ctx:        ctx,
			},
		}
		m.Unlock()
	} else {
		cancelFunc() // cancel the context that was not used
	}

	return nil
}

// helper function to create all the required syslog writers from the config
func (m *ExportMgr) createSyslogWriters(ctx context.Context, syslogExportCfg *monitoring.SyslogExportConfig, format string,
	targets []*monitoring.ExportConfig) (map[string]*syslogWriter, error) {
	priority := syslog.LogUser // is a combination of facility and priority
	if !utils.IsEmpty(syslogExportCfg.GetFacilityOverride()) {
		priority = syslog.Priority(monitoring.SyslogFacility_vvalue[syslogExportCfg.GetFacilityOverride()])
	}

	tag := "pen-events" // a.k.a prefix tag
	if !utils.IsEmpty(syslogExportCfg.GetPrefix()) {
		tag = syslogExportCfg.GetPrefix()
	}

	writers := make(map[string]*syslogWriter)
	for _, target := range targets {
		tmp := strings.Split(target.GetTransport(), "/")                                                      // e.g. transport = tcp/514
		network, remoteAddr := strings.ToLower(tmp[0]), fmt.Sprintf("%s:%s", target.GetDestination(), tmp[1]) // {tcp, udp, etc.}, <remote_addr>:<port>
		writer, err := m.createSyslogWriter(ctx, monitoring.MonitoringExportFormat(monitoring.MonitoringExportFormat_vvalue[format]), network, remoteAddr, tag, priority)
		if err != nil {
			return nil, err
		}

		writerKey := m.getWriterKey(*target)
		writers[writerKey] = &syslogWriter{
			format:     monitoring.MonitoringExportFormat(monitoring.MonitoringExportFormat_vvalue[format]),
			network:    network,
			remoteAddr: remoteAddr,
			tag:        tag,
			priority:   priority,
			writer:     writer}
	}

	return writers, nil
}

// helper function to create a syslog writer using the given params
func (m *ExportMgr) createSyslogWriter(ctx context.Context, format monitoring.MonitoringExportFormat, network, remoteAddr, tag string, priority syslog.Priority) (syslog.Writer, error) {
	var writer syslog.Writer
	var err error

	switch format {
	case monitoring.MonitoringExportFormat_SYSLOG_BSD:
		writer, err = syslog.NewBsd(network, remoteAddr, priority, m.hostname, tag, syslog.BSDWithContext(ctx))
		if err != nil {
			m.logger.Errorf("failed to create syslog BSD writer for config {%s/%s} (will try reconnecting in few secs), err: %v", remoteAddr, network, err)
		}
	case monitoring.MonitoringExportFormat_SYSLOG_RFC5424:
		writer, err = syslog.NewRfc5424(network, remoteAddr, priority, m.hostname, tag, syslog.RFCWithContext(ctx))
		if err != nil {
			m.logger.Errorf("failed to create syslog RFC5424 writer {%s/%s} (will try reconnecting in few secs), err: %v", remoteAddr, network, err)
		}
	default:
		return nil, fmt.Errorf("invalid syslog format {%v}", format)
	}

	return writer, nil
}

// getWriterKey constructs and returns the writer key from given export config
func (m *ExportMgr) getWriterKey(target monitoring.ExportConfig) string {
	return fmt.Sprintf("%s:%s", target.GetDestination(), target.GetTransport())
}

// helper function to start the syslog exporter
func (m *ExportMgr) startExporter(name string, ws []syslog.Writer) (events.Exporter, error) {
	exporterChLen := 1000
	syslogExporter, err := exporters.NewSyslogExporter(name, exporterChLen, ws, m.logger)
	if err != nil {
		return nil, err
	}

	// register the exporter
	eventsChan, offsetTracker, err := m.dispatcher.RegisterExporter(syslogExporter)
	if err != nil {
		return nil, err
	}

	// start the exporter
	syslogExporter.Start(eventsChan, offsetTracker)
	return syslogExporter, nil
}
