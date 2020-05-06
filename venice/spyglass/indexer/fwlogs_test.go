package indexer_test

import (
	"bytes"
	"context"
	"encoding/json"
	"expvar"
	"flag"
	"fmt"
	"io/ioutil"
	"net/http"
	"os"
	"os/exec"
	"strings"
	"testing"
	"time"

	dctypes "github.com/docker/docker/api/types"
	dc "github.com/docker/docker/client"
	"github.com/gorilla/mux"
	es "github.com/olivere/elastic"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/events/generated/eventtypes"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	tmagentstate "github.com/pensando/sw/nic/agent/tmagent/state"
	testelastic "github.com/pensando/sw/test/utils"
	servicetypes "github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/spyglass/cache"
	"github.com/pensando/sw/venice/spyglass/indexer"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/elastic"
	"github.com/pensando/sw/venice/utils/events/recorder"
	mockevtsrecorder "github.com/pensando/sw/venice/utils/events/recorder/mock"
	"github.com/pensando/sw/venice/utils/log"
	objstore "github.com/pensando/sw/venice/utils/objstore/client"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/resolver/mock"
	. "github.com/pensando/sw/venice/utils/testutils"
	vospkg "github.com/pensando/sw/venice/vos/pkg"
)

const (
	fwlogsSystemMetaBucketName  = "fwlogssystemmeta"
	lastProcessedKeysObjectName = "lastProcessedKeys"
)

var (
	logger             = log.GetNewLogger(log.GetDefaultConfig("flowlogs-integ-test"))
	apiServerAddr      = flag.String("api-server-addr", globals.APIServer, "ApiServer gRPC endpoint")
	mockEventsRecorder = mockevtsrecorder.NewRecorder("flowlogs-integ-test", logger)
	_                  = recorder.Override(mockEventsRecorder)
)

// TestAppendOnlyWriter tests the writer used for pushing fwlogs to elastic
// Skipping this test until it also starts spinning up tmagent and fwlog generator
func SkipTestAppendOnlyWriter(t *testing.T) {
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	logger := log.GetNewLogger(log.GetDefaultConfig("appendonlywriter_test"))
	url, _, err := testelastic.StartElasticsearch("testfwlogselastic", "", nil, nil)
	AssertOk(t, err, "failed to start elastic")
	defer testelastic.StopElasticsearch("testfwlogselastic", "")
	r := setupResolver(t, url)

	var esClient elastic.ESClient
	AssertEventually(t, func() (bool, interface{}) {
		esClient, err = testelastic.CreateElasticClient("", r, logger, nil, nil)
		return esClient != nil, esClient
	}, "failed to craete elastic client", "2s", "60s")

	// setupKibana(t, url)
	// defer stopKibana(t)

	setupVos(t, ctx, logger, "127.0.0.1")
	setupSpyglass(ctx, t, r, esClient, logger)
	setupTmAgent(ctx, t, r)
	startFwLogGen(100, 1000)

	// TestVerifyFirewallIndexName verifies that the firewall index is created
	t.Run("TestVerifyFirewallIndexName", func(t *testing.T) {
		verifyAndReturnFirewallIndexName(t, esClient)
	})

	// // TestVerifyDiskMonitoring verifies disk monitoring
	// t.Run("TestVerifyDiskMonitoring", func(t *testing.T) {
	// 	verifyDiskMonitoring(ctx, t, esClient, logger)
	// })

	// TestVerifyLastProcessedObjectKeys verifies that lastProcessedObjectKeys
	// are getting persisted in minio
	t.Run("TestVerifyLastProcessedObjectKeys", func(t *testing.T) {
		verifyLastProcessedObjectKeys(ctx, t, r, logger)
	})

	// Wait for 10 seconds and let the previous fwloggen finsih, otherwise it will impact
	// the next rate limiting related test cases.
	time.Sleep(10 * time.Second)

	// TestDebuhRESTHandle
	t.Run("TestDebugRESTHandle", func(t *testing.T) {
		verifyDebugRESTHandle(ctx, t, r, logger)
	})

	// Verify rate limiting done at Spyglass
	t.Run("TestVerifyVosRateLimitingAndEventsAtSpyglass", func(t *testing.T) {
		verifyVosRateLimitingAndEventsAtSpyglass(t)
	})
}

