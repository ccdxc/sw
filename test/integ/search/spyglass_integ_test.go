// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package search

import (
	"context"
	"fmt"
	"math/rand"
	"net/url"
	"os"
	"strings"
	"testing"
	"time"

	grpcruntime "github.com/pensando/grpc-gateway/runtime"
	uuid "github.com/satori/go.uuid"
	grpccodes "google.golang.org/grpc/codes"

	api "github.com/pensando/sw/api"
	apicache "github.com/pensando/sw/api/client"
	"github.com/pensando/sw/api/fields"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	evtsapi "github.com/pensando/sw/api/generated/events"
	_ "github.com/pensando/sw/api/generated/exports/apigw"
	_ "github.com/pensando/sw/api/generated/exports/apiserver"
	"github.com/pensando/sw/api/generated/search"
	_ "github.com/pensando/sw/api/hooks/apiserver"
	"github.com/pensando/sw/api/labels"
	testutils "github.com/pensando/sw/test/utils"
	"github.com/pensando/sw/venice/apigw"
	_ "github.com/pensando/sw/venice/apigw/svc"
	"github.com/pensando/sw/venice/apiserver"
	certsrv "github.com/pensando/sw/venice/cmd/grpc/server/certificates/mock"
	types "github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/spyglass/finder"
	"github.com/pensando/sw/venice/spyglass/indexer"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/kvstore/etcd/integration"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
	mockresolver "github.com/pensando/sw/venice/utils/resolver/mock"
	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/rpckit/tlsproviders"
	"github.com/pensando/sw/venice/utils/runtime"
	"github.com/pensando/sw/venice/utils/testenv"
	. "github.com/pensando/sw/venice/utils/testutils"
)

const (
	registryURL  = "registry.test.pensando.io:5000"
	elasticImage = "elasticsearch:6.3.0"
	// TLS keys and certificates used by mock CKM endpoint to generate control-plane certs
	certPath  = "../../../venice/utils/certmgr/testdata/ca.cert.pem"
	keyPath   = "../../../venice/utils/certmgr/testdata/ca.key.pem"
	rootsPath = "../../../venice/utils/certmgr/testdata/roots.pem"

	from       = int32(0)
	maxResults = int32(50)
	// objectCount is count of objects to be generated
	objectCount int64 = 5
	// test user
	testUser     = "test"
	testPassword = "pensando"
)

var (
	// create events recorder
	_, _ = recorder.NewRecorder(
		&evtsapi.EventSource{NodeName: utils.GetHostname(), Component: "search_integ_test"},
		evtsapi.GetEventTypes(), "", "/tmp")
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
	fdr               finder.Interface
	fdrAddr           string
	idr               indexer.Interface
	certSrv           *certsrv.CertSrv
	authzHeader       string
	mockResolver      *mockresolver.ResolverClient
}

var letters = []rune("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ")

var tInfo testInfo

