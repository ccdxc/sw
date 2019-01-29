package exporters

import (
	"github.com/pensando/sw/venice/utils/syslog"
)

// SyslogExporterConfig represents the config required for creating syslog events exporter
type SyslogExporterConfig struct {
	Name    string
	Writers []syslog.Writer
}

// VeniceExporterConfig represents the config required for creating venice events exporter
type VeniceExporterConfig struct {
	EvtsMgrURL string
}
