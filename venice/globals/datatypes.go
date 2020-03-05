// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package globals

// DataType is type of various objects & data in the system
type DataType uint16

const (

	// Configs object type
	Configs DataType = iota

	// Alerts object type
	Alerts

	// Events record type
	Events

	// AuditLogs type
	AuditLogs

	// DebugLogs type
	DebugLogs

	// Stats type
	Stats

	// FwLogs represents raw Firewall logs
	FwLogs

	// FwLogsObjects represents Firewall log Objects (csv or json zipped files) stored in object store
	FwLogsObjects
)
