package indexer_test

import (
	"context"
	"flag"
	"fmt"
	"os/exec"
	"strings"
	"testing"

	dctypes "github.com/docker/docker/api/types"
	dc "github.com/docker/docker/client"
	es "github.com/olivere/elastic"

	"github.com/pensando/sw/api"
	testelastic "github.com/pensando/sw/test/utils"
	servicetypes "github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/spyglass/cache"
	"github.com/pensando/sw/venice/spyglass/indexer"
	"github.com/pensando/sw/venice/utils/elastic"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/resolver/mock"
	. "github.com/pensando/sw/venice/utils/testutils"
	vospkg "github.com/pensando/sw/venice/vos/pkg"
)

var apiServerAddr = flag.String("api-server-addr", globals.APIServer, "ApiServer gRPC endpoint")

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

	// TestVerifyFirewallIndexName verifies that the firewall index is created
	t.Run("TestVerifyFirewallIndexName", func(t *testing.T) {
		verifyAndReturnFirewallIndexName(t, esClient)
	})

	t.Run("TestVerifyDiskMonitoring", func(t *testing.T) {
		verifyDiskMonitoring(ctx, t, esClient, logger)
	})

	done := make(chan bool)
	<-done
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
			8,
			1,
			indexer.WithElasticClient(es),
			indexer.WithDisableAPIServerWatcher(),
			indexer.WithNumVosObjectsToDelete(1))

		AssertOk(t, err, "failed to add indexer")
		Assert(t, idxer != nil, "failed to create indexer")

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
			vospkg.WithBucketDiskThresholds(map[string]float64{"/disk1/fwlogs.fwlogs": 0.00001}))
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
	query := es.NewMatchQuery("bucket", "fwlogs")
	oldCount := 0
	assert := func() (bool, interface{}) {
		result, err := esClient.Search(ctx,
			elastic.GetIndex(globals.FwLogsObjects, ""), // index
			"",           // skip the index type
			query,        // query to be executed
			nil,          // no aggregation
			0,            // from
			10000,        // to
			"creationTs", // sorting is required
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
