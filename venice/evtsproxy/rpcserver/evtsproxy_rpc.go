// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package rpcserver

import (
	"context"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/utils/events"
	"github.com/pensando/sw/venice/utils/log"
)

// EvtsProxyRPCHandler handles all event proxy RPC calls
type EvtsProxyRPCHandler struct {
	dispatcher events.Dispatcher
}

// NewEvtsProxyRPCHandler returns a events proxy RPC handler
func NewEvtsProxyRPCHandler(evtsDispatcher events.Dispatcher) (*EvtsProxyRPCHandler, error) {
	return &EvtsProxyRPCHandler{
		dispatcher: evtsDispatcher,
	}, nil
}

// ForwardEvent forwards the given event to the dispatcher.
func (e *EvtsProxyRPCHandler) ForwardEvent(ctx context.Context, event *monitoring.Event) (*api.Empty, error) {
	err := e.dispatcher.Action(*event)
	if err != nil {
		log.Errorf("failed to forward event {%s} from the proxy, err: %v", event.GetUUID(), err)
	}

	return &api.Empty{}, err
}

// ForwardEvents forwards the given list of events to the dispatcher.
func (e *EvtsProxyRPCHandler) ForwardEvents(ctx context.Context, events *monitoring.EventsList) (*api.Empty, error) {
	for _, event := range events.GetEvents() {
		temp := *event
		if err := e.dispatcher.Action(temp); err != nil {
			log.Errorf("failed to forward event {%s} from the proxy, err: %v", temp.GetUUID(), err)
		}
	}

	return &api.Empty{}, nil
}

// Stop stops/closes all the underlying connections with rest of venice components
func (e *EvtsProxyRPCHandler) Stop() {
	e.dispatcher.Shutdown() // closes all the event channels and offset trackers; as the writers the writers will be closed
}