// SkipTestFlowLogsRateLimitingAtDSC tests the flow logs rate limiting functionality at DSC
func SkipTestFlowLogsRateLimitingAtDSC(t *testing.T) {
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()
	logger := log.GetNewLogger(log.GetDefaultConfig("vos_test"))
	r := mock.New()
	r = updateResolver(t, r, globals.Vos, "127.0.0.1:9051")
	r = updateResolver(t, r, globals.VosMinio, "127.0.0.1:19001")
	setupTmAgent(ctx, t, r)
	// TestDebuhRESTHandle
	t.Run("TestVosRateLimitingAndEvents", func(t *testing.T) {
		verifyVosRateLimitingAndEventsAtDSC(ctx, t, r, logger)
	})
}

func setupSpyglass(ctx context.Context, t *testing.T,
	r resolver.Interface, es elastic.ESClient, logger log.Logger) {
	// Create the indexer
	go func(r resolver.Interface) {
		cache := cache.NewCache(logger)
		idxer, err := indexer.NewIndexer(ctx,
			*apiServerAddr,
			r,
			cache,
			logger,
			indexer.WithElasticClient(es),
			indexer.WithDisableAPIServerWatcher(),
			indexer.WithNumVosObjectsToDelete(1))

		AssertOk(t, err, "failed to add indexer")
		Assert(t, idxer != nil, "failed to create indexer")

		router := mux.NewRouter()
		router.Methods("GET").Subrouter().Handle("/debug/vars", expvar.Handler())
		router.Methods("GET", "POST").Subrouter().Handle("/debug/config", indexer.HandleDebugConfig(idxer))
		go http.ListenAndServe(fmt.Sprintf("127.0.0.1:%s", globals.SpyglassRESTPort), router)

		err = idxer.Start()
		AssertOk(t, err, "failed to start indexer")
	}(r)
}

func setupKibana(t *testing.T, elasticURL string) {
	kibanaCmdStr :=
		"docker run " +
			"--net=host -e 'ELASTICSEARCH_URL=http://" + elasticURL + "'" +
			" -p 5601:5601 docker.elastic.co/kibana/kibana:6.3.0"

	fmt.Println("kibana command", kibanaCmdStr)
	go exec.Command("/bin/sh", "-c", kibanaCmdStr).Output()
}

func stopKibana(t *testing.T) {
	cli, err := dc.NewEnvClient()
	AssertOk(t, err, "failed to create docker client")

	containers, err := cli.ContainerList(context.Background(), dctypes.ContainerListOptions{})
	AssertOk(t, err, "failed to list docker containers")

	if len(containers) == 0 {
		return
	}

	for _, container := range containers {
		if strings.Contains(container.Image, "elastic") ||
			strings.Contains(container.Image, "kibana") {
			fmt.Println("Stopping container", container.Image, container.ID[:10])
			err := cli.ContainerStop(context.Background(), container.ID[:10], nil)
			AssertOk(t, err, "failed to stop docker container")
		}
	}
}

func setupVos(t *testing.T, ctx context.Context, logger log.Logger, url string) {
	go func() {
		args := []string{globals.Vos, "server", "--address", fmt.Sprintf("%s:%s", url, globals.VosMinioPort), "/disk1"}
		_, err := vospkg.New(ctx, false, url,
			vospkg.WithBootupArgs(args),
			vospkg.WithBucketDiskThresholds(map[string]float64{"/disk1/default.fwlogs": 0.000001}))
		AssertOk(t, err, "error in initiating Vos")
	}()
}

