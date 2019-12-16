// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package search

import (
	"context"
	"crypto/tls"
	"crypto/x509"
	"fmt"
	"math/rand"
	"net/url"
	"os"
	"strconv"
	"testing"
	"time"

	ptypes "github.com/gogo/protobuf/types"
	grpcruntime "github.com/pensando/grpc-gateway/runtime"
	uuid "github.com/satori/go.uuid"
	grpccodes "google.golang.org/grpc/codes"

	"github.com/pensando/sw/api"
	apicache "github.com/pensando/sw/api/client"
	"github.com/pensando/sw/api/fields"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	_ "github.com/pensando/sw/api/generated/exports/apigw"
	_ "github.com/pensando/sw/api/generated/exports/apiserver"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/search"
	"github.com/pensando/sw/api/generated/security"
	_ "github.com/pensando/sw/api/hooks/apiserver"
	"github.com/pensando/sw/api/labels"
	"github.com/pensando/sw/api/login"
	testutils "github.com/pensando/sw/test/utils"
	"github.com/pensando/sw/venice/apigw"
	_ "github.com/pensando/sw/venice/apigw/svc"
	"github.com/pensando/sw/venice/apiserver"
	types "github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/ctrler/evtsmgr"
	"github.com/pensando/sw/venice/globals"
	pcache "github.com/pensando/sw/venice/spyglass/cache"
	"github.com/pensando/sw/venice/spyglass/finder"
	"github.com/pensando/sw/venice/spyglass/indexer"
	. "github.com/pensando/sw/venice/utils/authn/testutils"
	"github.com/pensando/sw/venice/utils/authz"
	"github.com/pensando/sw/venice/utils/certs"
	"github.com/pensando/sw/venice/utils/elastic"
	"github.com/pensando/sw/venice/utils/events"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
	mockresolver "github.com/pensando/sw/venice/utils/resolver/mock"
	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/runtime"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/testutils/serviceutils"
)

const (
	// TLS keys and certificates used by mock CKM endpoint to generate control-plane certs
	certPath  = "../../../venice/utils/certmgr/testdata/ca.cert.pem"
	keyPath   = "../../../venice/utils/certmgr/testdata/ca.key.pem"
	rootsPath = "../../../venice/utils/certmgr/testdata/roots.pem"

	from       = int32(0)
	maxResults = int32(50)
	// objectCount is count of objects perk Kind to be generated
	objectCount int64 = 5
	eventCount  int64 = 100
	// SGpolicy object count
	sgPolicyCount = 1
)

var (
	logger = log.SetConfig(log.GetDefaultConfig("spyglass_integ_test"))

	// create events recorder
	_, _ = recorder.NewRecorder(&recorder.Config{
		Component:     "spyglass_integ_test",
		BackupDir:     "/tmp",
		SkipEvtsProxy: true}, logger)
)

type SearchMethod uint8

const (
	GetWithURI SearchMethod = iota
	GetWithBody
	PostWithBody
)

type testInfo struct {
	l                 log.Logger
	apiServer         apiserver.Server
	apiServerAddr     string
	apiClient         apiclient.Services
	apiGw             apigw.APIGateway
	apiGwAddr         string
	elasticURL        string
	elasticServerName string
	elasticDir        string
	fdr               finder.Interface
	fdrAddr           string
	idr               indexer.Interface
	authzHeader       string
	mockResolver      *mockresolver.ResolverClient
	evtsMgr           *evtsmgr.EventsManager
	evtProxyServices  *testutils.EvtProxyServices
	evtsStoreConfig   *events.StoreConfig
	pcache            pcache.Interface
	esClient          elastic.ESClient    // elastic client
	signer            certs.CSRSigner     // function to sign CSRs for TLS
	trustRoots        []*x509.Certificate // trust roots to verify TLS certs
}

var tInfo testInfo

func (tInfo *testInfo) setup(t *testing.T) error {
	var err error

	tInfo.evtsStoreConfig = &events.StoreConfig{}

	// We need a fairly high limit because all clients are collapsed into a single process
	// so they hit the same rate limiter
	rpckit.SetDefaultListenerConnectionRateLimit(50)

	// start certificate server
	err = testutils.SetupIntegTLSProvider()
	if err != nil {
		log.Fatalf("Error setting up TLS provider: %v", err)
	}

	tInfo.signer, _, tInfo.trustRoots, err = testutils.GetCAKit()
	if err != nil {
		log.Errorf("Error getting CA artifacts: %v", err)
		return err
	}

	// start elasticsearch
	tInfo.elasticServerName = uuid.NewV4().String()
	tInfo.elasticURL, tInfo.elasticDir, err = testutils.StartElasticsearch(tInfo.elasticServerName, tInfo.elasticDir, tInfo.signer, tInfo.trustRoots)
	if err != nil {
		return fmt.Errorf("failed to start elasticsearch, err: %v", err)
	}
	tInfo.updateResolver(globals.ElasticSearch, tInfo.elasticURL) // add mock elastic service to mock resolver

	tInfo.esClient, err = testutils.CreateElasticClient(tInfo.elasticURL, tInfo.mockResolver, tInfo.l.WithContext("submodule", "elastic"), tInfo.signer, tInfo.trustRoots)
	if err != nil {
		log.Errorf("Error creating elastic client: %v", err)
		return err
	}

	if !testutils.IsElasticClusterHealthy(tInfo.esClient) {
		return fmt.Errorf("elasticsearch cluster not healthy")
	}

	// Create new policy cache for spyglass
	tInfo.pcache = pcache.NewCache(tInfo.l)

	// start spyglass finder
	fdr, fdrAddr, err := testutils.StartSpyglass("finder", "", tInfo.mockResolver, tInfo.pcache, tInfo.l, tInfo.esClient)
	if err != nil {
		return err
	}
	tInfo.fdr = fdr.(finder.Interface)
	tInfo.fdrAddr = fdrAddr
	tInfo.updateResolver(globals.Spyglass, tInfo.fdrAddr)

	// start API server
	tInfo.apiServer, tInfo.apiServerAddr, err = serviceutils.StartAPIServer(":0", t.Name(), tInfo.l)
	if err != nil {
		return err
	}
	tInfo.updateResolver(globals.APIServer, tInfo.apiServerAddr)

	// start API gateway
	tInfo.apiGw, tInfo.apiGwAddr, err = testutils.StartAPIGateway(":0", false,
		map[string]string{}, []string{"telemetry_query", "objstore", "tokenauth"}, []string{}, tInfo.mockResolver, tInfo.l)
	if err != nil {
		return err
	}

	// start spygalss indexer
	idr, _, err := testutils.StartSpyglass("indexer", globals.APIServer, tInfo.mockResolver, tInfo.pcache, tInfo.l, tInfo.esClient)
	if err != nil {
		return err
	}
	tInfo.idr = idr.(indexer.Interface)

	// start evtsmgr
	evtsMgr, evtsMgrURL, err := testutils.StartEvtsMgr(":0", tInfo.mockResolver, tInfo.l, tInfo.esClient, nil)
	if err != nil {
		log.Errorf("failed to start events manager, err: %v", err)
		return err
	}
	tInfo.evtsMgr = evtsMgr
	tInfo.updateResolver(globals.EvtsMgr, evtsMgrURL)

	// start evtsproxy
	evtProxyServices, evtsProxyURL, evtsStoreConfig, err := testutils.StartEvtsProxy(t.Name(), ":0", tInfo.mockResolver, tInfo.l, 10*time.Second, 100*time.Millisecond, tInfo.evtsStoreConfig)
	if err != nil {
		log.Errorf("failed to start events proxy, err: %v", err)
		return err
	}
	tInfo.evtProxyServices = evtProxyServices
	tInfo.evtsStoreConfig = evtsStoreConfig
	tInfo.updateResolver(globals.EvtsProxy, evtsProxyURL)

	// create API server client
	apiCl, err := apicache.NewGrpcUpstream("spyglass-integ-test", tInfo.apiServerAddr, tInfo.l)
	if err != nil {
		return fmt.Errorf("failed to create gRPC client, err: %v", err)
	}
	tInfo.apiClient = apiCl

	// setup auth
	userCreds := &auth.PasswordCredential{Username: testutils.TestLocalUser, Password: testutils.TestLocalPassword, Tenant: testutils.TestTenant}
	err = testutils.SetupAuth(tInfo.apiServerAddr, true, &auth.Ldap{Enabled: false}, &auth.Radius{Enabled: false}, userCreds, tInfo.l)
	if err != nil {
		return err
	}

	// get authZ header
	tInfo.authzHeader, err = testutils.GetAuthorizationHeader(tInfo.apiGwAddr, userCreds)
	if err != nil {
		return err
	}

	return nil
}

func (tInfo *testInfo) teardown() {
	// stop finder
	tInfo.fdr.Stop()

	// stop indexer
	tInfo.idr.Stop()

	// stop elasticsearch
	testutils.StopElasticsearch(tInfo.elasticServerName, tInfo.elasticDir)

	// stop apiGW
	tInfo.apiGw.Stop()

	// stop evtsmgr
	tInfo.evtsMgr.Stop()
	tInfo.esClient = nil

	// stop evtproxy services
	tInfo.evtProxyServices.Stop()

	// delete the tmp events directory
	os.RemoveAll(tInfo.evtsStoreConfig.Dir)

	// stop apiServer
	tInfo.apiServer.Stop()

	// stop certificate server
	testutils.CleanupIntegTLSProvider()

	tInfo.mockResolver = nil
}

func (tInfo *testInfo) verifyElasticDocumentCount(t *testing.T, expectedIndexes uint64) {
	AssertEventually(t,
		func() (bool, interface{}) {
			stats, err := tInfo.esClient.GetIndicesStats(context.Background(), []string{})
			if err != nil {
				return false, fmt.Errorf("Failed to get index stats. err %v", err)
			}
			index, ok := stats.Indices[elastic.GetIndex(globals.Configs, globals.DefaultTenant)]
			if !ok {
				return false, fmt.Errorf("Elastic didn't have index for default tenant")
			}
			elasticCount := index.Primaries.Docs.Count
			if uint64(elasticCount) < expectedIndexes {
				return false, fmt.Errorf("Elastic index document mismatch - expected: %d actual: %d",
					expectedIndexes, elasticCount)
			}
			return true, nil
		}, "Failed to match index operations counter", "1s", "2m")
}

// updateResolver helper function to update mock resolver with the given service and URL
func (tInfo *testInfo) updateResolver(serviceName, url string) {
	tInfo.mockResolver.AddServiceInstance(&types.ServiceInstance{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceInstance",
		},
		ObjectMeta: api.ObjectMeta{
			Name: serviceName,
		},
		Service: serviceName,
		URL:     url,
	})
}

// removeResolverEntry helper function to remove entry from mock resolver
func (tInfo *testInfo) removeResolverEntry(serviceName, url string) {
	tInfo.mockResolver.DeleteServiceInstance(&types.ServiceInstance{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceInstance",
		},
		ObjectMeta: api.ObjectMeta{
			Name: serviceName,
		},
		Service: serviceName,
		URL:     url,
	})
}

func getSearchURLWithParams(t *testing.T, query string, from, maxResults int32, mode, sortBy string,
	aggregate bool, tenants []string) string {

	// convert to query-string to url-encoded string
	// to escape special characters like space, comma, braces.
	u := url.URL{Path: query}
	urlQuery := url.QueryEscape(query)
	t.Logf("Query: %s Encoded: %s Escaped: %s Mode: %s SortBy: %s",
		query, u.String(), urlQuery, mode, sortBy)
	str := fmt.Sprintf("https://%s/search/v1/query?QueryString=%s&From=%d&MaxResults=%d",
		tInfo.apiGwAddr, urlQuery, from, maxResults)
	if mode != "" {
		str += fmt.Sprintf("&Mode=%s", mode)
	}
	if sortBy != "" {
		str += fmt.Sprintf("&SortBy=%s", sortBy)
	}
	str += fmt.Sprintf("&Aggregate=%s", strconv.FormatBool(aggregate))
	for _, tenant := range tenants {
		str += fmt.Sprintf("&Tenants=%s", tenant)
	}
	fmt.Printf("\n+++ QUERY URL: %s\n", str)
	return str
}

func getSearchURL() string {
	return fmt.Sprintf("https://%s/search/v1/query", tInfo.apiGwAddr)
}

func getPolicySearchURL() string {
	return fmt.Sprintf("https://%s/search/v1/policy-query", tInfo.apiGwAddr)
}

