package indexer_test

import (
	"context"
	"flag"
	"fmt"
	"os/exec"
	"strings"
	"sync"
	"testing"
	"time"

	dctypes "github.com/docker/docker/api/types"
	dc "github.com/docker/docker/client"

	"github.com/pensando/sw/api"
	servicetypes "github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/spyglass/cache"
	"github.com/pensando/sw/venice/spyglass/indexer"
	"github.com/pensando/sw/venice/utils/elastic"
	esmock "github.com/pensando/sw/venice/utils/elastic/mock/server"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/resolver/mock"
	. "github.com/pensando/sw/venice/utils/testutils"
	vospkg "github.com/pensando/sw/venice/vos/pkg"
)

const realEs = true

var apiServerAddr = flag.String("api-server-addr", globals.APIServer, "ApiServer gRPC endpoint")

func IndexerSetTest(i *indexer.Indexer) {
	i.VosTest = true
	i.VosTestGrpcURL = "127.0.0.1:9051"
	i.WatchAPIServer = false
}

// TestAppendOnlyWriter tests the writer used for pushing fwlogs to elastic
func SkipTestAppendOnlyWriter(t *testing.T) {
	wg := sync.WaitGroup{}
	url := ""
	logger := log.GetNewLogger(log.GetDefaultConfig("appendonlywriter_test"))
	if realEs {
		setupRealEs(t)
		defer stopRealEs(t)
		url = "127.0.0.1:9200"
	} else {
		es := esmock.NewElasticServer(logger)
		es.Start()
		url = es.URL
	}

	r := mock.New()
	err := r.AddServiceInstance(&servicetypes.ServiceInstance{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceInstance",
		},
		ObjectMeta: api.ObjectMeta{
			Name: globals.ElasticSearch,
		},
		Service: globals.ElasticSearch,
		URL:     url,
	})
	AssertOk(t, err, "failed to add elasticsearch sercvice")

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

	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	esClient, err := elastic.NewClient(url, r, logger)
	AssertOk(t, err, "failed to create elastic client")

	setupVos(t, ctx, logger, "127.0.0.1", &wg)
	setupIndexer(t, ctx, r, logger, &wg)

	indexName := verifyAndReturnFirewallIndexName(t, esClient)
	fmt.Println("found index", indexName)

	wg.Wait()
}

func setupIndexer(t *testing.T,
	ctx context.Context, resolver resolver.Interface, logger log.Logger, wg *sync.WaitGroup) {
	// Create the indexer
	wg.Add(1)
	go func() {
		defer wg.Done()
		idxer, err := indexer.NewIndexer(ctx,
			*apiServerAddr,
			resolver,
			cache.NewCache(logger),
			logger,
			8,
			1,
			IndexerSetTest)

		AssertOk(t, err, "failed to add indexer")
		Assert(t, idxer != nil, "failed to create indexer")

		err = idxer.Start()
		AssertOk(t, err, "failed to start indexer")
	}()
}

func setupRealEs(t *testing.T) {
	esCmdStr :=
		"docker run -p 9200:9200 -p 9300:9300 -e \"discovery.type=single-node\" docker.elastic.co/elasticsearch/elasticsearch:6.3.2"
	go exec.Command("/bin/sh", "-c", esCmdStr).Output()

	cli, err := dc.NewEnvClient()
	AssertOk(t, err, "failed to create docker client")

	esConatinerID := ""
loop:
	for {
		select {
		case <-time.After(time.Second * 1):
			containers, err := cli.ContainerList(context.Background(), dctypes.ContainerListOptions{})
			AssertOk(t, err, "failed to list docker containers")

			if len(containers) == 0 {
				continue
			}

			for _, container := range containers {
				fmt.Printf("%s %s\n", container.ID[:10], container.Image)
				if strings.Contains(container.Image, "elastic") {
					esConatinerID = container.ID[:10]
					break loop
				}
			}
		}
	}

	kibanaCmdStr :=
		"docker run --link " + esConatinerID + ":elasticsearch -p 5601:5601 docker.elastic.co/kibana/kibana:6.3.0"
	go exec.Command("/bin/sh", "-c", kibanaCmdStr).Output()
}

func stopRealEs(t *testing.T) {
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

func setupVos(t *testing.T, ctx context.Context, logger log.Logger, url string, wg *sync.WaitGroup) {
	wg.Add(1)
	go func() {
		defer wg.Done()
		args := []string{globals.Vos, "server", "--address", fmt.Sprintf("%s:%s", url, globals.VosMinioPort), "/disk1"}
		err := vospkg.New(ctx, false, args, url)
		AssertOk(t, err, "error in initiating Vos")
	}()
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
