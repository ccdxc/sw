// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package search

import (
	"context"
	"fmt"
	"net"
	"net/url"
	"os"
	"os/exec"
	"strings"
	"testing"

	apicache "github.com/pensando/sw/api/client"
	"github.com/pensando/sw/api/generated/apiclient"
	_ "github.com/pensando/sw/api/generated/exports/apigw"
	_ "github.com/pensando/sw/api/generated/exports/apiserver"
	"github.com/pensando/sw/api/generated/search"
	_ "github.com/pensando/sw/api/hooks/apiserver"
	"github.com/pensando/sw/venice/apigw"
	apigwpkg "github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/apiserver"
	apiserverpkg "github.com/pensando/sw/venice/apiserver/pkg"
	certsrv "github.com/pensando/sw/venice/cmd/grpc/server/certificates/mock"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/spyglass/finder"
	"github.com/pensando/sw/venice/spyglass/indexer"
	"github.com/pensando/sw/venice/utils/elastic"
	"github.com/pensando/sw/venice/utils/kvstore/etcd/integration"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
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
}

var tInfo testInfo

// Start elasticsearch service
func startElasticsearch(t *testing.T) error {

	t.Logf("Starting elastic search ..")
	err := setupElasticsearch(t, "start")
	if err != nil {
		t.Errorf("failed to start elasticsearch container, err: %v", err)
	}

	return err
}

// Stop elasticsearch service
func stopElasticsearch(t *testing.T) error {

	err := setupElasticsearch(t, "stop")
	if err != nil {
		t.Fatalf("failed to stop elasticsearch container, err: %v", err)
	}

	return err
}

// setupElasticsearch is helper function to start/stop elasticsearch container
func setupElasticsearch(t *testing.T, action string) error {

	// spin up a new 1 node elastic cluster
	if len(strings.TrimSpace(os.Getenv("SKIP_ELASTIC_SETUP"))) != 0 {
		return nil
	}

	var cmd []string
	switch action {
	case "start":
		t.Logf("starting elasticsearch container")

		// set max_map_count; this is a must requirement to run elasticsearch
		// https://www.elastic.co/guide/en/elasticsearch/reference/current/vm-max-map-count.html
		if out, err := exec.Command("sysctl", "-w", "vm.max_map_count=262144").CombinedOutput(); err != nil {
			log.Errorf("failed to set max_map_count %s", out)
		}

		cmd = []string{
			"run", "--rm", "-d", "-p",
			// Let docker pick the exposed host port to be any port
			// in the range 7000-8000
			fmt.Sprintf("7000-8000:%s", globals.ElasticsearchRESTPort),
			"--name=pen-test-elasticsearch",
			"-e", "cluster.name=pen-test-elasticcluster",
			"-e", "xpack.security.enabled=false",
			"-e", "xpack.monitoring.enabled=false",
			"-e", "xpack.graph.enabled=false",
			"-e", "xpack.watcher.enabled=false",
			"-e", "xpack.logstash.enabled=false",
			"-e", "xpack.ml.enabled=false",
			"-e", "ES_JAVA_OPTS=-Xms512m -Xmx512m",
			fmt.Sprintf("%s/%s", registryURL, elasticImage)}
	case "stop":
		t.Logf("stopping elasticsearch container")
		cmd = []string{"rm", "-f", "pen-test-elasticsearch"}
	default:
		return fmt.Errorf("requested action not supported: %v", action)
	}

	// run the command
	var out []byte
	var err error
	if out, err = exec.Command("docker", cmd...).CombinedOutput(); err != nil &&
		!strings.Contains(string(out), "No such container") {
		t.Logf("Dccker run cmd failed, err: %+v", err)
		return fmt.Errorf("%s, err: %v", out, err)
	}

	if action == "stop" {
		return nil
	}

	// find the exposed addr:port
	params := []string{"port", "pen-test-elasticsearch", globals.ElasticsearchRESTPort}
	if out, err = exec.Command("docker", params...).CombinedOutput(); err != nil &&
		!strings.Contains(string(out), "No such container") {
		t.Logf("Docker port cmd failed, err: %+v", err)
		return fmt.Errorf("%s, err: %v", out, err)
	}

	t.Logf("Docker port output: %s", string(out))
	strs := strings.Split(string(out), ":")
	t.Logf("Docker port: %s", string(strs[1]))
	// Save the elastic Addr
	tInfo.elasticURL = fmt.Sprintf("http://127.0.0.1:%s", strings.TrimSpace(string(strs[1])))
	t.Logf("Elastic Addr: %s", tInfo.elasticURL)

	return nil
}