// Starts Spyglass
// Checks objects are written to elastic
// restarts Api Server and checks indexer watch is restablished
// Deletes objects and checks elastic entries are deleted
// Kills elastic and new objects are created
// Startups elastic again and checks new objects were indexed
// Checks indexer's buffer overflows
// Test for deleted objects while spyglass is down
func TestSpyglassErrorHandling(t *testing.T) {
	var err error

	tInfo.mockResolver = mockresolver.New()
	tInfo.l = logger.WithContext("t_name", t.Name())

	tInfo.l.Info("Starting Spyglass Error Handling Test")

	AssertOk(t, tInfo.setup(t), "failed to setup test")
	defer tInfo.teardown()

	tInfo.l.Info("Setup complete")

	ctx := context.Background()

	// Generate objects in api-server to trigger indexer watch
	go DefaultTenantPolicyGenerator(ctx, tInfo.apiClient, objectCount, 0)

	// Objects that already exist: Auth policy, cluster, tenant, admin role, admin role binding, admin user
	existingObjects := uint64(6)
	expectedIndexes := uint64(objectCount) + existingObjects

	// Should be the same as the number of entries in elastic
	tInfo.verifyElasticDocumentCount(t, expectedIndexes)

	// Even though we have the correct number of entries, update events could still be processing
	// We wait to remove flakiness
	time.Sleep(5 * time.Second)

	currentWriteCount := tInfo.idr.GetWriteCount()

	// Stop API server
	t.Logf("Restarting API server...")
	tInfo.l.Info("Stopping API Server")
	tInfo.apiServer.Stop()
	tInfo.removeResolverEntry(globals.APIServer, tInfo.apiServerAddr)

	time.Sleep(5 * time.Second)

	// restart API server
	tInfo.l.Info("Restating API Server")
	tInfo.apiServer, tInfo.apiServerAddr, err = serviceutils.StartAPIServer(":0", t.Name(), tInfo.l)
	AssertOk(t, err, "Failed to start api server")
	tInfo.updateResolver(globals.APIServer, tInfo.apiServerAddr)

	// recreate apiclient
	AssertEventually(t, func() (bool, interface{}) {
		apiCl, err := apicache.NewGrpcUpstream("spyglass-integ-test", tInfo.apiServerAddr, tInfo.l)
		if err != nil {
			return false, err
		}
		tInfo.apiClient = apiCl
		return true, nil
	}, "failed to create gRPC client", "1s", "2m")

	DefaultTenantPolicyGenerator(ctx, tInfo.apiClient, objectCount, objectCount)
	expectedIndexes += uint64(objectCount)

	// Validate the object count matches after restart
	AssertEventually(t,
		func() (bool, interface{}) {
			// Should have only received updates for the new objects
			// We may also have updates for existing objects, but we should receive
			// less events than there are objects
			if tInfo.idr.GetWriteCount()-currentWriteCount < expectedIndexes {
				return false, fmt.Errorf("New indexed objects count mismatch expected: %d actual: %d",
					expectedIndexes, tInfo.idr.GetWriteCount()-currentWriteCount)
			}
			return true, nil
		}, "Failed to match count of indexed objects", "1s", "2m")

	tInfo.verifyElasticDocumentCount(t, expectedIndexes)

	tInfo.l.Info("Deleting objects")

	// Verify deleting objects
	DeleteDefaultTenantPolicyGenerator(ctx, tInfo.apiClient, objectCount, 0)
	expectedIndexes -= uint64(objectCount)

	tInfo.verifyElasticDocumentCount(t, expectedIndexes)

	// Killing elastic and writing new objects to apiserver
	// When elastic comes back up, indexer should write the new objects
	tInfo.l.Info("Stopping elastic ...")
	err = testutils.StopElasticsearch(tInfo.elasticServerName, tInfo.elasticDir)
	AssertOk(t, err, "Failed to stop elastic search")

	// Not run as a go routine because we want it to finish creating before we bring up elastic
	DefaultTenantPolicyGenerator(ctx, tInfo.apiClient, objectCount, 2*objectCount)

	tInfo.l.Info("Starting elastic")
	tInfo.elasticURL, tInfo.elasticDir, err = testutils.StartElasticsearch(tInfo.elasticServerName, tInfo.elasticDir, tInfo.signer, tInfo.trustRoots)
	AssertOk(t, err, "Failed to start elastic search")
	tInfo.updateResolver(globals.ElasticSearch, tInfo.elasticURL) // add mock elastic service to mock resolver

	// Should only write new objects into elastic
	expectedIndexes = uint64(objectCount)

	// Should be the same as the number of entries in elastic
	tInfo.verifyElasticDocumentCount(t, expectedIndexes)

	tInfo.l.Info("Test Complete")
	t.Logf("Done with Tests ....")
}

// TestSpyglass does e2e test for Spyglass search service
// - brings up elasticsearch docker
// - brings up finder gRPC server which is the search backend
// - brings up api-server, api-gw service
// - invokes policyGenerator to populate objects in api-server
// - brings up indexer which launches the watchers and indexes
//   the objects received into ElasticDB
// - performs Term & Text Query tests
// - shuts down elasticsearch, spyglass, api-gw & api-server.
func TestSpyglass(t *testing.T) {
	var err error

	tInfo.mockResolver = mockresolver.New()
	tInfo.l = logger.WithContext("t_name", t.Name())
	tInfo.l.Info("Starting Spyglass test")

	AssertOk(t, tInfo.setup(t), "failed to setup test")
	defer tInfo.teardown()

	tInfo.l.Info("Setup complete")

	ctx := context.Background()

	// Generate objects in api-server to trigger indexer watch
	go PolicyGenerator(ctx, tInfo.apiClient, objectCount)

	// generate events
	go recordEvents(tInfo.mockResolver.GetURLs(globals.EvtsProxy)[0], tInfo.evtsStoreConfig.Dir, eventCount, logger)

	// Validate the index operations counter
	expectedCount := uint64(3*objectCount+int64(len(Tenants))) + sgPolicyCount
	expectedCount += 15 // for cluster, default tenant, auth policy, test user, tesla admin role, tesla admin role binding, audi admin role, audi admin role binding, default admin role, default admin role binding & security groups
	AssertEventually(t,
		func() (bool, interface{}) {
			if tInfo.idr.GetWriteCount() < expectedCount {
				t.Logf("Retrying, index operations counter mismatch - expected: %d actual: %d",
					expectedCount, tInfo.idr.GetWriteCount())
				return false, nil
			}
			return true, nil
		}, "Failed to match index operations counter", "20ms", "2m")

	var resp search.SearchResponse

	// Validate the search REST endpoint
	t.Logf("Validating Search REST endpoint ...")
	tInfo.l.Info("Validating Search endpoint")
	AssertEventually(t,
		func() (bool, interface{}) {

			restcl := netutils.NewHTTPClient()
			restcl.WithTLSConfig(&tls.Config{InsecureSkipVerify: true})
			restcl.SetHeader("Authorization", tInfo.authzHeader)
			restcl.DisableKeepAlives()
			defer restcl.CloseIdleConnections()
			_, err = restcl.Req("GET", getSearchURLWithParams(t, "tesla", 0, 10, search.SearchRequest_Full.String(),
				"", true, []string{"tesla"}), nil, &resp)
			if err != nil {
				t.Logf("GET on search REST endpoint: %s failed, err:%+v",
					getSearchURLWithParams(t, "tesla", 0, 10, search.SearchRequest_Full.String(), "",
						true, []string{"tesla"}), err)
				return false, nil
			}
			return true, nil
		}, "Failed to validate Search REST endpoint", "20ms", "2m")

	// Perform search tests
	performSearchTests(t, GetWithURI)
	performSearchTests(t, GetWithBody)
	performSearchTests(t, PostWithBody)
	performPolicySearchTests(t, GetWithBody)
	performPolicySearchTests(t, PostWithBody)
	testAuthzInSearch(t, GetWithURI)

	// Stop Indexer
	t.Logf("Stopping indexer ...")
	tInfo.l.Info("Stopping indexer")
	tInfo.idr.Stop()

	// create the indexer again
	t.Logf("Creating and starting new indexer instance")
	// recreating elastic client as it should have been closed during idr.stop()
	tInfo.esClient, err = testutils.CreateElasticClient(tInfo.elasticURL, tInfo.mockResolver, tInfo.l.WithContext("submodule", "elastic"), tInfo.signer, tInfo.trustRoots)
	AssertOk(t, err, "Error creating elastic client")

	AssertEventually(t,
		func() (bool, interface{}) {
			spyglassOpts := func(idr *indexer.Indexer) {
				indexer.DisableVOSWatcher()(idr)
				indexer.WithElasticClient(tInfo.esClient)(idr)
			}
			tInfo.idr, err = indexer.NewIndexer(ctx, tInfo.apiServerAddr, tInfo.mockResolver, tInfo.pcache, tInfo.l, spyglassOpts)
			if err != nil {
				t.Logf("Error creating indexer: %v", err)
				return false, nil
			}
			return true, nil
		}, "Failed to create indexer", "20ms", "1m")

	// start the indexer
	tInfo.l.Info("Starting indexer")
	indexerCreate := make(chan error)
	go func() {
		tInfo.idr.Start() // start the indexer
		if err != nil {
			log.Errorf("Indexer exited err: %v", err)
			indexerCreate <- err
		}
	}()
	select {
	case <-indexerCreate:
		t.Fatal("failed to start indexer")
	case <-time.After(time.Second * 5):
	}

	// Validate the object count matches after restart
	AssertEventually(t,
		func() (bool, interface{}) {

			if tInfo.idr.GetWriteCount() < expectedCount {
				t.Logf("Retrying, indexed objects count mismatch expected: %d actual: %d",
					expectedCount, tInfo.idr.GetWriteCount())
				return false, nil
			}
			return true, nil
		}, "Failed to match count of indexed objects", "20ms", "2m")

	// Perform search tests again after indexer restart
	performSearchTests(t, GetWithURI)
	performSearchTests(t, GetWithBody)
	performSearchTests(t, PostWithBody)
	performPolicySearchTests(t, GetWithBody)
	performPolicySearchTests(t, PostWithBody)
	testAuthzInSearch(t, GetWithURI)
	tInfo.idr.Stop()
	tInfo.l.Info("Test complete")
	t.Logf("Done with Tests ....")
}

