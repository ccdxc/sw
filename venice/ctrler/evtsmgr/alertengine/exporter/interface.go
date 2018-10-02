package exporter

import "github.com/pensando/sw/api/generated/monitoring"

// Exporter represents the entity to export alerts from venice to external 3rd party services. e.g. Syslog
type Exporter interface {
	// Export exports the given alert to external servers and returns the
	// total number of notifications sent
	Export(*monitoring.Alert) int

	// Close closes the connection with external servers
	Close()
}
