// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package search

import (
	"context"
	"fmt"
	"math/rand"
	"net"
	"net/url"
	"os"
	"strings"
	"testing"
	"time"

	grpcruntime "github.com/pensando/grpc-gateway/runtime"
	grpccodes "google.golang.org/grpc/codes"

	api "github.com/pensando/sw/api"
	apicache "github.com/pensando/sw/api/client"
	"github.com/pensando/sw/api/fields"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	_ "github.com/pensando/sw/api/generated/exports/apigw"
	_ "github.com/pensando/sw/api/generated/exports/apiserver"
	"github.com/pensando/sw/api/generated/search"
	_ "github.com/pensando/sw/api/hooks/apiserver"
	"github.com/pensando/sw/api/labels"
	loginctx "github.com/pensando/sw/api/login/context"
	testutils "github.com/pensando/sw/test/utils"
	"github.com/pensando/sw/venice/apigw"
	apigwpkg "github.com/pensando/sw/venice/apigw/pkg"
	_ "github.com/pensando/sw/venice/apigw/svc"
	"github.com/pensando/sw/venice/apiserver"
	apiserverpkg "github.com/pensando/sw/venice/apiserver/pkg"
	certsrv "github.com/pensando/sw/venice/cmd/grpc/server/certificates/mock"
	types "github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/spyglass/finder"
	"github.com/pensando/sw/venice/spyglass/indexer"
	authntestutils "github.com/pensando/sw/venice/utils/authn/testutils"
	"github.com/pensando/sw/venice/utils/elastic"
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
	registryURL  = "docker.elastic.co"
	elasticImage = "elasticsearch/elasticsearch:6.2.2"
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

type testInfo struct {
	l             log.Logger
	apiServerPort string
	apiServerAddr string
	apiGwPort     string
	elasticURL    string
	finderURL     string
	finderPort    string
	apigw         apigw.APIGateway
	apiServer     apiserver.Server
	certSrv       *certsrv.CertSrv
	esClient      elastic.ESClient
	apiClient     apiclient.Services
	authzHeader   string
}

var tInfo testInfo

var letters = []rune("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ")

// helper to generate random string of requested length
func randStr(n int) string {
	r := make([]rune, n)
	for i := range r {
		r[i] = letters[rand.Intn(len(letters))]
	}
	return string(r)
}

func getSearchURLWithParams(query string, from, maxResults int32, sortBy string) string {

	// convert to query-string to url-encoded string
	// to escape special characters like space, comma, braces.
	u := url.URL{Path: query}
	urlQuery := url.QueryEscape(query)
	log.Debugf("Query: %s Encoded: %s Escaped: %s", query, u.String(), urlQuery)
	str := fmt.Sprintf("http://127.0.0.1:%s/v1/search/query?QueryString=%s&From=%d&MaxResults=%d",
		tInfo.apiGwPort, urlQuery, from, maxResults)
	if sortBy != "" {
		str += fmt.Sprintf("&SortBy=%s", sortBy)
	}
	return str
}

func getSearchURL() string {
	return fmt.Sprintf("http://127.0.0.1:%s/v1/search/query", tInfo.apiGwPort)
}

// Validate Elasticsearch is up and running
func validateElasticsearch(t *testing.T) {
	var err error

	// Create a client
	AssertEventually(t,
		func() (bool, interface{}) {
			tInfo.esClient, err = elastic.NewClient(tInfo.elasticURL, nil, tInfo.l)
			if err != nil {
				t.Logf("error creating client: %v", err)
				log.Errorf("error creating client: %v", err)
				return false, nil
			}
			return true, nil
		}, "failed to create elastic client", "20ms", "2m")

	// ping the elastic server to get version details
	AssertEventually(t,
		func() (bool, interface{}) {
			return tInfo.esClient.IsClusterHealthy(context.Background())
		}, "failed to ping elastic cluster")

	// Getting the ES version number is quite common, so there's a shortcut
	esversion, err := tInfo.esClient.Version()
	if err != nil {
		t.Fatal("failed to get elasticsearch version")
	}

	t.Logf("Elasticsearch is UP, version %s", esversion)
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

	var fdr finder.Interface
	var idr indexer.Interface
	var err error

	ctx := context.Background()

	elasticSearchName := t.Name()
	// Start Elasticsearch service
	testutils.StopElasticsearch(elasticSearchName)
	tInfo.elasticURL, err = testutils.StartElasticsearch(elasticSearchName)
	if err != nil {
		t.Errorf("Cannot start Elasticsearch service - %v", err)
	}
	defer testutils.StopElasticsearch(elasticSearchName)

	// validate elasticsearch service is up
	t.Logf("Validating Elasticsearch ...")
	validateElasticsearch(t)

	// create mock resolver
	rsr := mockresolver.New()
	si := &types.ServiceInstance{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceInstance",
		},
		ObjectMeta: api.ObjectMeta{
			Name: globals.ElasticSearch,
		},
		Service: globals.ElasticSearch,
		URL:     tInfo.elasticURL,
	}
	// add mock elastic service to mock resolver
	rsr.AddServiceInstance(si)

	// create the finder
	t.Logf("Starting finder ...")
	AssertEventually(t,
		func() (bool, interface{}) {
			fdr, err = finder.NewFinder(ctx, "localhost:0", rsr, tInfo.l)
			if err != nil {
				t.Logf("Error creating finder: %v", err)
				return false, nil
			}
			return true, nil
		}, "Failed to create finder", "20ms", "1m")

	// start the finder
	err = fdr.Start()
	if err != nil {
		t.Fatal("failed to start finder")
	}
	defer fdr.Stop()
	log.Infof("Finder search endpoint addr: %s", fdr.GetListenURL())
	tInfo.finderURL = fdr.GetListenURL()
	_, port, err := net.SplitHostPort(tInfo.finderURL)
	if err != nil {
		t.Errorf("Failed to parse finder addr - %v", err)
		os.Exit(-1)
	}
	tInfo.finderPort = port

	// Setup the api-server, api-gw
	startAPIserverAPIgw(t)
	defer stopAPIserverAPIgw(t)

	// create the indexer
	t.Logf("Starting indexer ...")
	AssertEventually(t,
		func() (bool, interface{}) {
			idr, err = indexer.NewIndexer(ctx, tInfo.apiServerAddr, rsr, tInfo.l)
			if err != nil {
				t.Logf("Error creating indexer: %v", err)
				return false, nil
			}
			return true, nil
		}, "Failed to create indexer", "20ms", "1m")

	// start the indexer
	err = idr.Start()
	if err != nil {
		t.Fatal("failed to get start indexer")
	}

	// Generate objects in api-server to trigger indexer watch
	go PolicyGenerator(ctx, tInfo.apiClient, objectCount)

	// Validate the object count
	expectedCount := uint64(3*objectCount + int64(len(Tenants)))
	expectedCount += 2 // for auth policy and test user
	AssertEventually(t,
		func() (bool, interface{}) {

			if expectedCount != idr.GetObjectCount() {
				log.Errorf("Retrying, indexed objects count mismatch - expected: %d actual: %d",
					expectedCount, idr.GetObjectCount())
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
			_, err = restcl.Req("GET", getSearchURLWithParams("tesla", 0, 10, ""), nil, &resp)
			if err != nil {
				log.Errorf("GET on search REST endpoint: %s failed, err:%+v",
					getSearchURLWithParams("tesla", 0, 10, ""), err)
				return false, nil
			}
			return true, nil
		}, "Failed to validate Search REST endpoint", "20ms", "2m")

	// Perform search tests
	performSearchTests(t)

	// Stop Indexer
	t.Logf("Stopping indexer ...")
	idr.Stop()

	// create the indexer again
	t.Logf("Creating and starting new indexer instance")
	AssertEventually(t,
		func() (bool, interface{}) {
			idr, err = indexer.NewIndexer(ctx, tInfo.apiServerAddr, rsr, tInfo.l)
			if err != nil {
				t.Logf("Error creating indexer: %v", err)
				return false, nil
			}
			return true, nil
		}, "Failed to create indexer", "20ms", "1m")

	// start the indexer
	err = idr.Start()
	if err != nil {
		t.Fatal("failed to get start indexer")
	}

	// Validate the object count matches after restart
	AssertEventually(t,
		func() (bool, interface{}) {

			if expectedCount != idr.GetObjectCount() {
				log.Errorf("Retrying, indexed objects count mismatch expected: %d actual: %d",
					expectedCount, idr.GetObjectCount())
				return false, nil
			}
			return true, nil
		}, "Failed to match count of indexed objects", "20ms", "2m")

	// Perform search tests again after indexer restart
	performSearchTests(t)
	idr.Stop()
	t.Logf("Done with Tests ....")
}

// Execute search test cases
func performSearchTests(t *testing.T) {

	// Http error for InvalidArgument test cases
	httpInvalidArgErrCode := grpcruntime.HTTPStatusFromCode(grpccodes.InvalidArgument)
	httpInvalidArgErr := fmt.Errorf("Server responded with %d", httpInvalidArgErrCode)

	// Testcases for various queries on config objects
	queryTestcases := []struct {
		query        search.SearchRequest
		sortBy       string
		expectedHits int64
		aggresults   map[string]map[string]map[string]map[string]interface{}
		err          error
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
			"",
			int64(len(Tenants)),
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
			"",
			objectCount,
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
			"meta.name",
			3,
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
			"meta.name",
			2,
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
			"",
			objectCount,
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
			"",
			objectCount,
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
			"",
			2 * objectCount,
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
			"",
			objectCount,
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
			"",
			objectCount,
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
			"",
			objectCount + int64(len(Tenants)),
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
			"",
			3*objectCount + int64(len(Tenants)),
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
			"",
			1,
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
			"",
			1,
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

		// Negative test cases
		{
			// Invalid Kind
			search.SearchRequest{
				QueryString: "kind:Contract",
				From:        from,
				MaxResults:  maxResults,
			},
			"",
			0,
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
			"",
			0,
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
			"",
			0,
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
			"",
			0,
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
			"",
			0,
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
			"",
			0,
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
			"",
			0,
			nil,
			httpInvalidArgErr,
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
			"",
			int64(len(Tenants)),
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
			"",
			objectCount,
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
				SortBy:     "meta.name",
			},
			"meta.name",
			3,
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
				SortBy:     "meta.name",
			},
			"meta.name",
			2,
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
			objectCount,
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
			objectCount,
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
			2 * objectCount,
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
			objectCount,
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
			objectCount,
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
			objectCount + int64(len(Tenants)),
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
			3*objectCount + int64(len(Tenants)),
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
			int64(len(Tenants)),
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
			1,
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
			4,
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
			2,
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
			3,
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
			1,
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
			0,
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
			0,
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
			0,
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
			0,
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
			0,
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
			0,
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
			0,
			nil,
			fmt.Errorf("Server responded with 400"),
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
						fmt.Printf("## QueryString query: %s\n", tc.query.QueryString)
						// Query using URI params - via GET method
						searchURL = getSearchURLWithParams(tc.query.QueryString, tc.query.From, tc.query.MaxResults, tc.sortBy)
						resp = search.SearchResponse{}
						restcl := netutils.NewHTTPClient()
						restcl.SetHeader("Authorization", tInfo.authzHeader)
						_, err = restcl.Req("GET", searchURL, nil, &resp)
					} else {
						// Query using Body - via POST method
						// GET with body is not supported by many http clients including
						// net/http package.
						fmt.Printf("## Query Body: %+v\n", tc.query)
						searchURL = getSearchURL()
						resp = search.SearchResponse{}
						restcl := netutils.NewHTTPClient()
						restcl.SetHeader("Authorization", tInfo.authzHeader)
						_, err = restcl.Req("POST", searchURL, &tc.query, &resp)
					}

					if (err != nil && tc.err == nil) ||
						(err == nil && tc.err != nil) ||
						(err != nil && tc.err != nil && err.Error() != tc.err.Error()) {
						log.Errorf("Search request URL: %s didn't match expected error, expected:%+v actual:%+v",
							searchURL, tc.err, err)
						return false, nil
					}

					// Match on expected hits
					if resp.ActualHits != tc.expectedHits {
						log.Errorf("Result mismatch expected: %d received: %d resp: {%+v}",
							tc.expectedHits, resp.ActualHits, resp)
						return false, nil
					}

					// For cases with expectedHits to 0, return here
					if tc.expectedHits == 0 {
						return true, nil
					}

					// Validate the response for exact match
					// Verify count of tenant entries
					log.Debugf("Query: %s, result : %+v", searchURL, resp)
					if len(tc.aggresults) != len(resp.AggregatedEntries.Tenants) {
						log.Errorf("Tenant entries count didn't match, expected %d actual:%d",
							len(tc.aggresults), len(resp.AggregatedEntries.Tenants))
						return false, nil
					}

					// Tenant verification
					for tenantKey, tenantVal := range tc.aggresults {
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

					return true, nil
				}, fmt.Sprintf("Query failed for: %s", tc.query.QueryString), "100ms", "1m")
		})
	}
}