// Execute search test cases
func performSearchTests(t *testing.T, searchMethod SearchMethod) {

	t.Logf("@@@ performSearchTests, method: %d", searchMethod)
	tInfo.l.Infof("@@@ performSearchTests, method: %d", searchMethod)

	// Http error for InvalidArgument test cases
	httpInvalidArgErrCode := grpcruntime.HTTPStatusFromCode(grpccodes.InvalidArgument)
	httpInvalidArgErr := fmt.Errorf("Server responded with %d", httpInvalidArgErrCode)

	// Testcases for various queries on config objects
	queryTestcases := []struct {
		query          search.SearchRequest
		mode           string
		sortBy         string
		expectedHits   int64
		previewResults map[string]map[string]map[string]int64
		aggResults     map[string]map[string]map[string]map[string]interface{}
		err            error
	}{
		//
		// Search Test cases with URI params, to test QueryString query
		// Uses GET method
		//
		{
			search.SearchRequest{
				QueryString: "kind:Tenant",
				From:        from,
				MaxResults:  maxResults,
				Aggregate:   true,
			},
			search.SearchRequest_Full.String(),
			"",
			int64(len(Tenants) + 1),
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"default": {
					"Cluster": {
						"Tenant": {
							"tesla":   nil,
							"audi":    nil,
							"default": nil,
						},
					},
				},
			},
			nil,
		},
		{
			search.SearchRequest{
				QueryString: "kind:DistributedServiceCard",
				From:        from,
				MaxResults:  maxResults,
				Aggregate:   true,
			},
			search.SearchRequest_Full.String(),
			"",
			objectCount,
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"default": {
					"Cluster": {
						"DistributedServiceCard": {
							"4444.4400.0000": nil,
							"4444.4400.0001": nil,
							"4444.4400.0002": nil,
							"4444.4400.0003": nil,
							"4444.4400.0004": nil,
						},
					},
				},
			},
			nil,
		},
		{ // no aggregation should be performed on the results
			search.SearchRequest{
				QueryString: "kind:DistributedServiceCard",
				From:        from,
				MaxResults:  maxResults,
				Aggregate:   false,
			},
			search.SearchRequest_Full.String(),
			"",
			objectCount,
			nil,
			nil,
			nil,
		},
		{
			// Test Paginated query #1
			// Use sort option for deterministic results
			search.SearchRequest{
				QueryString: "kind:DistributedServiceCard",
				From:        0, // from offset-0
				MaxResults:  3, // get 3 results
				Aggregate:   true,
			},
			search.SearchRequest_Full.String(),
			"meta.name.keyword",
			3,
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"default": {
					"Cluster": {
						"DistributedServiceCard": {
							"4444.4400.0000": nil,
							"4444.4400.0001": nil,
							"4444.4400.0002": nil,
						},
					},
				},
			},
			nil,
		},
		{
			// Test Paginated query #2
			// Use sort option for deterministic results
			search.SearchRequest{
				QueryString: "kind:DistributedServiceCard",
				From:        3, // from offset-3
				MaxResults:  2, // get 2 results
				Aggregate:   true,
			},
			search.SearchRequest_Full.String(),
			"meta.name.keyword",
			2,
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"default": {
					"Cluster": {
						"DistributedServiceCard": {
							"4444.4400.0003": nil,
							"4444.4400.0004": nil,
						},
					},
				},
			},
			nil,
		},
		{
			search.SearchRequest{
				QueryString: "kind:Network",
				From:        from,
				MaxResults:  maxResults,
				Tenants:     []string{"audi", "tesla"},
				Aggregate:   true,
			},
			search.SearchRequest_Full.String(),
			"",
			objectCount,
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"audi": {
					"Network": {
						"Network": {
							"net01": nil,
							"net03": nil,
						},
					},
				},
				"tesla": {
					"Network": {
						"Network": {
							"net00": nil,
							"net02": nil,
							"net04": nil,
						},
					},
				},
			},
			nil,
		},
		{
			search.SearchRequest{
				QueryString: "kind:SecurityGroup",
				From:        from,
				MaxResults:  maxResults,
				Tenants:     []string{"audi", "tesla"},
				Aggregate:   true,
			},
			search.SearchRequest_Full.String(),
			"",
			objectCount,
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"tesla": {
					"Security": {
						"SecurityGroup": {
							"sg00": nil,
							"sg02": nil,
							"sg04": nil,
						},
					},
				},
				"audi": {
					"Security": {
						"SecurityGroup": {
							"sg01": nil,
							"sg03": nil,
						},
					},
				},
			},
			nil,
		},
		{
			// Term query with multi-value match on Kind
			search.SearchRequest{
				QueryString: "kind:(Network OR SecurityGroup)",
				From:        from,
				MaxResults:  maxResults,
				Tenants:     []string{"audi", "tesla"},
				Aggregate:   true,
			},
			search.SearchRequest_Full.String(),
			"",
			2 * objectCount,
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"tesla": {
					"Network": {
						"Network": {
							"net00": nil,
							"net02": nil,
							"net04": nil,
						},
					},
					"Security": {
						"SecurityGroup": {
							"sg00": nil,
							"sg02": nil,
							"sg04": nil,
						},
					},
				},
				"audi": {
					"Network": {
						"Network": {
							"net01": nil,
							"net03": nil,
						},
					},
					"Security": {
						"SecurityGroup": {
							"sg01": nil,
							"sg03": nil,
						},
					},
				},
			},
			nil,
		},
		{
			// Term Match on Kind and Label attributes
			search.SearchRequest{
				QueryString: "kind:Network AND meta.labels.Application:MS-Exchange",
				From:        from,
				MaxResults:  maxResults,
				Tenants:     []string{"audi", "tesla"},
				Aggregate:   true,
			},
			search.SearchRequest_Full.String(),
			"",
			objectCount,
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"tesla": {
					"Network": {
						"Network": {
							"net00": nil,
							"net02": nil,
							"net04": nil,
						},
					},
				},
				"audi": {
					"Network": {
						"Network": {
							"net01": nil,
							"net03": nil,
						},
					},
				},
			},
			nil,
		},
		{
			// Text search that matches on Kind
			search.SearchRequest{
				QueryString: "DistributedServiceCard",
				From:        from,
				MaxResults:  maxResults,
				Aggregate:   true,
			},
			search.SearchRequest_Full.String(),
			"",
			objectCount,
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"default": {
					"Cluster": {
						"DistributedServiceCard": {
							"4444.4400.0000": nil,
							"4444.4400.0001": nil,
							"4444.4400.0002": nil,
							"4444.4400.0003": nil,
							"4444.4400.0004": nil,
						},
					},
				},
			},
			nil,
		},
		{
			// Text search that matches on meta.Namespace
			search.SearchRequest{
				QueryString: "infra",
				From:        from,
				MaxResults:  8192,
				Aggregate:   true,
			},
			search.SearchRequest_Full.String(),
			"",
			objectCount + int64(len(Tenants)),
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"default": {
					"Cluster": {
						"DistributedServiceCard": {
							"4444.4400.0000": nil,
							"4444.4400.0001": nil,
							"4444.4400.0002": nil,
							"4444.4400.0003": nil,
							"4444.4400.0004": nil,
						},
						"Tenant": {
							"tesla": nil,
							"audi":  nil,
						},
					},
				},
			},
			nil,
		},
		{
			// Text search that matches on meta.Labels.key
			search.SearchRequest{
				QueryString: "us-west",
				From:        from,
				MaxResults:  256,
				Tenants:     []string{"audi", "tesla"},
				Aggregate:   true,
			},
			search.SearchRequest_Full.String(),
			"",
			3*objectCount + int64(len(Tenants)),
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"tesla": {
					"Network": {
						"Network": {
							"net00": nil,
							"net02": nil,
							"net04": nil,
						},
					},
					"Security": {
						"SecurityGroup": {
							"sg00": nil,
							"sg02": nil,
							"sg04": nil,
						},
					},
				},
				"default": {
					"Cluster": {
						"DistributedServiceCard": {
							"4444.4400.0000": nil,
							"4444.4400.0001": nil,
							"4444.4400.0002": nil,
							"4444.4400.0003": nil,
							"4444.4400.0004": nil,
						},
						"Tenant": {
							"tesla": nil,
							"audi":  nil,
						},
					},
				},
				"audi": {
					"Network": {
						"Network": {
							"net01": nil,
							"net03": nil,
						},
					},
					"Security": {
						"SecurityGroup": {
							"sg01": nil,
							"sg03": nil,
						},
					},
				},
			},
			nil,
		},
		{
			// Precise match on a MAC address, part of smartNIC object
			search.SearchRequest{
				QueryString: "4444.4400.0001",
				From:        from,
				MaxResults:  maxResults,
				Aggregate:   true,
			},
			search.SearchRequest_Full.String(),
			"",
			2,
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"default": {
					"Cluster": {
						"DistributedServiceCard": {
							"4444.4400.0001": nil,
						},
					},
					"Monitoring": {
						"Module": {
							"4444.4400.0001-pen-netagent": nil,
						},
					},
				},
			},
			nil,
		},
		{
			// Precise match on a IP address, part of Network object
			search.SearchRequest{
				QueryString: "12.0.1.254",
				From:        from,
				MaxResults:  maxResults,
				Tenants:     []string{"audi", "tesla"},
				Aggregate:   true,
			},
			search.SearchRequest_Full.String(),
			"",
			1,
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"audi": {
					"Network": {
						"Network": {
							"net01": nil,
						},
					},
				},
			},
			nil,
		},

		// Text search (case insensitive) on kind
		{
			search.SearchRequest{
				QueryString: "distributedservicecard",
				From:        from,
				MaxResults:  maxResults,
				Aggregate:   true,
			},
			"",
			"",
			objectCount,
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"default": {
					"Cluster": {
						"DistributedServiceCard": {
							"4444.4400.0000": nil,
							"4444.4400.0001": nil,
							"4444.4400.0002": nil,
							"4444.4400.0003": nil,
							"4444.4400.0004": nil,
						},
					},
				},
			},
			nil,
		},
		// Text search (case insensitive and prefix match) on kind
		{
			search.SearchRequest{
				QueryString: "distributed*",
				From:        from,
				MaxResults:  maxResults,
				Aggregate:   true,
			},
			"",
			"",
			objectCount,
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"default": {
					"Cluster": {
						"DistributedServiceCard": {
							"4444.4400.0000": nil,
							"4444.4400.0001": nil,
							"4444.4400.0002": nil,
							"4444.4400.0003": nil,
							"4444.4400.0004": nil,
						},
					},
				},
			},
			nil,
		},

		// Negative test cases
		{
			// Invalid Kind
			search.SearchRequest{
				QueryString: "kind:Contract",
				From:        from,
				MaxResults:  maxResults,
				Aggregate:   true,
			},
			search.SearchRequest_Full.String(),
			"",
			0,
			nil,
			nil,
			nil,
		},
		{
			// Non-existent Text
			search.SearchRequest{
				QueryString: "OzzyOzbuorne",
				From:        from,
				MaxResults:  maxResults,
				Tenants:     []string{"audi", "tesla"},
				Aggregate:   true,
			},
			search.SearchRequest_Full.String(),
			"",
			0,
			nil,
			nil,
			nil,
		},
		{
			// Invalid QueryString length (> 256)
			search.SearchRequest{
				QueryString: CreateAlphabetString(512),
				From:        from,
				MaxResults:  maxResults,
				Aggregate:   true,
			},
			search.SearchRequest_Full.String(),
			"",
			0,
			nil,
			nil,
			httpInvalidArgErr,
		},
		{
			// Invalid From offset (-1)
			search.SearchRequest{
				QueryString: "Network",
				From:        -1,
				MaxResults:  maxResults,
				Tenants:     []string{"audi", "tesla"},
				Aggregate:   true,
			},
			search.SearchRequest_Full.String(),
			"",
			0,
			nil,
			nil,
			httpInvalidArgErr,
		},
		{
			// Invalid From offset (>1023)
			search.SearchRequest{
				QueryString: "Network",
				From:        1200,
				MaxResults:  maxResults,
				Tenants:     []string{"audi", "tesla"},
				Aggregate:   true,
			},
			search.SearchRequest_Full.String(),
			"",
			0,
			nil,
			nil,
			httpInvalidArgErr,
		},
		{
			// Invalid MaxResults offset (-1)
			search.SearchRequest{
				QueryString: "Network",
				From:        from,
				MaxResults:  -1,
				Tenants:     []string{"audi", "tesla"},
				Aggregate:   true,
			},
			search.SearchRequest_Full.String(),
			"",
			0,
			nil,
			nil,
			httpInvalidArgErr,
		},
		{
			// Invalid MaxResults offset (> 8192)
			search.SearchRequest{
				QueryString: "Network",
				From:        from,
				MaxResults:  9000,
				Tenants:     []string{"audi", "tesla"},
				Aggregate:   true,
			},
			search.SearchRequest_Full.String(),
			"",
			0,
			nil,
			nil,
			httpInvalidArgErr,
		},

		// Search preview tests - using GET/POST
		{
			// Term query with multi-value match on Kind
			search.SearchRequest{
				QueryString: "kind:(Network OR SecurityGroup)",
				From:        from,
				MaxResults:  maxResults,
				Tenants:     []string{"audi", "tesla"},
				Aggregate:   true,
			},
			search.SearchRequest_Preview.String(),
			"",
			2 * objectCount,
			map[string]map[string]map[string]int64{
				"tesla": {
					"Network": {
						"Network": 3,
					},
					"Security": {
						"SecurityGroup": 3,
					},
				},
				"audi": {
					"Network": {
						"Network": 2,
					},
					"Security": {
						"SecurityGroup": 2,
					},
				},
			},
			nil,
			nil,
		},
		{ // search events
			search.SearchRequest{
				QueryString: "kind:Event",
				From:        from,
				MaxResults:  maxResults * 2,
				Aggregate:   true,
			},
			search.SearchRequest_Preview.String(),
			"",
			eventCount,
			map[string]map[string]map[string]int64{
				"default": {
					"Monitoring": {
						"Event": eventCount,
					},
				},
			},
			nil,
			nil,
		},
		{
			// Non-existent Text in preview mode
			search.SearchRequest{
				QueryString: "OzzyOzbuorne",
				From:        from,
				MaxResults:  maxResults,
				Tenants:     []string{"audi", "tesla"},
				Aggregate:   true,
			},
			search.SearchRequest_Preview.String(),
			"",
			0,
			nil,
			nil,
			nil,
		},

		//
		// Search test cases with using GET/POST with BODY for advanced queries
		//
		{
			search.SearchRequest{
				Query: &search.SearchQuery{
					Kinds: []string{"Tenant"},
				},
				From:       from,
				MaxResults: maxResults,
				Aggregate:  true,
			},
			"", // default mode is Complete
			"",
			int64(len(Tenants) + 1),
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"default": {
					"Cluster": {
						"Tenant": {
							"tesla": nil,
							"audi":  nil,
						},
					},
				},
			},
			nil,
		},
		{
			search.SearchRequest{
				Query: &search.SearchQuery{
					Kinds: []string{"DistributedServiceCard"},
				},
				From:       from,
				MaxResults: maxResults,
				Aggregate:  true,
			},
			search.SearchRequest_Full.String(),
			"",
			objectCount,
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"default": {
					"Cluster": {
						"DistributedServiceCard": {
							"4444.4400.0000": nil,
							"4444.4400.0001": nil,
							"4444.4400.0002": nil,
							"4444.4400.0003": nil,
							"4444.4400.0004": nil,
						},
					},
				},
			},
			nil,
		},
		{
			// Test Paginated query #1
			// Use sort option for deterministic results
			search.SearchRequest{
				Query: &search.SearchQuery{
					Kinds: []string{"DistributedServiceCard"},
				},
				From:       0, // from offset-0
				MaxResults: 3, // get 3 results
				SortBy:     "meta.name.keyword",
				Aggregate:  true,
			},
			"",
			"meta.name.keyword",
			3,
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"default": {
					"Cluster": {
						"DistributedServiceCard": {
							"4444.4400.0000": nil,
							"4444.4400.0001": nil,
							"4444.4400.0002": nil,
						},
					},
				},
			},
			nil,
		},
		{
			// Test Paginated query #2
			// Use sort option for deterministic results
			search.SearchRequest{
				Query: &search.SearchQuery{
					Kinds: []string{"DistributedServiceCard"},
				},
				From:       3, // from offset-3
				MaxResults: 2, // get 2 results
				SortBy:     "meta.name.keyword",
				Aggregate:  true,
			},
			"",
			"meta.name.keyword",
			2,
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"default": {
					"Cluster": {
						"DistributedServiceCard": {
							"4444.4400.0003": nil,
							"4444.4400.0004": nil,
						},
					},
				},
			},
			nil,
		},
		{
			search.SearchRequest{
				Query: &search.SearchQuery{
					Kinds: []string{"Network"},
				},
				From:       from,
				MaxResults: maxResults,
				Tenants:    []string{"audi", "tesla"},
				Aggregate:  true,
			},
			"",
			"",
			objectCount,
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"audi": {
					"Network": {
						"Network": {
							"net01": nil,
							"net03": nil,
						},
					},
				},
				"tesla": {
					"Network": {
						"Network": {
							"net00": nil,
							"net02": nil,
							"net04": nil,
						},
					},
				},
			},
			nil,
		},
		{
			search.SearchRequest{
				Query: &search.SearchQuery{
					Kinds: []string{"SecurityGroup"},
				},
				From:       from,
				MaxResults: maxResults,
				Tenants:    []string{"audi", "tesla"},
				Aggregate:  true,
			},
			"",
			"",
			objectCount,
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"tesla": {
					"Security": {
						"SecurityGroup": {
							"sg00": nil,
							"sg02": nil,
							"sg04": nil,
						},
					},
				},
				"audi": {
					"Security": {
						"SecurityGroup": {
							"sg01": nil,
							"sg03": nil,
						},
					},
				},
			},
			nil,
		},
		{
			// Term query with multi-value match on Kind
			search.SearchRequest{
				Query: &search.SearchQuery{
					Kinds: []string{"Network", "SecurityGroup"},
				},
				From:       from,
				MaxResults: maxResults,
				Tenants:    []string{"audi", "tesla"},
				Aggregate:  true,
			},
			"",
			"",
			2 * objectCount,
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"tesla": {
					"Network": {
						"Network": {
							"net00": nil,
							"net02": nil,
							"net04": nil,
						},
					},
					"Security": {
						"SecurityGroup": {
							"sg00": nil,
							"sg02": nil,
							"sg04": nil,
						},
					},
				},
				"audi": {
					"Network": {
						"Network": {
							"net01": nil,
							"net03": nil,
						},
					},
					"Security": {
						"SecurityGroup": {
							"sg01": nil,
							"sg03": nil,
						},
					},
				},
			},
			nil,
		},
		{
			// Term Match on Kind and Label attributes
			search.SearchRequest{
				Query: &search.SearchQuery{
					Kinds: []string{"Network"},
					Labels: &labels.Selector{
						Requirements: []*labels.Requirement{
							{
								Key:      "meta.labels.Application",
								Operator: "equals",
								Values:   []string{"MS-Exchange"},
							},
						},
					},
				},
				From:       from,
				MaxResults: maxResults,
				Tenants:    []string{"audi", "tesla"},
				Aggregate:  true,
			},
			"",
			"",
			objectCount,
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"tesla": {
					"Network": {
						"Network": {
							"net00": nil,
							"net02": nil,
							"net04": nil,
						},
					},
				},
				"audi": {
					"Network": {
						"Network": {
							"net01": nil,
							"net03": nil,
						},
					},
				},
			},
			nil,
		},
		{
			// Text search that matches on Kind
			search.SearchRequest{
				Query: &search.SearchQuery{
					Texts: []*search.TextRequirement{
						{
							Text: []string{"DistributedServiceCard"},
						},
					},
				},
				From:       from,
				MaxResults: maxResults,
				Aggregate:  true,
			},
			"",
			"",
			objectCount,
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"default": {
					"Cluster": {
						"DistributedServiceCard": {
							"4444.4400.0000": nil,
							"4444.4400.0001": nil,
							"4444.4400.0002": nil,
							"4444.4400.0003": nil,
							"4444.4400.0004": nil,
						},
					},
				},
			},
			nil,
		},

		{
			// Text search (case insensitive) that matches on Kind
			search.SearchRequest{
				Query: &search.SearchQuery{
					Texts: []*search.TextRequirement{
						{
							Text: []string{"distributedservicecard"},
						},
					},
				},
				From:       from,
				MaxResults: maxResults,
				Aggregate:  true,
			},
			"",
			"",
			objectCount,
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"default": {
					"Cluster": {
						"DistributedServiceCard": {
							"4444.4400.0000": nil,
							"4444.4400.0001": nil,
							"4444.4400.0002": nil,
							"4444.4400.0003": nil,
							"4444.4400.0004": nil,
						},
					},
				},
			},
			nil,
		},
		{
			// Text search (case insensitive and prefix match) that matches on Kind
			search.SearchRequest{
				Query: &search.SearchQuery{
					Texts: []*search.TextRequirement{
						{
							Text: []string{"distributed*"},
						},
					},
				},
				From:       from,
				MaxResults: maxResults,
				Aggregate:  true,
			},
			"",
			"",
			objectCount,
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"default": {
					"Cluster": {
						"DistributedServiceCard": {
							"4444.4400.0000": nil,
							"4444.4400.0001": nil,
							"4444.4400.0002": nil,
							"4444.4400.0003": nil,
							"4444.4400.0004": nil,
						},
					},
				},
			},
			nil,
		},
		{
			// Text search that matches on meta.Namespace
			search.SearchRequest{
				Query: &search.SearchQuery{
					Texts: []*search.TextRequirement{
						{
							Text: []string{"infra"},
						},
					},
				},
				From:       from,
				MaxResults: maxResults,
				Aggregate:  true,
			},
			"",
			"",
			objectCount + int64(len(Tenants)),
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"default": {
					"Cluster": {
						"DistributedServiceCard": {
							"4444.4400.0000": nil,
							"4444.4400.0001": nil,
							"4444.4400.0002": nil,
							"4444.4400.0003": nil,
							"4444.4400.0004": nil,
						},
						"Tenant": {
							"tesla": nil,
							"audi":  nil,
						},
					},
				},
			},
			nil,
		},
		{
			// Text search that matches on meta.Labels.key
			search.SearchRequest{
				Query: &search.SearchQuery{
					Texts: []*search.TextRequirement{
						{
							Text: []string{"us-west"},
						},
					},
				},
				From:       from,
				MaxResults: maxResults,
				Tenants:    []string{"audi", "tesla"},
				Aggregate:  true,
			},
			"",
			"",
			3*objectCount + int64(len(Tenants)),
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"tesla": {
					"Network": {
						"Network": {
							"net00": nil,
							"net02": nil,
							"net04": nil,
						},
					},
					"Security": {
						"SecurityGroup": {
							"sg00": nil,
							"sg02": nil,
							"sg04": nil,
						},
					},
				},
				"default": {
					"Cluster": {
						"DistributedServiceCard": {
							"4444.4400.0000": nil,
							"4444.4400.0001": nil,
							"4444.4400.0002": nil,
							"4444.4400.0003": nil,
							"4444.4400.0004": nil,
						},
						"Tenant": {
							"tesla": nil,
							"audi":  nil,
						},
					},
				},
				"audi": {
					"Network": {
						"Network": {
							"net01": nil,
							"net03": nil,
						},
					},
					"Security": {
						"SecurityGroup": {
							"sg01": nil,
							"sg03": nil,
						},
					},
				},
			},
			nil,
		},
		{
			// Phrase query
			search.SearchRequest{
				Query: &search.SearchQuery{
					Texts: []*search.TextRequirement{
						{
							Text: []string{"human resources"},
						},
					},
				},
				From:       from,
				MaxResults: maxResults,
				Aggregate:  true,
			},
			"",
			"",
			int64(len(Tenants)),
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"default": {
					"Cluster": {
						"Tenant": {
							"tesla": nil,
							"audi":  nil,
						},
					},
				},
			},
			nil,
		},
		{
			// Precise match on a MAC address, part of smartNIC object
			search.SearchRequest{
				Query: &search.SearchQuery{
					Kinds: []string{"DistributedServiceCard"},
					Fields: &fields.Selector{
						Requirements: []*fields.Requirement{
							{
								Key:      "meta.name",
								Operator: "equals",
								Values:   []string{"4444.4400.0001"},
							},
						},
					},
				},
				From:       from,
				MaxResults: maxResults,
				Aggregate:  true,
			},
			"",
			"",
			1,
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"default": {
					"Cluster": {
						"DistributedServiceCard": {
							"4444.4400.0001": nil,
						},
					},
				},
			},
			nil,
		},
		{
			// NotEquals test case
			search.SearchRequest{
				Query: &search.SearchQuery{
					Kinds: []string{"DistributedServiceCard"},
					Fields: &fields.Selector{
						Requirements: []*fields.Requirement{
							{
								Key:      "meta.name",
								Operator: "notEquals",
								Values:   []string{"4444.4400.0001"},
							},
						},
					},
				},
				From:       from,
				MaxResults: maxResults,
				Aggregate:  true,
			},
			"",
			"",
			4,
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"default": {
					"Cluster": {
						"DistributedServiceCard": {
							"4444.4400.0000": nil,
							"4444.4400.0002": nil,
							"4444.4400.0003": nil,
							"4444.4400.0004": nil,
						},
					},
				},
			},
			nil,
		},
		{
			// IN test case
			search.SearchRequest{
				Query: &search.SearchQuery{
					Kinds: []string{"DistributedServiceCard"},
					Fields: &fields.Selector{
						Requirements: []*fields.Requirement{
							{
								Key:      "meta.name",
								Operator: "in",
								Values:   []string{"4444.4400.0000", "4444.4400.0001"},
							},
						},
					},
				},
				From:       from,
				MaxResults: maxResults,
				Aggregate:  true,
			},
			"",
			"",
			2,
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"default": {
					"Cluster": {
						"DistributedServiceCard": {
							"4444.4400.0000": nil,
							"4444.4400.0001": nil,
						},
					},
				},
			},
			nil,
		},
		{
			// Multiple IN test case
			search.SearchRequest{
				Query: &search.SearchQuery{
					Kinds: []string{"DistributedServiceCard"},
					Fields: &fields.Selector{
						Requirements: []*fields.Requirement{
							{
								Key:      "meta.name",
								Operator: "in",
								Values:   []string{"4444.4400.0000", "4444.4400.0001"},
							},
							{
								Key:      "meta.name",
								Operator: "in",
								Values:   []string{"4444.4400.0000", "4444.4400.0002"},
							},
						},
					},
				},
				From:       from,
				MaxResults: maxResults,
				Aggregate:  true,
			},
			"",
			"",
			1,
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"default": {
					"Cluster": {
						"DistributedServiceCard": {
							"4444.4400.0000": nil,
						},
					},
				},
			},
			nil,
		},
		{
			// NOT-IN test case
			search.SearchRequest{
				Query: &search.SearchQuery{
					Kinds: []string{"DistributedServiceCard"},
					Fields: &fields.Selector{
						Requirements: []*fields.Requirement{
							{
								Key:      "meta.name",
								Operator: "notIn",
								Values:   []string{"4444.4400.0000", "4444.4400.0001"},
							},
						},
					},
				},
				From:       from,
				MaxResults: maxResults,
				Aggregate:  true,
			},
			"",
			"",
			3,
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"default": {
					"Cluster": {
						"DistributedServiceCard": {
							"4444.4400.0002": nil,
							"4444.4400.0003": nil,
							"4444.4400.0004": nil,
						},
					},
				},
			},
			nil,
		},
		{
			// Precise match on a IP address, part of Network object
			search.SearchRequest{
				Query: &search.SearchQuery{
					Texts: []*search.TextRequirement{
						{
							Text: []string{"12.0.1.254"},
						},
					},
				},
				From:       from,
				MaxResults: maxResults,
				Tenants:    []string{"audi", "tesla"},
				Aggregate:  true,
			},
			"",
			"",
			1,
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"audi": {
					"Network": {
						"Network": {
							"net01": nil,
						},
					},
				},
			},
			nil,
		},
		// Test cases for sort by and sort order
		{
			search.SearchRequest{
				Query: &search.SearchQuery{
					Texts: []*search.TextRequirement{
						{
							Text: []string{"distributedservicecard"},
						},
					},
				},
				From:       from,
				MaxResults: maxResults,
				SortBy:     "meta.creation-time",
				SortOrder:  search.SearchRequest_Descending.String(),
				Aggregate:  true,
			},
			"",
			"",
			objectCount,
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"default": {
					"Cluster": {
						"DistributedServiceCard": {
							"4444.4400.0004": nil,
							"4444.4400.0003": nil,
							"4444.4400.0002": nil,
							"4444.4400.0001": nil,
							"4444.4400.0000": nil,
						},
					},
				},
			},
			nil,
		},
		{
			search.SearchRequest{
				QueryString: "infra",
				Query: &search.SearchQuery{
					Texts: []*search.TextRequirement{
						{
							Text: []string{"infra"},
						},
					},
					Categories: []string{"Cluster"},
				},
				From:       from,
				MaxResults: 8192,
				SortBy:     "meta.mod-time",
				Aggregate:  true,
			},
			search.SearchRequest_Full.String(),
			"",
			objectCount + int64(len(Tenants)),
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"default": {
					"Cluster": {
						"Tenant": {
							"audi":  nil,
							"tesla": nil,
						},
					},
				},
			},
			nil,
		},
		{
			search.SearchRequest{
				QueryString: "infra",
				Query: &search.SearchQuery{
					Texts: []*search.TextRequirement{
						{
							Text: []string{"infra"},
						},
					},
					Categories: []string{"Cluster"},
				},
				From:       from,
				MaxResults: 8192,
				SortBy:     "meta.fake-field",
				Aggregate:  true,
			},
			search.SearchRequest_Full.String(),
			"",
			objectCount + int64(len(Tenants)),
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"default": {
					"Cluster": {
						"Tenant": {
							"audi":  nil,
							"tesla": nil,
						},
					},
				},
			},
			nil,
		},
		// Test case for time based query using range operators [gt, gte, lt, lte]
		{
			// Query all DistributedServiceCard objects created in the last 10mins using using RFC3339Nano format
			search.SearchRequest{
				Query: &search.SearchQuery{
					Kinds: []string{"DistributedServiceCard"},
					Fields: &fields.Selector{
						Requirements: []*fields.Requirement{
							{
								Key:      "meta.creation-time",
								Operator: "gte",
								Values:   []string{time.Now().Add(-10 * time.Minute).Format(time.RFC3339Nano)},
							},
							{
								Key:      "meta.creation-time",
								Operator: "lte",
								Values:   []string{time.Now().Format(time.RFC3339Nano)},
							},
						},
					},
				},
				From:       from,
				MaxResults: maxResults,
				Aggregate:  true,
			},
			"",
			"",
			5,
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"default": {
					"Cluster": {
						"DistributedServiceCard": {
							"4444.4400.0000": nil,
							"4444.4400.0001": nil,
							"4444.4400.0002": nil,
							"4444.4400.0003": nil,
							"4444.4400.0004": nil,
						},
					},
				},
			},
			nil,
		},
		{
			// Query all Tenant objects modified in the last 3mins using RFC3339Nano format
			search.SearchRequest{
				Query: &search.SearchQuery{
					Kinds: []string{"Tenant"},
					Fields: &fields.Selector{
						Requirements: []*fields.Requirement{
							{
								Key:      "meta.mod-time",
								Operator: "gte",
								Values:   []string{time.Now().Add(-10 * time.Minute).Format(time.RFC3339Nano)},
							},
							{
								Key:      "meta.mod-time",
								Operator: "lte",
								Values:   []string{time.Now().Format(time.RFC3339Nano)},
							},
						},
					},
				},
				From:       from,
				MaxResults: maxResults,
				Aggregate:  true,
			},
			"",
			"",
			3,
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"default": {
					"Cluster": {
						"Tenant": {
							"tesla":   nil,
							"audi":    nil,
							"default": nil,
						},
					},
				},
			},
			nil,
		},
		{
			// Query all Tenant objects created in the last one day using YYYY-MM-DD format
			search.SearchRequest{
				Query: &search.SearchQuery{
					Kinds: []string{"Tenant"},
					Fields: &fields.Selector{
						Requirements: []*fields.Requirement{
							{
								Key:      "meta.creation-time",
								Operator: "gte",
								Values:   []string{time.Now().Add(-24 * time.Hour).Format("2006-01-02")},
							},
							{
								Key:      "meta.creation-time",
								Operator: "lte",
								Values:   []string{time.Now().Add(24 * time.Hour).Format("2006-01-02")},
							},
						},
					},
				},
				From:       from,
				MaxResults: maxResults,
				Aggregate:  true,
			},
			"",
			"",
			3,
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"default": {
					"Cluster": {
						"Tenant": {
							"tesla":   nil,
							"audi":    nil,
							"default": nil,
						},
					},
				},
			},
			nil,
		},
		{
			// Invalid Kind, negative test case
			search.SearchRequest{
				Query: &search.SearchQuery{
					Kinds: []string{"Contract"},
				},
				From:       from,
				MaxResults: maxResults,
				Aggregate:  true,
			},
			"",
			"",
			0,
			nil,
			nil,
			fmt.Errorf("Server responded with 400"),
		},
		{
			// Non-existent Text
			search.SearchRequest{
				Query: &search.SearchQuery{
					Texts: []*search.TextRequirement{
						{
							Text: []string{"OzzyOzbuorne"},
						},
					},
				},
				From:       from,
				MaxResults: maxResults,
				Aggregate:  true,
			},
			"",
			"",
			0,
			nil,
			nil,
			nil,
		},
		{
			// Invalid QueryString length (> 256)
			search.SearchRequest{
				Query: &search.SearchQuery{
					Texts: []*search.TextRequirement{
						{
							Text: []string{CreateAlphabetString(512)},
						},
					},
				},
				From:       from,
				MaxResults: maxResults,
				Aggregate:  true,
			},
			"",
			"",
			0,
			nil,
			nil,
			fmt.Errorf("Server responded with 400"),
		},
		{
			// Invalid From offset (-1)
			search.SearchRequest{
				Query: &search.SearchQuery{
					Kinds: []string{"Network"},
				},
				From:       -1,
				MaxResults: maxResults,
				Tenants:    []string{"audi", "tesla"},
				Aggregate:  true,
			},
			"",
			"",
			0,
			nil,
			nil,
			fmt.Errorf("Server responded with 400"),
		},
		{
			// Invalid From offset (>1023)
			search.SearchRequest{
				Query: &search.SearchQuery{
					Kinds: []string{"Network"},
				},
				From:       1200,
				MaxResults: maxResults,
				Tenants:    []string{"audi", "tesla"},
				Aggregate:  true,
			},
			"",
			"",
			0,
			nil,
			nil,
			fmt.Errorf("Server responded with 400"),
		},
		{
			// Invalid MaxResults offset (-1)
			search.SearchRequest{
				Query: &search.SearchQuery{
					Kinds: []string{"Network"},
				},
				From:       from,
				MaxResults: -1,
				Tenants:    []string{"audi", "tesla"},
				Aggregate:  true,
			},
			"",
			"",
			0,
			nil,
			nil,
			fmt.Errorf("Server responded with 400"),
		},
		{
			// Invalid MaxResults offset (> 8192)
			search.SearchRequest{
				Query: &search.SearchQuery{
					Kinds: []string{"Network"},
				},
				From:       from,
				MaxResults: 9000,
				Tenants:    []string{"audi", "tesla"},
				Aggregate:  true,
			},
			"",
			"",
			0,
			nil,
			nil,
			fmt.Errorf("Server responded with 400"),
		},

		// Preview mode test using POST
		{
			// Preview mode: Text search that matches on meta.Labels.key
			search.SearchRequest{
				Query: &search.SearchQuery{
					Texts: []*search.TextRequirement{
						{
							Text: []string{"us-west"},
						},
					},
				},
				From:       from,
				MaxResults: maxResults,
				Mode:       search.SearchRequest_Preview.String(),
				Tenants:    []string{"audi", "tesla"},
				Aggregate:  true,
			},
			"",
			"",
			3*objectCount + int64(len(Tenants)),
			map[string]map[string]map[string]int64{
				"tesla": {
					"Network": {
						"Network": 3,
					},
					"Security": {
						"SecurityGroup": 3,
					},
				},
				"default": {
					"Cluster": {
						"DistributedServiceCard": 5,
						"Tenant":                 2,
					},
				},
				"audi": {
					"Network": {
						"Network": 2,
					},
					"Security": {
						"SecurityGroup": 2,
					},
				},
			},
			nil,
			nil,
		},
		{
			// Preview mode : Non-existent Text
			search.SearchRequest{
				Query: &search.SearchQuery{
					Texts: []*search.TextRequirement{
						{
							Text: []string{"OzzyOzbuorne"},
						},
					},
				},
				From:       from,
				MaxResults: maxResults,
				Mode:       search.SearchRequest_Preview.String(),
				Aggregate:  true,
			},
			"",
			"",
			0,
			nil,
			nil,
			nil,
		},
	}

	httpClient := netutils.NewHTTPClient()
	httpClient.WithTLSConfig(&tls.Config{InsecureSkipVerify: true})
	httpClient.SetHeader("Authorization", tInfo.authzHeader)
	httpClient.DisableKeepAlives()
	defer httpClient.CloseIdleConnections()

	// Execute the Query Testcases
	for _, tc := range queryTestcases {

		t.Run(tc.query.QueryString, func(t *testing.T) {
			AssertEventually(t,
				func() (bool, interface{}) {

					// execute search
					var err error
					var resp search.SearchResponse
					var searchURL string
					if searchMethod == GetWithURI && len(tc.query.QueryString) != 0 {
						// Query using URI params - via GET method
						t.Logf("@@@ GET QueryString query: %s\n", tc.query.QueryString)
						searchURL = getSearchURLWithParams(t, tc.query.QueryString, tc.query.From,
							tc.query.MaxResults, tc.mode, tc.sortBy, tc.query.Aggregate, tc.query.Tenants)
						resp = search.SearchResponse{}
						start := time.Now().UTC()
						_, err = httpClient.Req("GET", searchURL, nil, &resp)
						t.Logf("@@@ Search response time: %+v\n", time.Since(start))
					} else {
						// Query using Body - via POST, GET
						var httpMethod string
						switch searchMethod {
						case GetWithBody:
							httpMethod = "GET"
						case PostWithBody:
							httpMethod = "POST"
						default:
							httpMethod = "GET"
						}

						t.Logf("@@@ Search request [%s] Body: %+v\n", httpMethod, tc.query)
						searchURL = getSearchURL()
						resp = search.SearchResponse{}
						start := time.Now().UTC()
						_, err = httpClient.Req(httpMethod, searchURL, &tc.query, &resp)
						t.Logf("@@@ Search response time: %+v\n", time.Since(start))
					}

					if (err != nil && tc.err == nil) ||
						(err == nil && tc.err != nil) ||
						(err != nil && tc.err != nil && err.Error() != tc.err.Error()) {
						tInfo.l.Infof("expected %+v, actual %+v", tc.err, err)
						t.Logf("@@@ Search response didn't match expected error, expected:%+v actual:%+v",
							tc.err, err)
						return false, nil
					}
					tInfo.l.Infof("Query: %s, result : %+v", searchURL, resp)
					return checkSearchQueryResponse(t, &tc.query, &resp, tc.expectedHits, tc.previewResults, tc.aggResults), nil
				}, fmt.Sprintf("Search response does not meet the expected: %+v, ", tc.query.QueryString), "100ms", "1m")
		})
	}
}