func getSearchURL(query string, from, maxResults int32) string {

	// convert to query-string to url-encoded string
	// to escape special characters like space, comma, braces.
	u := url.URL{Path: query}
	urlQuery := u.String()
	urlQuery = url.QueryEscape(query)
	log.Debugf("Query: %s Encoded: %s Escaped: %s", query, u.String(), urlQuery)
	return fmt.Sprintf("http://127.0.0.1:%s/v1/search/query?QueryString=%s&From=%d&MaxResults=%d",
		tInfo.apiGwPort, urlQuery, from, maxResults)
}

// Validate Elasticsearch is up and running
func validateElasticsearch(t *testing.T) {
	var err error

	// Create a client
	AssertEventually(t,
		func() (bool, interface{}) {
			tInfo.esClient, err = elastic.NewClient(tInfo.elasticURL, tInfo.l)
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
			if err := tInfo.esClient.Ping(context.Background()); err != nil {
				return false, nil
			}
			return true, nil
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

	// Start Elasticsearch service
	stopElasticsearch(t)
	err = startElasticsearch(t)
	if err != nil {
		t.Errorf("Cannot start Elasticsearch service - %v", err)
	}
	defer stopElasticsearch(t)

	// validate elasticsearch service is up
	t.Logf("Validating Elasticsearch ...")
	validateElasticsearch(t)

	// create the finder
	t.Logf("Starting finder ...")
	AssertEventually(t,
		func() (bool, interface{}) {
			fdr, err = finder.NewFinder(ctx, tInfo.elasticURL, "localhost:0", tInfo.l)
			if err != nil {
				t.Logf("Error creating finder: %v", err)
				return false, nil
			}
			return true, nil
		}, "Failed to create finder", "20ms", "1m")

	// start the finder
	err = fdr.Start()
	if err != nil {
		t.Fatal("failed to get start finder")
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
			idr, err = indexer.NewIndexer(ctx, tInfo.apiServerAddr, tInfo.elasticURL, nil, tInfo.l)
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

			err := netutils.HTTPGet(getSearchURL("tesla", 0, 10), &resp)
			if err != nil {
				log.Errorf("GET on search REST endpoint: %s failed, err:%+v",
					getSearchURL("tesla", 0, 10), err)
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
			idr, err = indexer.NewIndexer(ctx, tInfo.apiServerAddr, tInfo.elasticURL, nil, tInfo.l)
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
	// Testcases for various queries on config objects
	queryTestcases := []struct {
		query        string
		from         int32
		maxResults   int32
		expectedHits int64
		aggresults   map[string]map[string]map[string]interface{}
	}{
		{
			"kind:Tenant",
			from,
			maxResults,
			int64(len(Tenants)),
			map[string]map[string]map[string]interface{}{
				"default": {
					"Tenant": {
						"tesla": nil,
						"audi":  nil,
					},
				},
			},
		},
		{
			"kind:SmartNIC",
			from,
			maxResults,
			objectCount,
			map[string]map[string]map[string]interface{}{
				"default": {
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
		{
			"kind:SmartNIC",
			from,
			maxResults,
			objectCount,
			map[string]map[string]map[string]interface{}{
				"default": {
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
		{
			// Test Paginated query
			"kind:SmartNIC",
			0, // from offset-0
			3, // get 3 results
			objectCount,
			map[string]map[string]map[string]interface{}{
				"default": {
					"SmartNIC": {
						"44.44.44.00.00.00": nil,
						"44.44.44.00.00.01": nil,
						"44.44.44.00.00.02": nil,
					},
				},
			},
		},
		{
			// Test Paginated query
			"kind:SmartNIC",
			3, // from offset-3
			2, // get 2 results
			objectCount,
			map[string]map[string]map[string]interface{}{
				"default": {
					"SmartNIC": {
						"44.44.44.00.00.03": nil,
						"44.44.44.00.00.04": nil,
					},
				},
			},
		},
		{
			"kind:Network",
			from,
			maxResults,
			objectCount,
			map[string]map[string]map[string]interface{}{
				"audi": {
					"Network": {
						"net01": nil,
						"net03": nil,
					},
				},
				"tesla": {
					"Network": {
						"net00": nil,
						"net02": nil,
						"net04": nil,
					},
				},
			},
		},
		{
			"kind:SecurityGroup",
			from,
			maxResults,
			objectCount,
			map[string]map[string]map[string]interface{}{
				"tesla": {
					"SecurityGroup": {
						"sg00": nil,
						"sg02": nil,
						"sg04": nil,
					},
				},
				"audi": {
					"SecurityGroup": {
						"sg01": nil,
						"sg03": nil,
					},
				},
			},
		},
		{
			// Term query with multi-value match on Kind
			"kind:(Network OR SecurityGroup)",
			from,
			maxResults,
			2 * objectCount,
			map[string]map[string]map[string]interface{}{
				"tesla": {
					"Network": {
						"net00": nil,
						"net02": nil,
						"net04": nil,
					},
					"SecurityGroup": {
						"sg00": nil,
						"sg02": nil,
						"sg04": nil,
					},
				},
				"audi": {
					"Network": {
						"net01": nil,
						"net03": nil,
					},
					"SecurityGroup": {
						"sg01": nil,
						"sg03": nil,
					},
				},
			},
		},
		{
			// Term Match on Kind and Label attributes
			"kind:Network AND meta.labels.Application:MS-Exchange",
			from,
			maxResults,
			objectCount,
			map[string]map[string]map[string]interface{}{
				"tesla": {
					"Network": {
						"net00": nil,
						"net02": nil,
						"net04": nil,
					},
				},
				"audi": {
					"Network": {
						"net01": nil,
						"net03": nil,
					},
				},
			},
		},
		{
			// Text search that matches on Kind
			"SmartNIC",
			from,
			maxResults,
			objectCount,
			map[string]map[string]map[string]interface{}{
				"default": {
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
		{
			// Text search that matches on meta.Namespace
			"infra",
			from,
			maxResults,
			objectCount + int64(len(Tenants)),
			map[string]map[string]map[string]interface{}{
				"default": {
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
		{
			// Text search that matches on meta.Labels.key
			"us-west",
			from,
			maxResults,
			3*objectCount + int64(len(Tenants)),
			map[string]map[string]map[string]interface{}{
				"tesla": {
					"Network": {
						"net00": nil,
						"net02": nil,
						"net04": nil,
					},
					"SecurityGroup": {
						"sg00": nil,
						"sg02": nil,
						"sg04": nil,
					},
				},
				"default": {
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
				"audi": {
					"Network": {
						"net01": nil,
						"net03": nil,
					},
					"SecurityGroup": {
						"sg01": nil,
						"sg03": nil,
					},
				},
			},
		},
		{
			// Precise match on a MAC address, part of smartNIC object
			"44.44.44.00.00.01",
			from,
			maxResults,
			1,
			map[string]map[string]map[string]interface{}{
				"default": {
					"SmartNIC": {
						"44.44.44.00.00.01": nil,
					},
				},
			},
		},
		{
			// Precise match on a IP address, part of Network object
			"10.0.1.254",
			from,
			maxResults,
			1,
			map[string]map[string]map[string]interface{}{
				"audi": {
					"Network": {
						"net01": nil,
					},
				},
			},
		},
		{
			// Non-existent Kind
			"kind:Contract",
			from,
			maxResults,
			0,
			nil,
		},
		{
			// Non-existent Text
			"OzzyOzbuorne",
			from,
			maxResults,
			0,
			nil,
		},
	}

	// Execute the Query Testcases
	for _, tc := range queryTestcases {

		t.Run(tc.query, func(t *testing.T) {
			AssertEventually(t,
				func() (bool, interface{}) {

					// execute search
					searchURL := getSearchURL(tc.query, from, maxResults)
					resp := search.SearchResponse{}
					err := netutils.HTTPGet(searchURL, &resp)
					if err != nil {
						log.Errorf("GET on search REST endpoint: %s failed, err:%+v",
							searchURL, err)
						return false, nil
					}

					// Match on expected hits
					if resp.Result.ActualHits != tc.expectedHits {
						log.Errorf("Result mismatch expected: %d received: %d resp: {%+v}",
							tc.expectedHits, resp.Result.ActualHits, resp)
						return false, nil
					}
					log.Debugf("Query: %s, result : %+v", searchURL, *resp.Result)

					// Check size of aggregated results
					if len(tc.aggresults) != len(resp.Result.AggregatedEntries.Entries) {
						log.Errorf("Tenant agg entries count didn't match, expected %d actual:%d",
							len(tc.aggresults), len(resp.Result.AggregatedEntries.Entries))
						return false, nil
					}

					// Tenant verification
					for tenantKey, tenantVal := range tc.aggresults {
						log.Debugf("Verifying tenant Key: %s entries: %d", tenantKey, len(tenantVal))
						if _, ok := resp.Result.AggregatedEntries.Entries[tenantKey]; !ok {
							log.Errorf("Tenant %s not found", tenantKey)
							return false, nil
						}

						// Kind verification
						for kindKey, kindVal := range tenantVal {
							log.Debugf("Verifying Kind Key: %s entries: %d", kindKey, len(kindVal))
							if _, ok := resp.Result.AggregatedEntries.Entries[tenantKey].Entries[kindKey]; !ok {
								log.Errorf("Kind %s not found", kindKey)
								return false, nil
							}

							// make a interim object map from the entries slice
							entries := resp.Result.AggregatedEntries.Entries[tenantKey].Entries[kindKey].Entries
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

					return true, nil
				}, fmt.Sprintf("Query failed for: %s", tc.query), "100ms", "1m")
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
	scheme := runtime.NewScheme()
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

func TestMain(m *testing.M) {

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
