package iotakit

import (
	"fmt"
	"time"

	"github.com/pensando/sw/api"
	evtsapi "github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
)

// EventsCollection is the collection of events
type EventsCollection struct {
	list evtsapi.EventList
	err  error
}

// Events returns all the events from the default tenant and namespace. Max of 1000 events will be returned
func (sm *SysModel) Events() *EventsCollection {
	eventsList, err := sm.tb.ListEvents(&api.ListWatchOptions{
		ObjectMeta: api.ObjectMeta{Tenant: globals.DefaultTenant, Namespace: globals.DefaultNamespace}})
	if err != nil {
		log.Errorf("failed to list events, err: %v", err)
		return &EventsCollection{err: err}
	}

	return &EventsCollection{list: eventsList}
}

// LinkUpEventsSince returns all the link up events since the given time
func (sm *SysModel) LinkUpEventsSince(since time.Time) *EventsCollection {
	fieldSelector := fmt.Sprintf("type=%s,meta.mod-time>=%v",
		halproto.PortOperStatus_PORT_OPER_STATUS_UP, since.Format(time.RFC3339Nano))
	eventsList, err := sm.tb.ListEvents(&api.ListWatchOptions{FieldSelector: fieldSelector})
	if err != nil {
		log.Errorf("failed to list events matching options: %v, err: %v", fieldSelector, err)
		return &EventsCollection{err: err}
	}

	return &EventsCollection{list: eventsList}
}

// LinkDownEventsSince returns all the link down events since the given time
func (sm *SysModel) LinkDownEventsSince(since time.Time) *EventsCollection {
	fieldSelector := fmt.Sprintf("type=%s,meta.mod-time>=%v",
		halproto.PortOperStatus_PORT_OPER_STATUS_DOWN, since.Format(time.RFC3339Nano))
	eventsList, err := sm.tb.ListEvents(&api.ListWatchOptions{FieldSelector: fieldSelector})
	if err != nil {
		log.Errorf("failed to list events matching options: %v, err: %v", fieldSelector, err)
		return &EventsCollection{err: err}
	}

	return &EventsCollection{list: eventsList}
}

// LenGreaterThanEqualTo return true when the events collection/list matches the expected len
func (ec *EventsCollection) LenGreaterThanEqualTo(expectedLen int) bool {
	return len(ec.list.Items) >= expectedLen
}

// Len returns true when the events collection/list matches the expected len
func (ec *EventsCollection) Len(expectedLen int) bool {
	return len(ec.list.Items) == expectedLen
}

// Error returns the error in collection
func (ec *EventsCollection) Error() error {
	return ec.err
}
