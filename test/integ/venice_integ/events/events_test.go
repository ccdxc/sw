// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package events

import (
	"fmt"
	"sync"
	"testing"
	"time"

	uuid "github.com/satori/go.uuid"
	es "gopkg.in/olivere/elastic.v5"

	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

// This test tests the complete flow of an event from the recorder to elasticsearch
// (through events manager). This is the very similar flow of events from all the venice components.
//
// all the events go through below pipeline:
// events recorder -> events proxy (dispatcher, writer) -> events manager -> elasticsearch

var (
	eventType1     = "EVENT-TYPE1"
	eventType2     = "EVENT-TYPE2"
	eventType3     = "EVENT-TYPE2"
	testEventTypes = []string{eventType1, eventType2, eventType3}
)

// TestEvents tests events pipeline
// 1. record events using recorder.
// 2. verify the events reached elastic through the pipeline (proxy->writer->evtsmgr->elastic) using elastic client.
func TestEvents(t *testing.T) {
	ti := tInfo{}
	AssertOk(t, ti.setup(), "failed to setup test")
	defer ti.teardown()

	// uuid to make each source unique
	componentID := uuid.NewV4().String()
	testEventSource := &monitoring.EventSource{NodeName: "test-node", Component: componentID}
	// create recorder
	recorder, err := recorder.NewRecorder(testEventSource, testEventTypes, ti.evtsProxy.RPCServer.GetListenURL())
	AssertOk(t, err, "failed to create events recorder")

	// send events  (recorder -> proxy -> dispatcher -> writer -> evtsmgr -> elastic)
	err = recorder.Event(eventType1, "INFO", "test event - 1", nil)
	AssertOk(t, err, "failed to send event to the proxy")

	err = recorder.Event(eventType2, "INFO", "test event - 2", nil)
	AssertOk(t, err, "failed to send event to the proxy")

	// verify that it has reached elasticsearch; these are the first occurrences of an event
	// so it should have reached elasticsearch wthout being deduped.
	query := es.NewBoolQuery().Must(es.NewMatchQuery("source.component", componentID), es.NewTermQuery("type", eventType1))
	ti.assertElasticEvents(t, query, 1, "2s")
	query = es.NewBoolQuery().Must(es.NewMatchQuery("source.component", componentID), es.NewMatchQuery("message", "test event -2").Operator("and"))
	ti.assertElasticEvents(t, query, 1, "2s")

	// send duplicates and check whether they're compressed
	numDuplicates := 25
	for i := 0; i < numDuplicates; i++ {
		err = recorder.Event(eventType1, "INFO", "test dup event - 1", nil)
		AssertOk(t, err, "failed to send event to the proxy")

		err = recorder.Event(eventType2, "INFO", "test dup event - 2", nil)
		AssertOk(t, err, "failed to send event to the proxy")
	}

	// ensure the deduped events reached elasticsearch
	// test duplciate event - 1
	query = es.NewBoolQuery().Must(es.NewMatchQuery("source.component", componentID), es.NewMatchQuery("message", "test dup event - 1").Operator("and"))
	ti.assertElasticEvents(t, query, numDuplicates, "2s")

	// test duplicate event - 2
	query = es.NewBoolQuery().Must(es.NewMatchQuery("source.component", componentID), es.NewMatchQuery("message", "test dup event - 2").Operator("and"))
	ti.assertElasticEvents(t, query, numDuplicates, "2s")
}

// TestEventsProxyRestart tests the events flow with events proxy restart
// 1. record events using recorder.
// 2. restart events proxy.
//    - recorder -> events proxy becomes unavailable.
//    - events should be dropped at the recorder now (FIXME: they should be persisted at the recorder and replayed).
// 3. make sure none of the event is lost during restart.
// 4. verify that all the recorded events reached elastic using elastic client.
func TestEventsProxyRestart(t *testing.T) {
	ti := tInfo{}
	AssertOk(t, ti.setup(), "failed to setup test")
	defer ti.teardown()

	numRecorders := 3

	stopEventRecorders := make(chan struct{})
	wg := new(sync.WaitGroup)
	wg.Add(numRecorders + 1) // +1 for proxy restart go routine

	// uuid to make each source unique
	componentID := uuid.NewV4().String()
	totalEventsSentBySrc := make([]int, numRecorders)
	for i := 0; i < numRecorders; i++ {
		go func(i int) {
			testEventSource := &monitoring.EventSource{NodeName: "test-node", Component: fmt.Sprintf("%v-%v", componentID, i)}
			recorder, err := recorder.NewRecorder(testEventSource, testEventTypes, ti.evtsProxy.RPCServer.GetListenURL())
			if err != nil {
				log.Errorf("failed to create recorder for source %v", i)
				return
			}

			ticker := time.NewTicker(10 * time.Millisecond)
			for {
				select {
				case <-stopEventRecorders:
					wg.Done()
					return
				case <-ticker.C:
					if err = recorder.Event(eventType1, "INFO", "test event - 1", nil); err == nil {
						totalEventsSentBySrc[i]++
					}

					if err = recorder.Event(eventType2, "INFO", "test event - 2", nil); err == nil {
						totalEventsSentBySrc[i]++
					}

					if err = recorder.Event(eventType3, "CRITICAL", "test event - 3", nil); err == nil {
						totalEventsSentBySrc[i]++
					}
				}
			}
		}(i)
	}

	// restart events proxy
	go func() {
		proxyURL := ti.evtsProxy.RPCServer.GetListenURL()

		time.Sleep(1 * time.Second)
		ti.stopEvtsProxy()

		// proxy won't be able to accept any events for 2s
		time.Sleep(1 * time.Second)
		ti.startEvtsProxy(proxyURL)

		// let the recorders send some events after the proxy restart
		time.Sleep(1 * time.Second)

		// stop all the recorders
		close(stopEventRecorders)
		wg.Done()
	}()

	wg.Wait()

	// total events sent by all the recorders
	totalEventsSent := 0
	for _, val := range totalEventsSentBySrc {
		totalEventsSent += val
	}

	log.Infof("total events sent: %v", totalEventsSent)

	// total number of events received at elastic should match the total events sent
	// query all the events received from this source.component
	query := es.NewRegexpQuery("source.component", fmt.Sprintf("%v-.*", componentID))
	ti.assertElasticEvents(t, query, totalEventsSent, "5s")
}

// TestEventsMgrRestart tests the events flow with events manager restart
// 1. record events using recorder.
// 2. restart events manager.
//     - venice writer -> events manager becomes unavailable;
//     - events should be buffered at the writer and sent once the connection is established back again.
// 3. make sure none of the event is lost during restart (events can be lost only when the writer channel is full).
// 4. verify that all the recorded events reached elastic using elastic client.
func TestEventsMgrRestart(t *testing.T) {
	ti := tInfo{}
	AssertOk(t, ti.setup(), "failed to setup test")
	defer ti.teardown()

	numRecorders := 3

	stopEventRecorders := make(chan struct{})
	wg := new(sync.WaitGroup)
	wg.Add(numRecorders + 1) // +1 for events manager restart go routine

	// uuid to make each source unique
	componentID := uuid.NewV4().String()
	totalEventsSentBySrc := make([]int, numRecorders)
	for i := 0; i < numRecorders; i++ {
		go func(i int) {
			testEventSource := &monitoring.EventSource{NodeName: "test-node", Component: fmt.Sprintf("%v-%v", componentID, i)}
			recorder, err := recorder.NewRecorder(testEventSource, testEventTypes, ti.evtsProxy.RPCServer.GetListenURL())
			if err != nil {
				log.Errorf("failed to create recorder for source %v", i)
				return
			}

			ticker := time.NewTicker(10 * time.Millisecond)
			for {
				select {
				case <-stopEventRecorders:
					wg.Done()
					return
				case <-ticker.C:
					if err = recorder.Event(eventType1, "INFO", "test event - 1", nil); err == nil {
						totalEventsSentBySrc[i]++
					}

					if err = recorder.Event(eventType2, "INFO", "test event - 2", nil); err == nil {
						totalEventsSentBySrc[i]++
					}

					if err = recorder.Event(eventType3, "CRITICAL", "test event - 3", nil); err == nil {
						totalEventsSentBySrc[i]++
					}
				}
			}
		}(i)
	}

	// restart events manager
	go func() {
		evtsMgrURL := ti.evtsMgr.RPCServer.GetListenURL()

		time.Sleep(1 * time.Second)
		ti.stopEvtsMgr()

		// manager won't be able to accept any events for 2s; all the elastic writes will be denied
		// and all the events will be buffered at the writer for this time
		time.Sleep(1 * time.Second)

		// writers should be able to release all the holding events from the buffer
		ti.startEvtsMgr(evtsMgrURL)
		time.Sleep(1 * time.Second)

		// stop all the recorders
		close(stopEventRecorders)
		wg.Done()
	}()

	wg.Wait()

	// total events sent by all the recorders
	totalEventsSent := 0
	for _, val := range totalEventsSentBySrc {
		totalEventsSent += val
	}

	log.Infof("total events sent: %v", totalEventsSent)

	// total number of events received at elastic should match the total events sent
	// query all the events received from this source.component
	query := es.NewRegexpQuery("source.component", fmt.Sprintf("%v-.*", componentID))
	ti.assertElasticEvents(t, query, totalEventsSent, "5s")
}