// Execute policy search test cases
func performPolicySearchTests(t *testing.T, searchMethod SearchMethod) {

	t.Logf("@@@ performPolicySearchTests, method: %d", searchMethod)
	tInfo.l.Infof("@@@ performPolicySearchTests, method: %d", searchMethod)

	// Testcases for various queries on config objects
	queryTestcases := []struct {
		desc     string
		request  search.PolicySearchRequest
		response search.PolicySearchResponse
	}{
		{
			// Empty request
			"Empty Request",
			search.PolicySearchRequest{},
			search.PolicySearchResponse{
				Status: search.PolicySearchResponse_MISS.String(),
			},
		},
		{
			// Request with valid tenant alone
			"Empty Request with Valid Tenant",
			search.PolicySearchRequest{
				Tenant: "default",
			},
			search.PolicySearchResponse{
				Status: search.PolicySearchResponse_MISS.String(),
			},
		},
		{
			// Request with non-existent tenant
			"Empty Request with non-existent Tenant",
			search.PolicySearchRequest{
				Tenant: "GoDaddy",
			},
			search.PolicySearchResponse{
				Status: search.PolicySearchResponse_MISS.String(),
			},
		},
		{
			// Request with all any's, should return first match
			"Any request with specific SGP",
			search.PolicySearchRequest{
				Tenant:                "default",
				App:                   "any",
				FromIPAddress:         "any",
				ToIPAddress:           "any",
				NetworkSecurityPolicy: "sgp-aggregate",
			},
			search.PolicySearchResponse{
				Status: search.PolicySearchResponse_MATCH.String(),
				Results: map[string]*search.PolicyMatchEntries{
					"sgp-aggregate": {
						Entries: []*search.PolicyMatchEntry{
							{
								Rule: &security.SGRule{
									ProtoPorts: []security.ProtoPort{
										{Protocol: "tcp", Ports: "80"},
										{Protocol: "udp", Ports: "53"},
									},

									FromIPAddresses: []string{
										"172.0.0.1",
										"172.0.0.2",
										"10.0.0.1/30",
									},
									ToIPAddresses: []string{
										"229.204.171.210/16",
									},
									Action: security.SGRule_PERMIT.String(),
								},
								Index: 0,
							},
						},
					},
				},
			},
		},
		{
			// Match on specific NetworkSecurityPolicy
			"Exact match on IP, APP and SGP",
			search.PolicySearchRequest{
				Tenant:                "default",
				Protocol:              "tcp",
				Port:                  "443",
				FromIPAddress:         "37.232.218.135",
				ToIPAddress:           "37.232.218.136",
				NetworkSecurityPolicy: "sgp-aggregate",
			},
			search.PolicySearchResponse{
				Status: search.PolicySearchResponse_MATCH.String(),
				Results: map[string]*search.PolicyMatchEntries{
					"sgp-aggregate": {
						Entries: []*search.PolicyMatchEntry{
							{Rule: &security.SGRule{
								ProtoPorts: []security.ProtoPort{{Protocol: "tcp", Ports: "443"}},
								FromIPAddresses: []string{
									"37.232.218.135/22",
								},
								ToIPAddresses: []string{
									"37.232.218.136/30",
								},
								Action: security.SGRule_PERMIT.String(),
							},
								Index: 1,
							},
						},
					},
				},
			},
		},
		{
			// Exact match on IP and APP
			"Exact match on IP and APP",
			search.PolicySearchRequest{
				Tenant:        "default",
				Protocol:      "icmp",
				FromIPAddress: "10.1.1.1",
				ToIPAddress:   "20.1.1.1",
			},
			search.PolicySearchResponse{
				Status: search.PolicySearchResponse_MATCH.String(),
				Results: map[string]*search.PolicyMatchEntries{
					"sgp-aggregate": {
						Entries: []*search.PolicyMatchEntry{
							{Rule: &security.SGRule{
								ProtoPorts: []security.ProtoPort{{Protocol: "icmp"}},
								FromIPAddresses: []string{
									"10.1.1.1",
								},
								ToIPAddresses: []string{
									"20.1.1.1",
								},
								Action: security.SGRule_PERMIT.String(),
							},
								Index: 3,
							},
						},
					},
				},
			},
		},
		{
			// Exact match on APP, any on IP
			"Exact match on APP and Any IP",
			search.PolicySearchRequest{
				Tenant:        "default",
				Protocol:      "tcp",
				Port:          "22",
				FromIPAddress: "1.1.1.1",
				ToIPAddress:   "2.1.1.1",
			},
			search.PolicySearchResponse{
				Status: search.PolicySearchResponse_MATCH.String(),
				Results: map[string]*search.PolicyMatchEntries{
					"sgp-aggregate": {
						Entries: []*search.PolicyMatchEntry{
							{Rule: &security.SGRule{
								ProtoPorts: []security.ProtoPort{{Protocol: "tcp", Ports: "22"}},
								FromIPAddresses: []string{
									"any",
								},
								ToIPAddresses: []string{
									"any",
								},
								Action: security.SGRule_PERMIT.String(),
							},
								Index: 2,
							},
						},
					},
				},
			},
		},
		{
			// Subnet match on FromIP and ToIP
			"Subnet match on IP",
			search.PolicySearchRequest{
				Tenant:        "default",
				Protocol:      "tcp",
				Port:          "80",
				FromIPAddress: "10.0.0.2",
				ToIPAddress:   "229.204.172.212",
			},
			search.PolicySearchResponse{
				Status: search.PolicySearchResponse_MATCH.String(),
				Results: map[string]*search.PolicyMatchEntries{
					"sgp-aggregate": {
						Entries: []*search.PolicyMatchEntry{
							{
								Rule: &security.SGRule{
									ProtoPorts: []security.ProtoPort{
										{Protocol: "tcp", Ports: "80"},
										{Protocol: "udp", Ports: "53"},
									},
									FromIPAddresses: []string{
										"172.0.0.1",
										"172.0.0.2",
										"10.0.0.1/30",
									},
									ToIPAddresses: []string{
										"229.204.171.210/16",
									},
									Action: security.SGRule_PERMIT.String(),
								},
								Index: 0,
							},
						},
					},
				},
			},
		},
		{
			// Subnet mismatch on From-IP
			"Subnet mismatch on From-IP",
			search.PolicySearchRequest{
				Tenant:        "default",
				Protocol:      "tcp",
				Port:          "80",
				FromIPAddress: "10.0.0.6",
				ToIPAddress:   "229.204.172.212",
			},
			search.PolicySearchResponse{
				Status: search.PolicySearchResponse_MISS.String(),
			},
		},
		{
			// Subnet mismatch on To-IP
			"Subnet mismatch on To-IP",
			search.PolicySearchRequest{
				Tenant:        "default",
				Protocol:      "tcp",
				Port:          "80",
				FromIPAddress: "10.0.0.2",
				ToIPAddress:   "229.205.1.1",
			},
			search.PolicySearchResponse{
				Status: search.PolicySearchResponse_MISS.String(),
			},
		},
		{
			// Mismatch on Port
			"Exact match on IP",
			search.PolicySearchRequest{
				Tenant:        "default",
				Protocol:      "icmp",
				Port:          "1000",
				FromIPAddress: "10.1.1.1",
				ToIPAddress:   "20.1.1.1",
			},
			search.PolicySearchResponse{
				Status: search.PolicySearchResponse_MISS.String(),
			},
		},
		{
			// Match on SGs #1
			"Match on SGs #1",
			search.PolicySearchRequest{
				Tenant:            "default",
				Protocol:          "udp",
				Port:              "53",
				FromSecurityGroup: "dns-clients",
				ToSecurityGroup:   "dns-servers",
			},
			search.PolicySearchResponse{
				Status: search.PolicySearchResponse_MATCH.String(),
				Results: map[string]*search.PolicyMatchEntries{
					"sgp-aggregate": {
						Entries: []*search.PolicyMatchEntry{
							{
								Rule: &security.SGRule{
									ProtoPorts: []security.ProtoPort{{Protocol: "udp", Ports: "53"}},
									FromSecurityGroups: []string{
										"dns-clients",
									},
									ToSecurityGroups: []string{
										"dns-servers",
									},
									Action: security.SGRule_PERMIT.String(),
								},
								Index: 4,
							},
						},
					},
				},
			},
		},
		{
			// Match on SGs #2
			"Match on SGs #2",
			search.PolicySearchRequest{
				Tenant:                "default",
				Protocol:              "udp",
				Port:                  "53",
				FromSecurityGroup:     "test-servers",
				ToSecurityGroup:       "dns-servers",
				NetworkSecurityPolicy: "sgp-aggregate",
			},
			search.PolicySearchResponse{
				Status: search.PolicySearchResponse_MATCH.String(),
				Results: map[string]*search.PolicyMatchEntries{
					"sgp-aggregate": {
						Entries: []*search.PolicyMatchEntry{
							{Rule: &security.SGRule{
								ProtoPorts: []security.ProtoPort{{Protocol: "udp", Ports: "53"}},
								FromIPAddresses: []string{
									"any",
								},
								FromSecurityGroups: []string{
									"test-servers",
								},
								ToSecurityGroups: []string{
									"dns-servers",
								},
								Action: security.SGRule_DENY.String(),
							},
								Index: 5,
							},
						},
					},
				},
			},
		},
		{
			// Match on SGs #3
			"Match on SGs #3",
			search.PolicySearchRequest{
				Tenant:            "default",
				Protocol:          "tcp",
				Port:              "1024",
				FromSecurityGroup: "web-servers",
				ToSecurityGroup:   "app-servers",
			},
			search.PolicySearchResponse{
				Status: search.PolicySearchResponse_MATCH.String(),
				Results: map[string]*search.PolicyMatchEntries{
					"sgp-aggregate": {
						Entries: []*search.PolicyMatchEntry{
							{
								Rule: &security.SGRule{
									ProtoPorts: []security.ProtoPort{{Protocol: "tcp", Ports: "1024"}},
									FromSecurityGroups: []string{
										"web-servers",
									},
									ToSecurityGroups: []string{
										"app-servers",
									},
									Action: security.SGRule_PERMIT.String(),
								},
								Index: 6,
							},
						},
					},
				},
			},
		},
		{
			// Match on IP range #1
			"Match on IP range #1",
			search.PolicySearchRequest{
				Tenant:        "default",
				Protocol:      "tcp",
				Port:          "80",
				FromIPAddress: "30.1.1.1",
				ToIPAddress:   "40.1.1.10",
			},
			search.PolicySearchResponse{
				Status: search.PolicySearchResponse_MATCH.String(),
				Results: map[string]*search.PolicyMatchEntries{
					"sgp-aggregate": {
						Entries: []*search.PolicyMatchEntry{
							{
								Rule: &security.SGRule{
									ProtoPorts: []security.ProtoPort{{Protocol: "tcp", Ports: "80"}},
									FromIPAddresses: []string{
										"30.1.1.1-30.1.1.10",
									},
									ToIPAddresses: []string{
										"40.1.1.1-40.1.1.10",
									},
									Action: security.SGRule_PERMIT.String(),
								},
								Index: 7,
							},
						},
					},
				},
			},
		},
		{
			// Match on IP range #2
			"Match on IP range #2",
			search.PolicySearchRequest{
				Tenant:        "default",
				Protocol:      "tcp",
				Port:          "80",
				FromIPAddress: "30.1.1.5",
				ToIPAddress:   "40.1.1.5",
			},
			search.PolicySearchResponse{
				Status: search.PolicySearchResponse_MATCH.String(),
				Results: map[string]*search.PolicyMatchEntries{
					"sgp-aggregate": {
						Entries: []*search.PolicyMatchEntry{
							{
								Rule: &security.SGRule{
									ProtoPorts: []security.ProtoPort{{Protocol: "tcp", Ports: "80"}},
									FromIPAddresses: []string{
										"30.1.1.1-30.1.1.10",
									},
									ToIPAddresses: []string{
										"40.1.1.1-40.1.1.10",
									},
									Action: security.SGRule_PERMIT.String(),
								},
								Index: 7,
							},
						},
					},
				},
			},
		},
		{
			// Miss on IP range #1
			"Miss on IP range #1",
			search.PolicySearchRequest{
				Tenant:        "default",
				Protocol:      "tcp",
				Port:          "80",
				FromIPAddress: "30.1.1.5",
				ToIPAddress:   "40.1.1.20",
			},
			search.PolicySearchResponse{
				Status: search.PolicySearchResponse_MISS.String(),
			},
		},
		{
			// Miss on IP range #2
			"Miss on IP range #2",
			search.PolicySearchRequest{
				Tenant:        "default",
				Protocol:      "tcp",
				Port:          "80",
				FromIPAddress: "30.1.1.0",
				ToIPAddress:   "40.1.1.0",
			},
			search.PolicySearchResponse{
				Status: search.PolicySearchResponse_MISS.String(),
			},
		},
		{
			// Miss on IP range #3
			"Miss on IP range #3",
			search.PolicySearchRequest{
				Tenant:        "default",
				Protocol:      "tcp",
				Port:          "80",
				FromIPAddress: "30.1.1.15",
				ToIPAddress:   "40.1.1.15",
			},
			search.PolicySearchResponse{
				Status: search.PolicySearchResponse_MISS.String(),
			},
		},
		{
			// No match on Port & IP rules
			"No Match on IP",
			search.PolicySearchRequest{
				Tenant:        "default",
				Protocol:      "tcp",
				Port:          "32000",
				FromIPAddress: "any",
				ToIPAddress:   "any",
			},
			search.PolicySearchResponse{
				Status: search.PolicySearchResponse_MISS.String(),
			},
		},
		{
			// No match on Port & SG rules
			"No Match on SG",
			search.PolicySearchRequest{
				Tenant:            "default",
				Protocol:          "tcp",
				Port:              "32000",
				FromSecurityGroup: "any",
				ToSecurityGroup:   "any",
			},
			search.PolicySearchResponse{
				Status: search.PolicySearchResponse_MISS.String(),
			},
		},
		{
			// No match on App
			"No Match on App",
			search.PolicySearchRequest{
				Tenant:            "default",
				App:               "ftp",
				FromSecurityGroup: "any",
				ToSecurityGroup:   "any",
			},
			search.PolicySearchResponse{
				Status: search.PolicySearchResponse_MISS.String(),
			},
		},
		{
			// Scale profile, Any match, should match first rule
			"Scale test with match on any IP",
			search.PolicySearchRequest{
				Tenant:                "default",
				Protocol:              "tcp",
				Port:                  "1",
				FromIPAddress:         "any",
				ToIPAddress:           "any",
				NetworkSecurityPolicy: "sgp-aggregate",
			},
			search.PolicySearchResponse{
				Status: search.PolicySearchResponse_MATCH.String(),
				Results: map[string]*search.PolicyMatchEntries{
					"sgp-aggregate": {
						Entries: []*search.PolicyMatchEntry{
							{
								Rule: &security.SGRule{
									ProtoPorts: []security.ProtoPort{{Protocol: "tcp", Ports: "1"}},
									FromIPAddresses: []string{
										"10.0.0.0/32",
									},
									ToIPAddresses: []string{
										"20.0.0.0/32",
									},
									Action: security.SGRule_PERMIT.String(),
								},
								Index: 8, // zero base + 1st 8 rules
							},
						},
					},
				},
			},
		},
		{
			// Scale profile, match on 29000th rule
			"Scale test with match on 29000th Rule",
			search.PolicySearchRequest{
				Tenant:                "default",
				Protocol:              "tcp",
				Port:                  "29001",
				FromIPAddress:         "10.0.113.72",
				ToIPAddress:           "any",
				NetworkSecurityPolicy: "sgp-aggregate",
			},
			search.PolicySearchResponse{
				Status: search.PolicySearchResponse_MATCH.String(),
				Results: map[string]*search.PolicyMatchEntries{
					"sgp-aggregate": {
						Entries: []*search.PolicyMatchEntry{
							{
								Rule: &security.SGRule{
									ProtoPorts: []security.ProtoPort{{Protocol: "tcp", Ports: "29001"}},
									FromIPAddresses: []string{
										"10.0.113.72/32",
									},
									ToIPAddresses: []string{
										"20.0.113.72/32",
									},
									Action: security.SGRule_PERMIT.String(),
								},
								Index: 29008, // zero base + 1st 8 rules
							},
						},
					},
				},
			},
		},
		{
			// Scale profile, match on 29999th rule
			"Scale test with match on 29999th Rule",
			search.PolicySearchRequest{
				Tenant:                "default",
				Protocol:              "udp",
				Port:                  "30000",
				FromIPAddress:         "10.0.117.47",
				ToIPAddress:           "20.0.117.47",
				NetworkSecurityPolicy: "sgp-aggregate",
			},
			search.PolicySearchResponse{
				Status: search.PolicySearchResponse_MATCH.String(),
				Results: map[string]*search.PolicyMatchEntries{
					"sgp-aggregate": {
						Entries: []*search.PolicyMatchEntry{
							{
								Rule: &security.SGRule{
									ProtoPorts: []security.ProtoPort{{Protocol: "udp", Ports: "30000"}},
									FromIPAddresses: []string{
										"10.1.117.47/32",
									},
									ToIPAddresses: []string{
										"20.1.117.47/32",
									},
									Action: security.SGRule_DENY.String(),
								},
								Index: 30007, // zero base + 1st 8 rules
							},
						},
					},
				},
			},
		},
		{
			// Scale profile, MISS
			"Scale test MISS with 70k Rule",
			search.PolicySearchRequest{
				Tenant:                "default",
				Protocol:              "udp",
				Port:                  "80000",
				FromIPAddress:         "10.1.17.111",
				ToIPAddress:           "20.1.17.111",
				NetworkSecurityPolicy: "sgp-aggregate",
			},
			search.PolicySearchResponse{
				Status: search.PolicySearchResponse_MISS.String(),
			},
		},

		// TODO : Add tests for IPv6 addresses.
	}

	// Execute the Query Testcases
	for _, tc := range queryTestcases {

		t.Run(tc.desc, func(t *testing.T) {
			AssertEventually(t,
				func() (bool, interface{}) {

					// execute search
					var response search.PolicySearchResponse
					var searchURL string

					// Query using Body - via POST, GET
					var httpMethod string
					switch searchMethod {
					case GetWithBody:
						httpMethod = "GET"
					case PostWithBody:
						httpMethod = "POST"
					default:
						httpMethod = "GET"
					}

					t.Logf("@@@ PolicySearch request [%s]: %+v\n", httpMethod, tc.request)
					searchURL = getPolicySearchURL()
					restcl := netutils.NewHTTPClient()
					restcl.WithTLSConfig(&tls.Config{InsecureSkipVerify: true})
					restcl.SetHeader("Authorization", tInfo.authzHeader)
					restcl.DisableKeepAlives()
					defer restcl.CloseIdleConnections()

					start := time.Now().UTC()
					_, err := restcl.Req(httpMethod, searchURL, &tc.request, &response)
					t.Logf("@@@ PolicySearch response time: %+v\n", time.Since(start))
					if err != nil {
						t.Logf("Http request failed, err: {%+v}", err)
						return false, nil
					}

					if tc.response.Status != response.Status {
						t.Logf("@@@ PolicySearch result status didn't match, expected: %s actual: %s",
							tc.response.Status, response.Status)
						return false, nil
					}

					if tc.response.Status == search.PolicySearchResponse_MATCH.String() {

						if len(tc.response.Results) != len(response.Results) {
							t.Logf("@@@ PolicySearch result entry count didn't match, expected: %d actual: %d",
								len(tc.response.Results), len(response.Results))
							return false, nil
						}

						// Iterate over expected results
						for name, eEntry := range tc.response.Results {

							// validate actual result obtained
							aEntry, ok := response.Results[name]
							if !ok {
								t.Logf("@@@ PolicySearch required SGP object not found, obj: %s", name)
								return false, nil
							}
							for idx, entry := range eEntry.Entries {
								if entry.Index != aEntry.Entries[idx].Index {
									t.Logf("@@@ PolicySearch SGrule mismatch SGP object: %s expected: %d actual: %d",
										name, entry.Index, aEntry.Entries[idx].Index)
									return false, nil
								}
							}
						}
					}
					return true, nil
				}, fmt.Sprintf("Query failed for: %s", tc.desc), "100ms", "1m")
		})
	}
}

