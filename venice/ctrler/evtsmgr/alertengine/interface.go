package alertengine

import (
	evtsapi "github.com/pensando/sw/api/generated/events"
)

// Interface represents the alert engine responsible for triggering alerts
// based on the events and event based alert policies.
type Interface interface {
	// ProcessEvents processes the given events and generates any potential alerts
	ProcessEvents(*evtsapi.EventList)

	// Stop stops the alert engine
	Stop()
}
