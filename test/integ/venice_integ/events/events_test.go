// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package events

import (
	"fmt"
	"io/ioutil"
	"net/http"
	"os"
	"regexp"
	"sync"
	"testing"
	"time"

	uuid "github.com/satori/go.uuid"
	es "gopkg.in/olivere/elastic.v5"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/cluster"
	evtsapi "github.com/pensando/sw/api/generated/events"
	_ "github.com/pensando/sw/api/generated/exports/apigw"
	_ "github.com/pensando/sw/api/generated/exports/apiserver"
	_ "github.com/pensando/sw/api/hooks/apiserver"
	testutils "github.com/pensando/sw/test/utils"
	_ "github.com/pensando/sw/venice/apigw/svc"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/spyglass/finder"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
	"github.com/pensando/sw/venice/utils/runtime"
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
	eventType3     = "EVENT-TYPE3"
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
	testEventSource := &evtsapi.EventSource{NodeName: "test-node", Component: componentID}

	// create recorder events directory
	recorderEventsDir, err := ioutil.TempDir("", "")
	AssertOk(t, err, "failed to create recorder events directory")
	defer os.RemoveAll(recorderEventsDir)

	// create recorder
	evtsRecorder, err := recorder.NewRecorder(&recorder.Config{
		Source:       testEventSource,
		EvtTypes:     testEventTypes,
		EvtsProxyURL: ti.evtsProxy.RPCServer.GetListenURL(),
		BackupDir:    recorderEventsDir})
	AssertOk(t, err, "failed to create events recorder")

	// send events  (recorder -> proxy -> dispatcher -> writer -> evtsmgr -> elastic)
	evtsRecorder.Event(eventType1, evtsapi.SeverityLevel_INFO, "test event - 1", nil)
	evtsRecorder.Event(eventType2, evtsapi.SeverityLevel_INFO, "test event - 2", nil)

	// verify that it has reached elasticsearch; these are the first occurrences of an event
	// so it should have reached elasticsearch wthout being deduped.
	query := es.NewBoolQuery().Must(es.NewMatchQuery("source.component", componentID), es.NewTermQuery("type.keyword", eventType1))
	ti.assertElasticUniqueEvents(t, query, true, 1, "4s") // unique == 1
	ti.assertElasticTotalEvents(t, query, true, 1, "4s")  // total  == 1
	query = es.NewBoolQuery().Must(es.NewMatchQuery("source.component", componentID), es.NewMatchQuery("message", "test event -2").Operator("and"))
	ti.assertElasticUniqueEvents(t, query, true, 1, "4s") // unique == 1
	ti.assertElasticTotalEvents(t, query, true, 1, "4s")  // total == 1

	// send duplicates and check whether they're compressed
	numDuplicates := 25
	for i := 0; i < numDuplicates; i++ {
		evtsRecorder.Event(eventType1, evtsapi.SeverityLevel_INFO, "test dup event - 1", nil)
		evtsRecorder.Event(eventType2, evtsapi.SeverityLevel_INFO, "test dup event - 2", nil)
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
		evtsRecorder.Event(eventType1, evtsapi.SeverityLevel_INFO, "test dup event - 1", testNIC)
		evtsRecorder.Event(eventType2, evtsapi.SeverityLevel_INFO, "test dup event - 2", testNIC)
	}

	// query by kind
	queryByKind := es.NewTermQuery("object-ref.kind.keyword", testNIC.GetKind())
	ti.assertElasticUniqueEvents(t, queryByKind, true, 2, "4s")              // unique == 2 (eventType1 and eventType2)
	ti.assertElasticTotalEvents(t, queryByKind, true, numDuplicates*2, "4s") // total == numDuplicates
}

