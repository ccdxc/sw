// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package events

import (
	"context"
	"crypto/tls"
	"fmt"
	"io/ioutil"
	"net/http"
	"os"
	"regexp"
	"strings"
	"sync"
	"testing"
	"time"

	es "github.com/olivere/elastic"
	"github.com/satori/go.uuid"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/client"
	"github.com/pensando/sw/api/fields"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/cluster"
	evtsapi "github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/api/generated/monitoring"
	testutils "github.com/pensando/sw/test/utils"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/spyglass/finder"
	. "github.com/pensando/sw/venice/utils/authn/testutils"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
	"github.com/pensando/sw/venice/utils/syslog"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/testutils/policygen"
	"github.com/pensando/sw/venice/utils/testutils/serviceutils"

	// import gateway services
	_ "github.com/pensando/sw/api/generated/auth/gateway"
	_ "github.com/pensando/sw/api/generated/events/gateway"
	_ "github.com/pensando/sw/api/generated/monitoring/gateway"
	_ "github.com/pensando/sw/api/generated/search/gateway"
	_ "github.com/pensando/sw/venice/apigw/svc"

	// import API server services
	_ "github.com/pensando/sw/api/generated/auth/grpc/server"
	_ "github.com/pensando/sw/api/generated/events/grpc/server"
	_ "github.com/pensando/sw/api/generated/monitoring/grpc/server"
	_ "github.com/pensando/sw/api/generated/search/grpc/server"
	_ "github.com/pensando/sw/api/hooks/apiserver"
)

// This test tests the complete flow of an event from the recorder to elasticsearch
// (through events manager). This is the very similar flow of events from all the venice components.
//
// all the events go through below pipeline:
// events recorder -> events proxy (dispatcher, writer) -> events manager -> elasticsearch

var (
	eventType1     = "DUMMYEVENT-1"
	eventType2     = "DUMMYEVENT-2"
	eventType3     = "DUMMYEVENT-3"
	testEventTypes = []string{eventType1, eventType2, eventType3}
)

