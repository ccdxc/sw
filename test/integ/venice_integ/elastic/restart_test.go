// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package elastic

import (
	"context"
	"fmt"
	"runtime"
	"sync"
	"time"

	. "gopkg.in/check.v1"

	"github.com/pensando/sw/api"
	testutils "github.com/pensando/sw/test/utils"
	types "github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/elastic"
	mapper "github.com/pensando/sw/venice/utils/elastic/mapper"
	"github.com/pensando/sw/venice/utils/log"
	mockresolver "github.com/pensando/sw/venice/utils/resolver/mock"
	. "github.com/pensando/sw/venice/utils/testutils"
)

// make sure the elastic connection is intact during elasticsearch restarts
func (e *elasticsearchTestSuite) TestElasticsearchRestart(c *C) {
	elasticsearchName := c.TestName()
	elasticAddr, err := testutils.StartElasticsearch(elasticsearchName)
	Assert(c, err == nil, fmt.Sprintf("failed to start elasticsearch container, err: %v", err))
	defer testutils.StopElasticsearch(elasticsearchName)

	var esClient elastic.ESClient
	ctx := context.Background()

	// create mock resolver
	mr := mockresolver.New()

	si := &types.ServiceInstance{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceInstance",
		},
		ObjectMeta: api.ObjectMeta{
			Name: globals.ElasticSearch,
		},
		Service: globals.ElasticSearch,
		URL:     elasticAddr,
	}

	// add mock elastic service to mock resolver
	mr.AddServiceInstance(si)

	logger := log.GetNewLogger(&log.Config{
		Module:      c.TestName(),
		Format:      log.LogFmt,
		Filter:      log.AllowAllFilter,
		Debug:       true,
		CtxSelector: log.ContextAll,
		LogToStdout: true,
		LogToFile:   false,
	})

	// create elastic client
	AssertEventually(c,
		func() (bool, interface{}) {
			esClient, err = elastic.NewClient("", mr, logger)
			if err != nil {
				log.Errorf("error creating client: %v", err)
				return false, nil
			}

			log.Infof("created elastic client")
			return true, nil
		}, "failed to create elastic client", "10ms", "2m")

	// check elasticsearch cluster health
	AssertEventually(c,
		func() (bool, interface{}) {
			return esClient.IsClusterHealthy(ctx)
		}, "failed to get elasticsearch cluster health")

	mapping, err := mapper.ElasticMapper(eventObj, indexType, mapper.WithShardCount(1), mapper.WithReplicaCount(0))
	AssertOk(c, err, "Failed to generate elastic mapping for events")

	// Generate JSON string for the mapping
	configs, err := mapping.JSONString()
	AssertOk(c, err, "Failed to get JSONString from elastic mapper")

	runtime.GOMAXPROCS(runtime.NumCPU())

	errs := make(chan error, 10)
	defer close(errs)

	prevResetCount := 0
	for re := 0; re < 5; re++ {
		var wg sync.WaitGroup
		wg.Add(2)

		// restart elasticsearch server in intervals
		go func() {
			defer wg.Done()
			testutils.StopElasticsearch(elasticsearchName)
			mr.DeleteServiceInstance(si)

			time.Sleep(10 * time.Millisecond)

			elasticAddr, err = testutils.StartElasticsearch(elasticsearchName)
			if err != nil {
				errs <- fmt.Errorf("failed to start elasticsearch, err:%v", err)
				return
			}

			// update elastic address
			si.URL = elasticAddr
			mr.AddServiceInstance(si)
		}()

		go func() {
			defer wg.Done()
			// use the above created client to perform elastic operations
			for i := 0; i < 10; i++ {
				// create index
				if err := esClient.CreateIndex(ctx, fmt.Sprintf("%s%d", indexName, i), configs); err != nil &&
					!elastic.IsIndexExists(err) && !elastic.IsConnRefused(err) {
					errs <- fmt.Errorf("failed to create index, err: %v", err.Error())
					return
				}

				// index a document
				if err := esClient.Index(ctx, indexName, indexType, fmt.Sprintf("test%d", i), `{}`); err != nil &&
					!elastic.IsIndexNotExists(err) && !elastic.IsConnRefused(err) {
					errs <- fmt.Errorf("failed to perform index operation, err: %v", err.Error())
					return
				}

				// bulk index documents
				if _, err := esClient.Bulk(ctx, []*elastic.BulkRequest{
					&elastic.BulkRequest{RequestType: "index", Index: indexName, IndexType: indexType, Obj: "{}", ID: "dummy1"},
					&elastic.BulkRequest{RequestType: "index", Index: indexName, IndexType: indexType, Obj: "{}", ID: "dummy2"},
				}); err != nil && !elastic.IsConnRefused(err) {
					errs <- fmt.Errorf("failed to perform bulk operation, err: %v", err.Error())
					return
				}
			}
			// make the restart happened
			resetCount := esClient.GetResetCount()
			if resetCount < prevResetCount {
				errs <- fmt.Errorf("expected client to be reset")
				return
			}
			prevResetCount = resetCount
		}()

		wg.Wait()

		// non-blocking receive on the error channel
		select {
		case er := <-errs:
			log.Fatalf("failed with err: %v", er)
		default:
			break
		}
	}
}