// TestEventsProxyRestart tests the events flow with events proxy restart
// 1. record events using recorder.
// 2. restart events proxy.
//    - recorder -> events proxy becomes unavailable.
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
			testEventSource := &evtsapi.EventSource{NodeName: "test-node", Component: fmt.Sprintf("%v-%v", componentID, i)}
			evtsRecorder, err := recorder.NewRecorder(&recorder.Config{
				Source:       testEventSource,
				EvtTypes:     testEventTypes,
				EvtsProxyURL: ti.evtsProxy.RPCServer.GetListenURL(),
				BackupDir:    recorderEventsDir})
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
					evtsRecorder.Event(eventType1, evtsapi.SeverityLevel_INFO, "test event - 1", nil)
					totalEventsSentBySrc[i]++

					evtsRecorder.Event(eventType2, evtsapi.SeverityLevel_INFO, "test event - 2", nil)
					totalEventsSentBySrc[i]++

					evtsRecorder.Event(eventType3, evtsapi.SeverityLevel_CRITICAL, "test event - 3", nil)
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
			ti.evtsProxy.RPCServer.Stop()

			// proxy won't be able to accept any events for 2s
			time.Sleep(1 * time.Second)
			testutils.StartEvtsProxy(proxyURL, ti.mockResolver, ti.logger)
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
	query := es.NewRegexpQuery("source.component.keyword", fmt.Sprintf("%v-.*", componentID))
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
			testEventSource := &evtsapi.EventSource{NodeName: "test-node", Component: fmt.Sprintf("%v-%v", componentID, i)}
			evtsRecorder, err := recorder.NewRecorder(&recorder.Config{
				Source:       testEventSource,
				EvtTypes:     testEventTypes,
				EvtsProxyURL: ti.evtsProxy.RPCServer.GetListenURL(),
				BackupDir:    recorderEventsDir})
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
					evtsRecorder.Event(eventType1, evtsapi.SeverityLevel_INFO, "test event - 1", nil)
					totalEventsSentBySrc[i]++

					evtsRecorder.Event(eventType2, evtsapi.SeverityLevel_INFO, "test event - 2", nil)
					totalEventsSentBySrc[i]++

					evtsRecorder.Event(eventType3, evtsapi.SeverityLevel_CRITICAL, "test event - 3", nil)
					totalEventsSentBySrc[i]++
				}
			}
		}(i)
	}

	// restart events manager
	go func() {
		evtsMgrURL := ti.evtsMgr.RPCServer.GetListenURL()

		time.Sleep(1 * time.Second)
		ti.evtsMgr.RPCServer.Stop()

		// manager won't be able to accept any events for 2s; all the elastic writes will be denied
		// and all the events will be buffered at the writer for this time
		time.Sleep(1 * time.Second)

		// writers should be able to release all the holding events from the buffer
		testutils.StartEvtsMgr(evtsMgrURL, ti.mockResolver, ti.logger)
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
	query := es.NewRegexpQuery("source.component.keyword", fmt.Sprintf("%v-.*", componentID))
	ti.assertElasticUniqueEvents(t, query, true, 3*numRecorders, "10s")
	ti.assertElasticTotalEvents(t, query, true, totalEventsSent, "10s")
}

