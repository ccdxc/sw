// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package types

import (
	"github.com/pensando/sw/api"
)

// ClusterMetrics is the metrics type for the Cluster object.
type ClusterMetrics struct {
	AdmittedNICs       api.Gauge
	PendingNICs        api.Gauge
	RejectedNICs       api.Gauge
	DecommissionedNICs api.Gauge
	HealthyNICs        api.Gauge
	UnhealthyNICs      api.Gauge
	DisconnectedNICs   api.Gauge
}

// GetSmartNICMetricsZeroMap returns a map containing all SmartNIC-related ClusterMetrics fields
// with values initialized to zero.
func GetSmartNICMetricsZeroMap() map[string]int64 {
	return map[string]int64{
		"AdmittedNICs":       0,
		"PendingNICs":        0,
		"RejectedNICs":       0,
		"DecommissionedNICs": 0,
		"HealthyNICs":        0,
		"UnhealthyNICs":      0,
		"DisconnectedNICs":   0,
	}
}
