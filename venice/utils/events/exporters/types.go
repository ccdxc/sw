package exporters

import (
	"github.com/pensando/sw/venice/utils/syslog"
)

// Type represents different exporter types (venice, syslog, etc.)
type Type uint

const (
	// Venice represents the venice events exporter
	Venice Type = 0

	// Syslog represents the exporter exporting to external syslog server(s)
	Syslog Type = 1
)

// String returns the string name of the exporter
func (w Type) String() string {
	switch w {
	case Venice:
		return "venice"
	}

	return ""
}

// SyslogExporterConfig represents the config required for creating syslog events exporter
type SyslogExporterConfig struct {
	Name    string
	Writers []syslog.Writer
}

// VeniceExporterConfig represents the config required for creating venice events exporter
type VeniceExporterConfig struct {
	EvtsMgrURL string
}
