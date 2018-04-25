// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package rpcserver

import (
	"context"
	"fmt"
	"sync"
	"time"

	"github.com/pkg/errors"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/utils/elastic"
	"github.com/pensando/sw/venice/utils/log"
)

const indexType = "events"
const maxWatchersAllowed = 100

var (
	once sync.Once
	last string
)

// EvtsMgrRPCHandler handles all event RPC calls
type EvtsMgrRPCHandler struct {
	esClient    elastic.ESClient
	lastCreated lastCreatedIndex
}

// tracks the most recently created index
type lastCreatedIndex struct {
	sync.Mutex // for RW actions on the index
	index      string
}

// NewEvtsMgrRPCHandler returns a events RPC handler
func NewEvtsMgrRPCHandler(client elastic.ESClient) (*EvtsMgrRPCHandler, error) {
	evtsMgrRPCHandler := &EvtsMgrRPCHandler{
		esClient: client,
	}

	return evtsMgrRPCHandler, nil
}

// SendEvents sends the events to elastic by performing index or bulk operation.
// Once indexed, the same list of events is passed to the channel for further
// notification to all the watchers.
// Events library will gather events across tenants.
func (e *EvtsMgrRPCHandler) SendEvents(ctx context.Context, eventList *monitoring.EventsList) (*api.Empty, error) {
	events := eventList.Events
	if len(events) == 0 {
		return &api.Empty{}, nil
	}

	// TODO: Use globals.GetIndex() with tenant name
	index := e.getIndex()

	e.lastCreated.Lock()
	// create index only when it is different than the last created index
	if e.lastCreated.index != index {
		// FIXME: index mapping should be passed; we need to find a more generalized
		// mechanism than passing mapping strings.
		if err := e.esClient.CreateIndex(ctx, index, ""); err != nil && !elastic.IsIndexExists(err) {
			e.lastCreated.Unlock()
			return nil, err
		}

		e.lastCreated.index = index
	}
	e.lastCreated.Unlock()

	// index single event; it is costly to perform bulk operation for a single event (doc)
	if len(events) == 1 {
		event := events[0]
		if err := e.esClient.Index(ctx, index, indexType, event.GetUUID(), event); err != nil {
			log.Errorf("error sending event to elastic, err: %v", err)
			return nil, errors.Wrap(err, "error sending event to elastic")
		}
	} else {
		// index bulk events; construct bulk events request
		requests := make([]*elastic.BulkRequest, len(events))
		for i, evt := range events {
			requests[i] = &elastic.BulkRequest{
				RequestType: "index",
				IndexType:   indexType,
				ID:          evt.GetUUID(),
				Obj:         evt,

				// FIXME: there needs to be some intelligence here to find the exact index of an event.
				// It is possible for a single events (and it's update) to get indexed in 2 different indexes (2 days).
				// e.g. first event may get indexed on day 1 (at 23:59:30) then if the succeeding request at 00:00:00
				// carries an update for the same event, will get indexed on the next day.
				// But, Ideally it should be indexed on it's original index (day 1).
				Index: index,
			}
		}

		if bulkResp, err := e.esClient.Bulk(ctx, requests); err != nil {
			log.Errorf("error sending bulk events to elastic, err: %v", err)
			return nil, errors.Wrap(err, "error sending event to elastic")
		} else if len(bulkResp.Failed()) > 0 {
			log.Errorf("bulk operation failed on elastic")
			return nil, errors.New("bulk operation failed on elastic")
		}
	}

	return &api.Empty{}, nil
}

// getIndex returns the date string (YYYY-MM-DD) prefixed with venice.events
func (e *EvtsMgrRPCHandler) getIndex() string {
	currentTime := time.Now().Local()
	return fmt.Sprintf("%s.%s.%s", "venice", "events", currentTime.Format("2006-01-02"))
}
