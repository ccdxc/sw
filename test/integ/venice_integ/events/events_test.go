// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package events

import (
	"context"
	"crypto/tls"
	"fmt"
	"io/ioutil"
	"net"
	"net/http"
	"os"
	"regexp"
	"strconv"
	"strings"
	"sync"
	"testing"
	"time"

	es "github.com/olivere/elastic"
	uuid "github.com/satori/go.uuid"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/client"
	"github.com/pensando/sw/api/fields"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/cluster"
	evtsapi "github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/events/generated/eventattrs"
	"github.com/pensando/sw/events/generated/eventtypes"
	testutils "github.com/pensando/sw/test/utils"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/spyglass/finder"
	. "github.com/pensando/sw/venice/utils/authn/testutils"
	"github.com/pensando/sw/venice/utils/events"
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

// TestEvents tests events pipeline
// 1. record events using recorder.
// 2. verify the events reached elastic through the pipeline (proxy->writer->evtsmgr->elastic) using elastic client.
func TestEvents(t *testing.T) {
	ti := tInfo{}
	AssertOk(t, ti.setup(t), "failed to setup test")
	defer ti.teardown()

	// uuid to make each source unique
	componentID := uuid.NewV4().String()

	// create recorder events directory
	recorderEventsDir, err := ioutil.TempDir("", "")
	AssertOk(t, err, "failed to create recorder events directory")
	defer os.RemoveAll(recorderEventsDir)

	// create recorder
	evtsRecorder, err := recorder.NewRecorder(&recorder.Config{
		Component:                   componentID,
		EvtsProxyURL:                ti.evtProxyServices.EvtsProxy.RPCServer.GetListenURL(),
		BackupDir:                   recorderEventsDir,
		SkipCategoryBasedEventTypes: true}, ti.logger)
	AssertOk(t, err, "failed to create events recorder")
	defer evtsRecorder.Close()

	// send events  (recorder -> proxy -> dispatcher -> writer -> evtsmgr -> elastic)
	evtsRecorder.Event(eventtypes.SERVICE_STARTED, "test event - 1", nil)
	evtsRecorder.Event(eventtypes.SERVICE_RUNNING, "test event - 2", nil)

	// verify that it has reached elasticsearch; these are the first occurrences of an event
	// so it should have reached elasticsearch without being de-duped.
	query := es.NewBoolQuery().Must(es.NewMatchQuery("source.component", componentID),
		es.NewTermQuery("type.keyword", eventtypes.SERVICE_STARTED.String()))
	ti.assertElasticUniqueEvents(t, query, true, 1, "4s") // unique == 1
	ti.assertElasticTotalEvents(t, query, true, 1, "4s")  // total  == 1
	query = es.NewBoolQuery().Must(es.NewMatchQuery("source.component", componentID),
		es.NewMatchQuery("message", "test event -2").Operator("and"))
	ti.assertElasticUniqueEvents(t, query, true, 1, "4s") // unique == 1
	ti.assertElasticTotalEvents(t, query, true, 1, "4s")  // total == 1

	// send duplicates and check whether they're compressed
	numDuplicates := 25
	for i := 0; i < numDuplicates; i++ {
		evtsRecorder.Event(eventtypes.SERVICE_STARTED, "test dup event - 1", nil)
		evtsRecorder.Event(eventtypes.SERVICE_RUNNING, "test dup event - 2", nil)
	}

	// ensure the de-duped events reached elasticsearch
	// test duplicate event - 1
	query = es.NewBoolQuery().Must(es.NewMatchQuery("source.component", componentID),
		es.NewMatchQuery("message", "test dup event - 1").Operator("and"))
	ti.assertElasticUniqueEvents(t, query, true, 1, "4s")            // unique == 1
	ti.assertElasticTotalEvents(t, query, true, numDuplicates, "2s") // total == numDuplicates

	// test duplicate event - 2
	query = es.NewBoolQuery().Must(es.NewMatchQuery("source.component", componentID),
		es.NewMatchQuery("message", "test dup event - 2").Operator("and"))
	ti.assertElasticUniqueEvents(t, query, true, 1, "4s")            // unique == 1
	ti.assertElasticTotalEvents(t, query, true, numDuplicates, "2s") // total == numDuplicates

	// create test NIC object
	testNIC := policygen.CreateSmartNIC("00-14-22-01-23-45",
		cluster.DistributedServiceCardStatus_ADMITTED.String(),
		"esx-1",
		&cluster.DSCCondition{
			Type:   cluster.DSCCondition_HEALTHY.String(),
			Status: cluster.ConditionStatus_FALSE.String(),
		})

	// record events with reference object
	for i := 0; i < numDuplicates; i++ {
		evtsRecorder.Event(eventtypes.SERVICE_STARTED, "test dup event - 1", testNIC)
		evtsRecorder.Event(eventtypes.SERVICE_RUNNING, "test dup event - 2", testNIC)
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
			evtsRecorder, err := recorder.NewRecorder(&recorder.Config{
				Component:                   fmt.Sprintf("%v-%v", componentID, i),
				EvtsProxyURL:                ti.evtProxyServices.EvtsProxy.RPCServer.GetListenURL(),
				BackupDir:                   recorderEventsDir,
				SkipCategoryBasedEventTypes: true}, ti.logger)
			if err != nil {
				log.Errorf("failed to create recorder for source %v", i)
				return
			}
			ti.recorders.Lock()
			ti.recorders.list = append(ti.recorders.list, evtsRecorder)
			ti.recorders.Unlock()

			ticker := time.NewTicker(100 * time.Millisecond)
			for {
				select {
				case <-stopEventRecorders:
					wg.Done()
					return
				case <-ticker.C:
					evtsRecorder.Event(eventtypes.SERVICE_STARTED, "test event - 1", nil)
					totalEventsSentBySrc[i]++

					evtsRecorder.Event(eventtypes.SERVICE_RUNNING, "test event - 2", nil)
					totalEventsSentBySrc[i]++

					evtsRecorder.Event(eventtypes.SERVICE_STOPPED, "test event - 3", nil)
					totalEventsSentBySrc[i]++
				}
			}
		}(i)
	}

	// restart events proxy
	go func() {
		proxyURL := ti.evtProxyServices.EvtsProxy.RPCServer.GetListenURL()

		// try restarting events proxy multiple times and make sure the events pipeline is intact
		// and the events are delivered to elastic
		for i := 0; i < 3; i++ {
			time.Sleep(1 * time.Second)
			evtsProxyURL := ti.evtProxyServices.EvtsProxy.RPCServer.GetListenURL()
			ti.evtProxyServices.Stop()
			ti.removeResolverEntry(globals.EvtsProxy, evtsProxyURL)

			// proxy won't be able to accept any events for 2s
			time.Sleep(1 * time.Second)
			evtProxyServices, evtsProxyURL, storeConfig, err := testutils.StartEvtsProxy(t.Name(), proxyURL, ti.mockResolver, ti.logger, ti.dedupInterval, ti.batchInterval, ti.storeConfig)
			if err != nil {
				log.Errorf("failed to start events proxy, err: %v", err)
				continue
			}
			ti.evtProxyServices = evtProxyServices
			ti.storeConfig = storeConfig
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
	ti.assertElasticUniqueEvents(t, query, false, 3*numRecorders, "120s") // minimum of (3 event types * numRecorders = unique events)
	ti.assertElasticTotalEvents(t, query, false, totalEventsSent, "120s") // there can be duplicates because of proxy restarts; so check for received >= sent
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
			evtsRecorder, err := recorder.NewRecorder(&recorder.Config{
				Component:                   fmt.Sprintf("%v-%v", componentID, i),
				EvtsProxyURL:                ti.evtProxyServices.EvtsProxy.RPCServer.GetListenURL(),
				BackupDir:                   recorderEventsDir,
				SkipCategoryBasedEventTypes: true}, ti.logger)
			if err != nil {
				log.Errorf("failed to create recorder for source %v", i)
				return
			}
			ti.recorders.Lock()
			ti.recorders.list = append(ti.recorders.list, evtsRecorder)
			ti.recorders.Unlock()

			ticker := time.NewTicker(100 * time.Millisecond)
			for {
				select {
				case <-stopEventRecorders:
					wg.Done()
					return
				case <-ticker.C:
					evtsRecorder.Event(eventtypes.SERVICE_STARTED, "test event - 1", nil)
					totalEventsSentBySrc[i]++

					evtsRecorder.Event(eventtypes.SERVICE_RUNNING, "test event - 2", nil)
					totalEventsSentBySrc[i]++

					evtsRecorder.Event(eventtypes.SERVICE_STOPPED, "test event - 3", nil)
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

			// exporters should be able to release all the holding events from the buffer
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

	//total events sent by all the recorders
	totalEventsSent := 0
	for _, val := range totalEventsSentBySrc {
		totalEventsSent += val
	}

	log.Infof("total events sent: %v", totalEventsSent)

	// total number of events received at elastic should match the total events sent
	// query all the events received from this source.component
	query := es.NewRegexpQuery("source.component.keyword", fmt.Sprintf("%v-.*", componentID))
	ti.assertElasticUniqueEvents(t, query, true, 3*numRecorders, "60s")
	ti.assertElasticTotalEvents(t, query, false, totalEventsSent, "60s")
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
	ti.updateResolver(globals.Spyglass, fdrAddr)

	// API gateway
	apiGw, apiGwAddr, err := testutils.StartAPIGateway(":0", false,
		map[string]string{}, []string{"telemetry_query", "objstore", "tokenauth"}, []string{}, ti.mockResolver, ti.logger)
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
	MustCreateTenant(ti.apiClient, testTenant)
	defer MustDeleteTenant(ti.apiClient, testTenant)
	MustCreateTestUser(ti.apiClient, testutils.TestLocalUser, testutils.TestLocalPassword, testTenant)
	defer MustDeleteUser(ti.apiClient, testutils.TestLocalUser, testTenant)
	MustUpdateRoleBinding(ti.apiClient, globals.AdminRoleBinding, testTenant, globals.AdminRole, []string{testutils.TestLocalUser}, nil)
	defer MustUpdateRoleBinding(ti.apiClient, globals.AdminRoleBinding, testTenant, globals.AdminRole, nil, nil)
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
		eventType eventtypes.EventType
		message   string
		objRef    interface{}
	}{
		{eventtypes.SERVICE_STARTED, fmt.Sprintf("(tenant:%s) test %s started", globals.DefaultTenant, t.Name()), nil},
		{eventtypes.SERVICE_RUNNING, fmt.Sprintf("(tenant:%s) test %s running", globals.DefaultTenant, t.Name()), nil},
		{eventtypes.SERVICE_UNRESPONSIVE, fmt.Sprintf("(tenant:%s) test %s unresponsive", globals.DefaultTenant, t.Name()), nil},
		{eventtypes.SERVICE_STOPPED, fmt.Sprintf("(tenant:%s) test %s stopped", globals.DefaultTenant, t.Name()), nil},

		// events in non default tenant
		{eventtypes.SERVICE_STARTED, fmt.Sprintf("(tenant:%s) test %s started", testTenant, t.Name()), dummyObjRef},
		{eventtypes.SERVICE_RUNNING, fmt.Sprintf("(tenant:%s) test %s running", testTenant, t.Name()), dummyObjRef},
		{eventtypes.SERVICE_UNRESPONSIVE, fmt.Sprintf("(tenant:%s) test %s unresponsive", testTenant, t.Name()), dummyObjRef},
		{eventtypes.SERVICE_STOPPED, fmt.Sprintf("(tenant:%s) test %s stopped", testTenant, t.Name()), dummyObjRef},
	}

	wg := new(sync.WaitGroup)
	wg.Add(1)

	// start recorder
	recorderEventsDir, err := ioutil.TempDir("", "")
	AssertOk(t, err, "failed to create recorder events directory")
	defer os.RemoveAll(recorderEventsDir)
	go func() {
		defer wg.Done()

		evtsRecorder, err := recorder.NewRecorder(&recorder.Config{
			Component:                   uuid.NewV4().String(),
			EvtsProxyURL:                ti.evtProxyServices.EvtsProxy.RPCServer.GetListenURL(),
			BackupDir:                   recorderEventsDir,
			SkipCategoryBasedEventTypes: true}, ti.logger)
		if err != nil {
			log.Errorf("failed to create recorder")
			return
		}
		ti.recorders.Lock()
		ti.recorders.list = append(ti.recorders.list, evtsRecorder)
		ti.recorders.Unlock()

		// record events
		for _, evt := range recordEvents {
			evtsRecorder.Event(evt.eventType, evt.message, evt.objRef)
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
		{ // GET all events ; should match 4 events
			name:          "GET all events from default tenant; should match 4 events",
			authzHdr:      authzHeader,
			requestURI:    "events",
			requestBody:   &api.ListWatchOptions{}, // default max-results to 1000
			expStatusCode: http.StatusOK,
			expResponse: &expectedResponse{
				numEvents: 4,
				events: map[string]*evtsapi.Event{
					fmt.Sprintf("(tenant:%s) test %s started", globals.DefaultTenant, t.Name()): {
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventtypes.SERVICE_STARTED.String(),
							Severity: eventattrs.Severity_DEBUG.String(),
							Count:    1,
						},
					},
					fmt.Sprintf("(tenant:%s) test %s running", globals.DefaultTenant, t.Name()): {
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventtypes.SERVICE_RUNNING.String(),
							Severity: eventattrs.Severity_DEBUG.String(),
							Count:    1,
						},
					},
					fmt.Sprintf("(tenant:%s) test %s unresponsive", globals.DefaultTenant, t.Name()): {
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventtypes.SERVICE_UNRESPONSIVE.String(),
							Severity: eventattrs.Severity_CRITICAL.String(),
							Count:    1,
						},
					},
					fmt.Sprintf("(tenant:%s) test %s stopped", globals.DefaultTenant, t.Name()): {
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventtypes.SERVICE_STOPPED.String(),
							Severity: eventattrs.Severity_WARN.String(),
							Count:    1,
						},
					},
				},
			},
		},
		{ // GET events with severity = DEBUG; should match 2 events (1 * SERVICE_RUNNING + 1 * SERVICE_STARTED)
			name:          "GET events from default tenant with severity = DEBUG; should match 2 events",
			authzHdr:      authzHeader,
			requestURI:    "events",
			requestBody:   &api.ListWatchOptions{FieldSelector: fmt.Sprintf("severity=%s", eventattrs.Severity_DEBUG), MaxResults: 100},
			expStatusCode: http.StatusOK,
			expResponse: &expectedResponse{
				numEvents: 2,
				events: map[string]*evtsapi.Event{
					fmt.Sprintf("(tenant:%s) test %s started", globals.DefaultTenant, t.Name()): {
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventtypes.SERVICE_STARTED.String(),
							Severity: eventattrs.Severity_DEBUG.String(),
							Count:    1,
						},
					},
					fmt.Sprintf("(tenant:%s) test %s running", globals.DefaultTenant, t.Name()): {
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventtypes.SERVICE_RUNNING.String(),
							Severity: eventattrs.Severity_DEBUG.String(),
							Count:    1,
						},
					},
				},
			},
		},
		{ // GET events with severity = "CRITICAL" and source.node-name="TestEventsRESTEndpoints"; should match 1 events (1 * SERVICE_UNRESPONSIVE)
			name:          "GET events from default tenant with severity = 'CRITICAL' and source.node-name='TestEventsRESTEndpoints'; should match 2 events",
			authzHdr:      authzHeader,
			requestURI:    "events",
			requestBody:   &api.ListWatchOptions{FieldSelector: fmt.Sprintf("severity=%s,source.node-name=%s", eventattrs.Severity_CRITICAL, t.Name()), MaxResults: 100},
			expStatusCode: http.StatusOK,
			expResponse: &expectedResponse{
				numEvents: 1,
				events: map[string]*evtsapi.Event{
					fmt.Sprintf("(tenant:%s) test %s unresponsive", globals.DefaultTenant, t.Name()): {
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventtypes.SERVICE_UNRESPONSIVE.String(),
							Severity: eventattrs.Severity_CRITICAL.String(),
							Count:    1,
						},
					},
				},
			},
		},
		{ // Get events with severity = "CRITICAL" and type in ("SERVICE_UNRESPONSIVE") and tenant="default"; should match only one event
			name:       "Get events from default tenant with severity = 'CRITICAL' and type in ('SERVICE_UNRESPONSIVE'); should match only one event",
			authzHdr:   authzHeader,
			requestURI: "events",
			requestBody: &api.ListWatchOptions{FieldSelector: fmt.Sprintf("severity=%s,type in (%s),meta.tenant=%s",
				eventattrs.Severity_CRITICAL, eventtypes.SERVICE_UNRESPONSIVE.String(), globals.DefaultTenant), MaxResults: 100},
			expStatusCode: http.StatusOK,
			expResponse: &expectedResponse{
				numEvents: 1,
				events: map[string]*evtsapi.Event{
					fmt.Sprintf("(tenant:%s) test %s unresponsive", globals.DefaultTenant, t.Name()): {
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventtypes.SERVICE_UNRESPONSIVE.String(),
							Severity: eventattrs.Severity_CRITICAL.String(),
							Count:    1,
						},
					},
				},
			},
		},
		{ // GET events from timeNow to time.Now()+ 100s; should match ALL(4) events
			name:          "GET events from timeNow to time.Now()+ 100s; should match ALL(4) events",
			authzHdr:      authzHeader,
			requestURI:    "events",
			requestBody:   &api.ListWatchOptions{FieldSelector: fmt.Sprintf("meta.creation-time>=%v,meta.creation-time<=%v", timeNow.Format(time.RFC3339Nano), time.Now().Add(100*time.Second).Format(time.RFC3339Nano)), MaxResults: 100},
			expStatusCode: http.StatusOK,
			expResponse: &expectedResponse{
				numEvents: 4,
				events: map[string]*evtsapi.Event{
					fmt.Sprintf("(tenant:%s) test %s started", globals.DefaultTenant, t.Name()): {
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventtypes.SERVICE_STARTED.String(),
							Severity: eventattrs.Severity_DEBUG.String(),
							Count:    1,
						},
					},
					fmt.Sprintf("(tenant:%s) test %s running", globals.DefaultTenant, t.Name()): {
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventtypes.SERVICE_RUNNING.String(),
							Severity: eventattrs.Severity_DEBUG.String(),
							Count:    1,
						},
					},
					fmt.Sprintf("(tenant:%s) test %s unresponsive", globals.DefaultTenant, t.Name()): {
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventtypes.SERVICE_UNRESPONSIVE.String(),
							Severity: eventattrs.Severity_CRITICAL.String(),
							Count:    1,
						},
					},
					fmt.Sprintf("(tenant:%s) test %s stopped", globals.DefaultTenant, t.Name()): {
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventtypes.SERVICE_STOPPED.String(),
							Severity: eventattrs.Severity_WARN.String(),
							Count:    1,
						},
					},
				},
			},
		},
		{ // Get events with severity="CRITICAL" and creation-time>timeNow
			name:       "Get events from default tenant with severity='CRITICAL' and creation-time>timeNow; should match one",
			authzHdr:   authzHeader,
			requestURI: "events",
			requestBody: &api.ListWatchOptions{FieldSelector: fmt.Sprintf("severity=%s,meta.creation-time>%v",
				eventattrs.Severity_CRITICAL, timeNow.Format(time.RFC3339Nano)), MaxResults: 100},
			expStatusCode: http.StatusOK,
			expResponse: &expectedResponse{
				numEvents: 1,
				events: map[string]*evtsapi.Event{
					fmt.Sprintf("(tenant:%s) test %s unresponsive", globals.DefaultTenant, t.Name()): {
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventtypes.SERVICE_UNRESPONSIVE.String(),
							Severity: eventattrs.Severity_CRITICAL.String(),
							Count:    1,
						},
					},
				},
			},
		},
		{ // Get events with severity="CRITICAL" and type in ("SERVICE_UNRESPONSIVE") and creation-time<=timeNow
			name:       "Get events from default tenant with severity='CRITICAL' and creation-time<=timeNow; should match none",
			authzHdr:   authzHeader,
			requestURI: "events",
			requestBody: &api.ListWatchOptions{FieldSelector: fmt.Sprintf("severity=%s,type in (%s),meta.creation-time<=%v",
				eventattrs.Severity_CRITICAL, eventtypes.SERVICE_UNRESPONSIVE.String(), timeNow.Format(time.RFC3339Nano)), MaxResults: 100},
			expStatusCode: http.StatusOK,
			expResponse: &expectedResponse{
				numEvents: 0,
				events:    map[string]*evtsapi.Event{},
			},
		},
		{ // Get events with type in (SERVICE_RUNNING,SERVICE_STARTED),source.node-name notin (TestEventsRESTEndpoints); should match none
			name:       "Get events from default tenant with type in (SERVICE_RUNNING,SERVICE_STARTED),source.node-name notin (TestEventsRESTEndpoints); should match none",
			authzHdr:   authzHeader,
			requestURI: "events",
			requestBody: &api.ListWatchOptions{FieldSelector: fmt.Sprintf("type in (%s,%s),source.node-name notin (%s)",
				eventtypes.SERVICE_RUNNING.String(), eventtypes.SERVICE_STARTED.String(), t.Name()), MaxResults: 100},
			expStatusCode: http.StatusOK,
			expResponse: &expectedResponse{
				numEvents: 0,
				events:    map[string]*evtsapi.Event{},
			},
		},
		{ // GET events with severity="TEST"; should match none
			name:          "GET events from default tenant with severity='TEST'; should match none",
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
			name:          "GET events from default tenant with source.component='test 'and type= 'test'; should match none",
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
			name:       "Get events from default tenant with invalid field names",
			authzHdr:   authzHeader,
			requestURI: "events",
			requestBody: &api.ListWatchOptions{FieldSelector: fmt.Sprintf("invalid-field=%s,type in (%s)",
				eventattrs.Severity_CRITICAL, eventtypes.SERVICE_UNRESPONSIVE.String()), MaxResults: 100},
			expStatusCode: http.StatusInternalServerError,
			expResponse: &expectedResponse{
				numEvents: 0,
				events:    map[string]*evtsapi.Event{},
			},
		},
		{ // Get events with invalid field names
			name:          "Get events from default tenant with invalid field names",
			authzHdr:      authzHeader,
			requestURI:    "events",
			requestBody:   &api.ListWatchOptions{FieldSelector: fmt.Sprintf("meta.invalid<=%v", timeNow.Format(time.RFC3339Nano)), MaxResults: 100},
			expStatusCode: http.StatusInternalServerError,
			expResponse: &expectedResponse{
				numEvents: 0,
				events:    map[string]*evtsapi.Event{},
			},
		},
		{ // //TODO: check with Vishal, this doesn't look right. GET events from testtenant; should match 4 events
			name:          "GET events from testtenant; should match 4 events",
			authzHdr:      authzHeader,
			requestURI:    "events",
			requestBody:   &api.ListWatchOptions{ObjectMeta: api.ObjectMeta{Tenant: testTenant, Namespace: globals.DefaultNamespace}}, // default max-results to 1000
			expStatusCode: http.StatusOK,
			expResponse: &expectedResponse{
				numEvents: 4,
				events: map[string]*evtsapi.Event{
					fmt.Sprintf("(tenant:%s) test %s started", testTenant, t.Name()): {
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventtypes.SERVICE_STARTED.String(),
							Severity: eventattrs.Severity_DEBUG.String(),
							Count:    1,
						},
					},
					fmt.Sprintf("(tenant:%s) test %s running", testTenant, t.Name()): {
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventtypes.SERVICE_RUNNING.String(),
							Severity: eventattrs.Severity_DEBUG.String(),
							Count:    1,
						},
					},
					fmt.Sprintf("(tenant:%s) test %s unresponsive", testTenant, t.Name()): {
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventtypes.SERVICE_UNRESPONSIVE.String(),
							Severity: eventattrs.Severity_CRITICAL.String(),
							Count:    1,
						},
					},
					fmt.Sprintf("(tenant:%s) test %s stopped", testTenant, t.Name()): {
						EventAttributes: evtsapi.EventAttributes{
							Type:     eventtypes.SERVICE_STOPPED.String(),
							Severity: eventattrs.Severity_WARN.String(),
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
			httpClient.DisableKeepAlives()
			defer httpClient.CloseIdleConnections()

			// both GET and POST should behave the same
			for _, reqMethod := range []string{"GET", "POST"} {
				AssertEventually(t,
					func() (bool, interface{}) {
						statusCode, _ := httpClient.Req(reqMethod, url, rr.requestBody, &resp)
						if statusCode != rr.expStatusCode || len(resp.GetItems()) != rr.expResponse.numEvents {
							return false, fmt.Sprintf("failed to get expected events for {%s}: %#v", rr.name, *rr.requestBody)
						}

						return true, nil
					}, "failed to get events", "200ms", "6s")

				Assert(t, rr.expResponse.numEvents == len(resp.GetItems()), "failed to get expected number of events")

				// verity resp against the expected response
				for _, obtainedEvt := range resp.GetItems() {
					expectedEvt, ok := rr.expResponse.events[obtainedEvt.GetMessage()]
					Assert(t, ok, "{%s} obtained event is not in the expected list: %s", rr.name, obtainedEvt.GetMessage())
					Assert(t, expectedEvt.GetType() == obtainedEvt.GetType(), "{%s} expected event message: %s, got: %s", rr.name, expectedEvt.GetType(), obtainedEvt.GetType())
					Assert(t, expectedEvt.GetSeverity() == obtainedEvt.GetSeverity(), "{%s} expected event severity: %s, got: %s", rr.name, expectedEvt.GetSeverity(), obtainedEvt.GetSeverity())
					Assert(t, expectedEvt.GetCount() == obtainedEvt.GetCount(), "{%s} expected event count: %d, got: %d", rr.name, expectedEvt.GetCount(), obtainedEvt.GetCount())

					// make sure self-link works
					selfLink := obtainedEvt.GetSelfLink()
					evt := evtsapi.Event{}
					statusCode, err := httpClient.Req("GET", fmt.Sprintf("https://%s/%s", apiGwAddr, selfLink), nil, &evt)
					Assert(t, err == nil && statusCode == http.StatusOK, "{%s} failed to get the event using self-link: %v, status: %v, err: %v", rr.name, selfLink, statusCode, err)
					Assert(t, evt.GetUUID() == obtainedEvt.GetUUID(), "{%s} obtained: %v, expected: %v", rr.name, evt.GetUUID(), obtainedEvt.GetUUID())

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
			httpClient.DisableKeepAlives()
			defer httpClient.CloseIdleConnections()

			AssertEventually(t,
				func() (bool, interface{}) {
					statusCode, err := httpClient.Req("GET", url, rr.requestBody, &resp)
					if err != nil || statusCode != http.StatusOK {
						return false, nil
					}

					return true, nil
				}, "failed to get events", "200ms", "6s")

			Assert(t, resp.GetUUID() != "", "{%s} failed to get event by UUID: %v", rr.name, rr.requestURI)

			// make sure self-link works
			selfLink := resp.GetSelfLink()
			evt := evtsapi.Event{}
			statusCode, err := httpClient.Req("GET", fmt.Sprintf("https://%s/%s", apiGwAddr, selfLink), nil, &evt)
			Assert(t, err == nil && statusCode == http.StatusOK, "{%s} failed to get the event using self-link: %v, status: %v, err: %v", rr.name, selfLink, statusCode, err)
			Assert(t, evt.GetUUID() == resp.GetUUID(), "{%s} obtained: %v, expected: %v", rr.name, evt.GetUUID(), resp.GetUUID())
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

	// start spyglass (backend service for events)
	fdrTemp, fdrAddr, err := testutils.StartSpyglass("finder", "", ti.mockResolver, nil, ti.logger, ti.esClient)
	AssertOk(t, err, "failed to start spyglass finder, err: %v", err)
	fdr := fdrTemp.(finder.Interface)
	defer fdr.Stop()
	ti.updateResolver(globals.Spyglass, fdrAddr)

	// API gateway
	apiGw, apiGwAddr, err := testutils.StartAPIGateway(":0", false,
		map[string]string{}, []string{"telemetry_query", "objstore", "tokenauth"}, []string{}, ti.mockResolver, ti.logger)
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
		"Event", eventattrs.Severity_CRITICAL, "critical alerts from events",
		[]*fields.Requirement{
			{Key: "count", Operator: "gte", Values: []string{"15"}},
			{Key: "source.node-name", Operator: "equals", Values: []string{t.Name()}},
		}, []string{})

	alertPolicy1, err = ti.apiClient.MonitoringV1().AlertPolicy().Create(context.Background(), alertPolicy1)
	AssertOk(t, err, "failed to add alert policy{ap1-*}, err: %v", err)

	// policy - 2
	alertPolicy2 := policygen.CreateAlertPolicyObj(globals.DefaultTenant, globals.DefaultNamespace, fmt.Sprintf("ap2-%s", uuid.NewV4().String()),
		"Event", eventattrs.Severity_WARN, "warning alerts from events",
		[]*fields.Requirement{
			{Key: "count", Operator: "gte", Values: []string{"5"}},
			{Key: "count", Operator: "lt", Values: []string{"7"}},
			{Key: "severity", Operator: "equals", Values: []string{
				eventattrs.Severity_DEBUG.String(),
				eventattrs.Severity_WARN.String(),
				eventattrs.Severity_INFO.String()}},
		}, []string{})

	alertPolicy2, err = ti.apiClient.MonitoringV1().AlertPolicy().Create(context.Background(), alertPolicy2)
	AssertOk(t, err, "failed to add alert policy{ap2-*}, err: %v", err)

	alertPolicy3 := policygen.CreateAlertPolicyObj(globals.DefaultTenant, globals.DefaultNamespace, fmt.Sprintf("ap3-%s", uuid.NewV4().String()),
		"Event", eventattrs.Severity_WARN, "policy with no reqs", []*fields.Requirement{}, []string{})
	alertPolicy3, err = ti.apiClient.MonitoringV1().AlertPolicy().Create(context.Background(), alertPolicy3)
	AssertOk(t, err, "failed to add alert policy, err: %v", err)

	defer func() {
		err := ti.cleanupPolicies()
		AssertOk(t, err, "failed to cleanup policies")
	}()

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
		eventType eventtypes.EventType
		message   string
		objRef    interface{}
		repeat    int // number of times to repeat the event
	}{
		{eventtypes.SERVICE_STARTED, fmt.Sprintf("(tenant:%s) test %s started", dummyObjRef.Tenant, t.Name()), *dummyObjRef, 10},
		{eventtypes.SERVICE_RUNNING, fmt.Sprintf("(tenant:%s) test %s running", dummyObjRef.Tenant, t.Name()), *dummyObjRef, 10},
		{eventtypes.SERVICE_UNRESPONSIVE, fmt.Sprintf("(tenant:%s) test %s unresponsive", dummyObjRef.Tenant, t.Name()), *dummyObjRef, 15}, // this should generate an alert (alertPolicy1)
		{eventtypes.SERVICE_STOPPED, fmt.Sprintf("(tenant:%s) test %s stopped", dummyObjRef.Tenant, t.Name()), *dummyObjRef, 5},            // this should generate an alert (alertPolicy2)

		{eventtypes.ELECTION_STARTED, fmt.Sprintf("(tenant:%s) dummy election: election started %s", dummyObjRef.Tenant, t.Name()), *dummyObjRef, 10},
		{eventtypes.LEADER_ELECTED, fmt.Sprintf("(tenant:%s) dummy election: leader elected %s", dummyObjRef.Tenant, t.Name()), *dummyObjRef, 10},
		{eventtypes.LEADER_CHANGED, fmt.Sprintf("(tenant:%s) dummy election: leader changed %s", dummyObjRef.Tenant, t.Name()), *dummyObjRef, 15}, // this should generate an alert (alertPolicy1)
		{eventtypes.LEADER_LOST, fmt.Sprintf("(tenant:%s) dummy election: leader lost %s", dummyObjRef.Tenant, t.Name()), *dummyObjRef, 5},        // this should generate an alert (alertPolicy2)

		// events in non default tenant
		{eventtypes.SERVICE_STARTED, fmt.Sprintf("(tenant:%s) test %s started", globals.DefaultTenant, t.Name()), nil, 10},
		{eventtypes.SERVICE_RUNNING, fmt.Sprintf("(tenant:%s) test %s running", globals.DefaultTenant, t.Name()), nil, 10},
		{eventtypes.SERVICE_UNRESPONSIVE, fmt.Sprintf("(tenant:%s) test %s unresponsive", globals.DefaultTenant, t.Name()), nil, 15}, // this should generate an alert (alertPolicy1)
		{eventtypes.SERVICE_STOPPED, fmt.Sprintf("(tenant:%s) test %s stopped", globals.DefaultTenant, t.Name()), nil, 5},            // this should generate an alert (alertPolicy2)
	}

	wg := new(sync.WaitGroup)
	wg.Add(2)

	// start recorder
	recorderEventsDir, err := ioutil.TempDir("", "")
	AssertOk(t, err, "failed to create recorder events directory")
	defer os.RemoveAll(recorderEventsDir)
	go func() {
		defer wg.Done()

		evtsRecorder, err := recorder.NewRecorder(&recorder.Config{
			Component:                   uuid.NewV4().String(),
			EvtsProxyURL:                ti.evtProxyServices.EvtsProxy.RPCServer.GetListenURL(),
			BackupDir:                   recorderEventsDir,
			SkipCategoryBasedEventTypes: true}, ti.logger)
		if err != nil {
			log.Errorf("failed to create recorder, err: %v", err)
			return
		}
		ti.recorders.Lock()
		ti.recorders.list = append(ti.recorders.list, evtsRecorder)
		ti.recorders.Unlock()

		// record events
		for i := range recordEvents {
			if objRef, ok := recordEvents[i].objRef.(cluster.Node); ok {
				objRef.ObjectMeta.Name = CreateAlphabetString(5)
				recordEvents[i].objRef = &objRef
			}
			for j := 0; j < recordEvents[i].repeat; j++ {
				evtsRecorder.Event(recordEvents[i].eventType, recordEvents[i].message, recordEvents[i].objRef)
			}
		}

		// wait for the batch interval
		time.Sleep(1 * time.Second)
		// if objRef!=nil, this should increase the hits but not recreate the alerts.
		// it will recreate alerts otherwise.
		for i := range recordEvents {
			evtsRecorder.Event(recordEvents[i].eventType, recordEvents[i].message, recordEvents[i].objRef)
		}
	}()

	// TODO: cannot add criteria meta.tenant="default" or any meta.*
	// list of alerts to be generated by the alert engine
	tests := []struct {
		selector   string
		expMessage string // stings will spaces are not allowed in field selector; so, this attribute
		expSuccess bool
	}{
		{
			selector: fmt.Sprintf("status.reason.alert-policy-id=%s/%s,status.severity=%s,status.object-ref.kind=%s,meta.tenant=%s",
				alertPolicy1.GetName(), alertPolicy1.GetUUID(), alertPolicy1.Spec.GetSeverity(), dummyObjRef.GetKind(), dummyObjRef.GetTenant()),
			expMessage: fmt.Sprintf("(tenant:%s) test %s unresponsive", dummyObjRef.Tenant, t.Name()),
			expSuccess: true,
		},
		{
			selector: fmt.Sprintf("status.reason.alert-policy-id=%s/%s,status.severity=%s,status.object-ref.kind=%s,meta.tenant=%s",
				alertPolicy1.GetName(), alertPolicy1.GetUUID(), alertPolicy1.Spec.GetSeverity(), dummyObjRef.GetKind(), dummyObjRef.GetTenant()),
			expMessage: fmt.Sprintf("(tenant:%s) dummy election: leader changed %s", dummyObjRef.Tenant, t.Name()),
			expSuccess: true,
		},
		{
			selector: fmt.Sprintf("status.reason.alert-policy-id=%s/%s,status.severity=%s",
				alertPolicy1.GetName(), alertPolicy1.GetUUID(), alertPolicy1.Spec.GetSeverity()),
			expMessage: fmt.Sprintf("(tenant:%s) test %s unresponsive", globals.DefaultTenant, t.Name()),
			expSuccess: true,
		},
		{
			selector: fmt.Sprintf("status.reason.alert-policy-id=%s/%s,status.severity=%s,status.object-ref.kind=%s,meta.tenant=%s",
				alertPolicy2.GetName(), alertPolicy2.GetUUID(), alertPolicy2.Spec.GetSeverity(), dummyObjRef.GetKind(), dummyObjRef.GetTenant()),
			expMessage: fmt.Sprintf("(tenant:%s) test %s stopped", dummyObjRef.Tenant, t.Name()),
			expSuccess: true,
		},
		{
			selector: fmt.Sprintf("status.reason.alert-policy-id=%s/%s,status.severity=%s,meta.tenant=%s",
				alertPolicy2.GetName(), alertPolicy2.GetUUID(), alertPolicy2.Spec.GetSeverity(), globals.DefaultTenant),
			expMessage: fmt.Sprintf("(tenant:%s) test %s stopped", dummyObjRef.Tenant, t.Name()),
			expSuccess: true,
		},
		{
			selector: fmt.Sprintf("status.reason.alert-policy-id=%s/%s,status.severity=%s,status.object-ref.kind=%s,meta.tenant=%s",
				alertPolicy2.GetName(), alertPolicy2.GetUUID(), alertPolicy2.Spec.GetSeverity(),
				dummyObjRef.GetKind(), dummyObjRef.GetTenant()),
			expMessage: fmt.Sprintf("(tenant:%s) dummy election: leader lost %s", dummyObjRef.Tenant, t.Name()),
			expSuccess: true,
		},
		{
			selector: fmt.Sprintf("status.reason.alert-policy-id=%s/%s,status.severity=%s,status.object-ref.kind=%s",
				alertPolicy2.GetName(), alertPolicy2.GetUUID(), alertPolicy2.Spec.GetSeverity(), "invalid"),
			expMessage: fmt.Sprintf("(tenant:%s) dummy election: leader lost %s", dummyObjRef.Tenant, t.Name()),
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
				alerts, err := ti.apiClient.MonitoringV1().Alert().List(context.Background(),
					&api.ListWatchOptions{
						ObjectMeta:    api.ObjectMeta{Tenant: globals.DefaultTenant},
						FieldSelector: test.selector})
				if err != nil {
					return false, fmt.Sprintf("%v failed, err: %v", test.selector, err)
				}

				if test.expSuccess {
					for _, alert := range alerts {
						if alert.Status.Message == test.expMessage {
							return true, nil
						}
					}
				}

				if !test.expSuccess && len(alerts) == 0 {
					return true, nil
				}

				return false, fmt.Sprintf("expected: %v, obtained: %v", test, alerts)
			}, "did not receive the expected alert", string("200ms"), string("20s"))
		}
	}()

	wg.Wait()

	// make sure the policy status got updated
	expectedAlertStatus := []struct {
		policyMeta         *api.ObjectMeta
		minTotalHits       int32
		maxTotalHits       int32
		openAlerts         int32
		acknowledgedAlerts int32
	}{
		{policyMeta: alertPolicy1.GetObjectMeta(), minTotalHits: 3, maxTotalHits: 6, openAlerts: 3, acknowledgedAlerts: 0},
		{policyMeta: alertPolicy2.GetObjectMeta(), minTotalHits: 3, maxTotalHits: 6, openAlerts: 3, acknowledgedAlerts: 0},
		{policyMeta: alertPolicy3.GetObjectMeta(), minTotalHits: 0, maxTotalHits: 0, openAlerts: 0, acknowledgedAlerts: 0}, // no reqs so, there should be no alerts
	}
	for _, as := range expectedAlertStatus {
		AssertEventually(t, func() (bool, interface{}) {
			res, err := ti.apiClient.MonitoringV1().AlertPolicy().Get(context.Background(),
				&api.ObjectMeta{Name: as.policyMeta.GetName(), Tenant: as.policyMeta.GetTenant(), Namespace: as.policyMeta.GetNamespace(), UUID: as.policyMeta.GetUUID()})
			if err != nil {
				return false, fmt.Sprintf(":%v, err: %v", as.policyMeta.GetName(), err)
			}

			if (res.Status.GetTotalHits() < as.minTotalHits) || (res.Status.GetTotalHits() > as.maxTotalHits) {
				return false, fmt.Sprintf("total hits on policy %v expected total hits to be between (%v, %v) obtained: %v", res.GetObjectMeta().GetName(), as.minTotalHits, as.maxTotalHits, res.Status.GetTotalHits())
			}

			if as.openAlerts != res.Status.GetOpenAlerts() {
				return false, fmt.Sprintf("open alerts on policy %v expected: %v, obtained: %v", res.GetObjectMeta().GetName(), as.openAlerts, res.Status.GetOpenAlerts())
			}

			if as.acknowledgedAlerts != res.Status.GetAcknowledgedAlerts() {
				return false, fmt.Sprintf("acknowledged alerts on policy %v expected: %v, obtained: %v", res.GetObjectMeta().GetName(), as.acknowledgedAlerts, res.Status.GetAcknowledgedAlerts())
			}

			return true, nil
		}, "alert status does not match the expected", string("200ms"), string("10s"))
	}

	// resolve or acknowledge alerts
	alerts, err := ti.apiClient.MonitoringV1().Alert().List(context.Background(),
		&api.ListWatchOptions{
			ObjectMeta: api.ObjectMeta{Tenant: globals.DefaultTenant},
			FieldSelector: fmt.Sprintf("status.reason.alert-policy-id in (%s,%s,%s)",
				fmt.Sprintf("%s/%s", alertPolicy1.GetName(), alertPolicy1.GetUUID()),
				fmt.Sprintf("%s/%s", alertPolicy2.GetName(), alertPolicy2.GetUUID()),
				fmt.Sprintf("%s/%s", alertPolicy3.GetName(), alertPolicy3.GetUUID())),
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
		apURL := fmt.Sprintf("https://%s/configs/monitoring/v1/alertPolicies/%s", apiGwAddr,
			strings.Split(at.alert.Status.Reason.GetPolicyID(), "/")[0])

		httpClient := netutils.NewHTTPClient()
		httpClient.WithTLSConfig(&tls.Config{InsecureSkipVerify: true})
		httpClient.SetHeader("Authorization", authzHeader)
		httpClient.DisableKeepAlives()
		defer httpClient.CloseIdleConnections()

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
					at.alert.Spec.State = monitoring.AlertState_ACKNOWLEDGED.String()
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
				}, "failed to update alert state", "200ms", "6s")
		} else if at.resolve {
			resp := monitoring.Alert{}
			AssertEventually(t,
				func() (bool, interface{}) {
					at.alert.Spec.State = monitoring.AlertState_RESOLVED.String()
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
				}, "failed to update alert state", "200ms", "6s")
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

// TestEventsAlertEngineWithAPIServerShutdown shuts down the API server and ensures alert engine behaves as expected.
func TestEventsAlertEngineWithAPIServerShutdown(t *testing.T) {
	// define list of events to be recorded
	dummyObjRef := &cluster.Node{
		TypeMeta: api.TypeMeta{
			Kind: "Node",
		},
		ObjectMeta: api.ObjectMeta{
			Name:      CreateAlphabetString(5),
			Tenant:    globals.DefaultTenant,
			Namespace: globals.DefaultNamespace,
		},
	}
	recordEvents := []*struct {
		eventType eventtypes.EventType
		message   string
		objRef    interface{}
	}{
		{eventtypes.SERVICE_STARTED, fmt.Sprintf("(tenant:%s) test %s started on %s", dummyObjRef.Tenant, t.Name(), dummyObjRef.GetKind()), dummyObjRef},
		{eventtypes.SERVICE_RUNNING, fmt.Sprintf("(tenant:%s) test %s running on %s", dummyObjRef.Tenant, t.Name(), dummyObjRef.GetKind()), dummyObjRef},
		{eventtypes.SERVICE_UNRESPONSIVE, fmt.Sprintf("(tenant:%s) test %s unresponsive on %s", dummyObjRef.Tenant, t.Name(), dummyObjRef.GetKind()), dummyObjRef},
		{eventtypes.SERVICE_STOPPED, fmt.Sprintf("(tenant:%s) test %s stopped on %s", dummyObjRef.Tenant, t.Name(), dummyObjRef.GetKind()), dummyObjRef},
		{eventtypes.NAPLES_SERVICE_STOPPED, fmt.Sprintf("(tenant:%s) test %s naples service stopped", dummyObjRef.Tenant, t.Name()), dummyObjRef},
	}

	// setup events pipeline to record and distribute events
	ti := tInfo{batchInterval: 100 * time.Millisecond, dedupInterval: 100 * time.Second}
	AssertOk(t, ti.setup(t), "failed to setup test")
	defer ti.teardown()

	// start spyglass (apigw needs spyglass)
	fdrTemp, fdrAddr, err := testutils.StartSpyglass("finder", "", ti.mockResolver, nil, ti.logger, ti.esClient)
	AssertOk(t, err, "failed to start spyglass finder, err: %v", err)
	fdr := fdrTemp.(finder.Interface)
	defer fdr.Stop()
	ti.updateResolver(globals.Spyglass, fdrAddr)

	// API gateway
	apiGw, _, err := testutils.StartAPIGateway(":0", false,
		map[string]string{}, []string{"telemetry_query", "objstore", "tokenauth"}, []string{}, ti.mockResolver, ti.logger)
	AssertOk(t, err, "failed to start API gateway, err: %v", err)
	defer apiGw.Stop()

	// setup authn and get authz token
	userCreds := &auth.PasswordCredential{Username: testutils.TestLocalUser, Password: testutils.TestLocalPassword, Tenant: testutils.TestTenant}
	err = testutils.SetupAuth(ti.apiServerAddr, true, &auth.Ldap{Enabled: false}, &auth.Radius{Enabled: false}, userCreds, ti.logger)
	AssertOk(t, err, "failed to setup authN service, err: %v", err)
	defer testutils.CleanupAuth(ti.apiServerAddr, true, false, userCreds, ti.logger)

	// add event based alert policies
	// policy - 1 (convert critical events to alerts)
	alertPolicy1 := policygen.CreateAlertPolicyObj(globals.DefaultTenant, globals.DefaultNamespace, fmt.Sprintf("ap1-%s", uuid.NewV4().String()),
		"Event", eventattrs.Severity_CRITICAL, "critical alerts from events",
		[]*fields.Requirement{
			{Key: "severity", Operator: "equals", Values: []string{eventattrs.Severity_CRITICAL.String()}},
		}, []string{})
	alertPolicy1, err = ti.apiClient.MonitoringV1().AlertPolicy().Create(context.Background(), alertPolicy1)
	errStatus, ok := status.FromError(err)
	Assert(t, ok, "did not receive a proper http response from apiclient (add alert policy)")
	Assert(t, errStatus.Code() == codes.OK, "failed to add alert policy{ap1-*}, err: %+v", errStatus)
	defer ti.cleanupPolicies()

	// total alerts should be 0
	alerts, err := ti.apiClient.MonitoringV1().Alert().List(context.Background(), &api.ListWatchOptions{
		FieldSelector: fmt.Sprintf("status.reason.alert-policy-id=%s/%s", alertPolicy1.GetName(), alertPolicy1.GetUUID()),
	})
	AssertOk(t, err, "failed to list alerts")
	Assert(t, len(alerts) == 0, "expected: 0, got: %v alerts", len(alerts))

	// create events recorder
	recorderEventsDir, err := ioutil.TempDir("", "")
	AssertOk(t, err, "failed to create recorder events directory")
	defer os.RemoveAll(recorderEventsDir)
	evtsRecorder, err := recorder.NewRecorder(&recorder.Config{
		Component:                   uuid.NewV4().String(),
		EvtsProxyURL:                ti.evtProxyServices.EvtsProxy.RPCServer.GetListenURL(),
		BackupDir:                   recorderEventsDir,
		SkipCategoryBasedEventTypes: true}, ti.logger)
	AssertOk(t, err, "failed to create recorder")
	ti.recorders.Lock()
	ti.recorders.list = append(ti.recorders.list, evtsRecorder)
	ti.recorders.Unlock()

	// stop apiserver
	ti.stopAPIServer()

	// restart evtsmgr
	evtsMgrURL := ti.evtsMgr.RPCServer.GetListenURL()
	ti.evtsMgr.Stop()

	ti.evtsMgr, evtsMgrURL, err = testutils.StartEvtsMgr(evtsMgrURL, ti.mockResolver, ti.logger, ti.esClient)
	AssertOk(t, err, "failed to start events manager, err: %v", err)
	time.Sleep(2 * time.Second) // wait for the API client creations to fail

	// generate some more events
	for i := range recordEvents {
		evtsRecorder.Event(recordEvents[i].eventType, recordEvents[i].message, recordEvents[i].objRef)
	}

	time.Sleep(1 * time.Second) // wait for the request to alert engine fail

	// start API server back
	err = ti.startAPIServer(t.Name())
	AssertOk(t, err, "failed to start API server")
	defer ti.stopAPIServer()

	// total alerts should be 2
	AssertEventually(t,
		func() (bool, interface{}) {
			ctx, cancel := context.WithTimeout(context.Background(), 3*time.Second)
			defer cancel()

			alerts, err = ti.apiClient.MonitoringV1().Alert().List(ctx, &api.ListWatchOptions{
				FieldSelector: fmt.Sprintf("status.reason.alert-policy-id=%s/%s", alertPolicy1.GetName(), alertPolicy1.GetUUID()),
			})
			if err != nil {
				return false, nil
			}
			if len(alerts) == 2 {
				return true, nil
			}
			return false, fmt.Sprintf("expected: 2, got: %v alerts", len(alerts))
		}, fmt.Sprintf("expected number of alerts are not created"), "200ms", "6s")

	// ensure alert policy status counters are correct
	AssertEventually(t,
		func() (bool, interface{}) {
			ctx, cancel := context.WithTimeout(context.Background(), 3*time.Second)
			defer cancel()

			alertPolicy1, err = ti.apiClient.MonitoringV1().AlertPolicy().Get(ctx, alertPolicy1.GetObjectMeta())
			if err != nil {
				return false, nil
			}

			if alertPolicy1.Status.TotalHits == 2 && alertPolicy1.Status.OpenAlerts == 2 {
				return true, nil
			}

			return false, fmt.Sprintf("expected total-hits: 2 open-alerts: 2, got: %+v ", alertPolicy1.Status)
		}, fmt.Sprintf("alert policy status counters are not updated"), "2s", "20s")

	err = ti.cleanupPolicies()
	AssertOk(t, err, "failed to cleanup polices")

	testutils.CleanupAuth(ti.apiServerAddr, true, false, userCreds, ti.logger)
}

// TestEventsAlertEngineWithTCPSyslogExport tests the syslog export of alerts with dummy TCP server
// it tests both BSD and RFC style syslog exports.
func TestEventsAlertEngineWithTCPSyslogExport(t *testing.T) {
	// setup events pipeline to record and distribute events
	ti := tInfo{}
	AssertOk(t, ti.setup(t), "failed to setup test")
	defer ti.teardown()

	ti.apiClient.ClusterV1().Tenant().Create(context.Background(), &cluster.Tenant{
		TypeMeta:   api.TypeMeta{Kind: "Tenant"},
		ObjectMeta: api.ObjectMeta{Name: "default"},
	})

	// start TCP server to receive syslog messages
	ln, receivedMsgsAtTCPServer, err := serviceutils.StartTCPServer(":0", 100, 0)
	AssertOk(t, err, "failed to start TCP server, err: %v", err)
	defer ln.Close()
	tmp := strings.Split(ln.Addr().String(), ":")

	// alert destination - 1: BSD style syslog export
	alertDestBSDSyslog := policygen.CreateAlertDestinationObj(globals.DefaultTenant, globals.DefaultNamespace, uuid.NewV1().String(),
		&monitoring.SyslogExport{
			Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
			Targets: []*monitoring.ExportConfig{
				{
					Destination: "127.0.0.1",
					Transport:   fmt.Sprintf("tcp/%s", tmp[len(tmp)-1]),
				},
			},
		})
	alertDestBSDSyslog, err = ti.apiClient.MonitoringV1().AlertDestination().Create(context.Background(), alertDestBSDSyslog)
	AssertOk(t, err, "failed to add alert destination, err: %v", err)
	defer ti.apiClient.MonitoringV1().AlertDestination().Delete(context.Background(), alertDestBSDSyslog.GetObjectMeta())

	// alert destination - 2: RFC5424 style syslog export
	alertDestRFC5424Syslog := policygen.CreateAlertDestinationObj(globals.DefaultTenant, globals.DefaultNamespace, uuid.NewV1().String(),
		&monitoring.SyslogExport{
			Format: monitoring.MonitoringExportFormat_SYSLOG_RFC5424.String(),
			Targets: []*monitoring.ExportConfig{
				{
					Destination: "127.0.0.1",
					Transport:   fmt.Sprintf("tcp/%s", tmp[len(tmp)-1]),
				},
			},
		})
	alertDestRFC5424Syslog, err = ti.apiClient.MonitoringV1().AlertDestination().Create(context.Background(), alertDestRFC5424Syslog)
	AssertOk(t, err, "failed to add alert destination, err: %v", err)
	defer ti.apiClient.MonitoringV1().AlertDestination().Delete(context.Background(), alertDestRFC5424Syslog.GetObjectMeta())

	// policy - 1: convert CRITICAL events with occurrences > 10 to a CRITICAL alert and export it to the given alert dest
	alertPolicy1 := policygen.CreateAlertPolicyObj(globals.DefaultTenant, globals.DefaultNamespace, uuid.NewV1().String(), "Event", eventattrs.Severity_CRITICAL, "alerts from events", []*fields.Requirement{
		{Key: "severity", Operator: "in", Values: []string{eventattrs.Severity_CRITICAL.String()}},
		{Key: "count", Operator: "gt", Values: []string{"10"}},
	}, []string{alertDestBSDSyslog.GetName(), alertDestRFC5424Syslog.GetName()})
	alertPolicy1, err = ti.apiClient.MonitoringV1().AlertPolicy().Create(context.Background(), alertPolicy1)
	AssertOk(t, err, "failed to add alert policy, err: %v", err)
	defer ti.apiClient.MonitoringV1().AlertPolicy().Delete(context.Background(), alertPolicy1.GetObjectMeta())

	// policy - 2: convert WARNING events with occurrences = 10 to a WARNING alert and export it to the given alert dest
	alertPolicy2 := policygen.CreateAlertPolicyObj(globals.DefaultTenant, globals.DefaultNamespace, uuid.NewV1().String(), "Event", eventattrs.Severity_WARN, "alerts from events", []*fields.Requirement{
		{Key: "severity", Operator: "equals", Values: []string{eventattrs.Severity_WARN.String()}},
		{Key: "count", Operator: "gte", Values: []string{"10"}},
	}, []string{alertDestBSDSyslog.GetName(), alertDestRFC5424Syslog.GetName()})
	alertPolicy2, err = ti.apiClient.MonitoringV1().AlertPolicy().Create(context.Background(), alertPolicy2)
	AssertOk(t, err, "failed to add alert policy, err: %v", err)
	defer ti.apiClient.MonitoringV1().AlertPolicy().Delete(context.Background(), alertPolicy2.GetObjectMeta())

	// object reference for events
	dummyObjRef := &cluster.Node{
		TypeMeta: api.TypeMeta{
			Kind: "Node",
		},
		ObjectMeta: api.ObjectMeta{
			Tenant:    globals.DefaultTenant,
			Namespace: globals.DefaultNamespace,
		},
	}

	messages := map[chan string][]struct {
		Substrs   []string                          // syslog message should contain all these strings
		MsgFormat monitoring.MonitoringExportFormat // BSD style message contains the JSON formatted alert; RFC contains <msgID, structured data, msg>
	}{
		receivedMsgsAtTCPServer: {
			{
				Substrs:   []string{fmt.Sprintf("(tenant:%s) test %s unresponsive on %s", dummyObjRef.Tenant, t.Name(), dummyObjRef.GetKind())},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_BSD,
			},
			{
				Substrs:   []string{fmt.Sprintf("(tenant:%s) test %s stopped on %s", dummyObjRef.Tenant, t.Name(), dummyObjRef.GetKind())},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_BSD,
			},
			{
				Substrs:   []string{fmt.Sprintf("(tenant:%s) dummy election: election stopped %s", dummyObjRef.Tenant, t.Name())},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_BSD,
			},
			{
				Substrs:   []string{fmt.Sprintf("(tenant:%s) test %s unresponsive", globals.DefaultTenant, t.Name())},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_BSD,
			},
			{
				Substrs:   []string{fmt.Sprintf("(tenant:%s) test %s stopped", globals.DefaultTenant, t.Name())},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_BSD,
			},
			{
				Substrs:   []string{fmt.Sprintf("(tenant:%s) test %s unresponsive on %s", dummyObjRef.Tenant, t.Name(), dummyObjRef.GetKind())},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs:   []string{fmt.Sprintf("(tenant:%s) test %s stopped on %s", dummyObjRef.Tenant, t.Name(), dummyObjRef.GetKind())},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs:   []string{fmt.Sprintf("(tenant:%s) dummy election: election stopped %s", dummyObjRef.Tenant, t.Name())},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs:   []string{fmt.Sprintf("(tenant:%s) test %s unresponsive", globals.DefaultTenant, t.Name())},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs:   []string{fmt.Sprintf("(tenant:%s) test %s stopped", globals.DefaultTenant, t.Name())},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
		},
	}

	testSyslogMessageDelivery(t, ti, dummyObjRef, messages)

	AssertEventually(t,
		func() (bool, interface{}) {
			ad, err := ti.apiClient.MonitoringV1().AlertDestination().Get(context.Background(), alertDestBSDSyslog.GetObjectMeta())
			if err != nil {
				return false, err
			}

			if ad.Status.TotalNotificationsSent == 5 {
				return true, nil
			}

			return false, fmt.Sprintf("TotalNotificationSent expected:5, got: %v", ad.Status.TotalNotificationsSent)
		}, fmt.Sprintf("alert destionation %v is not updated", alertDestBSDSyslog.GetName()), "200ms", "10s")

	AssertEventually(t,
		func() (bool, interface{}) {
			ad, err := ti.apiClient.MonitoringV1().AlertDestination().Get(context.Background(), alertDestRFC5424Syslog.GetObjectMeta())
			if err != nil {
				return false, err
			}

			if ad.Status.TotalNotificationsSent == 5 {
				return true, nil
			}

			return false, fmt.Sprintf("TotalNotificationSent expected:5, got: %v", ad.Status.TotalNotificationsSent)
		}, fmt.Sprintf("alert destionation %v is not updated", alertDestRFC5424Syslog.GetName()), "200ms", "10s")
}

// TestEventsAlertEngineWithTCPSyslogExport tests the syslog export of alerts with dummy UDP server
// it tests both BSD and RFC style syslog exports.
func TestEventsAlertEngineWithUDPSyslogExport(t *testing.T) {
	// setup events pipeline to record and distribute events
	ti := tInfo{}
	AssertOk(t, ti.setup(t), "failed to setup test")
	defer ti.teardown()

	ti.apiClient.ClusterV1().Tenant().Create(context.Background(), &cluster.Tenant{
		TypeMeta:   api.TypeMeta{Kind: "Tenant"},
		ObjectMeta: api.ObjectMeta{Name: "default"},
	})

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
			Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
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
	alertDestBSDSyslog, err = ti.apiClient.MonitoringV1().AlertDestination().Create(context.Background(), alertDestBSDSyslog)
	AssertOk(t, err, "failed to add alert destination, err: %v", err)
	defer ti.apiClient.MonitoringV1().AlertDestination().Delete(context.Background(), alertDestBSDSyslog.GetObjectMeta())

	// alert destination - 2: RFC5424 style syslog export
	alertDestRFC5424Syslog := policygen.CreateAlertDestinationObj(globals.DefaultTenant, globals.DefaultNamespace, uuid.NewV1().String(),
		&monitoring.SyslogExport{
			Format: monitoring.MonitoringExportFormat_SYSLOG_RFC5424.String(),
			Targets: []*monitoring.ExportConfig{
				{
					Destination: "127.0.0.1",
					Transport:   fmt.Sprintf("udp/%s", tmp1[len(tmp1)-1]),
				},
			},
		})
	alertDestRFC5424Syslog, err = ti.apiClient.MonitoringV1().AlertDestination().Create(context.Background(), alertDestRFC5424Syslog)
	AssertOk(t, err, "failed to add alert destination, err: %v", err)
	defer ti.apiClient.MonitoringV1().AlertDestination().Delete(context.Background(), alertDestRFC5424Syslog.GetObjectMeta())

	// convert CRITICAL events with occurrences > 10 to a CRITICAL alert and export it to the given alert dest
	alertPolicy1 := policygen.CreateAlertPolicyObj(globals.DefaultTenant, globals.DefaultNamespace, uuid.NewV1().String(), "Event", eventattrs.Severity_CRITICAL, "alerts from events", []*fields.Requirement{
		{Key: "severity", Operator: "in", Values: []string{
			eventattrs.Severity_CRITICAL.String(),
			eventattrs.Severity_WARN.String()}},
		{Key: "count", Operator: "gt", Values: []string{"10"}},
	}, []string{alertDestBSDSyslog.GetName(), alertDestRFC5424Syslog.GetName()})
	alertPolicy1, err = ti.apiClient.MonitoringV1().AlertPolicy().Create(context.Background(), alertPolicy1)
	AssertOk(t, err, "failed to add alert policy, err: %v", err)
	defer ti.apiClient.MonitoringV1().AlertPolicy().Delete(context.Background(), alertPolicy1.GetObjectMeta())

	// object reference for events
	dummyObjRef := &cluster.Node{
		TypeMeta: api.TypeMeta{
			Kind: "Node",
		},
		ObjectMeta: api.ObjectMeta{
			Tenant:    globals.DefaultTenant,
			Namespace: globals.DefaultNamespace,
			Name:      t.Name(),
		},
	}

	messages := map[chan string][]struct {
		Substrs   []string                          // syslog message should contain all these strings
		MsgFormat monitoring.MonitoringExportFormat // BSD style message contains the JSON formatted alert; RFC contains <msgID, structured data, msg>
	}{
		receivedMsgsAtUDPServer1: {
			{
				Substrs:   []string{fmt.Sprintf("(tenant:%s) test %s unresponsive on %s", dummyObjRef.Tenant, t.Name(), dummyObjRef.GetKind())},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_BSD,
			},
			{
				Substrs:   []string{fmt.Sprintf("(tenant:%s) test %s stopped on %s", dummyObjRef.Tenant, t.Name(), dummyObjRef.GetKind())},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_BSD,
			},
			{
				Substrs:   []string{fmt.Sprintf("(tenant:%s) dummy election: election stopped %s", dummyObjRef.Tenant, t.Name())},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_BSD,
			},
			{
				Substrs:   []string{fmt.Sprintf("(tenant:%s) test %s unresponsive", globals.DefaultTenant, t.Name())},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_BSD,
			},
			{
				Substrs:   []string{fmt.Sprintf("(tenant:%s) test %s stopped", globals.DefaultTenant, t.Name())},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_BSD,
			},
			{
				Substrs:   []string{fmt.Sprintf("(tenant:%s) test %s unresponsive on %s", dummyObjRef.Tenant, t.Name(), dummyObjRef.GetKind())},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs:   []string{fmt.Sprintf("(tenant:%s) test %s stopped on %s", dummyObjRef.Tenant, t.Name(), dummyObjRef.GetKind())},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs:   []string{fmt.Sprintf("(tenant:%s) dummy election: election stopped %s", dummyObjRef.Tenant, t.Name())},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs:   []string{fmt.Sprintf("(tenant:%s) test %s unresponsive", globals.DefaultTenant, t.Name())},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs:   []string{fmt.Sprintf("(tenant:%s) test %s stopped", globals.DefaultTenant, t.Name())},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
		},
		receivedMsgsAtUDPServer2: {
			{
				Substrs:   []string{fmt.Sprintf("(tenant:%s) test %s unresponsive on %s", dummyObjRef.Tenant, t.Name(), dummyObjRef.GetKind())},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_BSD,
			},
			{
				Substrs:   []string{fmt.Sprintf("(tenant:%s) test %s stopped on %s", dummyObjRef.Tenant, t.Name(), dummyObjRef.GetKind())},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_BSD,
			},
			{
				Substrs:   []string{fmt.Sprintf("(tenant:%s) dummy election: election stopped %s", dummyObjRef.Tenant, t.Name())},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_BSD,
			},
			{
				Substrs:   []string{fmt.Sprintf("(tenant:%s) test %s unresponsive", globals.DefaultTenant, t.Name())},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_BSD,
			},
			{
				Substrs:   []string{fmt.Sprintf("(tenant:%s) test %s stopped", globals.DefaultTenant, t.Name())},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_BSD,
			},
		},
	}

	testSyslogMessageDelivery(t, ti, dummyObjRef, messages)

	AssertEventually(t,
		func() (bool, interface{}) {
			ad, err := ti.apiClient.MonitoringV1().AlertDestination().Get(context.Background(), alertDestBSDSyslog.GetObjectMeta())
			if err != nil {
				return false, err
			}

			// (5 * 2) for 2 targets
			if ad.Status.TotalNotificationsSent == 10 {
				return true, nil
			}

			return false, fmt.Sprintf("TotalNotificationSent expected: 10, got: %v", ad.Status.TotalNotificationsSent)
		}, fmt.Sprintf("alert destionation %v is not updated", alertDestBSDSyslog.GetName()), "200ms", "10s")

	AssertEventually(t,
		func() (bool, interface{}) {
			ad, err := ti.apiClient.MonitoringV1().AlertDestination().Get(context.Background(), alertDestRFC5424Syslog.GetObjectMeta())
			if err != nil {
				return false, err
			}

			if ad.Status.TotalNotificationsSent == 5 {
				return true, nil
			}

			return false, fmt.Sprintf("TotalNotificationSent expected: 5, got: %v", ad.Status.TotalNotificationsSent)
		}, fmt.Sprintf("alert destionation %v is not updated", alertDestRFC5424Syslog.GetName()), "200ms", "10s")
}

// TestEventsExport tests events export with dummy UDP and TCP servers as syslog server
func TestEventsExport(t *testing.T) {
	// setup events pipeline to record and distribute events
	ti := tInfo{}
	AssertOk(t, ti.setup(t), "failed to setup test")
	defer ti.teardown()

	var wg sync.WaitGroup

	// start UDP server to receive syslog messages
	pConn1, receivedMsgsAtUDPServer1, err := serviceutils.StartUDPServer(":0")
	AssertOk(t, err, "failed to start UDP server, err: %v", err)
	defer pConn1.Close()
	tmp1 := strings.Split(pConn1.LocalAddr().String(), ":")

	// start TCP server - 1 to receive syslog messages
	ln1, receivedMsgsAtTCPServer1, err := serviceutils.StartTCPServer(":0", 100, 0)
	AssertOk(t, err, "failed to start TCP server, err: %v", err)
	defer ln1.Close()
	tmp2 := strings.Split(ln1.Addr().String(), ":")

	// start TCP server - 2 to receive syslog messages
	ln2, receivedMsgsAtTCPServer2, err := serviceutils.StartTCPServer(":0", 100, 0)
	AssertOk(t, err, "failed to start TCP server, err: %v", err)
	defer ln2.Close()
	tmp3 := strings.Split(ln2.Addr().String(), ":")
	defTenant := cluster.Tenant{
		TypeMeta:   api.TypeMeta{Kind: "Tenant"},
		ObjectMeta: api.ObjectMeta{Name: "default"},
	}
	ti.apiClient.ClusterV1().Tenant().Create(context.Background(), &defTenant)

	// add event policy - 1
	eventPolicy1 := policygen.CreateEventPolicyObj(globals.DefaultTenant, globals.DefaultNamespace, "ep-1",
		monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
		[]*monitoring.ExportConfig{
			{ // receivedMsgsAtUDPServer1
				Destination: "127.0.0.1",
				Transport:   fmt.Sprintf("udp/%s", tmp1[len(tmp1)-1]),
			},
			{ // receivedMsgsAtTCPServer1
				Destination: "127.0.0.1",
				Transport:   fmt.Sprintf("tcp/%s", tmp2[len(tmp2)-1]),
			},
		}, nil)
	eventPolicy1, err = ti.apiClient.MonitoringV1().EventPolicy().Create(context.Background(), eventPolicy1)
	AssertOk(t, err, "failed to create event policy, err: %v", err)
	defer ti.apiClient.MonitoringV1().EventPolicy().Delete(context.Background(), eventPolicy1.GetObjectMeta())

	// add event policy - 2
	eventPolicy2 := policygen.CreateEventPolicyObj(globals.DefaultTenant, globals.DefaultNamespace, "ep-2",
		monitoring.MonitoringExportFormat_SYSLOG_RFC5424.String(),
		[]*monitoring.ExportConfig{
			{ // receivedMsgsAtTCPServer2
				Destination: "127.0.0.1",
				Transport:   fmt.Sprintf("tcp/%s", tmp3[len(tmp3)-1]),
			},
		},
		&monitoring.SyslogExportConfig{
			FacilityOverride: monitoring.SyslogFacility_LOG_SYSLOG.String(),
			Prefix:           CreateAlphabetString(5),
		})
	eventPolicy2, err = ti.apiClient.MonitoringV1().EventPolicy().Create(context.Background(), eventPolicy2)
	AssertOk(t, err, "failed to create event policy, err: %v", err)
	defer ti.apiClient.MonitoringV1().EventPolicy().Delete(context.Background(), eventPolicy2.GetObjectMeta())

	// to let the event policies reach the policy manager (api server -> evtsmgr -> policy watcher -> policy manager -> exporter)
	time.Sleep(2 * time.Second)

	// object reference for events
	dummyObjRef := &cluster.Node{
		TypeMeta: api.TypeMeta{
			Kind: "Node",
		},
		ObjectMeta: api.ObjectMeta{
			Tenant:    globals.DefaultTenant,
			Namespace: globals.DefaultNamespace,
		},
	}

	messages := map[chan string][]struct {
		Substrs   []string                          // syslog message should contain all these strings
		MsgFormat monitoring.MonitoringExportFormat // BSD style message contains the JSON formatted alert; RFC contains <msgID, structured data, msg>
	}{
		// all the messages that are sent should be received at the syslog server
		receivedMsgsAtUDPServer1: {
			{
				Substrs:   []string{fmt.Sprintf("(tenant:%s) test %s started on %s", dummyObjRef.Tenant, t.Name(), dummyObjRef.GetKind())},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_BSD,
			},
			{
				Substrs:   []string{fmt.Sprintf("(tenant:%s) test %s running on %s", dummyObjRef.Tenant, t.Name(), dummyObjRef.GetKind())},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_BSD,
			},
			{
				Substrs:   []string{fmt.Sprintf("(tenant:%s) test %s unresponsive on %s", dummyObjRef.Tenant, t.Name(), dummyObjRef.GetKind())},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_BSD,
			},
			{
				Substrs:   []string{fmt.Sprintf("(tenant:%s) test %s stopped on %s", dummyObjRef.Tenant, t.Name(), dummyObjRef.GetKind())},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_BSD,
			},
			{
				Substrs:   []string{fmt.Sprintf("(tenant:%s) dummy election: election started %s", dummyObjRef.Tenant, t.Name())},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_BSD,
			},
			{
				Substrs:   []string{fmt.Sprintf("(tenant:%s) dummy election: leader elected %s", dummyObjRef.Tenant, t.Name())},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_BSD,
			},
			{
				Substrs:   []string{fmt.Sprintf("(tenant:%s) dummy election: leader changed %s", dummyObjRef.Tenant, t.Name())},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_BSD,
			},
			{
				Substrs:   []string{fmt.Sprintf("(tenant:%s) dummy election: leader lost %s", dummyObjRef.Tenant, t.Name())},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_BSD,
			},
			{
				Substrs:   []string{fmt.Sprintf("(tenant:%s) dummy election: election stopped %s", dummyObjRef.Tenant, t.Name())},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_BSD,
			},
			{
				Substrs:   []string{fmt.Sprintf("(tenant:%s) test %s started", globals.DefaultTenant, t.Name())},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_BSD,
			},
			{
				Substrs:   []string{fmt.Sprintf("(tenant:%s) test %s running", globals.DefaultTenant, t.Name())},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_BSD,
			},
			{
				Substrs:   []string{fmt.Sprintf("(tenant:%s) test %s unresponsive", globals.DefaultTenant, t.Name())},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_BSD,
			},
			{
				Substrs:   []string{fmt.Sprintf("(tenant:%s) test %s stopped", globals.DefaultTenant, t.Name())},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_BSD,
			},
		},
		receivedMsgsAtTCPServer1: {
			{
				Substrs:   []string{fmt.Sprintf("(tenant:%s) test %s started on %s", dummyObjRef.Tenant, t.Name(), dummyObjRef.GetKind())},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_BSD,
			},
			{
				Substrs:   []string{fmt.Sprintf("(tenant:%s) test %s running on %s", dummyObjRef.Tenant, t.Name(), dummyObjRef.GetKind())},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_BSD,
			},
			{
				Substrs:   []string{fmt.Sprintf("(tenant:%s) test %s unresponsive on %s", dummyObjRef.Tenant, t.Name(), dummyObjRef.GetKind())},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_BSD,
			},
			{
				Substrs:   []string{fmt.Sprintf("(tenant:%s) test %s stopped on %s", dummyObjRef.Tenant, t.Name(), dummyObjRef.GetKind())},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_BSD,
			},
			{
				Substrs:   []string{fmt.Sprintf("(tenant:%s) dummy election: election started %s", dummyObjRef.Tenant, t.Name())},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_BSD,
			},
			{
				Substrs:   []string{fmt.Sprintf("(tenant:%s) dummy election: leader elected %s", dummyObjRef.Tenant, t.Name())},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_BSD,
			},
			{
				Substrs:   []string{fmt.Sprintf("(tenant:%s) dummy election: leader changed %s", dummyObjRef.Tenant, t.Name())},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_BSD,
			},
			{
				Substrs:   []string{fmt.Sprintf("(tenant:%s) dummy election: leader lost %s", dummyObjRef.Tenant, t.Name())},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_BSD,
			},
			{
				Substrs:   []string{fmt.Sprintf("(tenant:%s) dummy election: election stopped %s", dummyObjRef.Tenant, t.Name())},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_BSD,
			},
			{
				Substrs:   []string{fmt.Sprintf("(tenant:%s) test %s started", globals.DefaultTenant, t.Name())},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_BSD,
			},
			{
				Substrs:   []string{fmt.Sprintf("(tenant:%s) test %s running", globals.DefaultTenant, t.Name())},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_BSD,
			},
			{
				Substrs:   []string{fmt.Sprintf("(tenant:%s) test %s unresponsive", globals.DefaultTenant, t.Name())},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_BSD,
			},
			{
				Substrs:   []string{fmt.Sprintf("(tenant:%s) test %s stopped", globals.DefaultTenant, t.Name())},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_BSD,
			},
		},
		receivedMsgsAtTCPServer2: { // messages belonging to event policy - 2
			{
				Substrs: []string{fmt.Sprintf("(tenant:%s) test %s started on %s", dummyObjRef.Tenant, t.Name(), dummyObjRef.GetKind()),
					eventPolicy2.Spec.GetSyslogConfig().GetPrefix()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs: []string{fmt.Sprintf("(tenant:%s) test %s running on %s", dummyObjRef.Tenant, t.Name(), dummyObjRef.GetKind()),
					eventPolicy2.Spec.GetSyslogConfig().GetPrefix()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs: []string{fmt.Sprintf("(tenant:%s) test %s unresponsive on %s", dummyObjRef.Tenant, t.Name(), dummyObjRef.GetKind()),
					eventPolicy2.Spec.GetSyslogConfig().GetPrefix()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs: []string{fmt.Sprintf("(tenant:%s) test %s stopped on %s", dummyObjRef.Tenant, t.Name(), dummyObjRef.GetKind()),
					eventPolicy2.Spec.GetSyslogConfig().GetPrefix()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs: []string{fmt.Sprintf("(tenant:%s) dummy election: election started %s", dummyObjRef.Tenant, t.Name()),
					eventPolicy2.Spec.GetSyslogConfig().GetPrefix()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs: []string{fmt.Sprintf("(tenant:%s) dummy election: leader elected %s", dummyObjRef.Tenant, t.Name()),
					eventPolicy2.Spec.GetSyslogConfig().GetPrefix()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs: []string{fmt.Sprintf("(tenant:%s) dummy election: leader changed %s", dummyObjRef.Tenant, t.Name()),
					eventPolicy2.Spec.GetSyslogConfig().GetPrefix()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs: []string{fmt.Sprintf("(tenant:%s) dummy election: leader lost %s", dummyObjRef.Tenant, t.Name()),
					eventPolicy2.Spec.GetSyslogConfig().GetPrefix()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs: []string{fmt.Sprintf("(tenant:%s) dummy election: election stopped %s", dummyObjRef.Tenant, t.Name()),
					eventPolicy2.Spec.GetSyslogConfig().GetPrefix()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs: []string{fmt.Sprintf("(tenant:%s) test %s started", globals.DefaultTenant, t.Name()),
					eventPolicy2.Spec.GetSyslogConfig().GetPrefix()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs: []string{fmt.Sprintf("(tenant:%s) test %s running", globals.DefaultTenant, t.Name()),
					eventPolicy2.Spec.GetSyslogConfig().GetPrefix()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs: []string{fmt.Sprintf("(tenant:%s) test %s unresponsive", globals.DefaultTenant, t.Name()),
					eventPolicy2.Spec.GetSyslogConfig().GetPrefix()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs: []string{fmt.Sprintf("(tenant:%s) test %s stopped", globals.DefaultTenant, t.Name()),
					eventPolicy2.Spec.GetSyslogConfig().GetPrefix()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
		},
	}

	testSyslogMessageDelivery(t, ti, dummyObjRef, messages)

	// update event policy - 1; remove the existing target and add a new one
	// start UDP server to receive syslog messages
	pConn2, receivedMsgsAtUDPServer2, err := serviceutils.StartUDPServer(":0")
	AssertOk(t, err, "failed to start UDP server, err: %v", err)
	defer pConn2.Close()
	tmp4 := strings.Split(pConn2.LocalAddr().String(), ":")

	// add event policy - 1
	eventPolicy1 = policygen.CreateEventPolicyObj(globals.DefaultTenant, globals.DefaultNamespace, "ep-1",
		monitoring.MonitoringExportFormat_SYSLOG_RFC5424.String(),
		[]*monitoring.ExportConfig{
			{ // receivedMsgsAtUDPServer1
				Destination: "127.0.0.1",
				Transport:   fmt.Sprintf("udp/%s", tmp1[len(tmp1)-1]),
			},
			{ // receivedMsgsAtUDPServer2
				Destination: "127.0.0.1",
				Transport:   fmt.Sprintf("udp/%s", tmp4[len(tmp4)-1]),
			},
		}, nil)
	eventPolicy1, err = ti.apiClient.MonitoringV1().EventPolicy().Update(context.Background(), eventPolicy1)
	AssertOk(t, err, "failed to create event policy, err: %v", err)
	defer ti.apiClient.MonitoringV1().EventPolicy().Delete(context.Background(), eventPolicy1.GetObjectMeta())

	// `receivedMsgsAtTCPServer1` should receive no more messages as it is removed from the policy '"ep-1"' (refer above);
	// final should be the last message on this channel
	wg.Add(1)
	closeMsgCh := make(chan struct{})
	go func() {
		defer wg.Done()
		time.Sleep(100 * time.Millisecond)
		receivedMsgsAtTCPServer1 <- "final"

		shouldNotReceiveAnymoreMsgs := false

		for {
			select {
			case <-closeMsgCh:
				return
			case msg, ok := <-receivedMsgsAtTCPServer1:
				if !ok {
					return
				}

				if msg == "final" {
					shouldNotReceiveAnymoreMsgs = true
					continue
				}

				if shouldNotReceiveAnymoreMsgs {
					t.Fatalf("syslog target is removed from the policy. so, should not receive any more messages on this channel "+
						"but received: %v", msg)
				} else {
					log.Infof("receiving pending messages from syslog server: %v", msg)
				}
			}
		}
	}()

	// we should stop seeing messages on the old channel `receivedMsgsAtTCPServer1`
	// and start seeing messages on the new channel `receivedMsgsAtUDPServer2`
	messages = map[chan string][]struct {
		Substrs   []string                          // syslog message should contain all these strings
		MsgFormat monitoring.MonitoringExportFormat // BSD style message contains the JSON formatted alert; RFC contains <msgID, structured data, msg>
	}{
		receivedMsgsAtUDPServer1: { // target - 1 of event policy - 1
			{
				Substrs: []string{fmt.Sprintf("(tenant:%s) test %s started on %s", dummyObjRef.Tenant, t.Name(), dummyObjRef.GetKind()),
					eventPolicy1.Spec.GetSyslogConfig().GetPrefix()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs: []string{fmt.Sprintf("(tenant:%s) test %s running on %s", dummyObjRef.Tenant, t.Name(), dummyObjRef.GetKind()),
					eventPolicy1.Spec.GetSyslogConfig().GetPrefix()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs: []string{fmt.Sprintf("(tenant:%s) test %s unresponsive on %s", dummyObjRef.Tenant, t.Name(), dummyObjRef.GetKind()),
					eventPolicy1.Spec.GetSyslogConfig().GetPrefix()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs: []string{fmt.Sprintf("(tenant:%s) test %s stopped on %s", dummyObjRef.Tenant, t.Name(), dummyObjRef.GetKind()),
					eventPolicy1.Spec.GetSyslogConfig().GetPrefix()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs: []string{fmt.Sprintf("(tenant:%s) dummy election: election started %s", dummyObjRef.Tenant, t.Name()),
					eventPolicy1.Spec.GetSyslogConfig().GetPrefix()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs: []string{fmt.Sprintf("(tenant:%s) dummy election: leader elected %s", dummyObjRef.Tenant, t.Name()),
					eventPolicy1.Spec.GetSyslogConfig().GetPrefix()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs: []string{fmt.Sprintf("(tenant:%s) dummy election: leader changed %s", dummyObjRef.Tenant, t.Name()),
					eventPolicy1.Spec.GetSyslogConfig().GetPrefix()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs: []string{fmt.Sprintf("(tenant:%s) dummy election: leader lost %s", dummyObjRef.Tenant, t.Name()),
					eventPolicy1.Spec.GetSyslogConfig().GetPrefix()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs: []string{fmt.Sprintf("(tenant:%s) dummy election: election stopped %s", dummyObjRef.Tenant, t.Name()),
					eventPolicy1.Spec.GetSyslogConfig().GetPrefix()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs: []string{fmt.Sprintf("(tenant:%s) test %s started", globals.DefaultTenant, t.Name()),
					eventPolicy1.Spec.GetSyslogConfig().GetPrefix()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs: []string{fmt.Sprintf("(tenant:%s) test %s running", globals.DefaultTenant, t.Name()),
					eventPolicy1.Spec.GetSyslogConfig().GetPrefix()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs: []string{fmt.Sprintf("(tenant:%s) test %s unresponsive", globals.DefaultTenant, t.Name()),
					eventPolicy1.Spec.GetSyslogConfig().GetPrefix()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs: []string{fmt.Sprintf("(tenant:%s) test %s stopped", globals.DefaultTenant, t.Name()),
					eventPolicy1.Spec.GetSyslogConfig().GetPrefix()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
		},
		receivedMsgsAtUDPServer2: { // target - 2 of event policy - 1
			{
				Substrs: []string{fmt.Sprintf("(tenant:%s) test %s started on %s", dummyObjRef.Tenant, t.Name(), dummyObjRef.GetKind()),
					eventPolicy1.Spec.GetSyslogConfig().GetPrefix()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs: []string{fmt.Sprintf("(tenant:%s) test %s running on %s", dummyObjRef.Tenant, t.Name(), dummyObjRef.GetKind()),
					eventPolicy1.Spec.GetSyslogConfig().GetPrefix()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs: []string{fmt.Sprintf("(tenant:%s) test %s unresponsive on %s", dummyObjRef.Tenant, t.Name(), dummyObjRef.GetKind()),
					eventPolicy1.Spec.GetSyslogConfig().GetPrefix()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs: []string{fmt.Sprintf("(tenant:%s) test %s stopped on %s", dummyObjRef.Tenant, t.Name(), dummyObjRef.GetKind()),
					eventPolicy1.Spec.GetSyslogConfig().GetPrefix()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs: []string{fmt.Sprintf("(tenant:%s) dummy election: election started %s", dummyObjRef.Tenant, t.Name()),
					eventPolicy1.Spec.GetSyslogConfig().GetPrefix()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs: []string{fmt.Sprintf("(tenant:%s) dummy election: leader elected %s", dummyObjRef.Tenant, t.Name()),
					eventPolicy1.Spec.GetSyslogConfig().GetPrefix()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs: []string{fmt.Sprintf("(tenant:%s) dummy election: leader changed %s", dummyObjRef.Tenant, t.Name()),
					eventPolicy1.Spec.GetSyslogConfig().GetPrefix()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs: []string{fmt.Sprintf("(tenant:%s) dummy election: leader lost %s", dummyObjRef.Tenant, t.Name()),
					eventPolicy1.Spec.GetSyslogConfig().GetPrefix()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs: []string{fmt.Sprintf("(tenant:%s) dummy election: election stopped %s", dummyObjRef.Tenant, t.Name()),
					eventPolicy1.Spec.GetSyslogConfig().GetPrefix()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs: []string{fmt.Sprintf("(tenant:%s) test %s started", globals.DefaultTenant, t.Name()),
					eventPolicy1.Spec.GetSyslogConfig().GetPrefix()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs: []string{fmt.Sprintf("(tenant:%s) test %s running", globals.DefaultTenant, t.Name()),
					eventPolicy1.Spec.GetSyslogConfig().GetPrefix()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs: []string{fmt.Sprintf("(tenant:%s) test %s unresponsive", globals.DefaultTenant, t.Name()),
					eventPolicy1.Spec.GetSyslogConfig().GetPrefix()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs: []string{fmt.Sprintf("(tenant:%s) test %s stopped", globals.DefaultTenant, t.Name()),
					eventPolicy1.Spec.GetSyslogConfig().GetPrefix()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
		},
		receivedMsgsAtTCPServer2: { // messages belonging to event policy - 2
			{
				Substrs: []string{fmt.Sprintf("(tenant:%s) test %s started on %s", dummyObjRef.Tenant, t.Name(), dummyObjRef.GetKind()),
					eventPolicy2.Spec.GetSyslogConfig().GetPrefix()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs: []string{fmt.Sprintf("(tenant:%s) test %s running on %s", dummyObjRef.Tenant, t.Name(), dummyObjRef.GetKind()),
					eventPolicy2.Spec.GetSyslogConfig().GetPrefix()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs: []string{fmt.Sprintf("(tenant:%s) test %s unresponsive on %s", dummyObjRef.Tenant, t.Name(), dummyObjRef.GetKind()),
					eventPolicy2.Spec.GetSyslogConfig().GetPrefix()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs: []string{fmt.Sprintf("(tenant:%s) test %s stopped on %s", dummyObjRef.Tenant, t.Name(), dummyObjRef.GetKind()),
					eventPolicy2.Spec.GetSyslogConfig().GetPrefix()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs: []string{fmt.Sprintf("(tenant:%s) dummy election: election started %s", dummyObjRef.Tenant, t.Name()),
					eventPolicy2.Spec.GetSyslogConfig().GetPrefix()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs: []string{fmt.Sprintf("(tenant:%s) dummy election: leader elected %s", dummyObjRef.Tenant, t.Name()),
					eventPolicy2.Spec.GetSyslogConfig().GetPrefix()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs: []string{fmt.Sprintf("(tenant:%s) dummy election: leader changed %s", dummyObjRef.Tenant, t.Name()),
					eventPolicy2.Spec.GetSyslogConfig().GetPrefix()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs: []string{fmt.Sprintf("(tenant:%s) dummy election: leader lost %s", dummyObjRef.Tenant, t.Name()),
					eventPolicy2.Spec.GetSyslogConfig().GetPrefix()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs: []string{fmt.Sprintf("(tenant:%s) dummy election: election stopped %s", dummyObjRef.Tenant, t.Name()),
					eventPolicy2.Spec.GetSyslogConfig().GetPrefix()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs: []string{fmt.Sprintf("(tenant:%s) test %s started", globals.DefaultTenant, t.Name()),
					eventPolicy2.Spec.GetSyslogConfig().GetPrefix()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs: []string{fmt.Sprintf("(tenant:%s) test %s running", globals.DefaultTenant, t.Name()),
					eventPolicy2.Spec.GetSyslogConfig().GetPrefix()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs: []string{fmt.Sprintf("(tenant:%s) test %s unresponsive", globals.DefaultTenant, t.Name()),
					eventPolicy2.Spec.GetSyslogConfig().GetPrefix()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
			{
				Substrs: []string{fmt.Sprintf("(tenant:%s) test %s stopped", globals.DefaultTenant, t.Name()),
					eventPolicy2.Spec.GetSyslogConfig().GetPrefix()},
				MsgFormat: monitoring.MonitoringExportFormat_SYSLOG_RFC5424,
			},
		},
	}
	testSyslogMessageDelivery(t, ti, dummyObjRef, messages)

	close(closeMsgCh)

	// to avoid - panic: send on closed channel
	// this happens because the test completes before the dispatcher or evtsproxy is done sending all events.
	// test has completed prior because we check for occurrence of each message but it is possible that the sender is
	// sending in intervals which means there're are events coming in every batch interval. So, the TCP/UDP server gets
	// closed while the dispatcher is trying to send events to it. Stopping all the evtsproxy related services before
	// shutting down the TCP/UDP serer will solve the problem (there won't be anyone trying to send events anymore).
	ti.evtProxyServices.Stop()
}

// TestEventsExportWithSyslogReconnect tests the events export with TCP syslog server
// it starts with non-existent syslog server (internally, the code tries to reconnect)
// meantime, bring up the syslog server and ensure it starts receiving syslog messages.
func TestEventsExportWithSyslogReconnect(t *testing.T) {
	// setup events pipeline to record and distribute events
	ti := tInfo{}
	AssertOk(t, ti.setup(t), "failed to setup test")
	defer ti.teardown()

	var wg sync.WaitGroup
	stopGoRoutines := make(chan struct{})

	defTenant := &cluster.Tenant{
		TypeMeta:   api.TypeMeta{Kind: "Tenant"},
		ObjectMeta: api.ObjectMeta{Name: "default"},
	}
	defTenant, err := ti.apiClient.ClusterV1().Tenant().Create(context.Background(), defTenant)
	AssertOk(t, err, "failed to create tenant")
	defer ti.apiClient.ClusterV1().Tenant().Delete(context.Background(), defTenant.GetObjectMeta())

	// add event policy - 1
	port := getAvailablePort(45000, 45100)
	if port == 0 {
		t.Skip("could not find a open port from 45000 to 45100 to run TCP server")
	}
	ti.logger.Infof("available port to run TCP server: %d", port)
	eventPolicy1 := policygen.CreateEventPolicyObj(globals.DefaultTenant, globals.DefaultNamespace, "ep-6",
		monitoring.MonitoringExportFormat_SYSLOG_RFC5424.String(),
		[]*monitoring.ExportConfig{
			{ // receivedMsgsAtTCPServer1
				Destination: "127.0.0.1",
				Transport:   fmt.Sprintf("tcp/%d", port),
			},
		}, nil)
	eventPolicy1, err = ti.apiClient.MonitoringV1().EventPolicy().Create(context.Background(), eventPolicy1)
	AssertOk(t, err, "failed to create event policy, err: %v", err)
	defer ti.apiClient.MonitoringV1().EventPolicy().Delete(context.Background(), eventPolicy1.GetObjectMeta())

	// syslog will try reconnecting during this time
	time.Sleep(1 * time.Second)

	// start TCP server - 1 to receive syslog messages
	ln1, receiveMsgsAtTCPServer, err := serviceutils.StartTCPServer(fmt.Sprintf("127.0.0.1:%d", port), 100, 0)
	AssertOk(t, err, "failed to start TCP server, err: %v", err)
	defer ln1.Close()

	// record events
	wg.Add(1)
	count := 0
	go func() {
		defer wg.Done()
		recorderEventsDir, err := ioutil.TempDir("", t.Name())
		AssertOk(t, err, "failed to create recorder events directory")
		defer os.RemoveAll(recorderEventsDir)

		evtsRecorder, err := recorder.NewRecorder(&recorder.Config{
			Component:                   uuid.NewV4().String(),
			EvtsProxyURL:                ti.evtProxyServices.EvtsProxy.RPCServer.GetListenURL(),
			BackupDir:                   recorderEventsDir,
			SkipCategoryBasedEventTypes: true}, ti.logger)
		if err != nil {
			ti.logger.Errorf("failed to create recorder, err: %v", err)
			return
		}
		ti.recorders.Lock()
		ti.recorders.list = append(ti.recorders.list, evtsRecorder)
		ti.recorders.Unlock()

		for {
			count++
			select {
			case <-stopGoRoutines:
				return
			case <-time.After(100 * time.Millisecond):
				evtsRecorder.Event(eventtypes.SERVICE_RUNNING, fmt.Sprintf("message-%d", count), nil)
			}
		}
	}()

	// receive messages from the syslog server
	wg.Add(1)
	receivedMsgs := 0
	go func() {
		defer wg.Done()
		for {
			select {
			case <-stopGoRoutines:
				return
			case msg, ok := <-receiveMsgsAtTCPServer:
				if !ok {
					return
				}
				if !syslog.ValidateSyslogMessage(monitoring.MonitoringExportFormat_SYSLOG_RFC5424, msg) {
					ti.logger.Fatalf("invalid message format, expected: RFC5424, got: %v", msg)
				}
				receivedMsgs++
			}
		}
	}()

	// ensure the syslog server receives
	AssertEventually(t,
		func() (bool, interface{}) {
			if receivedMsgs == count {
				return true, nil
			}
			return true, fmt.Sprintf("expected: %d messages to be received on the TCP syslog server, got: %d", count, receivedMsgs)
		}, "syslog server did not receive the expected messages", "200ms", "2s")

	close(stopGoRoutines)
	wg.Wait()
}

// TestEventsExportWithSlowExporter tests events file rotation and restart of the exporters without losing events.
func TestEventsExportWithSlowExporter(t *testing.T) {
	// setup events pipeline to record and distribute events
	ti := tInfo{storeConfig: &events.StoreConfig{MaxFileSize: 50 * 1000, MaxNumFiles: 50}}
	AssertOk(t, ti.setup(t), "failed to setup test")
	defer ti.teardown()

	var wg sync.WaitGroup
	stopSyslogMessagesReceiver := make(chan struct{})
	stopEvtsRecorder := make(chan struct{})

	// create API server client
	apiClient, err := client.NewGrpcUpstream("events_integ_test", ti.apiServerAddr, ti.logger)
	AssertOk(t, err, "failed to create API server client, err: %v", err)
	defer apiClient.Close()
	defTenant := &cluster.Tenant{
		TypeMeta:   api.TypeMeta{Kind: "Tenant"},
		ObjectMeta: api.ObjectMeta{Name: "default"},
	}
	defTenant, err = apiClient.ClusterV1().Tenant().Create(context.Background(), defTenant)
	AssertOk(t, err, "failed to create tenant")
	defer apiClient.ClusterV1().Tenant().Delete(context.Background(), defTenant.GetObjectMeta())

	// start TCP server - 1 to receive syslog messages
	ln1, receivedMsgsAtTCPServer, err := serviceutils.StartTCPServer(":0", 5000, 10*time.Millisecond)
	AssertOk(t, err, "failed to start TCP server, err: %v", err)
	defer ln1.Close()
	tmp2 := strings.Split(ln1.Addr().String(), ":")

	// add event policy - 1
	eventPolicy1 := policygen.CreateEventPolicyObj(globals.DefaultTenant, globals.DefaultNamespace, "ep-1",
		monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
		[]*monitoring.ExportConfig{
			{ // receivedMsgsAtTCPServer
				Destination: "127.0.0.1",
				Transport:   fmt.Sprintf("tcp/%s", tmp2[len(tmp2)-1]),
			},
		}, nil)
	eventPolicy1, err = apiClient.MonitoringV1().EventPolicy().Create(context.Background(), eventPolicy1)
	AssertOk(t, err, "failed to create event policy, err: %v", err)
	defer apiClient.MonitoringV1().EventPolicy().Delete(context.Background(), eventPolicy1.GetObjectMeta())

	// record events
	wg.Add(1)
	count := 0
	go func() {
		defer wg.Done()
		recorderEventsDir, err := ioutil.TempDir("", t.Name())
		AssertOk(t, err, "failed to create recorder events directory")
		defer os.RemoveAll(recorderEventsDir)

		evtsRecorder, err := recorder.NewRecorder(&recorder.Config{
			Component:                   uuid.NewV4().String(),
			EvtsProxyURL:                ti.evtProxyServices.EvtsProxy.RPCServer.GetListenURL(),
			BackupDir:                   recorderEventsDir,
			SkipCategoryBasedEventTypes: true}, ti.logger)
		if err != nil {
			ti.logger.Errorf("failed to create recorder, err: %v", err)
			return
		}
		ti.recorders.Lock()
		ti.recorders.list = append(ti.recorders.list, evtsRecorder)
		ti.recorders.Unlock()

		for {
			select {
			case <-stopEvtsRecorder:
				return
			case <-time.After(10 * time.Millisecond):
				count++
				evtsRecorder.Event(eventtypes.SERVICE_RUNNING, fmt.Sprintf("message-%d", count), nil)
			}
		}
	}()

	// receive messages from the syslog server
	wg.Add(1)
	receivedMsgs := 0
	go func() {
		defer wg.Done()
		for {
			select {
			case <-stopSyslogMessagesReceiver:
				return
			case msg, ok := <-receivedMsgsAtTCPServer:
				if !ok {
					return
				}
				if !syslog.ValidateSyslogMessage(monitoring.MonitoringExportFormat_SYSLOG_BSD, msg) {
					ti.logger.Fatalf("invalid message format, expected: RFC5424, got: %v", msg)
				}
				receivedMsgs++
			}
		}
	}()

	time.Sleep(30 * time.Second)

	// restart events proxy
	wg.Add(1)
	go func() {
		defer wg.Done()
		proxyURL := ti.evtProxyServices.EvtsProxy.RPCServer.GetListenURL()

		time.Sleep(1 * time.Second)
		ti.evtProxyServices.Stop()

		// proxy won't be able to accept any events for 2s
		time.Sleep(1 * time.Second)
		evtProxyServices, evtsProxyURL, storeConfig, err := testutils.StartEvtsProxy(t.Name(), proxyURL, ti.mockResolver, ti.logger, ti.dedupInterval, ti.batchInterval, ti.storeConfig)
		if err != nil {
			log.Fatalf("failed to start events proxy, err: %v", err)
		}

		ti.evtProxyServices = evtProxyServices
		ti.storeConfig = storeConfig
		ti.updateResolver(globals.EvtsProxy, evtsProxyURL)

		// let the recorders send some events after the proxy restart
		time.Sleep(3 * time.Second)
		close(stopEvtsRecorder)
	}()

	// ensure the syslog server receives
	AssertEventually(t,
		func() (bool, interface{}) {
			if receivedMsgs >= count {
				return true, nil
			}
			return false, fmt.Sprintf("expected: %d messages to be received on the TCP syslog server, got: %d", count, receivedMsgs)
		}, "syslog server did not receive the expected messages", "1s", "120s")

	close(stopSyslogMessagesReceiver)
	wg.Wait()
}

// TestEventsMgrWithElasticRestart test evtsmgr behavior with elastic restarts. evtsmgr should be intact
// and ES client connection should be reset automatically.
func TestEventsMgrWithElasticRestart(t *testing.T) {
	ti := tInfo{dedupInterval: 300 * time.Second, batchInterval: 100 * time.Millisecond}
	AssertOk(t, ti.setup(t), "failed to setup test")
	defer ti.teardown()

	numRecorders := 3

	stopEventRecorders := make(chan struct{})
	wg := new(sync.WaitGroup)
	wg.Add(numRecorders + 1) // +1 for elastic restart go routine

	// uuid to make each source unique
	componentID := uuid.NewV4().String()
	totalEventsSentBySrc := make([]int, numRecorders)

	// create recorder events directory
	recorderEventsDir, err := ioutil.TempDir("", "")
	AssertOk(t, err, "failed to create recorder events directory")
	defer os.RemoveAll(recorderEventsDir)
	for i := 0; i < numRecorders; i++ {
		go func(i int) {
			evtsRecorder, err := recorder.NewRecorder(&recorder.Config{
				Component:                   fmt.Sprintf("%v-%v", componentID, i),
				EvtsProxyURL:                ti.evtProxyServices.EvtsProxy.RPCServer.GetListenURL(),
				BackupDir:                   recorderEventsDir,
				SkipCategoryBasedEventTypes: true}, ti.logger)
			if err != nil {
				log.Errorf("failed to create recorder for source %v", i)
				return
			}
			ti.recorders.Lock()
			ti.recorders.list = append(ti.recorders.list, evtsRecorder)
			ti.recorders.Unlock()

			ticker := time.NewTicker(100 * time.Millisecond)
			for {
				select {
				case <-stopEventRecorders:
					wg.Done()
					return
				case <-ticker.C:
					evtsRecorder.Event(eventtypes.SERVICE_RUNNING, "test event - 1", nil)
					totalEventsSentBySrc[i]++

					evtsRecorder.Event(eventtypes.SERVICE_UNRESPONSIVE, "test event - 2", nil)
					totalEventsSentBySrc[i]++

					evtsRecorder.Event(eventtypes.SERVICE_STOPPED, "test event - 3", nil)
					totalEventsSentBySrc[i]++
				}
			}
		}(i)
	}

	// restart elasticsearch multiple times
	go func() {
		defer wg.Done()

		for i := 0; i < 3; i++ {
			time.Sleep(3 * time.Second)
			testutils.StopElasticsearch(ti.elasticsearchName, ti.elasticsearchDir)
			ti.removeResolverEntry(globals.ElasticSearch, ti.elasticsearchAddr)

			// let elasticsearch come up on the same port as before.
			// so, wait for the port to become available
			AssertEventually(t,
				func() (bool, interface{}) {
					temp := strings.Split(ti.elasticsearchAddr, ":")
					if len(temp) != 2 {
						return false, fmt.Sprintf("invalid elastic addr: %v", ti.elasticsearchAddr)
					}

					port, err := strconv.Atoi(temp[1])
					if err != nil {
						return false, fmt.Sprintf("invalid elastic port: %v", temp[1])

					}
					if getAvailablePort(port, port) == port {
						return true, nil
					}

					return false, fmt.Sprintf("elastic port not yet available")
				}, "port not available to start elasticsearch", "50ms", "5s")
			ti.elasticsearchAddr, ti.elasticsearchDir, err = testutils.StartElasticsearch(ti.elasticsearchName, ti.elasticsearchDir, ti.signer, ti.trustRoots)
			AssertOk(t, err, "failed to start elasticsearch, err: %v", err)
			ti.updateResolver(globals.ElasticSearch, ti.elasticsearchAddr)
		}

		time.Sleep(5 * time.Second)
		close(stopEventRecorders) // stop all the recorders
	}()

	wg.Wait()

	//total events sent by all the recorders
	totalEventsSent := 0
	for _, val := range totalEventsSentBySrc {
		totalEventsSent += val
	}

	// total number of events received at elastic should match the total events sent
	// query all the events received from this source.component
	query := es.NewRegexpQuery("source.component.keyword", fmt.Sprintf("%v-.*", componentID))
	ti.assertElasticUniqueEvents(t, query, true, 3*numRecorders, "120s")
	ti.assertElasticTotalEvents(t, query, false, totalEventsSent, "120s")
	Assert(t, ti.esClient.GetResetCount() > 0, "client should have restarted")
}

// ensures the delivery of expected syslog messages
func testSyslogMessageDelivery(t *testing.T, ti tInfo, dummyObjRef *cluster.Node, messages map[chan string][]struct {
	Substrs   []string
	MsgFormat monitoring.MonitoringExportFormat
}) {

	var m sync.Mutex
	wg := new(sync.WaitGroup)
	wg.Add(1) // events recorder

	// define list of events to be recorded
	recordEvents := []*struct {
		eventType eventtypes.EventType
		message   string
		objRef    interface{}
		repeat    int // number of times to repeat the event
	}{
		{eventtypes.SERVICE_STARTED, fmt.Sprintf("(tenant:%s) test %s started on %s", dummyObjRef.Tenant, t.Name(), dummyObjRef.GetKind()), *dummyObjRef, 10},
		{eventtypes.SERVICE_RUNNING, fmt.Sprintf("(tenant:%s) test %s running on %s", dummyObjRef.Tenant, t.Name(), dummyObjRef.GetKind()), *dummyObjRef, 10},
		{eventtypes.SERVICE_UNRESPONSIVE, fmt.Sprintf("(tenant:%s) test %s unresponsive on %s", dummyObjRef.Tenant, t.Name(), dummyObjRef.GetKind()), *dummyObjRef, 15},
		{eventtypes.SERVICE_STOPPED, fmt.Sprintf("(tenant:%s) test %s stopped on %s", dummyObjRef.Tenant, t.Name(), dummyObjRef.GetKind()), *dummyObjRef, 11},

		{eventtypes.ELECTION_STARTED, fmt.Sprintf("(tenant:%s) dummy election: election started %s", dummyObjRef.Tenant, t.Name()), *dummyObjRef, 10},
		{eventtypes.LEADER_ELECTED, fmt.Sprintf("(tenant:%s) dummy election: leader elected %s", dummyObjRef.Tenant, t.Name()), *dummyObjRef, 10},
		{eventtypes.LEADER_CHANGED, fmt.Sprintf("(tenant:%s) dummy election: leader changed %s", dummyObjRef.Tenant, t.Name()), *dummyObjRef, 15},
		{eventtypes.LEADER_LOST, fmt.Sprintf("(tenant:%s) dummy election: leader lost %s", dummyObjRef.Tenant, t.Name()), *dummyObjRef, 11},
		{eventtypes.ELECTION_STOPPED, fmt.Sprintf("(tenant:%s) dummy election: election stopped %s", dummyObjRef.Tenant, t.Name()), *dummyObjRef, 15},

		// events in non default tenant
		{eventtypes.SERVICE_STARTED, fmt.Sprintf("(tenant:%s) test %s started", globals.DefaultTenant, t.Name()), nil, 10},
		{eventtypes.SERVICE_RUNNING, fmt.Sprintf("(tenant:%s) test %s running", globals.DefaultTenant, t.Name()), nil, 10},
		{eventtypes.SERVICE_UNRESPONSIVE, fmt.Sprintf("(tenant:%s) test %s unresponsive", globals.DefaultTenant, t.Name()), nil, 15},
		{eventtypes.SERVICE_STOPPED, fmt.Sprintf("(tenant:%s) test %s stopped", globals.DefaultTenant, t.Name()), nil, 11},
	}

	// start recorder
	recorderEventsDir, err := ioutil.TempDir("", t.Name())
	AssertOk(t, err, "failed to create recorder events directory")
	defer os.RemoveAll(recorderEventsDir)
	go func() {
		defer wg.Done()

		evtsRecorder, err := recorder.NewRecorder(&recorder.Config{
			Component:                   uuid.NewV4().String(),
			EvtsProxyURL:                ti.evtProxyServices.EvtsProxy.RPCServer.GetListenURL(),
			BackupDir:                   recorderEventsDir,
			SkipCategoryBasedEventTypes: true}, ti.logger)
		if err != nil {
			log.Errorf("failed to create recorder, err: %v", err)
			return
		}
		ti.recorders.Lock()
		ti.recorders.list = append(ti.recorders.list, evtsRecorder)
		ti.recorders.Unlock()

		// record events
		for i := range recordEvents {
			if objRef, ok := recordEvents[i].objRef.(cluster.Node); ok {
				objRef.ObjectMeta.Name = CreateAlphabetString(5)
				recordEvents[i].objRef = &objRef
			}
			for j := 0; j < recordEvents[i].repeat; j++ {
				evtsRecorder.Event(recordEvents[i].eventType, recordEvents[i].message, recordEvents[i].objRef)
			}
		}

		// wait for the batch interval
		time.Sleep(ti.batchInterval + 10*time.Millisecond)
		// resend the events again after batch interval, this should increase the hits but not recreate the alerts as per our alert policy
		// thus, no alert export for these events.
		for i := range recordEvents {
			evtsRecorder.Event(recordEvents[i].eventType, recordEvents[i].message, recordEvents[i].objRef)
		}
	}()

	for messageCh, expectedMessages := range messages {
		closeMsgCh := make(chan struct{})

		// ensure all the alerts are exported to the given syslog(UDP/TCP) server in the respective format.
		wg.Add(1)
		go func() {
			defer wg.Done()

			for {
				select {
				case <-closeMsgCh:
					return
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
				if len(expectedMessages) != 0 {
					return false, fmt.Sprintf("pending: %v", len(expectedMessages))
				}
				return true, nil
			}, "did not receive all the expected syslog messages", "200ms", "10s")

		close(closeMsgCh)
	}

	wg.Wait()
}

// getAvailablePort returns the port available between [from, to]
func getAvailablePort(from, to int) int {
	for port := from; port <= to; port++ {
		ln, err := net.Listen("tcp", fmt.Sprintf(":%d", port))
		if err != nil { // port not available
			continue
		}

		if err := ln.Close(); err != nil {
			continue
		}
		return port
	}

	return 0
}