func setupResolver(t *testing.T, elasticURL string) resolver.Interface {
	r := mock.New()
	err := r.AddServiceInstance(&servicetypes.ServiceInstance{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceInstance",
		},
		ObjectMeta: api.ObjectMeta{
			Name: globals.ElasticSearch,
		},
		Service: globals.ElasticSearch,
		URL:     elasticURL,
	})
	AssertOk(t, err, "failed to add elasticsearch service")

	err = r.AddServiceInstance(&servicetypes.ServiceInstance{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceInstance",
		},
		ObjectMeta: api.ObjectMeta{
			Name: globals.VosMinio,
		},
		Service: globals.VosMinio,
		URL:     "127.0.0.1:19001",
	})
	AssertOk(t, err, "failed to add VosMinio service")

	err = r.AddServiceInstance(&servicetypes.ServiceInstance{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceInstance",
		},
		ObjectMeta: api.ObjectMeta{
			Name: globals.Vos,
		},
		Service: globals.Vos,
		URL:     "127.0.0.1:9051",
	})
	AssertOk(t, err, "failed to add Vos service")
	return r
}

func updateResolver(t *testing.T, r *mock.ResolverClient, name, url string) *mock.ResolverClient {
	err := r.AddServiceInstance(&servicetypes.ServiceInstance{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceInstance",
		},
		ObjectMeta: api.ObjectMeta{
			Name: name,
		},
		Service: name,
		URL:     url,
	})
	AssertOk(t, err, "failed to add Vos service")
	return r
}

func verifyAndReturnFirewallIndexName(t *testing.T, esClient elastic.ESClient) string {
	firewallIndexName := ""
	assert := func() (bool, interface{}) {
		indices, err := esClient.IndexNames()
		if err != nil {
			return false, fmt.Errorf("failed to get index names (%s)", err.Error())
		}
		for _, index := range indices {
			if strings.Contains(index, "firewall") {
				firewallIndexName = index
				return true, nil
			}
		}
		return false, "firewall index is not created"
	}

	AssertEventually(t, assert, "firewall index not found", string("100ms"), string("100s"))
	return firewallIndexName
}

func verifyDiskMonitoring(ctx context.Context, t *testing.T, esClient elastic.ESClient, logger log.Logger) {
	query := es.NewMatchAllQuery()
	oldCount := 0
	assert := func() (bool, interface{}) {
		result, err := esClient.Search(ctx,
			elastic.GetIndex(globals.FwLogsObjects, ""), // index
			"",           // skip the index type
			query,        // query to be executed
			nil,          // no aggregation
			0,            // from
			10000,        // to
			"creationts", // sorting is required
			true)         // sort in desc order

		if err != nil {
			logger.Errorf("failed to query elasticsearch, err: %+v", err)
			return false, nil
		}

		if len(result.Hits.Hits) == 0 {
			return false, nil
		}

		if oldCount == 0 {
			oldCount = len(result.Hits.Hits)
			return false, nil
		}

		return oldCount > len(result.Hits.Hits), nil
	}

	AssertEventually(t, assert, "old objects are not getting deleted from elastic", string("1s"), string("200s"))
}

func verifyLastProcessedObjectKeys(ctx context.Context, t *testing.T, r resolver.Interface, logger log.Logger) {
	// Get the minio client for download the object
	var client objstore.Client
	var err error
	assert := func() (bool, interface{}) {
		client, err = objstore.NewClient(globals.DefaultTenant, fwlogsSystemMetaBucketName, r)
		return err == nil, client
	}
	AssertEventually(t, assert, "error in creating objstore client", string("1s"), string("200s"))

	data, err := utils.ExecuteWithRetry(func(ctx context.Context) (interface{}, error) {
		// PutObjectOfSize uploads object of "size' to object store
		rc, err := client.GetObject(ctx, lastProcessedKeysObjectName)
		if err != nil {
			return nil, err
		}

		// Read into a buffer
		buf := bytes.Buffer{}
		_, err = buf.ReadFrom(rc)
		if err != nil {
			return nil, err
		}

		// Unmarshall
		var data map[string]string
		err = json.Unmarshal(buf.Bytes(), &data)
		if err != nil {
			return nil, err
		}
		return data, nil
	}, time.Second, 200)

	AssertOk(t, err, "error in fetching lastProcessedObjectKeys from minio")
	lpk, ok := data.(map[string]string)
	Assert(t, ok, "error in casting lastProcessedObjectKeys")
	Assert(t, len(lpk) > 0, "corrupted data in lastProcessedObjectKeys object")
}