func startAPIserverAPIgw(t *testing.T) {

	t.Logf("Starting api-server and api-gw ...")

	// cluster bind mounts in local directory. certain filesystems (like vboxsf, nfs) dont support unix binds.
	os.Chdir("/tmp")
	cluster := integration.NewClusterV3(t)

	// Create api server
	apiServerAddress := ":0"
	scheme := runtime.GetDefaultScheme()
	srvConfig := apiserver.Config{
		GrpcServerPort: apiServerAddress,
		DebugMode:      false,
		Logger:         tInfo.l,
		Version:        "v1",
		Scheme:         scheme,
		KVPoolSize:     8,
		Kvstore: store.Config{
			Type:    store.KVStoreTypeEtcd,
			Servers: strings.Split(cluster.ClientURL(), ","),
			Codec:   runtime.NewJSONCodec(scheme),
		},
	}
	tInfo.apiServer = apiserverpkg.MustGetAPIServer()
	go tInfo.apiServer.Run(srvConfig)
	tInfo.apiServer.WaitRunning()
	addr, err := tInfo.apiServer.GetAddr()
	if err != nil {
		t.Errorf("Failed to get apiServer addr - %v", err)
	}
	_, port, err := net.SplitHostPort(addr)
	if err != nil {
		t.Errorf("Failed to parse apiServer addr - %v", err)
		os.Exit(-1)
	}
	tInfo.apiServerPort = port

	// Create api client
	tInfo.apiServerAddr = "localhost" + ":" + tInfo.apiServerPort
	apiCl, err := apicache.NewGrpcUpstream("spyglass-integ-test", tInfo.apiServerAddr, tInfo.l)
	if err != nil {
		fmt.Printf("Cannot create gRPC client - %v", err)
		os.Exit(-1)
	}
	tInfo.apiClient = apiCl

	// Start the API Gateway
	gwconfig := apigw.Config{
		HTTPAddr:  ":0",
		DebugMode: true,
		Logger:    tInfo.l,
		BackendOverride: map[string]string{
			"pen-apiserver": tInfo.apiServerAddr,
			"pen-spyglass":  "localhost:" + tInfo.finderPort,
		},
	}
	tInfo.apigw = apigwpkg.MustGetAPIGateway()
	go tInfo.apigw.Run(gwconfig)
	tInfo.apigw.WaitRunning()
	gwaddr, err := tInfo.apigw.GetAddr()
	if err != nil {
		t.Errorf("Failed to get apigw addr - %v", err)
	}
	_, port, err = net.SplitHostPort(gwaddr.String())
	if err != nil {
		t.Errorf("Failed to parse apigw addr - %v", err)
	}
	tInfo.apiGwPort = port

	setupAuth(t)
	t.Logf("ApiServer & ApiGW are UP: {%+v}", tInfo)
}