// TestEvents tests events pipeline
// 1. record events using recorder.
// 2. verify the events reached elastic through the pipeline (proxy->writer->evtsmgr->elastic) using elastic client.
func TestEvents(t *testing.T) {
	ti := tInfo{}
	AssertOk(t, ti.setup(t), "failed to setup test")
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
	// so it should have reached elasticsearch without being de-duped.
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

	// ensure the de-duped events reached elasticsearch
	// test duplicate event - 1
	query = es.NewBoolQuery().Must(es.NewMatchQuery("source.component", componentID), es.NewMatchQuery("message", "test dup event - 1").Operator("and"))
	ti.assertElasticUniqueEvents(t, query, true, 1, "4s")            // unique == 1
	ti.assertElasticTotalEvents(t, query, true, numDuplicates, "2s") // total == numDuplicates

	// test duplicate event - 2
	query = es.NewBoolQuery().Must(es.NewMatchQuery("source.component", componentID), es.NewMatchQuery("message", "test dup event - 2").Operator("and"))
	ti.assertElasticUniqueEvents(t, query, true, 1, "4s")            // unique == 1
	ti.assertElasticTotalEvents(t, query, true, numDuplicates, "2s") // total == numDuplicates

	// create test NIC object
	testNIC := policygen.CreateSmartNIC("00-14-22-01-23-45",
		cluster.SmartNICStatus_ADMITTED.String(),
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
	AssertOk(t, ti.setup(t), "failed to setup test")
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
			ti.evtsProxy.Stop()

			// proxy won't be able to accept any events for 2s
			time.Sleep(1 * time.Second)
			evtsProxy, evtsProxyURL, tmpProxyDir, err := testutils.StartEvtsProxy(proxyURL, ti.mockResolver, ti.logger, ti.dedupInterval, ti.batchInterval)
			if err != nil {
				log.Errorf("failed to start events proxy, err: %v", err)
				continue
			}
			ti.evtsProxy = evtsProxy
			ti.proxyEventsStoreDir = tmpProxyDir
			ti.updateResolver(globals.EvtsProxy, evtsProxyURL)
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
	ti.assertElasticUniqueEvents(t, query, false, 3*numRecorders, "60s") // minimum of (3 event types * numRecorders = unique events)
	ti.assertElasticTotalEvents(t, query, false, totalEventsSent, "60s") // there can be duplicates because of proxy restarts; so check for received >= sent
}

// TestEventsMgrRestart tests the events flow with events manager restart
// 1. record events using recorder.
// 2. restart events manager multiple times.
//     - venice writer -> events manager becomes unavailable;
//     - events should be buffered at the writer and sent once the connection is established back again.
// 3. make sure none of the event is lost during restart (events can be lost only when the writer channel is full).
// 4. verify that all the recorded events reached elastic using elastic client.
func TestEventsMgrRestart(t *testing.T) {
	ti := tInfo{}
	AssertOk(t, ti.setup(t), "failed to setup test")
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

		for i := 0; i < 3; i++ {
			time.Sleep(1 * time.Second)
			ti.evtsMgr.Stop()

			// manager won't be able to accept any events for 1s; all the elastic writes will be denied
			// and all the events will be buffered at the writer for this time
			time.Sleep(1 * time.Second)

			// writers should be able to release all the holding events from the buffer
			evtsMgr, _, err := testutils.StartEvtsMgr(evtsMgrURL, ti.mockResolver, ti.logger, ti.esClient)
			AssertOk(t, err, "failed to start events manager, err: %v", err)
			ti.evtsMgr = evtsMgr
		}

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
	ti.assertElasticUniqueEvents(t, query, true, 3*numRecorders, "60s")
	ti.assertElasticTotalEvents(t, query, true, totalEventsSent, "60s")
}

// TestEventsRESTEndpoints tests GET /events and /events/{UUID} endpoint
// 1. test GET /events endpoint with varying requests
// 2. fetch few UUIDs from the /events response to test /events/{UUID}
// 3. test /events/{UUID} endpoint
func TestEventsRESTEndpoints(t *testing.T) {
	timeNow := time.Now()

	// setup events pipeline to record and distribute events
	ti := tInfo{}
	AssertOk(t, ti.setup(t), "failed to setup test")
	defer ti.teardown()

	// start spyglass (backend service for events)
	fdrTemp, fdrAddr, err := testutils.StartSpyglass("finder", "", ti.mockResolver, nil, ti.logger, ti.esClient)
	AssertOk(t, err, "failed to start spyglass finder")
	fdr := fdrTemp.(finder.Interface)
	defer fdr.Stop()

	// API gateway
	apiGw, apiGwAddr, err := testutils.StartAPIGateway(":0", false,
		map[string]string{globals.APIServer: ti.apiServerAddr, globals.Spyglass: fdrAddr}, []string{"metrics_query"}, []string{}, ti.logger)
	AssertOk(t, err, "failed to start API gateway")
	defer apiGw.Stop()

	// setup authn and get authz token
	userCreds := &auth.PasswordCredential{Username: testutils.TestLocalUser, Password: testutils.TestLocalPassword, Tenant: testutils.TestTenant}
	err = testutils.SetupAuth(ti.apiServerAddr, true, &auth.Ldap{Enabled: false}, &auth.Radius{Enabled: false}, userCreds, ti.logger)
	AssertOk(t, err, "failed to setup authN service")
	defer testutils.CleanupAuth(ti.apiServerAddr, true, false, userCreds, ti.logger)
	authzHeader, err := testutils.GetAuthorizationHeader(apiGwAddr, userCreds)
	AssertOk(t, err, "failed to get authZ header")
	const testTenant = "testtenant"
	MustCreateTenant(ti.apicl, testTenant)
	defer MustDeleteTenant(ti.apicl, testTenant)
	MustCreateTestUser(ti.apicl, testutils.TestLocalUser, testutils.TestLocalPassword, testTenant)
	defer MustDeleteUser(ti.apicl, testutils.TestLocalUser, testTenant)
	MustCreateRoleBinding(ti.apicl, "AdminRoleBinding", testTenant, globals.AdminRole, []string{testutils.TestLocalUser}, nil)
	defer MustDeleteRoleBinding(ti.apicl, "AdminRoleBinding", testTenant)
	testtenantAuthzHeader, err := testutils.GetAuthorizationHeader(apiGwAddr, &auth.PasswordCredential{Username: testutils.TestLocalUser, Password: testutils.TestLocalPassword, Tenant: testTenant})
	AssertOk(t, err, fmt.Sprintf("failed to get authZ header for user (%s|%s)", testTenant, testutils.TestLocalUser))

	dummyObjRef := &auth.User{
		TypeMeta: api.TypeMeta{
			Kind: string(auth.KindUser),
		},
		ObjectMeta: api.ObjectMeta{
			Tenant:    testTenant,
			Namespace: globals.DefaultNamespace,
			Name:      "testUser",
		},
	}
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

		// events in non default tenant
		{eventType3, evtsapi.SeverityLevel_INFO, fmt.Sprintf("%s-%s", eventType3, evtsapi.SeverityLevel_INFO), dummyObjRef},
		{eventType3, evtsapi.SeverityLevel_WARNING, fmt.Sprintf("%s-%s", eventType3, evtsapi.SeverityLevel_WARNING), dummyObjRef},
		{eventType3, evtsapi.SeverityLevel_CRITICAL, fmt.Sprintf("%s-%s", eventType3, evtsapi.SeverityLevel_CRITICAL), dummyObjRef},
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
	// any of the /events requests and /events/{UUID} testcase is added accordingly.
	totalEventByUUIDRequests := make([]bool, 0, 3)

	// define TCs (request and responses)
	type expectedResponse struct {
		numEvents int
		events    map[string]*evtsapi.Event
	}
	type tc struct {
		name          string
		authzHdr      string
		requestURI    string
		requestBody   *api.ListWatchOptions
		expStatusCode int
		expResponse   *expectedResponse
	}

	validTCs := []*tc{
		{ // GET all events; should match 9 events
			name:          "GET all events for default tenant; should match 9 events",
			authzHdr:      authzHeader,
			requestURI:    "events",
			requestBody:   &api.ListWatchOptions{}, // default max-results to 1000
			expStatusCode: http.StatusOK,
			expResponse: &expectedResponse{
				numEvents: 9,
				events: map[string]*evtsapi.Event{
					fmt.Sprintf("%s-%s", eventType1, evtsapi.SeverityLevel_INFO): {
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventType1,
							Severity: evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_INFO)],
							Count:    1,
						},
					},
					fmt.Sprintf("%s-%s", eventType1, evtsapi.SeverityLevel_WARNING): {
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventType1,
							Severity: evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_WARNING)],
							Count:    1,
						},
					},
					fmt.Sprintf("%s-%s", eventType1, evtsapi.SeverityLevel_CRITICAL): {
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventType1,
							Severity: evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_CRITICAL)],
							Count:    1,
						},
					},
					fmt.Sprintf("%s-%s", eventType2, evtsapi.SeverityLevel_INFO): {
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventType2,
							Severity: evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_INFO)],
							Count:    1,
						},
					},
					fmt.Sprintf("%s-%s", eventType2, evtsapi.SeverityLevel_WARNING): {
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventType2,
							Severity: evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_WARNING)],
							Count:    1,
						},
					},
					fmt.Sprintf("%s-%s", eventType2, evtsapi.SeverityLevel_CRITICAL): {
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventType2,
							Severity: evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_CRITICAL)],
							Count:    1,
						},
					},
					fmt.Sprintf("%s-%s", eventType3, evtsapi.SeverityLevel_INFO): {
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventType3,
							Severity: evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_INFO)],
							Count:    1,
						},
					},
					fmt.Sprintf("%s-%s", eventType3, evtsapi.SeverityLevel_WARNING): {
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventType3,
							Severity: evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_WARNING)],
							Count:    1,
						},
					},
					fmt.Sprintf("%s-%s", eventType3, evtsapi.SeverityLevel_CRITICAL): {
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
			name:          "GET events in default tenant with severity = INFO; should match 3 events",
			authzHdr:      authzHeader,
			requestURI:    "events",
			requestBody:   &api.ListWatchOptions{FieldSelector: fmt.Sprintf("severity=%s", evtsapi.SeverityLevel_INFO), MaxResults: 100},
			expStatusCode: http.StatusOK,
			expResponse: &expectedResponse{
				numEvents: 3,
				events: map[string]*evtsapi.Event{
					fmt.Sprintf("%s-%s", eventType1, evtsapi.SeverityLevel_INFO): {
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventType1,
							Severity: evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_INFO)],
							Count:    1,
						},
					},
					fmt.Sprintf("%s-%s", eventType2, evtsapi.SeverityLevel_INFO): {
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventType2,
							Severity: evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_INFO)],
							Count:    1,
						},
					},
					fmt.Sprintf("%s-%s", eventType3, evtsapi.SeverityLevel_INFO): {
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventType3,
							Severity: evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_INFO)],
							Count:    1,
						},
					}},
			},
		},
		{ // GET events with severity = "CRITICAL" and source.node-name="test-node"; should match 3 events
			name:          "GET events in default tenant with severity = 'CRITICAL' and source.node-name='test-node'; should match 3 events",
			authzHdr:      authzHeader,
			requestURI:    "events",
			requestBody:   &api.ListWatchOptions{FieldSelector: fmt.Sprintf("severity=%s,source.node-name=%s", evtsapi.SeverityLevel_CRITICAL, "test-node"), MaxResults: 100},
			expStatusCode: http.StatusOK,
			expResponse: &expectedResponse{
				numEvents: 3,
				events: map[string]*evtsapi.Event{
					fmt.Sprintf("%s-%s", eventType1, evtsapi.SeverityLevel_CRITICAL): {
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventType1,
							Severity: evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_CRITICAL)],
							Count:    1,
						},
					},
					fmt.Sprintf("%s-%s", eventType2, evtsapi.SeverityLevel_CRITICAL): {
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventType2,
							Severity: evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_CRITICAL)],
							Count:    1,
						},
					},
					fmt.Sprintf("%s-%s", eventType3, evtsapi.SeverityLevel_CRITICAL): {
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
			name:          "Get events with severity = 'CRITICAL' and type in ('EVENT-TYPE2'); should match only one event",
			authzHdr:      authzHeader,
			requestURI:    "events",
			requestBody:   &api.ListWatchOptions{FieldSelector: fmt.Sprintf("severity=%s,type in (%s)", evtsapi.SeverityLevel_CRITICAL, eventType2), MaxResults: 100},
			expStatusCode: http.StatusOK,
			expResponse: &expectedResponse{
				numEvents: 1,
				events: map[string]*evtsapi.Event{
					fmt.Sprintf("%s-%s", eventType2, evtsapi.SeverityLevel_CRITICAL): {
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
			name:          "GET events from timeNow to time.Now()+ 100s; should match ALL(9) events",
			authzHdr:      authzHeader,
			requestURI:    "events",
			requestBody:   &api.ListWatchOptions{FieldSelector: fmt.Sprintf("meta.creation-time>=%v,meta.creation-time<=%v", timeNow.Format(time.RFC3339Nano), time.Now().Add(100*time.Second).Format(time.RFC3339Nano)), MaxResults: 100},
			expStatusCode: http.StatusOK,
			expResponse: &expectedResponse{
				numEvents: 9,
				events: map[string]*evtsapi.Event{
					fmt.Sprintf("%s-%s", eventType1, evtsapi.SeverityLevel_INFO): {
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventType1,
							Severity: evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_INFO)],
							Count:    1,
						},
					},
					fmt.Sprintf("%s-%s", eventType1, evtsapi.SeverityLevel_WARNING): {
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventType1,
							Severity: evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_WARNING)],
							Count:    1,
						},
					},
					fmt.Sprintf("%s-%s", eventType1, evtsapi.SeverityLevel_CRITICAL): {
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventType1,
							Severity: evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_CRITICAL)],
							Count:    1,
						},
					},
					fmt.Sprintf("%s-%s", eventType2, evtsapi.SeverityLevel_INFO): {
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventType2,
							Severity: evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_INFO)],
							Count:    1,
						},
					},
					fmt.Sprintf("%s-%s", eventType2, evtsapi.SeverityLevel_WARNING): {
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventType2,
							Severity: evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_WARNING)],
							Count:    1,
						},
					},
					fmt.Sprintf("%s-%s", eventType2, evtsapi.SeverityLevel_CRITICAL): {
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventType2,
							Severity: evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_CRITICAL)],
							Count:    1,
						},
					},
					fmt.Sprintf("%s-%s", eventType3, evtsapi.SeverityLevel_INFO): {
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventType3,
							Severity: evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_INFO)],
							Count:    1,
						},
					},
					fmt.Sprintf("%s-%s", eventType3, evtsapi.SeverityLevel_WARNING): {
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventType3,
							Severity: evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_WARNING)],
							Count:    1,
						},
					},
					fmt.Sprintf("%s-%s", eventType3, evtsapi.SeverityLevel_CRITICAL): {
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
			name:          "Get events with creation-time>timeNow",
			authzHdr:      authzHeader,
			requestURI:    "events",
			requestBody:   &api.ListWatchOptions{FieldSelector: fmt.Sprintf("severity=%s,type in (%s),meta.creation-time>%v", evtsapi.SeverityLevel_CRITICAL, eventType2, timeNow.Format(time.RFC3339Nano)), MaxResults: 100},
			expStatusCode: http.StatusOK,
			expResponse: &expectedResponse{
				numEvents: 1,
				events: map[string]*evtsapi.Event{
					fmt.Sprintf("%s-%s", eventType2, evtsapi.SeverityLevel_CRITICAL): {
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
			name:          "Get events with modified-time>timeNow",
			authzHdr:      authzHeader,
			requestURI:    "events",
			requestBody:   &api.ListWatchOptions{FieldSelector: fmt.Sprintf("severity=%s,type in (%s),meta.mod-time>%v", evtsapi.SeverityLevel_CRITICAL, eventType2, timeNow.Format(time.RFC3339Nano)), MaxResults: 100},
			expStatusCode: http.StatusOK,
			expResponse: &expectedResponse{
				numEvents: 1,
				events: map[string]*evtsapi.Event{
					fmt.Sprintf("%s-%s", eventType2, evtsapi.SeverityLevel_CRITICAL): {
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
			name:          "Get events with creation-time<=timeNow",
			authzHdr:      authzHeader,
			requestURI:    "events",
			requestBody:   &api.ListWatchOptions{FieldSelector: fmt.Sprintf("severity=%s,type in (%s),meta.creation-time<=%v", evtsapi.SeverityLevel_CRITICAL, eventType2, timeNow.Format(time.RFC3339Nano)), MaxResults: 100},
			expStatusCode: http.StatusOK,
			expResponse: &expectedResponse{
				numEvents: 0,
				events:    map[string]*evtsapi.Event{},
			},
		},
		{ // Get events with type in (EVENT-TYPE1,EVENT-TYPE2),source.node-name notin (test-node); should match none
			name:          "Get events with type in (EVENT-TYPE1,EVENT-TYPE2),source.node-name notin (test-node); should match none",
			authzHdr:      authzHeader,
			requestURI:    "events",
			requestBody:   &api.ListWatchOptions{FieldSelector: fmt.Sprintf("type in (%s,%s),source.node-name notin (%s)", eventType1, eventType2, "test-node"), MaxResults: 100},
			expStatusCode: http.StatusOK,
			expResponse: &expectedResponse{
				numEvents: 0,
				events:    map[string]*evtsapi.Event{},
			},
		},
		{ // GET events with severity="TEST"; should match none
			name:          "GET events with severity='TEST'; should match none",
			authzHdr:      authzHeader,
			requestURI:    "events",
			requestBody:   &api.ListWatchOptions{FieldSelector: "severity=TEST", MaxResults: 100},
			expStatusCode: http.StatusOK,
			expResponse: &expectedResponse{
				numEvents: 0,
				events:    map[string]*evtsapi.Event{},
			},
		},
		{ // GET events with source.component="test" and type= "test"; should match none
			name:          "GET events with source.component='test 'and type= 'test'; should match none",
			authzHdr:      authzHeader,
			requestURI:    "events",
			requestBody:   &api.ListWatchOptions{FieldSelector: "source.component=test,type=test", MaxResults: 100},
			expStatusCode: http.StatusOK,
			expResponse: &expectedResponse{
				numEvents: 0,
				events:    map[string]*evtsapi.Event{},
			},
		},
		{ // Get events with invalid field names
			name:          "Get events with invalid field names",
			authzHdr:      authzHeader,
			requestURI:    "events",
			requestBody:   &api.ListWatchOptions{FieldSelector: fmt.Sprintf("invalid-field=%s,type in (%s)", evtsapi.SeverityLevel_CRITICAL, eventType2), MaxResults: 100},
			expStatusCode: http.StatusInternalServerError,
			expResponse: &expectedResponse{
				numEvents: 0,
				events:    map[string]*evtsapi.Event{},
			},
		},
		{ // Get events with invalid field names
			name:          "Get events with invalid field names",
			authzHdr:      authzHeader,
			requestURI:    "events",
			requestBody:   &api.ListWatchOptions{FieldSelector: fmt.Sprintf("meta.invalid<=%v", timeNow.Format(time.RFC3339Nano)), MaxResults: 100},
			expStatusCode: http.StatusInternalServerError,
			expResponse: &expectedResponse{
				numEvents: 0,
				events:    map[string]*evtsapi.Event{},
			},
		},
		{ // GET events from testtenant; should match 3 events
			name:          "GET events from testtenant; should match 3 events",
			authzHdr:      authzHeader,
			requestURI:    "events",
			requestBody:   &api.ListWatchOptions{ObjectMeta: api.ObjectMeta{Tenant: testTenant, Namespace: globals.DefaultNamespace}}, // default max-results to 1000
			expStatusCode: http.StatusOK,
			expResponse: &expectedResponse{
				numEvents: 3,
				events: map[string]*evtsapi.Event{
					fmt.Sprintf("%s-%s", eventType3, evtsapi.SeverityLevel_INFO): {
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventType3,
							Severity: evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_INFO)],
							Count:    1,
						},
					},
					fmt.Sprintf("%s-%s", eventType3, evtsapi.SeverityLevel_WARNING): {
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventType3,
							Severity: evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_WARNING)],
							Count:    1,
						},
					},
					fmt.Sprintf("%s-%s", eventType3, evtsapi.SeverityLevel_CRITICAL): {
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventType3,
							Severity: evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_CRITICAL)],
							Count:    1,
						},
					},
				},
			},
		},
		{ // GET events from default tenant using testtenant user; should return authorization error
			name:          "GET events from default tenant using testtenant user credentials; should return authorization error",
			authzHdr:      testtenantAuthzHeader,
			requestURI:    "events",
			requestBody:   &api.ListWatchOptions{ObjectMeta: api.ObjectMeta{Tenant: globals.DefaultTenant, Namespace: globals.DefaultNamespace}}, // default max-results to 1000
			expStatusCode: http.StatusForbidden,
			expResponse:   &expectedResponse{},
		},
	}

	// add room for /events/{UUID} test cases
	for i := 0; i < cap(totalEventByUUIDRequests); i++ {
		validTCs = append(validTCs, &tc{})
	}

	// perform request and check the response
	for _, rr := range validTCs {
		ti.logger.Infof("executing TC: %v, %v, %v", rr.name, rr.requestURI, rr.requestBody)
		switch {
		case "events" == rr.requestURI: // */events/
			url := fmt.Sprintf("https://%s/events/v1/%s", apiGwAddr, rr.requestURI)
			resp := evtsapi.EventList{}
			httpClient := netutils.NewHTTPClient()
			httpClient.WithTLSConfig(&tls.Config{InsecureSkipVerify: true})
			httpClient.SetHeader("Authorization", rr.authzHdr)

			// both GET and POST should behave the same
			for _, reqMethod := range []string{"GET", "POST"} {
				AssertEventually(t,
					func() (bool, interface{}) {
						statusCode, _ := httpClient.Req(reqMethod, url, rr.requestBody, &resp)
						if statusCode != rr.expStatusCode || len(resp.GetItems()) != rr.expResponse.numEvents {
							return false, fmt.Sprintf("failed to get expected events for %#v", *rr.requestBody)
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
					statusCode, err := httpClient.Req("GET", fmt.Sprintf("https://%s/%s", apiGwAddr, selfLink), nil, &evt)
					Assert(t, err == nil && statusCode == http.StatusOK, "failed to get the event using self-link: %v, status: %v, err: %v", selfLink, statusCode, err)
					Assert(t, evt.GetUUID() == obtainedEvt.GetUUID(), "obtained: %v, expected: %v", evt.GetUUID(), obtainedEvt.GetUUID())

					// update the TC's request to test /events/{UUID}
					if len(totalEventByUUIDRequests) < cap(totalEventByUUIDRequests) {
						validTCs[len(validTCs)-len(totalEventByUUIDRequests)-1].requestURI = fmt.Sprintf("events/%s", obtainedEvt.GetUUID())
						totalEventByUUIDRequests = append(totalEventByUUIDRequests, true)
					}
				}
			}
		case regexp.MustCompile("events/*").MatchString(rr.requestURI): // */events/{uuid}
			url := fmt.Sprintf("https://%s/events/v1/%s", apiGwAddr, rr.requestURI)
			resp := evtsapi.Event{}
			httpClient := netutils.NewHTTPClient()
			httpClient.WithTLSConfig(&tls.Config{InsecureSkipVerify: true})
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
			statusCode, err := httpClient.Req("GET", fmt.Sprintf("https://%s/%s", apiGwAddr, selfLink), nil, &evt)
			Assert(t, err == nil && statusCode == http.StatusOK, "failed to get the event using self-link: %v, status: %v, err: %v", selfLink, statusCode, err)
			Assert(t, evt.GetUUID() == resp.GetUUID(), "obtained: %v, expected: %v", evt.GetUUID(), resp.GetUUID())
		}
	}

	// wait for the recorder to complete
	wg.Wait()
}

// TestEventsAlertEngine tests alert generation from events. And, ensures the
// respective alert policy is updated accordingly.
func TestEventsAlertEngine(t *testing.T) {
	// setup events pipeline to record and distribute events
	ti := tInfo{batchInterval: 100 * time.Millisecond, dedupInterval: 100 * time.Second}
	AssertOk(t, ti.setup(t), "failed to setup test")
	defer ti.teardown()

	// create API server client
	apiClient, err := client.NewGrpcUpstream("events_integ_test", ti.apiServerAddr, ti.logger)
	AssertOk(t, err, "failed to create API server client, err: %v", err)
	defer apiClient.Close()

	// start spyglass (backend service for events)
	fdrTemp, fdrAddr, err := testutils.StartSpyglass("finder", "", ti.mockResolver, nil, ti.logger, ti.esClient)
	AssertOk(t, err, "failed to start spyglass finder, err: %v", err)
	fdr := fdrTemp.(finder.Interface)
	defer fdr.Stop()

	// API gateway
	apiGw, apiGwAddr, err := testutils.StartAPIGateway(":0", false,
		map[string]string{globals.APIServer: ti.apiServerAddr, globals.Spyglass: fdrAddr}, []string{"metrics_query"}, []string{}, ti.logger)
	AssertOk(t, err, "failed to start API gateway, err: %v", err)
	defer apiGw.Stop()

	// setup authn and get authz token
	userCreds := &auth.PasswordCredential{Username: testutils.TestLocalUser, Password: testutils.TestLocalPassword, Tenant: testutils.TestTenant}
	err = testutils.SetupAuth(ti.apiServerAddr, true, &auth.Ldap{Enabled: false}, &auth.Radius{Enabled: false}, userCreds, ti.logger)
	AssertOk(t, err, "failed to setup authN service, err: %v", err)
	defer testutils.CleanupAuth(ti.apiServerAddr, true, false, userCreds, ti.logger)
	authzHeader, err := testutils.GetAuthorizationHeader(apiGwAddr, userCreds)
	AssertOk(t, err, "failed to get authZ header, err: %v", err)

	// add event based alert policies
	// policy - 1
	alertPolicy1 := policygen.CreateAlertPolicyObj(globals.DefaultTenant, globals.DefaultNamespace, fmt.Sprintf("ap1-%s", uuid.NewV4().String()),
		"Event", evtsapi.SeverityLevel_CRITICAL, "alerts from events",
		[]*fields.Requirement{
			{Key: "type", Operator: "in", Values: []string{eventType1, eventType2, eventType3}},
			{Key: "count", Operator: "gte", Values: []string{"15"}},
			{Key: "source.node-name", Operator: "equals", Values: []string{"test-node"}},
		}, []string{})

	alertPolicy1, err = apiClient.MonitoringV1().AlertPolicy().Create(context.Background(), alertPolicy1)
	AssertOk(t, err, "failed to add alert policy, err: %v", err)
	defer apiClient.MonitoringV1().AlertPolicy().Delete(context.Background(), alertPolicy1.GetObjectMeta())

	// policy - 2
	alertPolicy2 := policygen.CreateAlertPolicyObj(globals.DefaultTenant, globals.DefaultNamespace, fmt.Sprintf("ap2-%s", uuid.NewV4().String()),
		"Event", evtsapi.SeverityLevel_WARNING, "alerts from events",
		[]*fields.Requirement{
			{Key: "count", Operator: "gte", Values: []string{"5"}},
			{Key: "count", Operator: "lt", Values: []string{"7"}},
			{Key: "severity", Operator: "equals", Values: []string{evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_INFO)]}},
			{Key: "type", Operator: "in", Values: []string{eventType1, eventType2, eventType3}},
		}, []string{})

	alertPolicy2, err = apiClient.MonitoringV1().AlertPolicy().Create(context.Background(), alertPolicy2)
	AssertOk(t, err, "failed to add alert policy, err: %v", err)
	defer apiClient.MonitoringV1().AlertPolicy().Delete(context.Background(), alertPolicy2.GetObjectMeta())

	alertPolicy3 := policygen.CreateAlertPolicyObj(globals.DefaultTenant, globals.DefaultNamespace, fmt.Sprintf("ap3-%s", uuid.NewV4().String()),
		"Event", evtsapi.SeverityLevel_WARNING, "policy with no reqs", []*fields.Requirement{}, []string{})
	alertPolicy3, err = apiClient.MonitoringV1().AlertPolicy().Create(context.Background(), alertPolicy3)
	AssertOk(t, err, "failed to add alert policy, err: %v", err)
	defer apiClient.MonitoringV1().AlertPolicy().Delete(context.Background(), alertPolicy3.GetObjectMeta())

	// generate events
	// define list of events to be recorded
	dummyObjRef := &cluster.Node{
		TypeMeta: api.TypeMeta{
			Kind: "Node",
		},
		ObjectMeta: api.ObjectMeta{
			Tenant:    globals.DefaultTenant,
			Namespace: globals.DefaultNamespace,
		},
	}
	recordEvents := []*struct {
		eventType string
		severity  evtsapi.SeverityLevel
		message   string
		objRef    interface{}
		repeat    int // number of times to repeat the event
	}{
		{eventType1, evtsapi.SeverityLevel_INFO, fmt.Sprintf("%s-%s", eventType1, evtsapi.SeverityLevel_INFO), *dummyObjRef, 5}, // this should generate an alert (alertPolicy2)
		{eventType1, evtsapi.SeverityLevel_WARNING, fmt.Sprintf("%s-%s", eventType1, evtsapi.SeverityLevel_WARNING), *dummyObjRef, 10},
		{eventType1, evtsapi.SeverityLevel_CRITICAL, fmt.Sprintf("%s-%s", eventType1, evtsapi.SeverityLevel_CRITICAL), *dummyObjRef, 15}, // this should generate an alert (alertPolicy1)

		{eventType2, evtsapi.SeverityLevel_INFO, fmt.Sprintf("%s-%s", eventType2, evtsapi.SeverityLevel_INFO), *dummyObjRef, 5}, // this should generate an alert (alertPolicy2)
		{eventType2, evtsapi.SeverityLevel_WARNING, fmt.Sprintf("%s-%s", eventType2, evtsapi.SeverityLevel_WARNING), *dummyObjRef, 10},
		{eventType2, evtsapi.SeverityLevel_CRITICAL, fmt.Sprintf("%s-%s", eventType2, evtsapi.SeverityLevel_CRITICAL), *dummyObjRef, 15}, // this should generate an alert (alertPolicy1)

		{eventType3, evtsapi.SeverityLevel_INFO, fmt.Sprintf("%s-%s", eventType3, evtsapi.SeverityLevel_INFO), *dummyObjRef, 5}, // this should generate an alert (alertPolicy2)
		{eventType3, evtsapi.SeverityLevel_WARNING, fmt.Sprintf("%s-%s", eventType3, evtsapi.SeverityLevel_WARNING), *dummyObjRef, 10},
		{eventType3, evtsapi.SeverityLevel_CRITICAL, fmt.Sprintf("%s-%s", eventType3, evtsapi.SeverityLevel_CRITICAL), *dummyObjRef, 15}, // this should generate an alert (alertPolicy1)

		{eventType1, evtsapi.SeverityLevel_INFO, fmt.Sprintf("%s-%s", eventType1, evtsapi.SeverityLevel_INFO), nil, 5}, // this should generate an alert (alertPolicy2)
		{eventType2, evtsapi.SeverityLevel_WARNING, fmt.Sprintf("%s-%s", eventType2, evtsapi.SeverityLevel_WARNING), nil, 10},
		{eventType3, evtsapi.SeverityLevel_CRITICAL, fmt.Sprintf("%s-%s", eventType3, evtsapi.SeverityLevel_CRITICAL), nil, 15}, // this should generate an alert (alertPolicy1)
	}

	wg := new(sync.WaitGroup)
	wg.Add(2)

	// start recorder
	recorderEventsDir, err := ioutil.TempDir("", "")
	AssertOk(t, err, "failed to create recorder events directory")
	defer os.RemoveAll(recorderEventsDir)
	testEventSource := &evtsapi.EventSource{NodeName: "test-node", Component: uuid.NewV4().String()}
	go func() {
		defer wg.Done()

		evtsRecorder, err := recorder.NewRecorder(&recorder.Config{
			Source:       testEventSource,
			EvtTypes:     testEventTypes,
			EvtsProxyURL: ti.evtsProxy.RPCServer.GetListenURL(),
			BackupDir:    recorderEventsDir})
		if err != nil {
			log.Errorf("failed to create recorder, err: %v", err)
			return
		}

		// record events
		for i := range recordEvents {
			if objRef, ok := recordEvents[i].objRef.(cluster.Node); ok {
				objRef.ObjectMeta.Name = CreateAlphabetString(5)
				recordEvents[i].objRef = &objRef
			}
			for j := 0; j < recordEvents[i].repeat; j++ {
				evtsRecorder.Event(recordEvents[i].eventType, recordEvents[i].severity, recordEvents[i].message, recordEvents[i].objRef)
			}
		}

		// wait for the batch interval
		time.Sleep(1 * time.Second)
		// if objRef!=nil, this should increase the hits but not recreate the alerts.
		// it will recreate alerts otherwise.
		for i := range recordEvents {
			evtsRecorder.Event(recordEvents[i].eventType, recordEvents[i].severity, recordEvents[i].message, recordEvents[i].objRef)
		}
	}()

	// list of alerts to be generated by the alert engine
	tests := []struct {
		selector   string
		expSuccess bool
	}{
		{
			selector: fmt.Sprintf("status.reason.alert-policy-id=%s,status.message=%s,status.severity=%s,status.object-ref.kind=%s",
				alertPolicy1.GetName(), fmt.Sprintf("%s-%s", eventType1, evtsapi.SeverityLevel_CRITICAL), alertPolicy1.Spec.GetSeverity(), dummyObjRef.GetKind()),
			expSuccess: true,
		},
		{
			selector: fmt.Sprintf("status.reason.alert-policy-id=%s,status.message=%s,status.severity=%s,status.object-ref.kind=%s",
				alertPolicy1.GetName(), fmt.Sprintf("%s-%s", eventType2, evtsapi.SeverityLevel_CRITICAL), alertPolicy1.Spec.GetSeverity(), dummyObjRef.GetKind()),
			expSuccess: true,
		},
		{
			selector: fmt.Sprintf("status.reason.alert-policy-id=%s,status.message=%s,status.severity=%s,status.object-ref.kind=%s",
				alertPolicy1.GetName(), fmt.Sprintf("%s-%s", eventType3, evtsapi.SeverityLevel_CRITICAL), alertPolicy1.Spec.GetSeverity(), dummyObjRef.GetKind()),
			expSuccess: true,
		},
		{
			selector: fmt.Sprintf("status.reason.alert-policy-id=%s,status.message=%s,status.severity=%s,status.object-ref.kind=%s",
				alertPolicy2.GetName(), fmt.Sprintf("%s-%s", eventType1, evtsapi.SeverityLevel_INFO), alertPolicy2.Spec.GetSeverity(), dummyObjRef.GetKind()),
			expSuccess: true,
		},
		{
			selector: fmt.Sprintf("status.reason.alert-policy-id=%s,status.message=%s,status.severity=%s,status.object-ref.kind=%s",
				alertPolicy2.GetName(), fmt.Sprintf("%s-%s", eventType2, evtsapi.SeverityLevel_INFO), alertPolicy2.Spec.GetSeverity(), dummyObjRef.GetKind()),
			expSuccess: true,
		},
		{
			selector: fmt.Sprintf("status.reason.alert-policy-id=%s,status.message=%s,status.severity=%s,status.object-ref.kind=%s",
				alertPolicy2.GetName(), fmt.Sprintf("%s-%s", eventType3, evtsapi.SeverityLevel_INFO), alertPolicy2.Spec.GetSeverity(), dummyObjRef.GetKind()),
			expSuccess: true,
		},
		{
			selector: fmt.Sprintf("status.reason.alert-policy-id=%s,status.message=%s,status.severity=%s,status.object-ref.kind=%s",
				alertPolicy2.GetName(), fmt.Sprintf("%s-%s", eventType3, evtsapi.SeverityLevel_INFO), alertPolicy2.Spec.GetSeverity(), "invalid"),
			expSuccess: false,
		},
		{
			selector:   fmt.Sprintf("status.object-ref.kind=invalid"),
			expSuccess: false,
		},
	}

	// test if the expected alerts are generated
	go func() {
		defer wg.Done()
		for _, test := range tests {
			AssertEventually(t, func() (bool, interface{}) {
				alerts, err := apiClient.MonitoringV1().Alert().List(context.Background(),
					&api.ListWatchOptions{
						ObjectMeta:    api.ObjectMeta{Tenant: globals.DefaultTenant},
						FieldSelector: test.selector})
				if err != nil {
					return false, fmt.Sprintf("%v failed, err: %v", test.selector, err)
				}

				if test.expSuccess && len(alerts) != 1 {
					return false, fmt.Sprintf("expected: %v, obtained: %v", test.selector, alerts)
				}

				return true, nil
			}, "did not receive the expected alert", string("20ms"), string("10s"))
		}
	}()

	wg.Wait()

	// make sure the policy status got updated
	expectedAlertStatus := []struct {
		policyMeta         *api.ObjectMeta
		totalHits          int32
		openAlerts         int32
		acknowledgedAlerts int32
	}{
		{policyMeta: alertPolicy1.GetObjectMeta(), totalHits: 8, openAlerts: 4, acknowledgedAlerts: 0},
		{policyMeta: alertPolicy2.GetObjectMeta(), totalHits: 8, openAlerts: 4, acknowledgedAlerts: 0},
		{policyMeta: alertPolicy3.GetObjectMeta(), totalHits: 0, openAlerts: 0, acknowledgedAlerts: 0}, // no reqs so, there should be no alerts
	}
	for _, as := range expectedAlertStatus {
		AssertEventually(t, func() (bool, interface{}) {
			res, err := apiClient.MonitoringV1().AlertPolicy().Get(context.Background(),
				&api.ObjectMeta{Name: as.policyMeta.GetName(), Tenant: as.policyMeta.GetTenant(), Namespace: as.policyMeta.GetNamespace(), UUID: as.policyMeta.GetUUID()})
			if err != nil {
				return false, fmt.Sprintf(":%v, err: %v", as.policyMeta.GetName(), err)
			}

			if res.Status.GetTotalHits() != as.totalHits {
				return false, fmt.Sprintf("total hits on policy %v expected: %v, obtained: %v", res.GetObjectMeta().GetName(), as.totalHits, res.Status.GetTotalHits())
			}

			if as.openAlerts != res.Status.GetOpenAlerts() {
				return false, fmt.Sprintf("open alerts on policy %v expected: %v, obtained: %v", res.GetObjectMeta().GetName(), as.openAlerts, res.Status.GetOpenAlerts())
			}

			if as.acknowledgedAlerts != res.Status.GetAcknowledgedAlerts() {
				return false, fmt.Sprintf("acknowledged alerts on policy %v expected: %v, obtained: %v", res.GetObjectMeta().GetName(), as.acknowledgedAlerts, res.Status.GetAcknowledgedAlerts())
			}

			return true, nil
		}, "alert status does not match the expected", string("20ms"), string("10s"))
	}

	// resolve or acknowledge alerts
	alerts, err := apiClient.MonitoringV1().Alert().List(context.Background(),
		&api.ListWatchOptions{
			ObjectMeta: api.ObjectMeta{Tenant: globals.DefaultTenant},
			FieldSelector: fmt.Sprintf("status.reason.alert-policy-id in (%s,%s,%s)",
				alertPolicy1.GetName(), alertPolicy2.GetName(), alertPolicy3.GetName()),
		})
	AssertOk(t, err, "failed to list alerts, err: %v", err)
	Assert(t, len(alerts) > 2, "expected more than 2 alerts, got: %v", len(alerts))

	alertTests := []*struct {
		alert       monitoring.Alert
		resolve     bool
		acknowledge bool
	}{
		{alert: *alerts[0], resolve: true, acknowledge: false},
		{alert: *alerts[len(alerts)-1], resolve: false, acknowledge: true},
	}

	for _, at := range alertTests {
		aURL := fmt.Sprintf("https://%s/configs/monitoring/v1/alerts/%s", apiGwAddr, at.alert.GetName())
		apURL := fmt.Sprintf("https://%s/configs/monitoring/v1/alertPolicies/%s", apiGwAddr, at.alert.Status.Reason.GetPolicyID())

		httpClient := netutils.NewHTTPClient()
		httpClient.WithTLSConfig(&tls.Config{InsecureSkipVerify: true})
		httpClient.SetHeader("Authorization", authzHeader)

		// check alert policy before update
		ap := &monitoring.AlertPolicy{}
		statusCode, err := httpClient.Req("GET", apURL, &api.ListWatchOptions{}, &ap)
		AssertOk(t, err, "failed to get alert policy, err: %v", err)
		Assert(t, statusCode == http.StatusOK, "failed to get alert policy")

		// UPDATE alert state (to acknowledged or resolved)
		if at.acknowledge {
			resp := monitoring.Alert{}
			AssertEventually(t,
				func() (bool, interface{}) {
					at.alert.Spec.State = monitoring.AlertSpec_AlertState_name[int32(monitoring.AlertSpec_ACKNOWLEDGED)]
					statusCode, err := httpClient.Req("PUT", aURL, at.alert, &resp)
					if err != nil {
						return false, fmt.Sprintf("err: %v", err)
					}

					if statusCode != http.StatusOK {
						return false, fmt.Sprintf("update failed with status: %d", statusCode)
					}

					if resp.Status.Acknowledged == nil {
						return false, fmt.Sprintf("alert status not updated, acknowledged: nil")
					}

					return true, nil
				}, "failed to update alert state", "20ms", "6s")
		} else if at.resolve {
			resp := monitoring.Alert{}
			AssertEventually(t,
				func() (bool, interface{}) {
					at.alert.Spec.State = monitoring.AlertSpec_AlertState_name[int32(monitoring.AlertSpec_RESOLVED)]
					statusCode, err := httpClient.Req("PUT", aURL, at.alert, &resp)
					if err != nil {
						return false, fmt.Sprintf("err: %v", err)
					}

					if statusCode != http.StatusOK {
						return false, fmt.Sprintf("update failed with status: %d", statusCode)
					}

					if resp.Status.Resolved == nil {
						return false, fmt.Sprintf("alert status not updated, resolved: nil")
					}

					return true, nil
				}, "failed to update alert state", "20ms", "6s")
		}

		updatedAp := &monitoring.AlertPolicy{}
		statusCode, err = httpClient.Req("GET", apURL, &api.ListWatchOptions{}, &updatedAp)
		AssertOk(t, err, "failed to get alert policy, err: %v", err)
		Assert(t, statusCode == http.StatusOK, "failed to get alert policy")
		Assert(t, !at.acknowledge || (at.acknowledge && updatedAp.Status.AcknowledgedAlerts > ap.Status.AcknowledgedAlerts),
			"expected #acknowledged alerts: >%d, got: %d", ap.Status.AcknowledgedAlerts, updatedAp.Status.AcknowledgedAlerts)
		Assert(t, !at.resolve || (at.resolve && updatedAp.Status.OpenAlerts < ap.Status.OpenAlerts),
			"expected #acknowledged alerts: <%d, got: %d", ap.Status.OpenAlerts, updatedAp.Status.OpenAlerts)
	}
}