func checkSearchQueryResponse(t *testing.T, query *search.SearchRequest, resp *search.SearchResponse, expectedHits int64, previewResults map[string]map[string]map[string]int64,
	aggResults map[string]map[string]map[string]map[string]interface{}) bool {

	// Match on expected hits
	if resp.ActualHits != expectedHits {
		t.Logf("Result mismatch expected: %d received: %d resp: {%+v}",
			expectedHits, resp.ActualHits, resp)
		return false
	}

	// For cases with expectedHits to 0, return here
	if expectedHits == 0 {
		return true
	}

	if !query.Aggregate {
		return resp.AggregatedEntries == nil
	}

	// Verification for "Complete" request mode
	if query.Mode == search.SearchRequest_Full.String() {

		if len(aggResults) != len(resp.AggregatedEntries.Tenants) {
			log.Errorf("Tenant entries count didn't match, expected %d actual:%d",
				len(aggResults), len(resp.AggregatedEntries.Tenants))
			return false
		}

		// Tenant verification
		for tenantKey, tenantVal := range aggResults {
			log.Debugf("Verifying tenant Key: %s entries: %d", tenantKey, len(tenantVal))
			if _, ok := resp.AggregatedEntries.Tenants[tenantKey]; !ok {
				log.Errorf("Tenant %s not found", tenantKey)
				return false
			}

			// Verify count of category entries match for each tenant
			if len(tenantVal) != len(resp.AggregatedEntries.Tenants[tenantKey].Categories) {
				log.Errorf("Category entries count didn't match for tenant: %s, expected %d actual:%d",
					tenantKey, len(tenantVal), len(resp.AggregatedEntries.Tenants[tenantKey].Categories))
				return false
			}

			// Category verification
			for categoryKey, categoryVal := range tenantVal {
				log.Debugf("Verifying Category Key: %s entries: %d", categoryKey, len(categoryVal))
				if _, ok := resp.AggregatedEntries.Tenants[tenantKey].Categories[categoryKey]; !ok {
					log.Errorf("Category %s not found", categoryKey)
					return false
				}

				// Verify count of kind entries match for each (tenant,category)
				if len(categoryVal) != len(resp.AggregatedEntries.Tenants[tenantKey].Categories[categoryKey].Kinds) {
					log.Errorf("Kind entries count didn't match for tenant: %s category: %s, expected %d actual:%d",
						tenantKey, categoryKey, len(categoryVal), len(resp.AggregatedEntries.Tenants[tenantKey].Categories[categoryKey].Kinds))
					return false
				}

				// Kind verification
				for kindKey, kindVal := range categoryVal {
					log.Debugf("Verifying Kind Key: %s entries: %d", kindKey, len(kindVal))
					if _, ok := resp.AggregatedEntries.Tenants[tenantKey].Categories[categoryKey].Kinds[kindKey]; !ok {
						log.Errorf("Kind %s not found", kindKey)
						return false
					}

					// make an interim object map from the entries slice
					log.Debugf("Kinds: %+v", resp.AggregatedEntries.Tenants[tenantKey].Categories[categoryKey].Kinds[kindKey])
					entries := resp.AggregatedEntries.Tenants[tenantKey].Categories[categoryKey].Kinds[kindKey].Entries
					omap := make(map[string]interface{}, len(entries))
					for _, val := range entries {

						var robj runtime.Object
						obj := &ptypes.DynamicAny{}
						err := ptypes.UnmarshalAny(&val.Object.Any, obj)
						if err != nil {
							log.Errorf("Failed to get unmarshalAny object: %+v, err: %+v",
								val, err)
							continue
						}
						var ok bool
						if robj, ok = obj.Message.(runtime.Object); ok {
							log.Errorf("Failed to get unmarshalAny object: %+v, err: %+v",
								val, err)
							continue
						}
						ometa, err := runtime.GetObjectMeta(robj)
						if err != nil {
							log.Errorf("Failed to get obj-meta for object: %+v, err: %+v",
								obj, err)
							continue
						}
						omap[ometa.GetName()] = nil
					}

					// object verification
					for objKey := range kindVal {
						log.Debugf("Verifying Object Key: %s", objKey)
						if _, ok := omap[objKey]; !ok {
							log.Errorf("Object %s not found", objKey)
							return false
						}
					}
				}
			}
		}
	}

	// Verification for "Preview" request mode
	if query.Mode == search.SearchRequest_Preview.String() {

		if len(previewResults) != len(resp.PreviewEntries.Tenants) {
			log.Errorf("Tenant entries count didn't match, expected %d actual:%d",
				len(previewResults), len(resp.PreviewEntries.Tenants))
			return false
		}

		// Tenant verification
		for tenantKey, tenantVal := range previewResults {
			log.Debugf("Verifying tenant Key: %s entries: %d", tenantKey, len(tenantVal))
			if _, ok := resp.PreviewEntries.Tenants[tenantKey]; !ok {
				log.Errorf("Tenant %s not found", tenantKey)
				return false
			}

			// Verify count of category entries match for each tenant
			if len(tenantVal) != len(resp.PreviewEntries.Tenants[tenantKey].Categories) {
				log.Errorf("Category entries count didn't match for tenant: %s, expected %d actual:%d",
					tenantKey, len(tenantVal), len(resp.PreviewEntries.Tenants[tenantKey].Categories))
				return false
			}

			// Category verification
			for categoryKey, categoryVal := range tenantVal {
				log.Debugf("Verifying Category Key: %s entries: %d", categoryKey, len(categoryVal))
				if _, ok := resp.PreviewEntries.Tenants[tenantKey].Categories[categoryKey]; !ok {
					log.Errorf("Category %s not found", categoryKey)
					return false
				}

				// Verify count of kind entries match for each (tenant,category)
				if len(categoryVal) != len(resp.PreviewEntries.Tenants[tenantKey].Categories[categoryKey].Kinds) {
					log.Errorf("Kind entries count didn't match for tenant: %s category: %s, expected %d actual:%d",
						tenantKey, categoryKey, len(categoryVal), len(resp.PreviewEntries.Tenants[tenantKey].Categories[categoryKey].Kinds))
					return false
				}

				// Kind verification
				for kindKey, expNum := range categoryVal {
					var actualNum int64
					var ok bool
					log.Debugf("Verifying Kind Key: %s entries: %d", kindKey, expNum)
					if actualNum, ok = resp.PreviewEntries.Tenants[tenantKey].Categories[categoryKey].Kinds[kindKey]; !ok {
						log.Errorf("Kind %s not found", kindKey)
						return false
					}

					if expNum != actualNum {
						log.Errorf("Entries mismatch for Kind: %s expected: %d obtained: %d", kindKey, expNum, actualNum)
						return false
					}
				}
			}
		}
	}
	return true
}

