package exporter

import (
	"fmt"
	"strings"
	"sync"

	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/events/generated/eventattrs"
	"github.com/pensando/sw/venice/utils"
	aeutils "github.com/pensando/sw/venice/utils/alertengine"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/syslog"
)

// TODO: add retry

// syslogExport is a syslog exporter to export alerts to syslog servers (both local and remote).
type syslogExport struct {
	sync.Mutex
	writers []syslog.Writer // list of syslog writers
	logger  log.Logger      // logger
}

// NewSyslogExporter creates a new syslog exporter using the given syslog server configs.
func NewSyslogExporter(syslogExpConfig *monitoring.SyslogExport, logger log.Logger) (Exporter, error) {
	e := &syslogExport{writers: []syslog.Writer{}, logger: logger}

	var err error
	if e.writers, err = e.createWriters(syslogExpConfig); err != nil {
		return nil, err
	}

	return e, nil
}

// Export exports the alert using syslog writers to the underlying syslog servers.
func (s *syslogExport) Export(alert *monitoring.Alert) int {
	var totalNotificationsSent int
	var err error
	sMsg := aeutils.GenerateSyslogMessage(alert)

	s.Lock()
	defer s.Unlock()
	for _, writer := range s.writers {
		switch eventattrs.Severity(eventattrs.Severity_vvalue[alert.Status.GetSeverity()]) {
		case eventattrs.Severity_INFO:
			err = writer.Info(sMsg)
		case eventattrs.Severity_WARN:
			err = writer.Warning(sMsg)
		case eventattrs.Severity_CRITICAL:
			err = writer.Crit(sMsg)
		}

		if err != nil {
			s.logger.Errorf("failed to write syslog message: %v, err: %v", sMsg, err)
		} else {
			totalNotificationsSent++
		}
	}

	return totalNotificationsSent
}

// Close closes the syslog writer which in turn terminates the underlying connection with syslog servers.
func (s *syslogExport) Close() {
	s.Lock()
	defer s.Unlock()
	for _, writer := range s.writers {
		writer.Close()
	}
}

// helper function to create syslog writer by connecting with the given syslog server config.
func (s *syslogExport) createWriters(syslogExpConfig *monitoring.SyslogExport) ([]syslog.Writer, error) {
	if _, ok := monitoring.MonitoringExportFormat_vvalue[syslogExpConfig.GetFormat()]; !ok {
		return nil, fmt.Errorf("invalid syslog format: %v", syslogExpConfig.GetFormat())
	}

	priority := syslog.LogUser // is a combination of facility and priority
	tag := "pen-events"        // a.k.a prefix tag
	if !utils.IsEmpty(syslogExpConfig.GetConfig().GetFacilityOverride()) {
		priority = syslog.Priority(monitoring.SyslogFacility_vvalue[syslogExpConfig.GetConfig().GetFacilityOverride()])
	}
	if !utils.IsEmpty(syslogExpConfig.GetConfig().GetPrefix()) {
		tag = syslogExpConfig.GetConfig().GetPrefix()
	}

	var writers []syslog.Writer
	var err error

	for _, target := range syslogExpConfig.GetTargets() {
		var writer syslog.Writer
		network, remoteAddr := s.getSyslogServerInfo(target)
		switch monitoring.MonitoringExportFormat(monitoring.MonitoringExportFormat_vvalue[syslogExpConfig.GetFormat()]) {
		case monitoring.MonitoringExportFormat_SYSLOG_BSD:
			writer, err = syslog.NewBsd(network, remoteAddr, priority, utils.GetHostname(), tag)
			if err != nil {
				s.logger.Errorf("failed to create syslog BSD writer, err: %v", err)
				return nil, err
			}
		case monitoring.MonitoringExportFormat_SYSLOG_RFC5424:
			writer, err = syslog.NewRfc5424(network, remoteAddr, priority, utils.GetHostname(), tag)
			if err != nil {
				s.logger.Errorf("failed to create syslog RFC5424 writer, err: %v", err)
				return nil, err
			}
		default:
			return nil, fmt.Errorf("syslog format not supported: %v", syslogExpConfig.GetFormat())
		}

		writers = append(writers, writer)
	}

	return writers, nil
}

// helper function to parse network(tcp) and remote address(127.0.0.1:514) from the export config.
// e.g exportConfig {
//   transport: "tcp/514",
//   destination: "10.30.4.34",
// }
// should be returned as `tcp` and `10.30.4.34:514`
func (s *syslogExport) getSyslogServerInfo(cfg *monitoring.ExportConfig) (string, string) {
	remoteAddr := cfg.GetDestination() // 10.30.4.34
	network := ""                      // tcp, udp, etc.

	tmp := strings.Split(cfg.GetTransport(), "/") // tcp/514
	if len(tmp) > 0 {
		network = tmp[0]
	}
	if len(tmp) > 1 {
		remoteAddr += ":" + tmp[1]
	}

	return strings.ToLower(network), remoteAddr
}