// TestEventsAlertEngineWithTCPSyslogExport tests the syslog export of alerts with dummy TCP server
// it tests both BSD and RFC style syslog exports.
func TestEventsAlertEngineWithTCPSyslogExport(t *testing.T) {
	// setup events pipeline to record and distribute events
	ti := tInfo{}
	AssertOk(t, ti.setup(t), "failed to setup test")
	defer ti.teardown()

	// create API server client
	apiClient, err := client.NewGrpcUpstream("events_integ_test", ti.apiServerAddr, ti.logger)
	AssertOk(t, err, "failed to create API server client, err: %v", err)
	defer apiClient.Close()

	// start TCP server to receive syslog messages
	ln, receivedMsgsAtTCPServer, err := serviceutils.StartTCPServer(":0")
	AssertOk(t, err, "failed to start TCP server, err: %v", err)
	defer ln.Close()
	tmp := strings.Split(ln.Addr().String(), ":")

	// alert destination - 1: BSD style syslog export
	alertDestBSDSyslog := policygen.CreateAlertDestinationObj(globals.DefaultTenant, globals.DefaultNamespace, uuid.NewV1().String(),
		&monitoring.SyslogExport{
			Format: monitoring.MonitoringExportFormat_name[int32(monitoring.MonitoringExportFormat_SYSLOG_BSD)],
			Targets: []*monitoring.ExportConfig{
				{
					Destination: "127.0.0.1",
					Transport:   fmt.Sprintf("tcp/%s", tmp[len(tmp)-1]),
				},
			},
		})
	alertDestBSDSyslog, err = apiClient.MonitoringV1().AlertDestination().Create(context.Background(), alertDestBSDSyslog)
	AssertOk(t, err, "failed to add alert destination, err: %v", err)

	// alert destination - 2: RFC5424 style syslog export
	alertDestRFC5424Syslog := policygen.CreateAlertDestinationObj(globals.DefaultTenant, globals.DefaultNamespace, uuid.NewV1().String(),
		&monitoring.SyslogExport{
			Format: monitoring.MonitoringExportFormat_name[int32(monitoring.MonitoringExportFormat_SYSLOG_RFC5424)],
			Targets: []*monitoring.ExportConfig{
				{
					Destination: "127.0.0.1",
					Transport:   fmt.Sprintf("tcp/%s", tmp[len(tmp)-1]),
				},
			},
		})
	alertDestRFC5424Syslog, err = apiClient.MonitoringV1().AlertDestination().Create(context.Background(), alertDestRFC5424Syslog)
	AssertOk(t, err, "failed to add alert destination, err: %v", err)

	// policy - 1: convert CRITICAL events with occurrences > 10 to a CRITICAL alert and export it to the given alert dest
	alertPolicy1 := policygen.CreateAlertPolicyObj(globals.DefaultTenant, globals.DefaultNamespace, uuid.NewV1().String(), "Event", evtsapi.SeverityLevel_CRITICAL, "alerts from events", []*fields.Requirement{
		{Key: "severity", Operator: "in", Values: []string{evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_CRITICAL)]}},
		{Key: "count", Operator: "gt", Values: []string{"10"}},
	}, []string{alertDestBSDSyslog.GetName(), alertDestRFC5424Syslog.GetName()})
	alertPolicy1, err = apiClient.MonitoringV1().AlertPolicy().Create(context.Background(), alertPolicy1)
	AssertOk(t, err, "failed to add alert policy, err: %v", err)

	// policy - 2: convert WARNING events with occurrences = 10 to a WARNING alert and export it to the given alert dest
	alertPolicy2 := policygen.CreateAlertPolicyObj(globals.DefaultTenant, globals.DefaultNamespace, uuid.NewV1().String(), "Event", evtsapi.SeverityLevel_WARNING, "alerts from events", []*fields.Requirement{
		{Key: "severity", Operator: "equals", Values: []string{evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_WARNING)]}},
		{Key: "count", Operator: "equals", Values: []string{"10"}},
	}, []string{alertDestBSDSyslog.GetName(), alertDestRFC5424Syslog.GetName()})
	alertPolicy2, err = apiClient.MonitoringV1().AlertPolicy().Create(context.Background(), alertPolicy2)
	AssertOk(t, err, "failed to add alert policy, err: %v", err)

	messages := map[chan string][]struct {
		Substrs   []string                          // syslog message should contain all these strings
		MsgFormat monitoring.MonitoringExportFormat // BSD style message contains the JSON formatted alert; RFC contains <msgID, structured data, msg>
	}{
		receivedMsgsAtTCPServer: {
			{
				Substrs:   []string{fmt.Sprintf("%s-%s", eventType1, evtsapi.SeverityLevel_CRITICAL), alertPolicy1.GetName()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_BSD,
			},
			{
				Substrs:   []string{fmt.Sprintf("%s-%s", eventType2, evtsapi.SeverityLevel_CRITICAL), alertPolicy1.GetName()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_BSD,
			},
			{
				Substrs:   []string{fmt.Sprintf("%s-%s", eventType3, evtsapi.SeverityLevel_CRITICAL), alertPolicy1.GetName()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_BSD,
			},
			{
				Substrs:   []string{fmt.Sprintf("%s-%s", eventType1, evtsapi.SeverityLevel_WARNING), alertPolicy2.GetName()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_BSD,
			},
			{
				Substrs:   []string{fmt.Sprintf("%s-%s", eventType2, evtsapi.SeverityLevel_WARNING), alertPolicy2.GetName()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_BSD,
			},
			{
				Substrs:   []string{fmt.Sprintf("%s-%s", eventType3, evtsapi.SeverityLevel_WARNING), alertPolicy2.GetName()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_BSD,
			},
			{
				Substrs:   []string{fmt.Sprintf("%s-%s", eventType1, evtsapi.SeverityLevel_CRITICAL), alertPolicy1.GetName()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs:   []string{fmt.Sprintf("%s-%s", eventType2, evtsapi.SeverityLevel_CRITICAL), alertPolicy1.GetName()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs:   []string{fmt.Sprintf("%s-%s", eventType3, evtsapi.SeverityLevel_CRITICAL), alertPolicy1.GetName()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs:   []string{fmt.Sprintf("%s-%s", eventType1, evtsapi.SeverityLevel_WARNING), alertPolicy2.GetName()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs:   []string{fmt.Sprintf("%s-%s", eventType2, evtsapi.SeverityLevel_WARNING), alertPolicy2.GetName()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs:   []string{fmt.Sprintf("%s-%s", eventType3, evtsapi.SeverityLevel_WARNING), alertPolicy2.GetName()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
		},
	}

	testSyslogMessageDelivery(t, ti, messages)

	AssertEventually(t,
		func() (bool, interface{}) {
			ad, err := apiClient.MonitoringV1().AlertDestination().Get(context.Background(), alertDestBSDSyslog.GetObjectMeta())
			if err != nil {
				return false, err
			}

			if ad.Status.TotalNotificationsSent == 6 {
				return true, nil
			}

			return false, fmt.Sprintf("TotalNotificationSent expected:6, got: %v", ad.Status.TotalNotificationsSent)
		}, fmt.Sprintf("alert destionation %v is not updated", alertDestBSDSyslog.GetName()), "20ms", "10s")

	AssertEventually(t,
		func() (bool, interface{}) {
			ad, err := apiClient.MonitoringV1().AlertDestination().Get(context.Background(), alertDestRFC5424Syslog.GetObjectMeta())
			if err != nil {
				return false, err
			}

			if ad.Status.TotalNotificationsSent == 6 {
				return true, nil
			}

			return false, fmt.Sprintf("TotalNotificationSent expected:6, got: %v", ad.Status.TotalNotificationsSent)
		}, fmt.Sprintf("alert destionation %v is not updated", alertDestRFC5424Syslog.GetName()), "20ms", "10s")
}

// TestEventsAlertEngineWithTCPSyslogExport tests the syslog export of alerts with dummy UDP server
// it tests both BSD and RFC style syslog exports.
func TestEventsAlertEngineWithUDPSyslogExport(t *testing.T) {
	// setup events pipeline to record and distribute events
	ti := tInfo{}
	AssertOk(t, ti.setup(t), "failed to setup test")
	defer ti.teardown()

	// create API server client
	apiClient, err := client.NewGrpcUpstream("events_integ_test", ti.apiServerAddr, ti.logger)
	AssertOk(t, err, "failed to create API server client, err: %v", err)
	defer apiClient.Close()

	// start UDP server - 1 to receive syslog messages
	pConn1, receivedMsgsAtUDPServer1, err := serviceutils.StartUDPServer(":0")
	AssertOk(t, err, "failed to start UDP server, err: %v", err)
	defer pConn1.Close()
	tmp1 := strings.Split(pConn1.LocalAddr().String(), ":")
	log.Infof("UDP server-1 running at: %s", pConn1.LocalAddr().String())

	// start UDP server - 2 to receive syslog messages
	pConn2, receivedMsgsAtUDPServer2, err := serviceutils.StartUDPServer(":0")
	AssertOk(t, err, "failed to start UDP server, err: %v", err)
	defer pConn2.Close()
	tmp2 := strings.Split(pConn2.LocalAddr().String(), ":")
	log.Infof("UDP server-2 running at: %s", pConn2.LocalAddr().String())

	// alert destination - 1: BSD style syslog export
	alertDestBSDSyslog := policygen.CreateAlertDestinationObj(globals.DefaultTenant, globals.DefaultNamespace, uuid.NewV1().String(),
		&monitoring.SyslogExport{
			Format: monitoring.MonitoringExportFormat_name[int32(monitoring.MonitoringExportFormat_SYSLOG_BSD)],
			Targets: []*monitoring.ExportConfig{
				{
					Destination: "127.0.0.1",
					Transport:   fmt.Sprintf("udp/%s", tmp1[len(tmp1)-1]),
				},
				{
					Destination: "127.0.0.1",
					Transport:   fmt.Sprintf("udp/%s", tmp2[len(tmp2)-1]),
				},
			},
		})
	alertDestBSDSyslog, err = apiClient.MonitoringV1().AlertDestination().Create(context.Background(), alertDestBSDSyslog)
	AssertOk(t, err, "failed to add alert destination, err: %v", err)

	// alert destination - 2: RFC5424 style syslog export
	alertDestRFC5424Syslog := policygen.CreateAlertDestinationObj(globals.DefaultTenant, globals.DefaultNamespace, uuid.NewV1().String(),
		&monitoring.SyslogExport{
			Format: monitoring.MonitoringExportFormat_name[int32(monitoring.MonitoringExportFormat_SYSLOG_RFC5424)],
			Targets: []*monitoring.ExportConfig{
				{
					Destination: "127.0.0.1",
					Transport:   fmt.Sprintf("udp/%s", tmp1[len(tmp1)-1]),
				},
			},
		})
	alertDestRFC5424Syslog, err = apiClient.MonitoringV1().AlertDestination().Create(context.Background(), alertDestRFC5424Syslog)
	AssertOk(t, err, "failed to add alert destination, err: %v", err)

	// convert CRITICAL events with occurrences > 10 to a CRITICAL alert and export it to the given alert dest
	alertPolicy1 := policygen.CreateAlertPolicyObj(globals.DefaultTenant, globals.DefaultNamespace, uuid.NewV1().String(), "Event", evtsapi.SeverityLevel_CRITICAL, "alerts from events", []*fields.Requirement{
		{Key: "severity", Operator: "in", Values: []string{evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_CRITICAL)]}},
		{Key: "count", Operator: "gt", Values: []string{"10"}},
	}, []string{alertDestBSDSyslog.GetName(), alertDestRFC5424Syslog.GetName()})
	alertPolicy1, err = apiClient.MonitoringV1().AlertPolicy().Create(context.Background(), alertPolicy1)
	AssertOk(t, err, "failed to add alert policy, err: %v", err)

	messages := map[chan string][]struct {
		Substrs   []string                          // syslog message should contain all these strings
		MsgFormat monitoring.MonitoringExportFormat // BSD style message contains the JSON formatted alert; RFC contains <msgID, structured data, msg>
	}{
		receivedMsgsAtUDPServer1: {
			{
				Substrs:   []string{fmt.Sprintf("%s-%s", eventType1, evtsapi.SeverityLevel_CRITICAL), alertPolicy1.GetName()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_BSD,
			},
			{
				Substrs:   []string{fmt.Sprintf("%s-%s", eventType2, evtsapi.SeverityLevel_CRITICAL), alertPolicy1.GetName()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_BSD,
			},
			{
				Substrs:   []string{fmt.Sprintf("%s-%s", eventType3, evtsapi.SeverityLevel_CRITICAL), alertPolicy1.GetName()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_BSD,
			},
			{
				Substrs:   []string{fmt.Sprintf("%s-%s", eventType1, evtsapi.SeverityLevel_CRITICAL), alertPolicy1.GetName()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs:   []string{fmt.Sprintf("%s-%s", eventType2, evtsapi.SeverityLevel_CRITICAL), alertPolicy1.GetName()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs:   []string{fmt.Sprintf("%s-%s", eventType3, evtsapi.SeverityLevel_CRITICAL), alertPolicy1.GetName()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
		},
		receivedMsgsAtUDPServer2: {
			{
				Substrs:   []string{fmt.Sprintf("%s-%s", eventType1, evtsapi.SeverityLevel_CRITICAL), alertPolicy1.GetName()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_BSD,
			},
			{
				Substrs:   []string{fmt.Sprintf("%s-%s", eventType2, evtsapi.SeverityLevel_CRITICAL), alertPolicy1.GetName()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_BSD,
			},
			{
				Substrs:   []string{fmt.Sprintf("%s-%s", eventType3, evtsapi.SeverityLevel_CRITICAL), alertPolicy1.GetName()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_BSD,
			},
		},
	}

	testSyslogMessageDelivery(t, ti, messages)

	AssertEventually(t,
		func() (bool, interface{}) {
			ad, err := apiClient.MonitoringV1().AlertDestination().Get(context.Background(), alertDestBSDSyslog.GetObjectMeta())
			if err != nil {
				return false, err
			}

			// (3 * 2) for 2 targets
			if ad.Status.TotalNotificationsSent == 6 {
				return true, nil
			}

			return false, fmt.Sprintf("TotalNotificationSent expected: 6, got: %v", ad.Status.TotalNotificationsSent)
		}, fmt.Sprintf("alert destionation %v is not updated", alertDestBSDSyslog.GetName()), "20ms", "10s")

	AssertEventually(t,
		func() (bool, interface{}) {
			ad, err := apiClient.MonitoringV1().AlertDestination().Get(context.Background(), alertDestRFC5424Syslog.GetObjectMeta())
			if err != nil {
				return false, err
			}

			if ad.Status.TotalNotificationsSent == 3 {
				return true, nil
			}

			return false, fmt.Sprintf("TotalNotificationSent expected: 3, got: %v", ad.Status.TotalNotificationsSent)
		}, fmt.Sprintf("alert destionation %v is not updated", alertDestRFC5424Syslog.GetName()), "20ms", "10s")
}

// ensures the delivery of expected syslog messages
func testSyslogMessageDelivery(t *testing.T, ti tInfo, messages map[chan string][]struct {
	Substrs   []string
	MsgFormat monitoring.MonitoringExportFormat
}) {
	var m sync.Mutex
	wg := new(sync.WaitGroup)
	wg.Add(1) // events recorder

	// define list of events to be recorded
	dummyObjRef := &cluster.Node{
		TypeMeta: api.TypeMeta{
			Kind: "Node",
		},
		ObjectMeta: api.ObjectMeta{
			Tenant:    globals.DefaultTenant,
			Namespace: globals.DefaultNamespace,
		},
	}
	recordEvents := []*struct {
		eventType string
		severity  evtsapi.SeverityLevel
		message   string
		objRef    interface{}
		repeat    int // number of times to repeat the event
	}{
		{eventType1, evtsapi.SeverityLevel_INFO, fmt.Sprintf("%s-%s", eventType1, evtsapi.SeverityLevel_INFO), *dummyObjRef, 5},
		{eventType1, evtsapi.SeverityLevel_WARNING, fmt.Sprintf("%s-%s", eventType1, evtsapi.SeverityLevel_WARNING), *dummyObjRef, 10},
		{eventType1, evtsapi.SeverityLevel_CRITICAL, fmt.Sprintf("%s-%s", eventType1, evtsapi.SeverityLevel_CRITICAL), *dummyObjRef, 15},

		{eventType2, evtsapi.SeverityLevel_INFO, fmt.Sprintf("%s-%s", eventType2, evtsapi.SeverityLevel_INFO), *dummyObjRef, 5},
		{eventType2, evtsapi.SeverityLevel_WARNING, fmt.Sprintf("%s-%s", eventType2, evtsapi.SeverityLevel_WARNING), *dummyObjRef, 10},
		{eventType2, evtsapi.SeverityLevel_CRITICAL, fmt.Sprintf("%s-%s", eventType2, evtsapi.SeverityLevel_CRITICAL), *dummyObjRef, 15},

		{eventType3, evtsapi.SeverityLevel_INFO, fmt.Sprintf("%s-%s", eventType3, evtsapi.SeverityLevel_INFO), *dummyObjRef, 5},
		{eventType3, evtsapi.SeverityLevel_WARNING, fmt.Sprintf("%s-%s", eventType3, evtsapi.SeverityLevel_WARNING), *dummyObjRef, 10},
		{eventType3, evtsapi.SeverityLevel_CRITICAL, fmt.Sprintf("%s-%s", eventType3, evtsapi.SeverityLevel_CRITICAL), *dummyObjRef, 15},
	}

	// start recorder
	recorderEventsDir, err := ioutil.TempDir("", t.Name())
	AssertOk(t, err, "failed to create recorder events directory")
	defer os.RemoveAll(recorderEventsDir)
	testEventSource := &evtsapi.EventSource{NodeName: "test-node", Component: uuid.NewV4().String()}
	go func() {
		defer wg.Done()

		evtsRecorder, err := recorder.NewRecorder(&recorder.Config{
			Source:       testEventSource,
			EvtTypes:     testEventTypes,
			EvtsProxyURL: ti.evtsProxy.RPCServer.GetListenURL(),
			BackupDir:    recorderEventsDir})
		if err != nil {
			log.Errorf("failed to create recorder, err: %v", err)
			return
		}

		// record events
		for i := range recordEvents {
			if objRef, ok := recordEvents[i].objRef.(cluster.Node); ok {
				objRef.ObjectMeta.Name = CreateAlphabetString(5)
				recordEvents[i].objRef = &objRef
			}
			for j := 0; j < recordEvents[i].repeat; j++ {
				evtsRecorder.Event(recordEvents[i].eventType, recordEvents[i].severity, recordEvents[i].message, recordEvents[i].objRef)
			}
		}

		// wait for the batch interval
		time.Sleep(ti.batchInterval + 10*time.Millisecond)
		// resend the events again after batch interval, this should increase the hits but not recreate the alerts as per our alert policy
		// thus, no alert export for these events.
		for i := range recordEvents {
			evtsRecorder.Event(recordEvents[i].eventType, recordEvents[i].severity, recordEvents[i].message, recordEvents[i].objRef)
		}
	}()

	for messageCh, expectedMessages := range messages {
		// ensure all the alerts are exported to the given syslog(TCP) server in the respective format.
		go func() {
			for {
				select {
				case msg, ok := <-messageCh:
					if !ok {
						return
					}

					m.Lock()
					for i := 0; i < len(expectedMessages); i++ {
						if len(expectedMessages[i].Substrs) > 0 {
							match := true
							for _, substr := range expectedMessages[i].Substrs {
								match = match && strings.Contains(msg, substr)
							}
							if match && syslog.ValidateSyslogMessage(expectedMessages[i].MsgFormat, msg) {
								expectedMessages = append(expectedMessages[:i], expectedMessages[i+1:]...)
								break
							}
						}
					}
					m.Unlock()
				}
			}
		}()

		AssertEventually(t,
			func() (bool, interface{}) {
				m.Lock()
				defer m.Unlock()
				return len(expectedMessages) == 0, nil
			}, fmt.Sprintf("did not receive all the expected syslog messages, pending: %v", len(expectedMessages)), "20ms", "10s")
	}

	wg.Wait()
}
