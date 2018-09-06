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
	evtsapi "github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/venice/apiserver"
	types "github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/ctrler/evtsmgr"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/elastic"
	mockes "github.com/pensando/sw/venice/utils/elastic/mock/server"
	"github.com/pensando/sw/venice/utils/events"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/log"
	mockresolver "github.com/pensando/sw/venice/utils/resolver/mock"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/testutils/serviceutils"
	// import API server services
	_ "github.com/pensando/sw/api/generated/monitoring/grpc/server"
)

var (
	veniceBufferLen = 30
	testServerURL   = "localhost:0"

	indexType   = elastic.GetDocType(globals.Events)
	sortByField = ""
	sortAsc     = true

	logger = log.GetNewLogger(log.GetDefaultConfig("venice_writer_test"))
	mr     = mockresolver.New() // create mock resolver

	// create events recorder
	_, _ = recorder.NewRecorder(&recorder.Config{
		Source:        &evtsapi.EventSource{NodeName: utils.GetHostname(), Component: "venice_writer_test"},
		EvtTypes:      evtsapi.GetEventTypes(),
		BackupDir:     "/tmp",
		SkipEvtsProxy: true})
)

// This file tests venice writer using events manager -> mock elasticsearch

// adds the given service to mock resolver
func addMockService(mr *mockresolver.ResolverClient, serviceName, serviceURL string) {
	mr.AddServiceInstance(&types.ServiceInstance{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceInstance",
		},
		ObjectMeta: api.ObjectMeta{
			Name: serviceName,
		},
		Service: serviceName,
		URL:     serviceURL,
	})
}

// setup creates events manager service, venice writer and elastic client
func setup(t *testing.T) (*mockes.ElasticServer, apiserver.Server, *evtsmgr.EventsManager,
	events.Writer, elastic.ESClient) {
	// create elastic mock server
	ms := mockes.NewElasticServer()
	ms.Start()

	// start API server
	apiServer, apiServerURL, err := serviceutils.StartAPIServer("", logger)
	AssertOk(t, err, "failed to start API server")

	// update resolver
	addMockService(mr, globals.ElasticSearch, ms.GetElasticURL())
	addMockService(mr, globals.APIServer, apiServerURL)

	// run gRPC events manager server
	evtsMgr, err := evtsmgr.NewEventsManager(globals.EvtsMgr, testServerURL, mr, logger)
	AssertOk(t, err, "failed to run gRPC events manager server")

	// create venice writer
	veniceWriter, err := NewVeniceWriter("venice_writer", veniceBufferLen, evtsMgr.RPCServer.GetListenURL(), nil, logger)
	AssertOk(t, err, "failed to create venice events writer")

	// create elastic client; this is used to confirm the events have reached elasticsearch through events manager
	elasticClient, err := elastic.NewClient(ms.GetElasticURL(), nil, logger)
	AssertOk(t, err, "failed to create elastic client")

	return ms, apiServer, evtsMgr, veniceWriter, elasticClient
}

// TestVeniceEventsWriter tests venice writer
func TestVeniceEventsWriter(t *testing.T) {
	mockElasticServer, apiServer, evtsMgrServer, veniceWriter, elasticClient := setup(t)
	defer apiServer.Stop()
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
		mockEventsChan.result <- newMockBatch([]*evtsapi.Event{&temp}, 0)
	}

	// make sure the events reached elasticsearch through events manager
	AssertEventually(t,
		func() (bool, interface{}) {
			searchStr := "TEST-1"
			resp, err := elasticClient.Search(context.Background(), elastic.GetIndex(globals.Events, dummyEvt.GetTenant()),
				indexType, es.NewRawStringQuery(fmt.Sprintf("{\"match_all\":\"%s\"}", searchStr)), nil, 0, 10, sortByField, sortAsc)
			if err != nil {
				return false, fmt.Sprintf("failed to find events matching {%s}, err: %v", searchStr, err)
			}

			if resp.TotalHits() != 10 {
				return false, fmt.Sprintf("expected: %d number of events, got: %d", 10, resp.TotalHits())
			}

			return true, nil
		}, "failed to perform search on mock elastic server", "20ms", "20s")

	// send few more events and check
	for i := 0; i < 15; i++ {
		temp := *dummyEvt
		temp.EventAttributes.Type = "TEST-2"
		temp.ObjectMeta.UUID = uuid.New().String()

		// send events to mock writer
		mockEventsChan.result <- newMockBatch([]*evtsapi.Event{&temp}, 0)
	}

	// make sure the events reached elasticsearch through events manager
	AssertEventually(t,
		func() (bool, interface{}) {
			searchStr := "TEST-2"
			resp, err := elasticClient.Search(context.Background(), elastic.GetIndex(globals.Events, dummyEvt.GetTenant()),
				indexType, es.NewRawStringQuery(fmt.Sprintf("{\"match_all\":\"%s\"}", searchStr)), nil, 0, 10, sortByField, sortAsc)
			if err != nil {
				return false, fmt.Sprintf("failed to find events matching {%s}, err: %v", searchStr, err)
			}

			if resp.TotalHits() != 15 {
				return false, fmt.Sprintf("expected: %d number of events, got: %d", 15, resp.TotalHits())
			}

			return true, nil
		}, "failed to perform search on mock elastic server", "20ms", "20s")

	// try sending more than 1 event at a time
	var evts []*evtsapi.Event
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
				indexType, es.NewRawStringQuery(fmt.Sprintf("{\"match_all\":\"%s\"}", searchStr)), nil, 0, 10, sortByField, sortAsc)
			if err != nil {
				return false, fmt.Sprintf("failed to find events matching {%s}, err: %v", searchStr, err)
			}

			if resp.TotalHits() != 10 {
				return false, fmt.Sprintf("expected: %d number of events, got: %d", 10, resp.TotalHits())
			}

			return true, nil
		}, "failed to perform search on mock elastic server", "20ms", "4s")
}

// Te stVeniceWriterWithEvtsMgrRestart tests the venice writer with events manager restart
func TestVeniceWriterWithEvtsMgrRestart(t *testing.T) {
	mockElasticServer, apiServer, evtsMgrServer, veniceWriter, elasticClient := setup(t)
	defer apiServer.Stop()
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
				mockEventsChan.result <- newMockBatch([]*evtsapi.Event{&temp}, 0)
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
				nil, 0, 10000, sortByField, sortAsc)
			if err == nil {
				var evt evtsapi.Event
				for _, res := range resp.Hits.Hits {
					_ = json.Unmarshal(*res.Source, &evt)
					totalEventsReceived += int(evt.GetCount())
				}

				if totalEventsSent == totalEventsReceived {
					return true, nil
				}
			}

			return false, fmt.Sprintf("expected: %d, got: %v", totalEventsSent, totalEventsReceived)
		}, "elasticsearch did not receive all the events recorded by the recorders", "20ms", "20s")
}