func stopAPIserverAPIgw(t *testing.T) {

	// close the apiGW
	tInfo.apigw.Stop()

	// stop the apiServer
	tInfo.apiServer.Stop()

	// stop certificate server
	tInfo.certSrv.Stop()

	t.Logf("ApiGW, ApiServer and Elastic server are STOPPED")
}

func setupAuth(t *testing.T) {
	// create authentication policy with local auth enabled
	authntestutils.CreateAuthenticationPolicy(tInfo.apiClient, &auth.Local{Enabled: true}, &auth.Ldap{Enabled: false})
	// create user
	authntestutils.CreateTestUser(tInfo.apiClient, testUser, testPassword, "default")
	getAuthorizationHeader(t)
}

func getAuthorizationHeader(t *testing.T) {
	ctx, err := authntestutils.NewLoggedInContext(context.Background(), "http://127.0.0.1:"+tInfo.apiGwPort, &auth.PasswordCredential{
		Username: testUser,
		Password: testPassword,
		Tenant:   "default",
	})
	if err != nil {
		t.Fatalf("failed to create logged in context - %v", err)
	}
	authzHeader, ok := loginctx.AuthzHeaderFromContext(ctx)
	if !ok {
		t.Fatal("failed to get authorization header from context")
	}
	tInfo.authzHeader = authzHeader
	return
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
