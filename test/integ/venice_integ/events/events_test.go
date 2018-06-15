// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package events

import (
	"fmt"
	"io/ioutil"
	"os"
	"sync"
	"testing"
	"time"

	uuid "github.com/satori/go.uuid"
	es "gopkg.in/olivere/elastic.v5"

	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/testutils/policygen"
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

	// create recorder events directory
	recorderEventsDir, err := ioutil.TempDir("", "")
	AssertOk(t, err, "failed to create recorder events directory")
	defer os.RemoveAll(recorderEventsDir)

	// create recorder
	evtsRecorder, err := recorder.NewRecorder(testEventSource, testEventTypes,
		ti.evtsProxy.RPCServer.GetListenURL(), recorderEventsDir)
	AssertOk(t, err, "failed to create events recorder")

	// send events  (recorder -> proxy -> dispatcher -> writer -> evtsmgr -> elastic)
	evtsRecorder.Event(eventType1, monitoring.SeverityLevel_INFO, "test event - 1", nil)
	evtsRecorder.Event(eventType2, monitoring.SeverityLevel_INFO, "test event - 2", nil)

	// verify that it has reached elasticsearch; these are the first occurrences of an event
	// so it should have reached elasticsearch wthout being deduped.
	query := es.NewBoolQuery().Must(es.NewMatchQuery("source.component", componentID), es.NewTermQuery("type", eventType1))
	ti.assertElasticUniqueEvents(t, query, true, 1, "4s") // unique == 1
	ti.assertElasticTotalEvents(t, query, true, 1, "4s")  // total  == 1
	query = es.NewBoolQuery().Must(es.NewMatchQuery("source.component", componentID), es.NewMatchQuery("message", "test event -2").Operator("and"))
	ti.assertElasticUniqueEvents(t, query, true, 1, "4s") // unique == 1
	ti.assertElasticTotalEvents(t, query, true, 1, "4s")  // total == 1

	// send duplicates and check whether they're compressed
	numDuplicates := 25
	for i := 0; i < numDuplicates; i++ {
		evtsRecorder.Event(eventType1, monitoring.SeverityLevel_INFO, "test dup event - 1", nil)
		evtsRecorder.Event(eventType2, monitoring.SeverityLevel_INFO, "test dup event - 2", nil)
	}

	// ensure the deduped events reached elasticsearch
	// test duplciate event - 1
	query = es.NewBoolQuery().Must(es.NewMatchQuery("source.component", componentID), es.NewMatchQuery("message", "test dup event - 1").Operator("and"))
	ti.assertElasticUniqueEvents(t, query, true, 1, "4s")            // unique == 1
	ti.assertElasticTotalEvents(t, query, true, numDuplicates, "2s") // total == numDuplicates

	// test duplicate event - 2
	query = es.NewBoolQuery().Must(es.NewMatchQuery("source.component", componentID), es.NewMatchQuery("message", "test dup event - 2").Operator("and"))
	ti.assertElasticUniqueEvents(t, query, true, 1, "4s")            // unique == 1
	ti.assertElasticTotalEvents(t, query, true, numDuplicates, "2s") // total == numDuplicates

	// create test NIC object
	testNIC := policygen.CreateSmartNIC("00-14-22-01-23-45",
		cluster.SmartNICSpec_ADMITTED.String(),
		"esx-1",
		&cluster.SmartNICCondition{
			Type:   cluster.SmartNICCondition_HEALTHY.String(),
			Status: cluster.ConditionStatus_FALSE.String(),
		})

	// record events with reference object
	for i := 0; i < numDuplicates; i++ {
		evtsRecorder.Event(eventType1, monitoring.SeverityLevel_INFO, "test dup event - 1", testNIC)
		evtsRecorder.Event(eventType2, monitoring.SeverityLevel_INFO, "test dup event - 2", testNIC)
	}

	// query by kind
	queryByKind := es.NewTermQuery("object-ref.kind", testNIC.GetKind())
	ti.assertElasticUniqueEvents(t, queryByKind, true, 2, "4s")              // unique == 2 (eventType1 and eventType2)
	ti.assertElasticTotalEvents(t, queryByKind, true, numDuplicates*2, "4s") // total == numDuplicates
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

	// create recorder events directory
	recorderEventsDir, err := ioutil.TempDir("", "")
	AssertOk(t, err, "failed to create recorder events directory")
	defer os.RemoveAll(recorderEventsDir)

	for i := 0; i < numRecorders; i++ {
		go func(i int) {
			testEventSource := &monitoring.EventSource{NodeName: "test-node", Component: fmt.Sprintf("%v-%v", componentID, i)}
			evtsRecorder, err := recorder.NewRecorder(testEventSource, testEventTypes,
				ti.evtsProxy.RPCServer.GetListenURL(), recorderEventsDir)
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
					evtsRecorder.Event(eventType1, monitoring.SeverityLevel_INFO, "test event - 1", nil)
					totalEventsSentBySrc[i]++

					evtsRecorder.Event(eventType2, monitoring.SeverityLevel_INFO, "test event - 2", nil)
					totalEventsSentBySrc[i]++

					evtsRecorder.Event(eventType3, monitoring.SeverityLevel_CRITICAL, "test event - 3", nil)
					totalEventsSentBySrc[i]++
				}
			}
		}(i)
	}

	// restart events proxy
	go func() {
		proxyURL := ti.evtsProxy.RPCServer.GetListenURL()

		// try restarting events proxy multiple times and make sure the events pipeline is intact
		// and the events are dlivered to elastic
		for i := 0; i < 3; i++ {
			time.Sleep(1 * time.Second)
			ti.stopEvtsProxy()

			// proxy won't be able to accept any events for 2s
			time.Sleep(1 * time.Second)
			ti.startEvtsProxy(proxyURL)
		}

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
	ti.assertElasticUniqueEvents(t, query, false, 3*numRecorders, "6s") // mininum of (3 event types * numRecorders = unique events)
	ti.assertElasticTotalEvents(t, query, false, totalEventsSent, "6s") // there can be duplicates because of proxy restarts; so check for received >= sent
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

	// create recorder events directory
	recorderEventsDir, err := ioutil.TempDir("", "")
	AssertOk(t, err, "failed to create recorder events directory")
	defer os.RemoveAll(recorderEventsDir)

	for i := 0; i < numRecorders; i++ {
		go func(i int) {
			testEventSource := &monitoring.EventSource{NodeName: "test-node", Component: fmt.Sprintf("%v-%v", componentID, i)}
			evtsRecorder, err := recorder.NewRecorder(testEventSource, testEventTypes,
				ti.evtsProxy.RPCServer.GetListenURL(), recorderEventsDir)
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
					evtsRecorder.Event(eventType1, monitoring.SeverityLevel_INFO, "test event - 1", nil)
					totalEventsSentBySrc[i]++

					evtsRecorder.Event(eventType2, monitoring.SeverityLevel_INFO, "test event - 2", nil)
					totalEventsSentBySrc[i]++

					evtsRecorder.Event(eventType3, monitoring.SeverityLevel_CRITICAL, "test event - 3", nil)
					totalEventsSentBySrc[i]++
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
	ti.assertElasticUniqueEvents(t, query, true, 3*numRecorders, "10s")
	ti.assertElasticTotalEvents(t, query, true, totalEventsSent, "10s")
}
