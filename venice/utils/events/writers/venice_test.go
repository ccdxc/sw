// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package writers

import (
	"context"
	"encoding/json"
	"fmt"
	"sync"
	"testing"
	"time"

	"github.com/google/uuid"
	es "gopkg.in/olivere/elastic.v5"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	types "github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/ctrler/evtsmgr"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/elastic"
	mockes "github.com/pensando/sw/venice/utils/elastic/mock/server"
	"github.com/pensando/sw/venice/utils/events"
	"github.com/pensando/sw/venice/utils/log"
	mockresolver "github.com/pensando/sw/venice/utils/resolver/mock"
	. "github.com/pensando/sw/venice/utils/testutils"
)

var (
	veniceBufferLen = 30
	testServerURL   = "localhost:0"

	indexType = elastic.GetDocType(globals.Events)
	sortBy    = ""

	logger = log.GetNewLogger(log.GetDefaultConfig("venice_writer_test"))
	mr     = mockresolver.New() // create mock resolver
)

// This file tests venice writer using events manager -> mock elasticsearch

// setup creates events manager service, venice writer and elastic client
func setup(t *testing.T) (*mockes.ElasticServer, *evtsmgr.EventsManager,
	events.Writer, elastic.ESClient) {
	// create elastic mock server
	ms := mockes.NewElasticServer()
	ms.Start()

	// add mock elastic service to mock resolver
	mr.AddServiceInstance(&types.ServiceInstance{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceInstance",
		},
		ObjectMeta: api.ObjectMeta{
			Name: globals.ElasticSearch,
		},
		Service: globals.ElasticSearch,
		URL:     ms.GetElasticURL(),
	})

	// run gRPC events manager server
	evtsMgr, err := evtsmgr.NewEventsManager(globals.EvtsMgr, testServerURL, mr, logger)
	AssertOk(t, err, "failed to run gRPC events manager server")
	testServerURL = evtsMgr.RPCServer.GetListenURL()

	// create venice writer
	veniceWriter, err := NewVeniceWriter("venice_writer", veniceBufferLen, evtsMgr.RPCServer.GetListenURL(), logger)
	AssertOk(t, err, "failed to create venice events writer")

	// create elastic client; this is used to confirm the events have reached elasticsearch through events manager
	elasticClient, err := elastic.NewClient(ms.GetElasticURL(), nil, logger)
	AssertOk(t, err, "failed to create elastic client")

	return ms, evtsMgr, veniceWriter, elasticClient
}

// TestVeniceEventsWriter tests venice writer
func TestVeniceEventsWriter(t *testing.T) {
	mockElasticServer, evtsMgrServer, veniceWriter, elasticClient := setup(t)
	defer mockElasticServer.Stop()
	defer evtsMgrServer.RPCServer.Stop()

	// create mock events channel; in the real case, it will come from the dispatcher
	mockEventsChan := &mockEventChanImpl{
		result:  make(chan events.Batch, veniceBufferLen),
		stopped: make(chan struct{}),
	}

	mockOffsetTracker := &mockOffsetTrackerImpl{}

	// start the writer
	veniceWriter.Start(mockEventsChan, mockOffsetTracker)
	defer veniceWriter.Stop()

	// send some events to the venice writer
	for i := 0; i < 10; i++ {
		temp := *dummyEvt
		temp.EventAttributes.Type = "TEST-1"
		temp.ObjectMeta.UUID = uuid.New().String()

		// send events to mock writer
		mockEventsChan.result <- newMockBatch([]*monitoring.Event{&temp}, 0)
	}

	// make sure the events reached elasticsearch through events manager
	AssertEventually(t,
		func() (bool, interface{}) {
			searchStr := "TEST-1"
			resp, err := elasticClient.Search(context.Background(), elastic.GetIndex(globals.Events, dummyEvt.GetTenant()),
				indexType, es.NewRawStringQuery(fmt.Sprintf("{\"match_all\":\"%s\"}", searchStr)), nil, 0, 10, sortBy)
			if err != nil {
				return false, fmt.Sprintf("failed to find events matching {%s}, err: %v", searchStr, err)
			}

			if resp.TotalHits() != 10 {
				return false, fmt.Sprintf("expected: %d number of events, got: %d", 10, resp.TotalHits())
			}

			return true, nil
		}, "failed to perform search on mock elastic server", "20ms", "2s")

	// send few more events and check
	for i := 0; i < 15; i++ {
		temp := *dummyEvt
		temp.EventAttributes.Type = "TEST-2"
		temp.ObjectMeta.UUID = uuid.New().String()

		// send events to mock writer
		mockEventsChan.result <- newMockBatch([]*monitoring.Event{&temp}, 0)
	}

	// make sure the events reached elasticsearch through events manager
	AssertEventually(t,
		func() (bool, interface{}) {
			searchStr := "TEST-2"
			resp, err := elasticClient.Search(context.Background(), elastic.GetIndex(globals.Events, dummyEvt.GetTenant()),
				indexType, es.NewRawStringQuery(fmt.Sprintf("{\"match_all\":\"%s\"}", searchStr)), nil, 0, 10, sortBy)
			if err != nil {
				return false, fmt.Sprintf("failed to find events matching {%s}, err: %v", searchStr, err)
			}

			if resp.TotalHits() != 15 {
				return false, fmt.Sprintf("expected: %d number of events, got: %d", 15, resp.TotalHits())
			}

			return true, nil
		}, "failed to perform search on mock elastic server", "20ms", "2s")

	// try sending more than 1 event at a time
	var evts []*monitoring.Event
	for i := 0; i < 10; i++ {
		temp := *dummyEvt
		temp.EventAttributes.Type = "TEST-3"
		temp.ObjectMeta.UUID = uuid.New().String()
		evts = append(evts, &temp)
	}
	mockEventsChan.result <- newMockBatch(evts, 0)

	// make sure the events reached elasticsearch through events manager
	AssertEventually(t,
		func() (bool, interface{}) {
			searchStr := "TEST-3"
			resp, err := elasticClient.Search(context.Background(), elastic.GetIndex(globals.Events, dummyEvt.GetTenant()),
				indexType, es.NewRawStringQuery(fmt.Sprintf("{\"match_all\":\"%s\"}", searchStr)), nil, 0, 10, sortBy)
			if err != nil {
				return false, fmt.Sprintf("failed to find events matching {%s}, err: %v", searchStr, err)
			}

			if resp.TotalHits() != 10 {
				return false, fmt.Sprintf("expected: %d number of events, got: %d", 10, resp.TotalHits())
			}

			return true, nil
		}, "failed to perform search on mock elastic server", "20ms", "4s")
}