func verifyDebugRESTHandle(ctx context.Context, t *testing.T, r resolver.Interface, logger log.Logger) {
	time.Sleep(time.Second * 5)
	uri := strings.TrimSpace(fmt.Sprintf("http://127.0.0.1:%s", globals.SpyglassRESTPort) + "/debug/config")

	getHelper := func() map[string]interface{} {
		resp, err := http.Get(uri)
		AssertOk(t, err, "error is getting /debug/config")
		defer resp.Body.Close()
		body, err := ioutil.ReadAll(resp.Body)
		AssertOk(t, err, "error is reading response of /debug/config")
		config := map[string]interface{}{}
		err = json.Unmarshal(body, &config)
		AssertOk(t, err, "error is unmarshaling response of /debug/config")
		return config
	}

	// Veirfy that indexing is enabled
	config := getHelper()
	v, ok := config["disableFwlogIndex"]
	Assert(t, ok, "disableFwlogIndex is not present in /debug/config response, response:", config)
	Assert(t, v == true, "incorrect value of disableFwlogIndex, expected", true)

	// verify flowlog rate limit config
	fv, ok := config["flowlogsRLConfig"]
	Assert(t, ok, "flowlogsRLConfig is not present in /debug/config response, response:", config)
	rls, ok := fv.([]interface{})
	Assert(t, ok, "flowlogsRLConfig type is not []map[string]interface{}, response:", fv)
	Assert(t, len(rls) == 1, "atleast global rate limiter should be present, response:", fv)
	globalRl, ok := rls[0].(map[string]interface{})
	Assert(t, ok, "globalRl rate limiter type is not map[string]interface{}, response:", fv)
	Assert(t, globalRl["name"].(string) == "globalFlowlogsRateLimiter", "incorrect global rate limiter's name")
	Assert(t, globalRl["rate"].(float64) == float64(1000), "incorrect global rate limiter's rate")
	Assert(t, globalRl["burst"].(float64) == float64(50000), "incorrect global rate limiter's burst")

	reqBody, err := json.Marshal(map[string]interface{}{
		"disableFwlogIndex": false,
		"flowlogsRLConfig": []map[string]interface{}{
			{
				"name":  "globalFlowlogsRateLimiter",
				"rate":  100,
				"burst": 200,
			},
			{
				"name":  "default",
				"rate":  100,
				"burst": 200,
			},
		},
	})

	AssertOk(t, err, "error is marshaling POST request for /debug/config")
	req, err := http.NewRequest("POST", uri, bytes.NewBuffer(reqBody))
	AssertOk(t, err, "error is creating POST request for /debug/config")
	req.Header.Set("Content-Type", "application/json; charset=utf-8")
	client := &http.Client{}
	resp, err := client.Do(req)
	AssertOk(t, err, "error is POSTing request for /debug/config")
	defer resp.Body.Close()
	Assert(t, resp.Status == "200 OK", "incorrect response status")

	// Veirfy that indexing is disabled
	config = getHelper()
	v, ok = config["disableFwlogIndex"]
	Assert(t, ok, "disableFwlogIndex is not present in /debug/config response, response:", config)
	Assert(t, v == false, "incorrect value of disableFwlogIndex, expected", false)

	// verify flowlog rate limit config
	fv, ok = config["flowlogsRLConfig"]
	Assert(t, ok, "flowlogsRLConfig is not present in /debug/config response, response:", config)
	rls, ok = fv.([]interface{})
	Assert(t, ok, "flowlogsRLConfig type is not []map[string]interface{}, response:", fv)
	Assert(t, len(rls) == 2, "incorrect number of limiters, response:", fv)
	for _, temp := range rls {
		rl := temp.(map[string]interface{})
		if rl["name"].(string) == "globalFlowlogsRateLimiter" {
			Assert(t, rl["rate"].(float64) == float64(100), "incorrect global rate limiter's rate")
			Assert(t, rl["burst"].(float64) == float64(200), "incorrect global rate limiter's burst")
		} else {
			Assert(t, rl["rate"].(float64) == float64(100), "incorrect default tenant's rate limiter's rate")
			Assert(t, rl["burst"].(float64) == float64(200), "incorrect default tenant's rate limiter's burst")
		}
	}

	// Enable the indexing again
	reqBody, err = json.Marshal(map[string]interface{}{
		"disableFwlogIndex": true,
	})
	req, err = http.NewRequest("POST", uri, bytes.NewBuffer(reqBody))
	AssertOk(t, err, "error is creating POST request for /debug/config")
	req.Header.Set("Content-Type", "application/json; charset=utf-8")
	_, err = client.Do(req)
	AssertOk(t, err, "error is POSTing request for /debug/config")
}

