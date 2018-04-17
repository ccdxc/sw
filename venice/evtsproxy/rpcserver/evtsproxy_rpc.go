// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package rpcserver

import (
	"context"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/events"
)

// TODO:
// 1. Add dispatcher and writers.
// 2. Implement handlers.

// EvtsProxyRPCHandler handles all event proxy RPC calls
type EvtsProxyRPCHandler struct {
}

// NewEvtsProxyRPCHandler returns a events proxy RPC handler
func NewEvtsProxyRPCHandler() (*EvtsProxyRPCHandler, error) {
	evtsProxyRPCHandler := &EvtsProxyRPCHandler{}

	// TODO: dispatcher and writers

	return evtsProxyRPCHandler, nil
}

// ForwardEvent forwards the given event to the dispatcher.
func (e *EvtsProxyRPCHandler) ForwardEvent(ctx context.Context, event *events.Event) (*api.Empty, error) {
	// TODO: call dispatcher action
	// e.dispatcher.Action(*event)
	return &api.Empty{}, nil
}

// ForwardEvents forwards the given list of events to the dispatcher.
func (e *EvtsProxyRPCHandler) ForwardEvents(ctx context.Context, events *events.EventList) (*api.Empty, error) {
	// TODO: call dispatcher action
	// for each event  {
	// e.dispatcher.Action(*event)
	// }

	return &api.Empty{}, nil
}