func (tInfo *testInfo) setup(kvstoreConfig *store.Config) error {
	var err error

	// start elasticsearch
	tInfo.elasticServerName = uuid.NewV4().String()
	testutils.StopElasticsearch(tInfo.elasticServerName)
	tInfo.elasticURL, err = testutils.StartElasticsearch(tInfo.elasticServerName)
	if err != nil {
		return fmt.Errorf("failed to start elasticsearch, err: %v", err)
	}
	tInfo.updateResolver(globals.ElasticSearch, tInfo.elasticURL) // add mock elastic service to mock resolver

	if !testutils.IsEalsticClusterHealthy(tInfo.elasticURL) {
		return fmt.Errorf("elasticsearch cluster not healthy")
	}

	// start spyglass finder
	fdr, fdrAddr, err := testutils.StartSpyglass("finder", "", tInfo.mockResolver, tInfo.l)
	if err != nil {
		return err
	}
	tInfo.fdr = fdr.(finder.Interface)
	tInfo.fdrAddr = fdrAddr

	// start API server
	tInfo.apiServer, tInfo.apiServerAddr, err = testutils.StartAPIServer(":0", kvstoreConfig, tInfo.l)
	if err != nil {
		return err
	}

	// start API gateway
	tInfo.apiGw, tInfo.apiGwAddr, err = testutils.StartAPIGateway(":0",
		map[string]string{globals.APIServer: tInfo.apiServerAddr, globals.Spyglass: tInfo.fdrAddr},
		[]string{}, tInfo.l)
	if err != nil {
		return err
	}

	// start spygalss indexer
	idr, _, err := testutils.StartSpyglass("indexer", tInfo.apiServerAddr, tInfo.mockResolver, tInfo.l)
	if err != nil {
		return err
	}
	tInfo.idr = idr.(indexer.Interface)

	// create API server client
	apiCl, err := apicache.NewGrpcUpstream("spyglass-integ-test", tInfo.apiServerAddr, tInfo.l)
	if err != nil {
		return fmt.Errorf("failed to create gRPC client, err: %v", err)
	}
	tInfo.apiClient = apiCl

	// setup auth
	userCreds := &auth.PasswordCredential{Username: testutils.TestLocalUser, Password: testutils.TestLocalPassword, Tenant: testutils.TestTenant}
	err = testutils.SetupAuth(tInfo.apiServerAddr, true, false, userCreds, tInfo.l)
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

	// stop elasticsearch
	testutils.StopElasticsearch(tInfo.elasticServerName)

	// stop apiGW
	tInfo.apiGw.Stop()

	// stop apiServer
	tInfo.apiServer.Stop()

	// stop certificate server
	tInfo.certSrv.Stop()
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

// helper to generate random string of requested length
func randStr(n int) string {
	r := make([]rune, n)
	for i := range r {
		r[i] = letters[rand.Intn(len(letters))]
	}
	return string(r)
}

func getSearchURLWithParams(t *testing.T, query string, from, maxResults int32, mode, sortBy string) string {

	// convert to query-string to url-encoded string
	// to escape special characters like space, comma, braces.
	u := url.URL{Path: query}
	urlQuery := url.QueryEscape(query)
	t.Logf("Query: %s Encoded: %s Escaped: %s Mode: %s SortBy: %s",
		query, u.String(), urlQuery, mode, sortBy)
	str := fmt.Sprintf("http://%s/search/v1/query?QueryString=%s&From=%d&MaxResults=%d",
		tInfo.apiGwAddr, urlQuery, from, maxResults)
	if mode != "" {
		str += fmt.Sprintf("&Mode=%s", mode)
	}
	if sortBy != "" {
		str += fmt.Sprintf("&SortBy=%s", sortBy)
	}
	fmt.Printf("\n+++ QUERY URL: %s\n", str)
	return str
}

func getSearchURL() string {
	return fmt.Sprintf("http://%s/search/v1/query", tInfo.apiGwAddr)
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
	logConfig := log.GetDefaultConfig("spyglass_integ_test")
	tInfo.l = log.GetNewLogger(logConfig)

	// cluster bind mounts in local directory. certain filesystems (like vboxsf, nfs) dont support unix binds.
	os.Chdir("/tmp")
	cluster := integration.NewClusterV3(t) //create etcd cluster

	AssertOk(t, tInfo.setup(&store.Config{
		Type:    store.KVStoreTypeEtcd,
		Servers: strings.Split(cluster.ClientURL(), ","),
		Codec:   runtime.NewJSONCodec(runtime.GetDefaultScheme())}),
		"failed to setup test")
	defer tInfo.teardown()

	ctx := context.Background()

	// Generate objects in api-server to trigger indexer watch
	go PolicyGenerator(ctx, tInfo.apiClient, objectCount)

	// Validate the object count
	expectedCount := uint64(3*objectCount + int64(len(Tenants)))
	expectedCount += 2 // for auth policy and test user
	AssertEventually(t,
		func() (bool, interface{}) {

			if expectedCount != tInfo.idr.GetObjectCount() {
				t.Logf("Retrying, indexed objects count mismatch - expected: %d actual: %d",
					expectedCount, tInfo.idr.GetObjectCount())
				return false, nil
			}
			return true, nil
		}, "Failed to match count of indexed objects", "20ms", "2m")

	var resp search.SearchResponse

	// Validate the search REST endpoint
	t.Logf("Validating Search REST endpoint ...")
	AssertEventually(t,
		func() (bool, interface{}) {

			restcl := netutils.NewHTTPClient()
			restcl.SetHeader("Authorization", tInfo.authzHeader)
			_, err = restcl.Req("GET", getSearchURLWithParams(t, "tesla", 0, 10, search.SearchRequest_Full.String(), ""), nil, &resp)
			if err != nil {
				t.Logf("GET on search REST endpoint: %s failed, err:%+v",
					getSearchURLWithParams(t, "tesla", 0, 10, search.SearchRequest_Full.String(), ""), err)
				return false, nil
			}
			return true, nil
		}, "Failed to validate Search REST endpoint", "20ms", "2m")

	// Perform search tests
	performSearchTests(t)

	// Stop Indexer
	t.Logf("Stopping indexer ...")
	tInfo.idr.Stop()

	// create the indexer again
	t.Logf("Creating and starting new indexer instance")
	AssertEventually(t,
		func() (bool, interface{}) {
			tInfo.idr, err = indexer.NewIndexer(ctx, tInfo.apiServerAddr, tInfo.mockResolver, tInfo.l)
			if err != nil {
				t.Logf("Error creating indexer: %v", err)
				return false, nil
			}
			return true, nil
		}, "Failed to create indexer", "20ms", "1m")

	// start the indexer
	err = tInfo.idr.Start()
	if err != nil {
		t.Fatal("failed to get start indexer")
	}

	// Validate the object count matches after restart
	AssertEventually(t,
		func() (bool, interface{}) {

			if expectedCount != tInfo.idr.GetObjectCount() {
				t.Logf("Retrying, indexed objects count mismatch expected: %d actual: %d",
					expectedCount, tInfo.idr.GetObjectCount())
				return false, nil
			}
			return true, nil
		}, "Failed to match count of indexed objects", "20ms", "2m")

	// Perform search tests again after indexer restart
	performSearchTests(t)
	tInfo.idr.Stop()
	t.Logf("Done with Tests ....")
}

// Execute search test cases
func performSearchTests(t *testing.T) {

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
		// Uses GET method (GET with body is not supported in many http clients)
		//
		{
			search.SearchRequest{
				QueryString: "kind:Tenant",
				From:        from,
				MaxResults:  maxResults,
			},
			search.SearchRequest_Full.String(),
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
			search.SearchRequest{
				QueryString: "kind:SmartNIC",
				From:        from,
				MaxResults:  maxResults,
			},
			search.SearchRequest_Full.String(),
			"",
			objectCount,
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"default": {
					"Cluster": {
						"SmartNIC": {
							"44.44.44.00.00.00": nil,
							"44.44.44.00.00.01": nil,
							"44.44.44.00.00.02": nil,
							"44.44.44.00.00.03": nil,
							"44.44.44.00.00.04": nil,
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
				QueryString: "kind:SmartNIC",
				From:        0, // from offset-0
				MaxResults:  3, // get 3 results
			},
			search.SearchRequest_Full.String(),
			"meta.name.keyword",
			3,
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"default": {
					"Cluster": {
						"SmartNIC": {
							"44.44.44.00.00.00": nil,
							"44.44.44.00.00.01": nil,
							"44.44.44.00.00.02": nil,
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
				QueryString: "kind:SmartNIC",
				From:        3, // from offset-3
				MaxResults:  2, // get 2 results
			},
			search.SearchRequest_Full.String(),
			"meta.name.keyword",
			2,
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"default": {
					"Cluster": {
						"SmartNIC": {
							"44.44.44.00.00.03": nil,
							"44.44.44.00.00.04": nil,
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
				QueryString: "SmartNIC",
				From:        from,
				MaxResults:  maxResults,
			},
			search.SearchRequest_Full.String(),
			"",
			objectCount,
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"default": {
					"Cluster": {
						"SmartNIC": {
							"44.44.44.00.00.00": nil,
							"44.44.44.00.00.01": nil,
							"44.44.44.00.00.02": nil,
							"44.44.44.00.00.03": nil,
							"44.44.44.00.00.04": nil,
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
				MaxResults:  maxResults,
			},
			search.SearchRequest_Full.String(),
			"",
			objectCount + int64(len(Tenants)),
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"default": {
					"Cluster": {
						"SmartNIC": {
							"44.44.44.00.00.00": nil,
							"44.44.44.00.00.01": nil,
							"44.44.44.00.00.02": nil,
							"44.44.44.00.00.03": nil,
							"44.44.44.00.00.04": nil,
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
				MaxResults:  maxResults,
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
						"SmartNIC": {
							"44.44.44.00.00.00": nil,
							"44.44.44.00.00.01": nil,
							"44.44.44.00.00.02": nil,
							"44.44.44.00.00.03": nil,
							"44.44.44.00.00.04": nil,
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
				QueryString: "44.44.44.00.00.01",
				From:        from,
				MaxResults:  maxResults,
			},
			search.SearchRequest_Full.String(),
			"",
			1,
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"default": {
					"Cluster": {
						"SmartNIC": {
							"44.44.44.00.00.01": nil,
						},
					},
				},
			},
			nil,
		},
		{
			// Precise match on a IP address, part of Network object
			search.SearchRequest{
				QueryString: "10.0.1.254",
				From:        from,
				MaxResults:  maxResults,
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
				QueryString: "smartnic",
				From:        from,
				MaxResults:  maxResults,
			},
			"",
			"",
			objectCount,
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"default": {
					"Cluster": {
						"SmartNIC": {
							"44.44.44.00.00.00": nil,
							"44.44.44.00.00.01": nil,
							"44.44.44.00.00.02": nil,
							"44.44.44.00.00.03": nil,
							"44.44.44.00.00.04": nil,
						},
					},
				},
			},
			nil,
		},
		// Text search (case insensitive and prefix match) on kind
		{
			search.SearchRequest{
				QueryString: "smart*",
				From:        from,
				MaxResults:  maxResults,
			},
			"",
			"",
			objectCount,
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"default": {
					"Cluster": {
						"SmartNIC": {
							"44.44.44.00.00.00": nil,
							"44.44.44.00.00.01": nil,
							"44.44.44.00.00.02": nil,
							"44.44.44.00.00.03": nil,
							"44.44.44.00.00.04": nil,
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
				QueryString: randStr(512),
				From:        from,
				MaxResults:  maxResults,
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
			},
			search.SearchRequest_Full.String(),
			"",
			0,
			nil,
			nil,
			httpInvalidArgErr,
		},

		// Search preview tests - using GET
		{
			// Term query with multi-value match on Kind
			search.SearchRequest{
				QueryString: "kind:(Network OR SecurityGroup)",
				From:        from,
				MaxResults:  maxResults,
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
		{
			// Non-existent Text in preview mode
			search.SearchRequest{
				QueryString: "OzzyOzbuorne",
				From:        from,
				MaxResults:  maxResults,
			},
			search.SearchRequest_Preview.String(),
			"",
			0,
			nil,
			nil,
			nil,
		},

		//
		// Search test cases with using Post with BODY for advanced queries
		//
		{
			search.SearchRequest{
				Query: &search.SearchQuery{
					Kinds: []string{"Tenant"},
				},
				From:       from,
				MaxResults: maxResults,
			},
			"", // default mode is Complete
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
			search.SearchRequest{
				Query: &search.SearchQuery{
					Kinds: []string{"SmartNIC"},
				},
				From:       from,
				MaxResults: maxResults,
			},
			search.SearchRequest_Full.String(),
			"",
			objectCount,
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"default": {
					"Cluster": {
						"SmartNIC": {
							"44.44.44.00.00.00": nil,
							"44.44.44.00.00.01": nil,
							"44.44.44.00.00.02": nil,
							"44.44.44.00.00.03": nil,
							"44.44.44.00.00.04": nil,
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
					Kinds: []string{"SmartNIC"},
				},
				From:       0, // from offset-0
				MaxResults: 3, // get 3 results
				SortBy:     "meta.name.keyword",
			},
			"",
			"meta.name.keyword",
			3,
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"default": {
					"Cluster": {
						"SmartNIC": {
							"44.44.44.00.00.00": nil,
							"44.44.44.00.00.01": nil,
							"44.44.44.00.00.02": nil,
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
					Kinds: []string{"SmartNIC"},
				},
				From:       3, // from offset-3
				MaxResults: 2, // get 2 results
				SortBy:     "meta.name.keyword",
			},
			"",
			"meta.name.keyword",
			2,
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"default": {
					"Cluster": {
						"SmartNIC": {
							"44.44.44.00.00.03": nil,
							"44.44.44.00.00.04": nil,
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
							&labels.Requirement{
								Key:      "meta.labels.Application",
								Operator: "equals",
								Values:   []string{"MS-Exchange"},
							},
						},
					},
				},
				From:       from,
				MaxResults: maxResults,
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
						&search.TextRequirement{
							Text: []string{"SmartNIC"},
						},
					},
				},
				From:       from,
				MaxResults: maxResults,
			},
			"",
			"",
			objectCount,
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"default": {
					"Cluster": {
						"SmartNIC": {
							"44.44.44.00.00.00": nil,
							"44.44.44.00.00.01": nil,
							"44.44.44.00.00.02": nil,
							"44.44.44.00.00.03": nil,
							"44.44.44.00.00.04": nil,
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
						&search.TextRequirement{
							Text: []string{"smartnic"},
						},
					},
				},
				From:       from,
				MaxResults: maxResults,
			},
			"",
			"",
			objectCount,
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"default": {
					"Cluster": {
						"SmartNIC": {
							"44.44.44.00.00.00": nil,
							"44.44.44.00.00.01": nil,
							"44.44.44.00.00.02": nil,
							"44.44.44.00.00.03": nil,
							"44.44.44.00.00.04": nil,
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
						&search.TextRequirement{
							Text: []string{"smart*"},
						},
					},
				},
				From:       from,
				MaxResults: maxResults,
			},
			"",
			"",
			objectCount,
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"default": {
					"Cluster": {
						"SmartNIC": {
							"44.44.44.00.00.00": nil,
							"44.44.44.00.00.01": nil,
							"44.44.44.00.00.02": nil,
							"44.44.44.00.00.03": nil,
							"44.44.44.00.00.04": nil,
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
						&search.TextRequirement{
							Text: []string{"infra"},
						},
					},
				},
				From:       from,
				MaxResults: maxResults,
			},
			"",
			"",
			objectCount + int64(len(Tenants)),
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"default": {
					"Cluster": {
						"SmartNIC": {
							"44.44.44.00.00.00": nil,
							"44.44.44.00.00.01": nil,
							"44.44.44.00.00.02": nil,
							"44.44.44.00.00.03": nil,
							"44.44.44.00.00.04": nil,
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
						&search.TextRequirement{
							Text: []string{"us-west"},
						},
					},
				},
				From:       from,
				MaxResults: maxResults,
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
						"SmartNIC": {
							"44.44.44.00.00.00": nil,
							"44.44.44.00.00.01": nil,
							"44.44.44.00.00.02": nil,
							"44.44.44.00.00.03": nil,
							"44.44.44.00.00.04": nil,
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
						&search.TextRequirement{
							Text: []string{"human resources"},
						},
					},
				},
				From:       from,
				MaxResults: maxResults,
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
					Kinds: []string{"SmartNIC"},
					Fields: &fields.Selector{
						Requirements: []*fields.Requirement{
							&fields.Requirement{
								Key:      "meta.name",
								Operator: "equals",
								Values:   []string{"44.44.44.00.00.01"},
							},
						},
					},
				},
				From:       from,
				MaxResults: maxResults,
			},
			"",
			"",
			1,
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"default": {
					"Cluster": {
						"SmartNIC": {
							"44.44.44.00.00.01": nil,
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
					Kinds: []string{"SmartNIC"},
					Fields: &fields.Selector{
						Requirements: []*fields.Requirement{
							&fields.Requirement{
								Key:      "meta.name",
								Operator: "notEquals",
								Values:   []string{"44.44.44.00.00.01"},
							},
						},
					},
				},
				From:       from,
				MaxResults: maxResults,
			},
			"",
			"",
			4,
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"default": {
					"Cluster": {
						"SmartNIC": {
							"44.44.44.00.00.00": nil,
							"44.44.44.00.00.02": nil,
							"44.44.44.00.00.03": nil,
							"44.44.44.00.00.04": nil,
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
					Kinds: []string{"SmartNIC"},
					Fields: &fields.Selector{
						Requirements: []*fields.Requirement{
							&fields.Requirement{
								Key:      "meta.name",
								Operator: "in",
								Values:   []string{"44.44.44.00.00.00", "44.44.44.00.00.01"},
							},
						},
					},
				},
				From:       from,
				MaxResults: maxResults,
			},
			"",
			"",
			2,
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"default": {
					"Cluster": {
						"SmartNIC": {
							"44.44.44.00.00.00": nil,
							"44.44.44.00.00.01": nil,
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
					Kinds: []string{"SmartNIC"},
					Fields: &fields.Selector{
						Requirements: []*fields.Requirement{
							&fields.Requirement{
								Key:      "meta.name",
								Operator: "notIn",
								Values:   []string{"44.44.44.00.00.00", "44.44.44.00.00.01"},
							},
						},
					},
				},
				From:       from,
				MaxResults: maxResults,
			},
			"",
			"",
			3,
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"default": {
					"Cluster": {
						"SmartNIC": {
							"44.44.44.00.00.02": nil,
							"44.44.44.00.00.03": nil,
							"44.44.44.00.00.04": nil,
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
						&search.TextRequirement{
							Text: []string{"10.0.1.254"},
						},
					},
				},
				From:       from,
				MaxResults: maxResults,
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
		// Test case for time based query using range operators [gt, gte, lt, lte]
		{
			// Query all SmartNIC objects created in the last 60secs using using RFC3339Nano format
			search.SearchRequest{
				Query: &search.SearchQuery{
					Kinds: []string{"SmartNIC"},
					Fields: &fields.Selector{
						Requirements: []*fields.Requirement{
							&fields.Requirement{
								Key:      "meta.creation-time",
								Operator: "gte",
								Values:   []string{time.Now().Add(-60 * time.Second).Format(time.RFC3339Nano)},
							},
							&fields.Requirement{
								Key:      "meta.creation-time",
								Operator: "lte",
								Values:   []string{time.Now().Format(time.RFC3339Nano)},
							},
						},
					},
				},
				From:       from,
				MaxResults: maxResults,
			},
			"",
			"",
			5,
			nil,
			map[string]map[string]map[string]map[string]interface{}{
				"default": {
					"Cluster": {
						"SmartNIC": {
							"44.44.44.00.00.00": nil,
							"44.44.44.00.00.01": nil,
							"44.44.44.00.00.02": nil,
							"44.44.44.00.00.03": nil,
							"44.44.44.00.00.04": nil,
						},
					},
				},
			},
			nil,
		},
		{
			// Query all Tenant objects modified in the last 60secs using RFC3339Nano format
			search.SearchRequest{
				Query: &search.SearchQuery{
					Kinds: []string{"Tenant"},
					Fields: &fields.Selector{
						Requirements: []*fields.Requirement{
							&fields.Requirement{
								Key:      "meta.mod-time",
								Operator: "gte",
								Values:   []string{time.Now().Add(-60 * time.Second).Format(time.RFC3339Nano)},
							},
							&fields.Requirement{
								Key:      "meta.mod-time",
								Operator: "lte",
								Values:   []string{time.Now().Format(time.RFC3339Nano)},
							},
						},
					},
				},
				From:       from,
				MaxResults: maxResults,
			},
			"",
			"",
			2,
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
			// Query all Tenant objects created in the last one day using YYYY-MM-DD format
			search.SearchRequest{
				Query: &search.SearchQuery{
					Kinds: []string{"Tenant"},
					Fields: &fields.Selector{
						Requirements: []*fields.Requirement{
							&fields.Requirement{
								Key:      "meta.creation-time",
								Operator: "gte",
								Values:   []string{time.Now().Add(-24 * time.Hour).Format("2006-01-02")},
							},
							&fields.Requirement{
								Key:      "meta.creation-time",
								Operator: "lte",
								Values:   []string{time.Now().Add(24 * time.Hour).Format("2006-01-02")},
							},
						},
					},
				},
				From:       from,
				MaxResults: maxResults,
			},
			"",
			"",
			2,
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
			// Invalid Kind, negative test case
			search.SearchRequest{
				Query: &search.SearchQuery{
					Kinds: []string{"Contract"},
				},
				From:       from,
				MaxResults: maxResults,
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
						&search.TextRequirement{
							Text: []string{"OzzyOzbuorne"},
						},
					},
				},
				From:       from,
				MaxResults: maxResults,
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
						&search.TextRequirement{
							Text: []string{randStr(512)},
						},
					},
				},
				From:       from,
				MaxResults: maxResults,
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
						&search.TextRequirement{
							Text: []string{"us-west"},
						},
					},
				},
				From:       from,
				MaxResults: maxResults,
				Mode:       search.SearchRequest_Preview.String(),
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
						"SmartNIC": 5,
						"Tenant":   2,
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
						&search.TextRequirement{
							Text: []string{"OzzyOzbuorne"},
						},
					},
				},
				From:       from,
				MaxResults: maxResults,
				Mode:       search.SearchRequest_Preview.String(),
			},
			"",
			"",
			0,
			nil,
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
					if len(tc.query.QueryString) != 0 {
						t.Logf("@@@ GET QueryString query: %s\n", tc.query.QueryString)
						// Query using URI params - via GET method
						searchURL = getSearchURLWithParams(t, tc.query.QueryString, tc.query.From, tc.query.MaxResults, tc.mode, tc.sortBy)
						resp = search.SearchResponse{}
						restcl := netutils.NewHTTPClient()
						restcl.SetHeader("Authorization", tInfo.authzHeader)
						_, err = restcl.Req("GET", searchURL, nil, &resp)
					} else {
						// Query using Body - via POST method
						// GET with body is not supported by many http clients including
						// net/http package.
						t.Logf("@@@ POST Query Body: %+v\n", tc.query)
						searchURL = getSearchURL()
						resp = search.SearchResponse{}
						restcl := netutils.NewHTTPClient()
						restcl.SetHeader("Authorization", tInfo.authzHeader)
						_, err = restcl.Req("POST", searchURL, &tc.query, &resp)
					}

					if (err != nil && tc.err == nil) ||
						(err == nil && tc.err != nil) ||
						(err != nil && tc.err != nil && err.Error() != tc.err.Error()) {
						t.Logf("## Search request URL: %s didn't match expected error, expected:%+v actual:%+v",
							searchURL, tc.err, err)
						return false, nil
					}

					// Match on expected hits
					if resp.ActualHits != tc.expectedHits {
						t.Logf("Result mismatch expected: %d received: %d resp: {%+v}",
							tc.expectedHits, resp.ActualHits, resp)
						return false, nil
					}

					// For cases with expectedHits to 0, return here
					if tc.expectedHits == 0 {
						return true, nil
					}

					log.Debugf("Query: %s, result : %+v", searchURL, resp)

					// Verification for "Complete" request mode
					if tc.mode == search.SearchRequest_Full.String() {

						if len(tc.aggResults) != len(resp.AggregatedEntries.Tenants) {
							log.Errorf("Tenant entries count didn't match, expected %d actual:%d",
								len(tc.aggResults), len(resp.AggregatedEntries.Tenants))
							return false, nil
						}

						// Tenant verification
						for tenantKey, tenantVal := range tc.aggResults {
							log.Debugf("Verifying tenant Key: %s entries: %d", tenantKey, len(tenantVal))
							if _, ok := resp.AggregatedEntries.Tenants[tenantKey]; !ok {
								log.Errorf("Tenant %s not found", tenantKey)
								return false, nil
							}

							// Verify count of category entries match for each tenant
							if len(tenantVal) != len(resp.AggregatedEntries.Tenants[tenantKey].Categories) {
								log.Errorf("Category entries count didn't match for tenant: %s, expected %d actual:%d",
									tenantKey, len(tenantVal), len(resp.AggregatedEntries.Tenants[tenantKey].Categories))
								return false, nil
							}

							// Category verification
							for categoryKey, categoryVal := range tenantVal {
								log.Debugf("Verifying Category Key: %s entries: %d", categoryKey, len(categoryVal))
								if _, ok := resp.AggregatedEntries.Tenants[tenantKey].Categories[categoryKey]; !ok {
									log.Errorf("Category %s not found", categoryKey)
									return false, nil
								}

								// Verify count of kind entries match for each (tenant,category)
								if len(categoryVal) != len(resp.AggregatedEntries.Tenants[tenantKey].Categories[categoryKey].Kinds) {
									log.Errorf("Kind entries count didn't match for tenant: %s category: %s, expected %d actual:%d",
										tenantKey, categoryKey, len(categoryVal), len(resp.AggregatedEntries.Tenants[tenantKey].Categories[categoryKey].Kinds))
									return false, nil
								}

								// Kind verification
								for kindKey, kindVal := range categoryVal {
									log.Debugf("Verifying Kind Key: %s entries: %d", kindKey, len(kindVal))
									if _, ok := resp.AggregatedEntries.Tenants[tenantKey].Categories[categoryKey].Kinds[kindKey]; !ok {
										log.Errorf("Kind %s not found", kindKey)
										return false, nil
									}

									// make an interim object map from the entries slice
									log.Debugf("Kinds: %+v", resp.AggregatedEntries.Tenants[tenantKey].Categories[categoryKey].Kinds[kindKey])
									entries := resp.AggregatedEntries.Tenants[tenantKey].Categories[categoryKey].Kinds[kindKey].Entries
									omap := make(map[string]interface{}, len(entries))
									for _, val := range entries {
										omap[val.GetName()] = nil
									}

									// object verification
									for objKey := range kindVal {
										log.Debugf("Verifying Object Key: %s", objKey)
										if _, ok := omap[objKey]; !ok {
											log.Errorf("Object %s not found", objKey)
											return false, nil
										}
									}
								}
							}
						}
					}

					// Verification for "Preview" request mode
					if tc.mode == search.SearchRequest_Preview.String() {

						if len(tc.previewResults) != len(resp.PreviewEntries.Tenants) {
							log.Errorf("Tenant entries count didn't match, expected %d actual:%d",
								len(tc.previewResults), len(resp.PreviewEntries.Tenants))
							return false, nil
						}

						// Tenant verification
						for tenantKey, tenantVal := range tc.previewResults {
							log.Debugf("Verifying tenant Key: %s entries: %d", tenantKey, len(tenantVal))
							if _, ok := resp.PreviewEntries.Tenants[tenantKey]; !ok {
								log.Errorf("Tenant %s not found", tenantKey)
								return false, nil
							}

							// Verify count of category entries match for each tenant
							if len(tenantVal) != len(resp.PreviewEntries.Tenants[tenantKey].Categories) {
								log.Errorf("Category entries count didn't match for tenant: %s, expected %d actual:%d",
									tenantKey, len(tenantVal), len(resp.PreviewEntries.Tenants[tenantKey].Categories))
								return false, nil
							}

							// Category verification
							for categoryKey, categoryVal := range tenantVal {
								log.Debugf("Verifying Category Key: %s entries: %d", categoryKey, len(categoryVal))
								if _, ok := resp.PreviewEntries.Tenants[tenantKey].Categories[categoryKey]; !ok {
									log.Errorf("Category %s not found", categoryKey)
									return false, nil
								}

								// Verify count of kind entries match for each (tenant,category)
								if len(categoryVal) != len(resp.PreviewEntries.Tenants[tenantKey].Categories[categoryKey].Kinds) {
									log.Errorf("Kind entries count didn't match for tenant: %s category: %s, expected %d actual:%d",
										tenantKey, categoryKey, len(categoryVal), len(resp.PreviewEntries.Tenants[tenantKey].Categories[categoryKey].Kinds))
									return false, nil
								}

								// Kind verification
								for kindKey, expNum := range categoryVal {
									var actualNum int64
									var ok bool
									log.Debugf("Verifying Kind Key: %s entries: %d", kindKey, expNum)
									if actualNum, ok = resp.PreviewEntries.Tenants[tenantKey].Categories[categoryKey].Kinds[kindKey]; !ok {
										log.Errorf("Kind %s not found", kindKey)
										return false, nil
									}

									if expNum != actualNum {
										log.Errorf("Entries mismatch for Kind: %s expected: %d obtained: %d", kindKey, expNum, actualNum)
										return false, nil
									}
								}
							}
						}
					}

					return true, nil
				}, fmt.Sprintf("Query failed for: %s", tc.query.QueryString), "100ms", "1m")
		})
	}
}

func TestMain(m *testing.M) {

	rand.Seed(time.Now().UnixNano())

	// Fill logger config params
	logConfig := &log.Config{
		Module:      "search-integ-test",
		Format:      log.LogFmt,
		Filter:      log.AllowAllFilter,
		Debug:       false,
		CtxSelector: log.ContextAll,
		LogToStdout: true,
		LogToFile:   false,
	}

	// Initialize logger config
	tInfo.l = log.SetConfig(logConfig)

	// start certificate server
	// need to do this before Chdir() so that it finds the certificates on disk
	certSrv, err := certsrv.NewCertSrv("localhost:0", certPath, keyPath, rootsPath)
	if err != nil {
		log.Errorf("Failed to create certificate server: %v", err)
		os.Exit(-1)
	}
	tInfo.certSrv = certSrv
	log.Infof("Created cert endpoint at %s", globals.CMDCertAPIPort)

	// instantiate a CKM-based TLS provider and make it default for all rpckit clients and servers
	testenv.EnableRpckitTestMode()
	tlsProvider := func(svcName string) (rpckit.TLSProvider, error) {
		return tlsproviders.NewDefaultCMDBasedProvider(certSrv.GetListenURL(), svcName)
	}
	rpckit.SetTestModeDefaultTLSProvider(tlsProvider)

	// Run tests
	rcode := m.Run()

	os.Exit(rcode)
}
