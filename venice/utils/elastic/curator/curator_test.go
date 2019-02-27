package curator

import (
	"context"
	"sync"
	"testing"
	"time"

	"github.com/pensando/sw/venice/utils/elastic"
	"github.com/pensando/sw/venice/utils/elastic/mock/server"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

var (
	logConfig = log.GetDefaultConfig("curator")
	logger    = log.SetConfig(logConfig)
)

func setup(t *testing.T) (*mock.ElasticServer, elastic.ESClient, log.Logger) {
	tLogger := logger.WithContext("t_name", t.Name())
	mElasticServer := mock.NewElasticServer(tLogger.WithContext("submodule", "elasticsearch-mock-server"))
	mElasticServer.Start()

	esClient, err := elastic.NewClient(mElasticServer.GetElasticURL(), nil, tLogger)
	AssertOk(t, err, "failed to create elastic client")

	return mElasticServer, esClient, tLogger
}

func TestCurator(t *testing.T) {
	mElasticServer, esClient, tLogger := setup(t)
	defer mElasticServer.Stop()
	defer esClient.Close()

	curator, err := NewCurator(nil, nil, tLogger)
	Assert(t, err != nil, "expected failure, but curator created")
	Assert(t, curator == nil, "expected nil curator")

	curator, err = NewCurator(esClient, nil, tLogger)
	AssertOk(t, err, "failed to create curator")
	defer curator.Stop()
	curator.Start()
	curator.Start() // start a curator that is already running

	testCurator(t, curator, esClient) // test retention using curator

	// test curator with restarts
	for i := 0; i < 3; i++ {
		curator.Stop()
		curator.Start()
		testCurator(t, curator, esClient)
	}
}

func testCurator(t *testing.T, curator Interface, esClient elastic.ESClient) {
	var wg sync.WaitGroup

	// create elastic indices and scan using curator
	indices := []string{"test.index.aa.1", "test.index.aa.2"}
	for _, index := range indices {
		Assert(t, esClient.CreateIndex(context.Background(), index, "") == nil,
			"failed to create index: %s", index)

		// scan the indices using curator service
		curator.Scan(&Config{
			IndexName:       index,
			RetentionPeriod: 1 * time.Second,
			ScanInterval:    500 * time.Millisecond,
		})

		wg.Add(1)
		go func(index string) {
			defer wg.Done()

			// ensure the indices are gone after the retention period
			AssertEventually(t,
				func() (bool, interface{}) {
					var err error
					var resp map[string]elastic.SettingsResponse
					if resp, err = esClient.GetIndexSettings(context.Background(), []string{index}); err != nil {
						log.Errorf("failed to get index settings, err: %v", err)
						return false, err
					}

					// Verify there are no matching older indices found
					if len(resp) != 0 {
						log.Errorf("indices not deleted yet, found old indices, resp: %+v", resp)
						return false, err
					}
					return true, nil

				}, "failed to get indices", "500ms", "10s")
		}(index)
	}

	wg.Wait()
}
