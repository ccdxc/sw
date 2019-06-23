package alertengine

import (
	evtsapi "github.com/pensando/sw/api/generated/events"
)

// Interface represents the alert engine responsible for triggering alerts
// based on the events and event based alert policies.
type Interface interface {
	// ProcessEvents processes the given events and generates any potential alerts
	ProcessEvents(*evtsapi.EventList)

	// SetMaintenanceMode sets the maintenance mode flag in alert engine
	SetMaintenanceMode()

	// UnsetMaintenanceMode unsets the maintenance mode flag if it is already set in alert engine
	UnsetMaintenanceMode()

	// Stop stops the alert engine
	Stop()
}