// TestEventsRESTEndpoints tests GET /events and /event/{UUID} endpoint
// 1. test GET /events endpoint with varying requests
// 2. fetch few UUIDs from the /events response to test /event/{UUID}
// 3. test /event/{UUID} endpoint
func TestEventsRESTEndpoints(t *testing.T) {
	timeNow := time.Now()

	// setup events pipeline to record and distribute events
	ti := tInfo{}
	AssertOk(t, ti.setup(), "failed to setup test")
	defer ti.teardown()

	// start spyglass (backend service for events)
	fdrTemp, fdrAddr, err := testutils.StartSpyglass("finder", "", ti.mockResolver, ti.logger)
	AssertOk(t, err, "failed to start spyglass finder")
	fdr := fdrTemp.(finder.Interface)
	defer fdr.Stop()

	// API server, is needed for the authentication service
	apiServer, apiServerAddr, err := testutils.StartAPIServer(":0", &store.Config{
		Type:    store.KVStoreTypeMemkv,
		Codec:   runtime.NewJSONCodec(runtime.GetDefaultScheme()),
		Servers: []string{"test-cluster"},
	}, ti.logger)
	AssertOk(t, err, "failed to start API server")
	defer apiServer.Stop()

	// API gateway
	apiGw, apiGwAddr, err := testutils.StartAPIGateway(":0",
		map[string]string{globals.APIServer: apiServerAddr, globals.Spyglass: fdrAddr}, []string{}, []string{}, ti.logger)
	AssertOk(t, err, "failed to start API gateway")
	defer apiGw.Stop()

	// setup authn and get authz token
	userCreds := &auth.PasswordCredential{Username: testutils.TestLocalUser, Password: testutils.TestLocalPassword, Tenant: testutils.TestTenant}
	err = testutils.SetupAuth(apiServerAddr, true, false, userCreds, ti.logger)
	AssertOk(t, err, "failed to setup authN service")
	authzHeader, err := testutils.GetAuthorizationHeader(apiGwAddr, userCreds)
	AssertOk(t, err, "failed to get authZ header")

	// define list of events to be recorded
	recordEvents := []struct {
		eventType string
		severity  evtsapi.SeverityLevel
		message   string
		objRef    interface{}
	}{
		{eventType1, evtsapi.SeverityLevel_INFO, fmt.Sprintf("%s-%s", eventType1, evtsapi.SeverityLevel_INFO), nil},
		{eventType1, evtsapi.SeverityLevel_WARNING, fmt.Sprintf("%s-%s", eventType1, evtsapi.SeverityLevel_WARNING), nil},
		{eventType1, evtsapi.SeverityLevel_CRITICAL, fmt.Sprintf("%s-%s", eventType1, evtsapi.SeverityLevel_CRITICAL), nil},

		{eventType2, evtsapi.SeverityLevel_INFO, fmt.Sprintf("%s-%s", eventType2, evtsapi.SeverityLevel_INFO), nil},
		{eventType2, evtsapi.SeverityLevel_WARNING, fmt.Sprintf("%s-%s", eventType2, evtsapi.SeverityLevel_WARNING), nil},
		{eventType2, evtsapi.SeverityLevel_CRITICAL, fmt.Sprintf("%s-%s", eventType2, evtsapi.SeverityLevel_CRITICAL), nil},

		{eventType3, evtsapi.SeverityLevel_INFO, fmt.Sprintf("%s-%s", eventType3, evtsapi.SeverityLevel_INFO), nil},
		{eventType3, evtsapi.SeverityLevel_WARNING, fmt.Sprintf("%s-%s", eventType3, evtsapi.SeverityLevel_WARNING), nil},
		{eventType3, evtsapi.SeverityLevel_CRITICAL, fmt.Sprintf("%s-%s", eventType3, evtsapi.SeverityLevel_CRITICAL), nil},
	}

	wg := new(sync.WaitGroup)
	wg.Add(1)

	// start recorder
	recorderEventsDir, err := ioutil.TempDir("", "")
	AssertOk(t, err, "failed to create recorder events directory")
	defer os.RemoveAll(recorderEventsDir)
	go func() {
		defer wg.Done()

		testEventSource := &evtsapi.EventSource{NodeName: "test-node", Component: uuid.NewV4().String()}
		evtsRecorder, err := recorder.NewRecorder(&recorder.Config{
			Source:       testEventSource,
			EvtTypes:     testEventTypes,
			EvtsProxyURL: ti.evtsProxy.RPCServer.GetListenURL(),
			BackupDir:    recorderEventsDir})
		if err != nil {
			log.Errorf("failed to create recorder")
			return
		}

		// record events
		for _, evt := range recordEvents {
			evtsRecorder.Event(evt.eventType, evt.severity, evt.message, evt.objRef)
		}
	}()

	// total number of event by UUID requests to execute; UUIDs will be captured while executing
	// any of the /events requests and /event/{UUID} testcase is added accordingly.
	totalEventByUUIDRequests := make([]bool, 0, 3)

	// define TCs (request and responses)
	type expectedResponse struct {
		numEvents int
		events    map[string]*evtsapi.Event
	}
	type tc struct {
		requestURI    string
		requestBody   *api.ListWatchOptions
		expStatusCode int
		expResponse   *expectedResponse
	}

	validTCs := []*tc{
		{ // GET all events; should match 9 events
			requestURI:    "events",
			requestBody:   &api.ListWatchOptions{}, // default max-results to 1000
			expStatusCode: http.StatusOK,
			expResponse: &expectedResponse{
				numEvents: 9,
				events: map[string]*evtsapi.Event{
					fmt.Sprintf("%s-%s", eventType1, evtsapi.SeverityLevel_INFO): &evtsapi.Event{
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventType1,
							Severity: evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_INFO)],
							Count:    1,
						},
					},
					fmt.Sprintf("%s-%s", eventType1, evtsapi.SeverityLevel_WARNING): &evtsapi.Event{
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventType1,
							Severity: evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_WARNING)],
							Count:    1,
						},
					},
					fmt.Sprintf("%s-%s", eventType1, evtsapi.SeverityLevel_CRITICAL): &evtsapi.Event{
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventType1,
							Severity: evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_CRITICAL)],
							Count:    1,
						},
					},
					fmt.Sprintf("%s-%s", eventType2, evtsapi.SeverityLevel_INFO): &evtsapi.Event{
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventType2,
							Severity: evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_INFO)],
							Count:    1,
						},
					},
					fmt.Sprintf("%s-%s", eventType2, evtsapi.SeverityLevel_WARNING): &evtsapi.Event{
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventType2,
							Severity: evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_WARNING)],
							Count:    1,
						},
					},
					fmt.Sprintf("%s-%s", eventType2, evtsapi.SeverityLevel_CRITICAL): &evtsapi.Event{
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventType2,
							Severity: evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_CRITICAL)],
							Count:    1,
						},
					},
					fmt.Sprintf("%s-%s", eventType3, evtsapi.SeverityLevel_INFO): &evtsapi.Event{
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventType3,
							Severity: evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_INFO)],
							Count:    1,
						},
					},
					fmt.Sprintf("%s-%s", eventType3, evtsapi.SeverityLevel_WARNING): &evtsapi.Event{
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventType3,
							Severity: evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_WARNING)],
							Count:    1,
						},
					},
					fmt.Sprintf("%s-%s", eventType3, evtsapi.SeverityLevel_CRITICAL): &evtsapi.Event{
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventType3,
							Severity: evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_CRITICAL)],
							Count:    1,
						},
					},
				},
			},
		},
		{ // GET events with severity = INFO; should match 3 events
			requestURI:    "events",
			requestBody:   &api.ListWatchOptions{FieldSelector: fmt.Sprintf("severity=%s", evtsapi.SeverityLevel_INFO), MaxResults: 100},
			expStatusCode: http.StatusOK,
			expResponse: &expectedResponse{
				numEvents: 3,
				events: map[string]*evtsapi.Event{
					fmt.Sprintf("%s-%s", eventType1, evtsapi.SeverityLevel_INFO): &evtsapi.Event{
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventType1,
							Severity: evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_INFO)],
							Count:    1,
						},
					},
					fmt.Sprintf("%s-%s", eventType2, evtsapi.SeverityLevel_INFO): &evtsapi.Event{
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventType2,
							Severity: evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_INFO)],
							Count:    1,
						},
					},
					fmt.Sprintf("%s-%s", eventType3, evtsapi.SeverityLevel_INFO): &evtsapi.Event{
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventType3,
							Severity: evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_INFO)],
							Count:    1,
						},
					}},
			},
		},
		{ // GET events with severity = "CIRITCAL" and source.node-name="test-node"; should match 3 events
			requestURI:    "events",
			requestBody:   &api.ListWatchOptions{FieldSelector: fmt.Sprintf("severity=%s,source.node-name=%s", evtsapi.SeverityLevel_CRITICAL, "test-node"), MaxResults: 100},
			expStatusCode: http.StatusOK,
			expResponse: &expectedResponse{
				numEvents: 3,
				events: map[string]*evtsapi.Event{
					fmt.Sprintf("%s-%s", eventType1, evtsapi.SeverityLevel_CRITICAL): &evtsapi.Event{
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventType1,
							Severity: evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_CRITICAL)],
							Count:    1,
						},
					},
					fmt.Sprintf("%s-%s", eventType2, evtsapi.SeverityLevel_CRITICAL): &evtsapi.Event{
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventType2,
							Severity: evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_CRITICAL)],
							Count:    1,
						},
					},
					fmt.Sprintf("%s-%s", eventType3, evtsapi.SeverityLevel_CRITICAL): &evtsapi.Event{
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventType3,
							Severity: evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_CRITICAL)],
							Count:    1,
						},
					},
				},
			},
		},
		{ // Get events with severity = "CRITICAL" and type in ("EVENT-TYPE2"); should match only one event
			requestURI:    "events",
			requestBody:   &api.ListWatchOptions{FieldSelector: fmt.Sprintf("severity=%s,type in (%s)", evtsapi.SeverityLevel_CRITICAL, eventType2), MaxResults: 100},
			expStatusCode: http.StatusOK,
			expResponse: &expectedResponse{
				numEvents: 1,
				events: map[string]*evtsapi.Event{
					fmt.Sprintf("%s-%s", eventType2, evtsapi.SeverityLevel_CRITICAL): &evtsapi.Event{
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventType2,
							Severity: evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_CRITICAL)],
							Count:    1,
						},
					},
				},
			},
		},
		{ // GET events from timeNow to time.Now()+ 100s; should match ALL(9) events
			requestURI:    "events",
			requestBody:   &api.ListWatchOptions{FieldSelector: fmt.Sprintf("meta.creation-time>=%v,meta.creation-time<=%v", timeNow.Format(time.RFC3339Nano), time.Now().Add(100*time.Second).Format(time.RFC3339Nano)), MaxResults: 100},
			expStatusCode: http.StatusOK,
			expResponse: &expectedResponse{
				numEvents: 9,
				events: map[string]*evtsapi.Event{
					fmt.Sprintf("%s-%s", eventType1, evtsapi.SeverityLevel_INFO): &evtsapi.Event{
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventType1,
							Severity: evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_INFO)],
							Count:    1,
						},
					},
					fmt.Sprintf("%s-%s", eventType1, evtsapi.SeverityLevel_WARNING): &evtsapi.Event{
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventType1,
							Severity: evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_WARNING)],
							Count:    1,
						},
					},
					fmt.Sprintf("%s-%s", eventType1, evtsapi.SeverityLevel_CRITICAL): &evtsapi.Event{
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventType1,
							Severity: evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_CRITICAL)],
							Count:    1,
						},
					},
					fmt.Sprintf("%s-%s", eventType2, evtsapi.SeverityLevel_INFO): &evtsapi.Event{
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventType2,
							Severity: evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_INFO)],
							Count:    1,
						},
					},
					fmt.Sprintf("%s-%s", eventType2, evtsapi.SeverityLevel_WARNING): &evtsapi.Event{
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventType2,
							Severity: evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_WARNING)],
							Count:    1,
						},
					},
					fmt.Sprintf("%s-%s", eventType2, evtsapi.SeverityLevel_CRITICAL): &evtsapi.Event{
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventType2,
							Severity: evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_CRITICAL)],
							Count:    1,
						},
					},
					fmt.Sprintf("%s-%s", eventType3, evtsapi.SeverityLevel_INFO): &evtsapi.Event{
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventType3,
							Severity: evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_INFO)],
							Count:    1,
						},
					},
					fmt.Sprintf("%s-%s", eventType3, evtsapi.SeverityLevel_WARNING): &evtsapi.Event{
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventType3,
							Severity: evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_WARNING)],
							Count:    1,
						},
					},
					fmt.Sprintf("%s-%s", eventType3, evtsapi.SeverityLevel_CRITICAL): &evtsapi.Event{
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventType3,
							Severity: evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_CRITICAL)],
							Count:    1,
						},
					},
				},
			},
		},
		{ // Get events with creation-time>timeNow
			requestURI:    "events",
			requestBody:   &api.ListWatchOptions{FieldSelector: fmt.Sprintf("severity=%s,type in (%s),meta.creation-time>%v", evtsapi.SeverityLevel_CRITICAL, eventType2, timeNow.Format(time.RFC3339Nano)), MaxResults: 100},
			expStatusCode: http.StatusOK,
			expResponse: &expectedResponse{
				numEvents: 1,
				events: map[string]*evtsapi.Event{
					fmt.Sprintf("%s-%s", eventType2, evtsapi.SeverityLevel_CRITICAL): &evtsapi.Event{
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventType2,
							Severity: evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_CRITICAL)],
							Count:    1,
						},
					},
				},
			},
		},
		{ // Get events with modified-time>timeNow
			requestURI:    "events",
			requestBody:   &api.ListWatchOptions{FieldSelector: fmt.Sprintf("severity=%s,type in (%s),meta.mod-time>%v", evtsapi.SeverityLevel_CRITICAL, eventType2, timeNow.Format(time.RFC3339Nano)), MaxResults: 100},
			expStatusCode: http.StatusOK,
			expResponse: &expectedResponse{
				numEvents: 1,
				events: map[string]*evtsapi.Event{
					fmt.Sprintf("%s-%s", eventType2, evtsapi.SeverityLevel_CRITICAL): &evtsapi.Event{
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventType2,
							Severity: evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_CRITICAL)],
							Count:    1,
						},
					},
				},
			},
		},
		{ // Get events with creation-time<=timeNow
			requestURI:    "events",
			requestBody:   &api.ListWatchOptions{FieldSelector: fmt.Sprintf("severity=%s,type in (%s),meta.creation-time<=%v", evtsapi.SeverityLevel_CRITICAL, eventType2, timeNow.Format(time.RFC3339Nano)), MaxResults: 100},
			expStatusCode: http.StatusOK,
			expResponse: &expectedResponse{
				numEvents: 0,
				events:    map[string]*evtsapi.Event{},
			},
		},
		{ // Get events with type in (EVENT-TYPE1,EVENT-TYPE2),source.node-name notin (test-node); should match none
			requestURI:    "events",
			requestBody:   &api.ListWatchOptions{FieldSelector: fmt.Sprintf("type in (%s,%s),source.node-name notin (%s)", eventType1, eventType2, "test-node"), MaxResults: 100},
			expStatusCode: http.StatusOK,
			expResponse: &expectedResponse{
				numEvents: 0,
				events:    map[string]*evtsapi.Event{},
			},
		},
		{ // GET events with severity="TEST"; should match none
			requestURI:    "events",
			requestBody:   &api.ListWatchOptions{FieldSelector: "severity=TEST", MaxResults: 100},
			expStatusCode: http.StatusOK,
			expResponse: &expectedResponse{
				numEvents: 0,
				events:    map[string]*evtsapi.Event{},
			},
		},
		{ // GET events with source.component="test" and type= "test"; should match none
			requestURI:    "events",
			requestBody:   &api.ListWatchOptions{FieldSelector: "source.component=test,type=test", MaxResults: 100},
			expStatusCode: http.StatusOK,
			expResponse: &expectedResponse{
				numEvents: 0,
				events:    map[string]*evtsapi.Event{},
			},
		},
		{ // Get events with invalid field names
			requestURI:    "events",
			requestBody:   &api.ListWatchOptions{FieldSelector: fmt.Sprintf("invalid-field=%s,type in (%s)", evtsapi.SeverityLevel_CRITICAL, eventType2), MaxResults: 100},
			expStatusCode: http.StatusInternalServerError,
			expResponse: &expectedResponse{
				numEvents: 0,
				events:    map[string]*evtsapi.Event{},
			},
		},
		{ // Get events with invalid field names
			requestURI:    "events",
			requestBody:   &api.ListWatchOptions{FieldSelector: fmt.Sprintf("meta.invalid<=%v", timeNow.Format(time.RFC3339Nano)), MaxResults: 100},
			expStatusCode: http.StatusInternalServerError,
			expResponse: &expectedResponse{
				numEvents: 0,
				events:    map[string]*evtsapi.Event{},
			},
		},
	}

	// add room for /event/{UUID} test cases
	for i := 0; i < cap(totalEventByUUIDRequests); i++ {
		validTCs = append(validTCs, &tc{})
	}

	// perform request and check the response
	for _, rr := range validTCs {
		log.Infof("executing TC: %v, %v", rr.requestURI, rr.requestBody)
		switch {
		case regexp.MustCompile("events").MatchString(rr.requestURI):
			url := fmt.Sprintf("http://%s/events/v1/%s", apiGwAddr, rr.requestURI)
			resp := evtsapi.EventList{}
			httpClient := netutils.NewHTTPClient()
			httpClient.SetHeader("Authorization", authzHeader)

			// both GET and POST should behave the same
			for _, reqMethod := range []string{"GET", "POST"} {
				AssertEventually(t,
					func() (bool, interface{}) {
						statusCode, _ := httpClient.Req(reqMethod, url, rr.requestBody, &resp)
						if statusCode != rr.expStatusCode || len(resp.GetItems()) != rr.expResponse.numEvents {
							return false, fmt.Sprintf("failed to get expected events for %v", rr.requestBody)
						}

						return true, nil
					}, "failed to get events", "20ms", "6s")

				Assert(t, rr.expResponse.numEvents == len(resp.GetItems()), "failed to get expected number of events")

				// verity resp against the expected response
				for _, obtainedEvt := range resp.GetItems() {
					expectedEvt, ok := rr.expResponse.events[obtainedEvt.GetMessage()]
					Assert(t, ok, "obtained event is not in the expected list: %s", obtainedEvt.GetMessage())
					Assert(t, expectedEvt.GetType() == obtainedEvt.GetType(), "expected event message: %s, got: %s", expectedEvt.GetType(), obtainedEvt.GetType())
					Assert(t, expectedEvt.GetSeverity() == obtainedEvt.GetSeverity(), "expected event severity: %s, got: %s", expectedEvt.GetSeverity(), obtainedEvt.GetSeverity())
					Assert(t, expectedEvt.GetCount() == obtainedEvt.GetCount(), "expected event count: %d, got: %d", expectedEvt.GetCount(), obtainedEvt.GetCount())

					// make sure self-link works
					selfLink := obtainedEvt.GetSelfLink()
					evt := evtsapi.Event{}
					statusCode, err := httpClient.Req("GET", fmt.Sprintf("http://%s/%s", apiGwAddr, selfLink), nil, &evt)
					Assert(t, err == nil && statusCode == http.StatusOK, "failed to get the event using self-link: %v, status: %v, err: %v", selfLink, statusCode, err)
					Assert(t, evt.GetUUID() == obtainedEvt.GetUUID(), "obtained: %v, expected: %v", evt.GetUUID(), obtainedEvt.GetUUID())

					// update the TC's request to test /event/{UUID}
					if len(totalEventByUUIDRequests) < cap(totalEventByUUIDRequests) {
						validTCs[len(validTCs)-len(totalEventByUUIDRequests)-1].requestURI = fmt.Sprintf("event/%s", obtainedEvt.GetUUID())
						totalEventByUUIDRequests = append(totalEventByUUIDRequests, true)
					}
				}
			}
		case regexp.MustCompile("event/*").MatchString(rr.requestURI):
			url := fmt.Sprintf("http://%s/events/v1/%s", apiGwAddr, rr.requestURI)
			resp := evtsapi.Event{}
			httpClient := netutils.NewHTTPClient()
			httpClient.SetHeader("Authorization", authzHeader)

			AssertEventually(t,
				func() (bool, interface{}) {
					statusCode, err := httpClient.Req("GET", url, rr.requestBody, &resp)
					if err != nil || statusCode != http.StatusOK {
						return false, nil
					}

					return true, nil
				}, "failed to get events", "20ms", "6s")

			Assert(t, resp.GetUUID() != "", "failed to get event by UUID: %v", rr.requestURI)

			// make sure self-link works
			selfLink := resp.GetSelfLink()
			evt := evtsapi.Event{}
			statusCode, err := httpClient.Req("GET", fmt.Sprintf("http://%s/%s", apiGwAddr, selfLink), nil, &evt)
			Assert(t, err == nil && statusCode == http.StatusOK, "failed to get the event using self-link: %v, status: %v, err: %v", selfLink, statusCode, err)
			Assert(t, evt.GetUUID() == resp.GetUUID(), "obtained: %v, expected: %v", evt.GetUUID(), resp.GetUUID())
		}
	}

	// wait for the recorder to complete
	wg.Wait()
}
