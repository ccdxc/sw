package finder

import (
	"context"

	"github.com/pensando/sw/api"
	evtsapi "github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/venice/utils/log"
)

// This file implements EventsV1Server; spyglass will use these handlers to serve /events requests.

// GetEvent retuns the event identified by given UUID
func (fdr *Finder) GetEvent(ctx context.Context, t *evtsapi.GetEventRequest) (*evtsapi.Event, error) {
	log.Debugf("get event by UUID: %v", t)
	// TODO: yet to implement
	return &evtsapi.Event{}, nil
}

// GetEvents return the list of events matching the given listOptions
func (fdr *Finder) GetEvents(ctx context.Context, t *api.ListWatchOptions) (*evtsapi.EventList, error) {
	log.Debugf("get events with list options: %v", t)
	// TODO: yet to implement
	return &evtsapi.EventList{}, nil
}