func verifyVosRateLimitingAndEventsAtSpyglass(t *testing.T) {
	// Clear the old events
	mockEventsRecorder.ClearEvents()

	// No events should be raised when logs are coming at the given rate
	startFwLogGen(100, 600)
	eventFound := false
	for i := 0; i < 15; i++ {
		if len(mockEventsRecorder.GetEvents()) != 0 {
			eventFound = true
		}
		time.Sleep(time.Second)
	}
	Assert(t, eventFound == false, "rate limit event generated by Spyglass")

	// Now generate logs above rate so that it hits the burst limit in 5 seconds
	// The global rate limit has been updated to {100,200} in the last test case.
	// If we send 200logs/s, then we should see a rate limit event in 5th try.
	startFwLogGen(200, 1200)
	AssertEventually(t, func() (bool, interface{}) {
		for _, ev := range mockEventsRecorder.GetEvents() {
			if ev.EventType == eventtypes.FLOWLOGS_RATE_LIMITED.String() &&
				ev.Category == "system" &&
				ev.Severity == "critical" &&
				ev.Message != "" &&
				ev.ObjRef.(*cluster.Tenant).ObjectMeta.Tenant != "" {
				return true, ev
			}
		}
		return false, nil
	}, "failed to find flow logs rate limited event", "2s", "60s")
}

func verifyVosRateLimitingAndEventsAtDSC(ctx context.Context, t *testing.T, r resolver.Interface, logger log.Logger) {
	vosCtx, stopVos := context.WithCancel(ctx)
	defer func() {
		stopVos()
		os.Unsetenv("MINIO_API_REQUESTS_MAX")
		os.Unsetenv("MINIO_API_REQUESTS_DEADLINE")
	}()
	os.Setenv("MINIO_API_REQUESTS_MAX", "1")
	os.Setenv("MINIO_API_REQUESTS_DEADLINE", "1ns")
	setupVos(t, vosCtx, logger, "127.0.0.1")
	startFwLogGen(10000, 200000)
	AssertEventually(t, func() (bool, interface{}) {
		for _, ev := range mockEventsRecorder.GetEvents() {
			if ev.EventType == eventtypes.FLOWLOGS_REPORTING_ERROR.String() &&
				ev.Category == "system" &&
				ev.Severity == "warn" &&
				ev.Message != "" &&
				ev.ObjRef.(*cluster.DistributedServiceCard).ObjectMeta.Tenant != "" {
				return true, ev
			}
		}
		return false, nil
	}, "failed to find flow logs reporting error event", "2s", "60s")
}

func setupTmAgent(ctx context.Context, t *testing.T, r resolver.Interface) {
	ps, err := tmagentstate.NewTpAgent(ctx, strings.Split(types.DefaultAgentRestURL, ":")[1])
	AssertOk(t, err, "failed to create tp agent")
	Assert(t, ps != nil, "invalid policy state received")

	err = ps.FwlogInit(tmagentstate.FwlogIpcShm)
	AssertOk(t, err, "failed to init FwLog")

	err = ps.ObjStoreInit("1", r, time.Duration(1)*time.Second, nil)
	AssertOk(t, err, "objstore init failed")
}

func startFwLogGen(rate, total int) {
	gen := "../../../nic/agent/tests/fwloggen/fwloggen -rate " + fmt.Sprintf("%d", rate) + " -num " + fmt.Sprintf("%d", total)

	fmt.Println("fwloggen command", gen)
	go exec.Command("/bin/sh", "-c", gen).Output()
}