// TestVeniceWriterWithEvtsMgrRestart tests the venice writer with events manager restart
func TestVeniceWriterWithEvtsMgrRestart(t *testing.T) {
	mockElasticServer, evtsMgrServer, veniceWriter, elasticClient := setup(t)
	defer mockElasticServer.Stop()
	defer evtsMgrServer.RPCServer.Stop()

	// create mock events channel; in the real case, it will come from the dispatcher
	mockEventsChan := &mockEventChanImpl{
		result:  make(chan events.Batch, veniceBufferLen),
		stopped: make(chan struct{}),
	}

	mockOffsetTracker := &mockOffsetTrackerImpl{}

	// start the writer
	veniceWriter.Start(mockEventsChan, mockOffsetTracker)
	defer veniceWriter.Stop()

	wg := new(sync.WaitGroup)
	wg.Add(2)
	stopSendingEvents := make(chan struct{})
	totalEventsSent := 0

	// start sending events to the writer
	go func() {
		defer wg.Done()
		ticker := time.NewTicker(10 * time.Millisecond)
		for {
			select {
			case <-stopSendingEvents:
				return
			case <-ticker.C:
				temp := *dummyEvt
				temp.EventAttributes.Type = t.Name()
				temp.ObjectMeta.UUID = uuid.New().String()

				// send events to mock writer
				mockEventsChan.result <- newMockBatch([]*monitoring.Event{&temp}, 0)
				totalEventsSent++
			}
		}
	}()

	// restart events manager
	go func() {
		var err error
		defer wg.Done()

		evtsMgrURL := evtsMgrServer.RPCServer.GetListenURL()

		time.Sleep(2 * time.Second)
		evtsMgrServer.RPCServer.Stop()

		// manager won't be able to accept any events for 2s; all the elastic writes will be denied
		// and all the events will be buffered at the writer for this time
		time.Sleep(1 * time.Second)

		// writers should be able to release all the holding events from the buffer
		// run gRPC events manager server
		if evtsMgrServer, err = evtsmgr.NewEventsManager(globals.EvtsMgr, evtsMgrURL, mr, logger); err != nil {
			log.Errorf("failed to start events manager on URL: %s, err: %v", evtsMgrURL, err)
			return
		}

		// let the producer send some more events
		time.Sleep(2 * time.Second)

		// stop all the recorders
		close(stopSendingEvents)
	}()

	wg.Wait()

	// total number of events received at elastic should match the total events sent
	AssertEventually(t,
		func() (bool, interface{}) {
			totalEventsReceived := 0

			resp, err := elasticClient.Search(context.Background(),
				elastic.GetIndex(globals.Events, globals.DefaultTenant),
				indexType,
				es.NewRawStringQuery(fmt.Sprintf("{\"match_all\":\"%s\"}", t.Name())),
				nil, 0, 10000, sortBy)
			if err == nil {
				var evt monitoring.Event
				for _, res := range resp.Hits.Hits {
					_ = json.Unmarshal(*res.Source, &evt)
					totalEventsReceived += int(evt.GetCount())
				}

				if totalEventsSent == totalEventsReceived {
					return true, nil
				}
			}

			return false, fmt.Sprintf("expected: %d, got: %v", totalEventsSent, totalEventsReceived)
		}, "elasticsearch did not receive all the events recorded by the recorders", "20ms", "2s")
}