func testAuthzInSearch(t *testing.T, searchMethod SearchMethod) {
	// create audi admin
	MustCreateTestUser(tInfo.apiClient, testutils.TestLocalUser, testutils.TestLocalPassword, "audi")
	defer MustDeleteUser(tInfo.apiClient, testutils.TestLocalUser, "audi")
	MustUpdateRoleBinding(tInfo.apiClient, globals.AdminRoleBinding, "audi", globals.AdminRole, []string{testutils.TestLocalUser}, nil)
	defer MustUpdateRoleBinding(tInfo.apiClient, globals.AdminRoleBinding, "audi", globals.AdminRole, nil, nil)
	// create tesla admin
	MustCreateTestUser(tInfo.apiClient, testutils.TestLocalUser, testutils.TestLocalPassword, "tesla")
	defer MustDeleteUser(tInfo.apiClient, testutils.TestLocalUser, "tesla")
	MustUpdateRoleBinding(tInfo.apiClient, globals.AdminRoleBinding, "tesla", globals.AdminRole, []string{testutils.TestLocalUser}, nil)
	defer MustUpdateRoleBinding(tInfo.apiClient, globals.AdminRoleBinding, "tesla", globals.AdminRole, nil, nil)
	// create user with no role in tesla tenant
	MustCreateTestUser(tInfo.apiClient, "noRoleUser", testutils.TestLocalPassword, "tesla")
	defer MustDeleteUser(tInfo.apiClient, "noRoleUser", "tesla")
	// create network search role
	MustCreateRole(tInfo.apiClient, "NetworkSearchRole", "tesla",
		login.NewPermission("tesla", string(apiclient.GroupNetwork), string(network.KindNetwork), authz.ResourceNamespaceAll, "", auth.Permission_Read.String()))
	defer MustDeleteRole(tInfo.apiClient, "NetworkSearchRole", "tesla")
	// create user with network search role
	MustCreateTestUser(tInfo.apiClient, "networkSearchUser", testutils.TestLocalPassword, "tesla")
	defer MustDeleteUser(tInfo.apiClient, "networkSearchUser", "tesla")
	MustCreateRoleBinding(tInfo.apiClient, "NetworkSearchRoleBinding", "tesla", "NetworkSearchRole", []string{"networkSearchUser"}, nil)
	defer MustDeleteRoleBinding(tInfo.apiClient, "NetworkSearchRoleBinding", "tesla")
	// create network search role in default tenant
	MustCreateRole(tInfo.apiClient, "NetworkSearchRole", globals.DefaultTenant,
		login.NewPermission(globals.DefaultTenant, string(apiclient.GroupNetwork), string(network.KindNetwork), authz.ResourceNamespaceAll, "", auth.Permission_Read.String()))
	defer MustDeleteRole(tInfo.apiClient, "NetworkSearchRole", globals.DefaultTenant)
	// create user with network search role
	MustCreateTestUser(tInfo.apiClient, "networkSearchUser", testutils.TestLocalPassword, globals.DefaultTenant)
	defer MustDeleteUser(tInfo.apiClient, "networkSearchUser", globals.DefaultTenant)
	MustCreateRoleBinding(tInfo.apiClient, "NetworkSearchRoleBinding", globals.DefaultTenant, "NetworkSearchRole", []string{"networkSearchUser"}, nil)
	defer MustDeleteRoleBinding(tInfo.apiClient, "NetworkSearchRoleBinding", globals.DefaultTenant)
	// create event search role in default tenant
	MustCreateRole(tInfo.apiClient, "EventSearchRole", globals.DefaultTenant,
		login.NewPermission(globals.DefaultTenant, "", auth.Permission_Event.String(), authz.ResourceNamespaceAll, "", auth.Permission_Read.String()))
	defer MustDeleteRole(tInfo.apiClient, "EventSearchRole", globals.DefaultTenant)
	// create user with event search role
	MustCreateTestUser(tInfo.apiClient, "eventSearchUser", testutils.TestLocalPassword, globals.DefaultTenant)
	defer MustDeleteUser(tInfo.apiClient, "eventSearchUser", globals.DefaultTenant)
	MustCreateRoleBinding(tInfo.apiClient, "EventSearchRoleBinding", globals.DefaultTenant, "EventSearchRole", []string{"eventSearchUser"}, nil)
	defer MustDeleteRoleBinding(tInfo.apiClient, "EventSearchRoleBinding", globals.DefaultTenant)

	// authenticate users
	audiAdminHdr, err := testutils.GetAuthorizationHeader(tInfo.apiGwAddr, &auth.PasswordCredential{
		Username: testutils.TestLocalUser,
		Password: testutils.TestLocalPassword,
		Tenant:   "audi",
	})
	if err != nil {
		t.Fatalf("audi admin login failed, err: {%+v}", err)
	}
	teslaAdminHdr, err := testutils.GetAuthorizationHeader(tInfo.apiGwAddr, &auth.PasswordCredential{
		Username: testutils.TestLocalUser,
		Password: testutils.TestLocalPassword,
		Tenant:   "tesla",
	})
	if err != nil {
		t.Fatalf("tesla admin login failed, err: {%+v}", err)
	}
	noRoleTeslaUserHdr, err := testutils.GetAuthorizationHeader(tInfo.apiGwAddr, &auth.PasswordCredential{
		Username: "noRoleUser",
		Password: testutils.TestLocalPassword,
		Tenant:   "tesla",
	})
	if err != nil {
		t.Fatalf("tesla no role user login failed, err: {%+v}", err)
	}
	networkSearchUserHdr, err := testutils.GetAuthorizationHeader(tInfo.apiGwAddr, &auth.PasswordCredential{
		Username: "networkSearchUser",
		Password: testutils.TestLocalPassword,
		Tenant:   "tesla",
	})
	if err != nil {
		t.Fatalf("tesla network search user login failed, err: {%+v}", err)
	}
	networkSearchDefaultUserHdr, err := testutils.GetAuthorizationHeader(tInfo.apiGwAddr, &auth.PasswordCredential{
		Username: "networkSearchUser",
		Password: testutils.TestLocalPassword,
		Tenant:   globals.DefaultTenant,
	})
	if err != nil {
		t.Fatalf("default tenant network search user login failed, err: {%+v}", err)
	}
	eventSearchDefaultUserHdr, err := testutils.GetAuthorizationHeader(tInfo.apiGwAddr, &auth.PasswordCredential{
		Username: "eventSearchUser",
		Password: testutils.TestLocalPassword,
		Tenant:   globals.DefaultTenant,
	})
	if err != nil {
		t.Fatalf("default tenant event search user login failed, err: {%+v}", err)
	}
	// Http error for no search authorization
	httpUnauthorizedErrCode := grpcruntime.HTTPStatusFromCode(grpccodes.PermissionDenied)
	httpUnauthorizedErr := fmt.Errorf("Server responded with %d", httpUnauthorizedErrCode)
	// Testcases for various queries on config objects
	queryTestcases := []struct {
		authzHeader    string
		query          *search.SearchRequest
		mode           string
		sortBy         string
		expectedHits   int64
		previewResults map[string]map[string]map[string]int64
		aggResults     map[string]map[string]map[string]map[string]interface{}
		err            error
	}{
		//
		// Search Test cases with URI params, to test QueryString query
		// Uses GET method
		// tenant admin searching cluster scoped objects
		{
			audiAdminHdr,
			&search.SearchRequest{
				QueryString: "kind:Tenant",
				From:        from,
				MaxResults:  maxResults,
				Aggregate:   true,
			},
			search.SearchRequest_Full.String(),
			"",
			0,
			nil,
			nil,
			nil,
		},
		// tesla tenant admin searching networks in audi and tesla tenant
		{
			teslaAdminHdr,
			&search.SearchRequest{
				QueryString: "kind:Network",
				From:        from,
				MaxResults:  maxResults,
				Tenants:     []string{"audi", "tesla"},
				Aggregate:   true,
			},
			search.SearchRequest_Full.String(),
			"",
			objectCount - 2,
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"tesla": {
					"Network": {
						"Network": {
							"net00": nil,
							"net02": nil,
							"net04": nil,
						},
					},
				},
			},
			nil,
		},
		// tesla no role user searching networks in tesla tenant
		{
			noRoleTeslaUserHdr,
			&search.SearchRequest{
				QueryString: "kind:Network",
				From:        from,
				MaxResults:  maxResults,
				Tenants:     []string{"tesla"},
				Aggregate:   true,
			},
			search.SearchRequest_Full.String(),
			"",
			0,
			nil,
			nil,
			httpUnauthorizedErr,
		},
		// tesla network search user searching security groups in tesla tenant
		{
			networkSearchUserHdr,
			&search.SearchRequest{
				QueryString: "kind:SecurityGroup",
				From:        from,
				MaxResults:  maxResults,
				Tenants:     []string{"tesla"},
				Aggregate:   true,
			},
			search.SearchRequest_Full.String(),
			"",
			0,
			nil,
			nil,
			nil,
		},
		// tesla network search user searching networks in tesla tenant
		{
			networkSearchUserHdr,
			&search.SearchRequest{
				QueryString: "kind:Network",
				From:        from,
				MaxResults:  maxResults,
				Tenants:     []string{"tesla"},
				Aggregate:   true,
			},
			search.SearchRequest_Full.String(),
			"",
			objectCount - 2,
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"tesla": {
					"Network": {
						"Network": {
							"net00": nil,
							"net02": nil,
							"net04": nil,
						},
					},
				},
			},
			nil,
		},
		// default tenant network search user searching events in default tenant
		{
			networkSearchDefaultUserHdr,
			&search.SearchRequest{
				QueryString: "kind:Event",
				From:        from,
				MaxResults:  maxResults,
				Tenants:     []string{globals.DefaultTenant},
				Aggregate:   true,
			},
			search.SearchRequest_Full.String(),
			"",
			0,
			nil,
			nil,
			nil,
		},
		// default tenant event search user searching events in default tenant
		{
			eventSearchDefaultUserHdr,
			&search.SearchRequest{
				QueryString: "kind:Event",
				From:        from,
				MaxResults:  maxResults * 2,
				Tenants:     []string{globals.DefaultTenant},
				Aggregate:   true,
			},
			search.SearchRequest_Preview.String(),
			"",
			eventCount,
			map[string]map[string]map[string]int64{
				"default": {
					"Monitoring": {
						"Event": eventCount,
					},
				},
			},
			nil,
			nil,
		},
	}
	// Execute the Query Testcases
	for _, tc := range queryTestcases {
		t.Run(tc.query.QueryString, func(t *testing.T) {
			AssertEventually(t,
				func() (bool, interface{}) {

					// execute search
					var err error
					var resp search.SearchResponse
					var searchURL string
					if searchMethod == GetWithURI && len(tc.query.QueryString) != 0 {
						// Query using URI params - via GET method
						t.Logf("@@@ GET QueryString query: %s\n", tc.query.QueryString)
						searchURL = getSearchURLWithParams(t, tc.query.QueryString, tc.query.From,
							tc.query.MaxResults, tc.mode, tc.sortBy, tc.query.Aggregate, tc.query.Tenants)
						resp = search.SearchResponse{}
						restcl := netutils.NewHTTPClient()
						restcl.WithTLSConfig(&tls.Config{InsecureSkipVerify: true})
						restcl.SetHeader("Authorization", tc.authzHeader)
						restcl.DisableKeepAlives()
						defer restcl.CloseIdleConnections()

						start := time.Now().UTC()
						_, err = restcl.Req("GET", searchURL, nil, &resp)
						t.Logf("@@@ Search response time: %+v\n", time.Since(start))
					} else {
						// Query using Body - via POST, GET
						var httpMethod string
						switch searchMethod {
						case GetWithBody:
							httpMethod = "GET"
						case PostWithBody:
							httpMethod = "POST"
						default:
							httpMethod = "GET"
						}

						t.Logf("@@@ Search request [%s] Body: %+v\n", httpMethod, tc.query)
						searchURL = getSearchURL()
						resp = search.SearchResponse{}
						restcl := netutils.NewHTTPClient()
						restcl.WithTLSConfig(&tls.Config{InsecureSkipVerify: true})
						restcl.SetHeader("Authorization", tInfo.authzHeader)
						restcl.DisableKeepAlives()
						defer restcl.CloseIdleConnections()

						start := time.Now().UTC()
						_, err = restcl.Req(httpMethod, searchURL, &tc.query, &resp)
						t.Logf("@@@ Search response time: %+v\n", time.Since(start))
					}

					if (err != nil && tc.err == nil) ||
						(err == nil && tc.err != nil) ||
						(err != nil && tc.err != nil && err.Error() != tc.err.Error()) {
						t.Logf("@@@ Search response didn't match expected error, expected:%+v actual:%+v",
							tc.err, err)
						return false, nil
					}
					tInfo.l.Infof("Query: %s, result : %+v", searchURL, resp)
					return checkSearchQueryResponse(t, tc.query, &resp, tc.expectedHits, tc.previewResults, tc.aggResults), nil
				}, fmt.Sprintf("Query failed for: %s", tc.query.QueryString), "100ms", "1m")
		})
	}
}

func TestMain(m *testing.M) {

	rand.Seed(time.Now().UnixNano())
	tInfo.l = logger

	// Run tests
	rcode := m.Run()

	os.Exit(rcode)
}
