// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package elastic

import (
	"context"
	"crypto/x509"
	"encoding/json"
	"flag"
	"fmt"
	"strings"
	"testing"
	"time"

	"github.com/gogo/protobuf/types"
	es "github.com/olivere/elastic"
	uuid "github.com/satori/go.uuid"
	. "gopkg.in/check.v1"

	"github.com/pensando/sw/api"
	evtsapi "github.com/pensando/sw/api/generated/events"
	testutils "github.com/pensando/sw/test/utils"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/certs"
	"github.com/pensando/sw/venice/utils/elastic"
	mapper "github.com/pensando/sw/venice/utils/elastic/mapper"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

// sub-objects (e.g. ObjectMeta) needs to be nested only if they are slices. If not, it need
// not be nested, as a result all the fields will be flattened thus, searching becomes easy.
// e.g. `ObjectMeta.Namespace` can be queried directly without having to go through nested query.

var (
	tenantName         = "ford"
	indexName          = elastic.GetIndex(globals.Events, tenantName)
	indexType          = elastic.GetDocType(globals.Events)
	from               = int32(0)
	maxResults         = int32(10)
	infraNamespace     = "infra"
	eventsTemplateName = elastic.GetTemplateName(globals.Events)
	sortByField        = ""
	sortAsc            = true

	// test command line args
	preserveEvents = flag.Bool("preserve-events", false, "Preserve events?")
	skipESSetup    = flag.Bool("skip-es-setup", false, "Skip elasticsearch setup")
	numEvents      = flag.Int("num-events", 5000, "Number of events to be indexed")
	eventType      = flag.String("event-type", "NodeJoined", "Event type of the events")
	severity       = flag.String("severity", "INFO", "Severity of the events")

	cTime           = time.Now()
	creationTime, _ = types.TimestampProto(cTime)
	event           = evtsapi.Event{}

	logConfig = log.GetDefaultConfig("elastic-client-test")

	// generate elastic mapping and settings for event
	eventObj = evtsapi.Event{
		EventAttributes: evtsapi.EventAttributes{
			// Need to make sure pointer fields are valid to
			// generate right mappings using reflect
			ObjectRef: &api.ObjectRef{},
			Source:    &evtsapi.EventSource{},
		},
	}

	elasticAddr = ""
	elasticDir  = ""
)

// Hook up gocheck into the "go test" runner.
func Test(t *testing.T) {
	var _ = Suite(&elasticsearchTestSuite{})
	TestingT(t)
}

func (e *elasticsearchTestSuite) TestElastic(c *C) {
	var esClient elastic.ESClient
	var err error

	elasticsearchName := c.TestName()

	//context must be passed for each elastic call
	ctx := context.Background()

	signer, _, trustRoots, err := testutils.GetCAKit()
	Assert(c, err == nil, "Error getting CA artifacts")

	setup(c, elasticsearchName, signer, trustRoots)
	defer teardown(ctx, esClient, c, elasticsearchName)

	// Create a client
	AssertEventually(c,
		func() (bool, interface{}) {
			// CreateElasticClient also checks cluster health
			esClient, err = testutils.CreateElasticClient(elasticAddr, nil, log.GetNewLogger(logConfig), signer, trustRoots)
			if err != nil {
				log.Errorf("error creating client: %v", err)
				return false, nil
			}
			return true, nil
		}, "failed to create elastic client", "20ms", "2m")

	// check elasticsearch cluster health
	AssertEventually(c,
		func() (bool, interface{}) {
			healthy, er := esClient.IsClusterHealthy(ctx)
			return healthy, er
		}, "failed to get elasticsearch cluster health")

	// Getting the ES version number is quite common, so there's a shortcut
	esversion, err := esClient.Version()
	if err != nil {
		log.Fatal("failed to get elasticsearch version")
	}
	log.Infof("elasticsearch version %s", esversion)

	// get a mapping with index pattern
	mapping, err := mapper.ElasticMapper(eventObj, indexType, mapper.WithShardCount(1), mapper.WithReplicaCount(0),
		mapper.WithIndexPatterns(fmt.Sprintf("*.%s.*", indexType)))
	AssertOk(c, err, "Failed to generate elastic mapping for events")

	// Generate JSON string for the mapping
	configs, err := mapping.JSONString()
	AssertOk(c, err, "Failed to get JSONString from elastic mapper")

	// create events template; once the template is created, elasticsearch will automatically apply the
	// properties for new indices that're matching the template's index pattern.
	if err := esClient.CreateIndexTemplate(ctx, eventsTemplateName, configs); err != nil {
		log.Fatalf("failed to create events template: %v", err)
	}

	// index events
	indexEventsSequential(ctx, esClient, c)
	indexEventsBulk(ctx, esClient, c)
	// index a nil document
	Assert(c, esClient.Index(ctx, indexName, indexType, "dummy-id", nil) != nil,
		"Cannot index a nil document; expected failure")

	// search events
	searchEvents(ctx, esClient, c)

	// perform bulk request with update and delete operations
	updateEventsThroughBulk(ctx, esClient, c)

	// get index settings test
	AssertEventually(c,
		func() (bool, interface{}) {
			indices := []string{"venice.external.ford.events.*"}
			var err error
			var resp map[string]elastic.SettingsResponse
			if resp, err = esClient.GetIndexSettings(ctx, indices); err != nil {
				log.Errorf("Failed to get index settings, err: %v", err)
				return false, err
			}
			for index, settings := range resp {
				if strings.Contains(index, "venice.external.ford.events") == false {
					log.Errorf("Index name mismatch, exp: %s actual: %s",
						"venice.external.ford.events", index)
					return false, err
				}
				if strings.Contains(settings.ProvidedName, "venice.external.ford.events") == false {
					log.Errorf("Index provided name mismatch, exp: %s actual: %s",
						"venice.external.ford.events", index)
					return false, err
				}
			}
			return true, nil

		}, "failed to get indices", "20ms", "2s")

	// test search shards and get nodes info APIs
	shardsInfo, err := esClient.GetSearchShards(ctx, []string{indexName})
	AssertOk(c, err, "failed to get search shards info, err: %v", err)
	for _, shards := range shardsInfo.Shards {
		for _, shard := range shards {
			nodesInfo, err := esClient.GetNodesInfo(ctx, []string{shard.Node})
			AssertOk(c, err, "failed to get nodes info, err: %v", err)
			Assert(c, len(nodesInfo.Nodes) > 0, "failed to get nodes info")
		}
	}

	// delete index template
	if err := esClient.DeleteIndexTemplate(ctx, eventsTemplateName); err != nil {
		log.Fatalf("failed to delete events template: %v", err)
	}
}

// updateEventsThroughBulk tests the bulk update/delete operation
func updateEventsThroughBulk(ctx context.Context, client elastic.ESClient, c *C) {
	log.Infof("updating docs on the index: %v", indexName)

	totalEvents := 10
	requests := make([]*elastic.BulkRequest, totalEvents)

	// add requests for the bulk operation
	for i := 0; i < totalEvents; i++ {
		event.ObjectMeta.UUID = uuid.NewV4().String()
		event.ObjectMeta.Name = event.ObjectMeta.UUID

		// change the namespace from `default` to infraNamespace
		event.ObjectMeta.Namespace = infraNamespace

		event.ObjectMeta.CreationTime.Timestamp = *creationTime
		ts, _ := types.TimestampProto(time.Now())
		event.ObjectMeta.ModTime.Timestamp = *ts

		event.EventAttributes.Message = "test - index operation"

		requests[i] = &elastic.BulkRequest{
			RequestType: elastic.Index,
			Index:       indexName,
			IndexType:   indexType,
			ID:          event.ObjectMeta.UUID,
			Obj:         event,
		}
	}

	// perform bulk index
	bulkResp, err := client.Bulk(ctx, requests)
	if err != nil {
		log.Fatalf("failed to perform bulk indexing, err: %v", err)
	}

	// check the number of succeeded operation
	AssertEquals(c, totalEvents, len(bulkResp.Succeeded()),
		fmt.Sprintf("requests succeeded - expected: %v, got: %v", totalEvents, len(bulkResp.Succeeded())))

	// make sure there are 0 failed operation
	AssertEquals(c, 0, len(bulkResp.Failed()),
		fmt.Sprintf("requests failed - expected: %v, got: %v", 0, len(bulkResp.Failed())))

	// search the indexed documents
	AssertEventually(c,
		func() (bool, interface{}) {
			var result *es.SearchResult
			query := es.NewBoolQuery().Must(
				es.NewMatchPhraseQuery("message", "test - index operation"),
				es.NewMatchQuery("meta.creation-time", cTime))
			result, err = client.Search(ctx, indexName, indexType, query, nil, from, maxResults, sortByField, sortAsc)
			if err != nil {
				log.Fatalf("failed to search events for query: %v, err:%v", query, err)
				return false, nil
			}

			if int(result.TotalHits()) == totalEvents {
				return true, nil
			}

			return false, nil
		}, "failed to get search result", "20ms", "2s")

	// update events half events
	for i := 0; i < len(requests); i += 2 {
		obj := requests[i].Obj.(evtsapi.Event)
		obj.EventAttributes.Message = "test - update operation"
		requests[i].Obj = obj
		requests[i].RequestType = elastic.Update
	}

	// delete the remaining events
	for i := 1; i < len(requests); i += 2 {
		requests[i].RequestType = elastic.Delete
	}

	// perform bulk update and delete
	bulkResp, err = client.Bulk(ctx, requests)
	if err != nil {
		log.Fatalf("failed to perform bulk update, err: %v", err)
	}

	// check the number of succeeded operation
	AssertEquals(c, totalEvents, len(bulkResp.Succeeded()),
		fmt.Sprintf("requests succeeded - expected: %v, got: %v", totalEvents, len(bulkResp.Succeeded())))

	// make sure there are 0 failed operation
	AssertEquals(c, 0, len(bulkResp.Failed()),
		fmt.Sprintf("requests failed - expected: %v, got: %v", 0, len(bulkResp.Failed())))

	// make sure there are 5 updated operation
	AssertEquals(c, totalEvents/2, len(bulkResp.Updated()),
		fmt.Sprintf("requests failed - expected: %v, got: %v", totalEvents/2, len(bulkResp.Updated())))

	// make sure there are 5 deleted operation
	AssertEquals(c, totalEvents/2, len(bulkResp.Deleted()),
		fmt.Sprintf("requests failed - expected: %v, got: %v", totalEvents/2, len(bulkResp.Deleted())))

	// ensure the updates have taken effect
	// there should be no event with "test - index operation" message
	AssertEventually(c,
		func() (bool, interface{}) {
			var result *es.SearchResult
			query := es.NewBoolQuery().Must(
				es.NewMatchPhraseQuery("message", "test - index operation"),
				es.NewMatchQuery("meta.creation-time", cTime))
			result, err = client.Search(ctx, indexName, indexType, query, nil, from, maxResults, sortByField, sortAsc)
			if err != nil {
				log.Fatalf("failed to search events for query: %v, err:%v", query, err)
				return false, nil
			}

			if int(result.TotalHits()) == 0 {
				return true, nil
			}

			return false, fmt.Sprintf("expected: 0, got: %v", result.TotalHits())
		}, "failed to get search result", "20ms", "2s")

	// query for the updated message
	AssertEventually(c,
		func() (bool, interface{}) {
			var result *es.SearchResult
			query := es.NewBoolQuery().Must(
				es.NewMatchPhraseQuery("message", "test - update operation"),
				es.NewMatchQuery("meta.creation-time", cTime))
			result, err = client.Search(ctx, indexName, indexType, query, nil, from, maxResults, sortByField, sortAsc)
			if err != nil {
				log.Fatalf("failed to search events for query: %v, err:%v", query, err)
				return false, nil
			}

			if int(result.TotalHits()) == totalEvents/2 {
				return true, nil
			}

			return false, fmt.Sprintf("expected: %v, got: %v", totalEvents/2, result.TotalHits())
		}, "failed to get search result", "20ms", "2s")
}

// searchEvents runs a couple of queries on the elastic cluster
func searchEvents(ctx context.Context, client elastic.ESClient, c *C) {
	log.Infof("performing search queries on the index: %v", indexName)
	Assert(c, client.FlushIndex(ctx, indexName) == nil,
		"Flush operation failed, cannot perform search")

	// Query 1: date range; search for events with events.ObjectMeta.ModTime falls in last 60 seconds
	// this is a query on the nested object events.ObjectMeta.ModTime
	now := time.Now()
	query1 := es.NewRangeQuery("meta.mod-time").Gte(now.Add(-30 * time.Second)).Lte(now)
	result, err := client.Search(ctx, indexName, indexType, query1, nil, from, maxResults, sortByField, sortAsc)
	if err != nil {
		log.Fatalf("failed to search events for query: %v, err:%v", query1, err)
	}

	src, _ := query1.Source()
	jsonQuery, _ := json.Marshal(src)
	Assert(c, result.TotalHits() > 0,
		fmt.Sprintf("Something wrong, atleast few events should match the query: %s", jsonQuery))

	// Query 2: term query; look for events with severity == event.Severity
	// term queries are used for keyword searches (exact values)
	// whereas, match queries are full_text searches
	query2 := es.NewTermQuery("severity.keyword", event.Severity)
	result, err = client.Search(ctx, indexName, indexType, query2, nil, from, maxResults, sortByField, sortAsc)
	if err != nil {
		log.Fatalf("failed to search events for query: %v, err:%v", query2, err)
	}

	src, _ = query2.Source()
	jsonQuery, _ = json.Marshal(src)
	Assert(c, result.TotalHits() > 0,
		fmt.Sprintf("Something wrong, atleast few events should match the query: %s", jsonQuery))

	// Query 3: match all the events;
	// creationTime is the same for all the events indexed during this run.
	query3 := es.NewMatchQuery("meta.creation-time", cTime)
	result, err = client.Search(ctx, indexName, indexType, query3, nil, from, maxResults, sortByField, sortAsc)
	if err != nil {
		log.Fatalf("failed to search events for query: %v, err:%v", query3, err)
	}

	src, _ = query3.Source()
	jsonQuery, _ = json.Marshal(src)
	Assert(c, result.TotalHits() == int64(2*(*numEvents)), // sequential + bulk indexing (2*numEvents)
		fmt.Sprintf("Something wrong, all the events should match the query: %s", jsonQuery))

	// Query 4: combine queries 1 & 2;
	// look for severity == event.Severity events within the given 30 seconds
	query4 := es.NewBoolQuery().Must(query1, query2)
	result, err = client.Search(ctx, indexName, indexType, query4, nil, from, maxResults, sortByField, sortAsc)
	if err != nil {
		log.Fatalf("failed to search events for query: %v, err:%v", query4, err)
	}

	src, _ = query4.Source()
	jsonQuery, _ = json.Marshal(src)
	Assert(c, result.TotalHits() > 0,
		fmt.Sprintf("Something wrong, atleast few events should match the query: %s", jsonQuery))

	// query 5: combine 2 & 3;
	// search for events with severity == event.Severity with the creation time of this run
	query5 := es.NewBoolQuery().Must(query2, query3)
	result, err = client.Search(ctx, indexName, indexType, query5, nil, from, maxResults, sortByField, sortAsc)
	if err != nil {
		log.Fatalf("failed to search events for query: %v, err:%v", query5, err)
	}

	src, _ = query5.Source()
	jsonQuery, _ = json.Marshal(src)
	Assert(c, result.TotalHits() == int64(2*(*numEvents)),
		fmt.Sprintf("Something wrong, all the events should match the query: %s", jsonQuery))

	// full-text search queries

	// query 6: find all the event that has string defined in `infraNamespace`;
	// atleast half the total events should match this query which is `numEvents`
	query6 := es.NewQueryStringQuery(infraNamespace)
	result, err = client.Search(ctx, indexName, indexType, query6, nil, from, maxResults, sortByField, sortAsc)
	if err != nil {
		log.Fatalf("failed to search events for query: %v, err:%v", query6, err)
	}

	src, _ = query6.Source()
	jsonQuery, _ = json.Marshal(src)
	Assert(c, result.TotalHits() >= int64(*numEvents),
		fmt.Sprintf("Something wrong, half the total events indexed should match the query: %s", jsonQuery))

	// query 7: find events by `ObjectMeta.Namespace: string`
	// atleast half the total events should match this query which is `numEvents`
	query7 := es.NewQueryStringQuery(fmt.Sprintf("%s:%s", "meta.namespace", infraNamespace))
	result, err = client.Search(ctx, indexName, indexType, query7, nil, from, maxResults, sortByField, sortAsc)
	if err != nil {
		log.Fatalf("failed to search events for query: %v, err:%v", query7, err)
	}

	src, _ = query7.Source()
	jsonQuery, _ = json.Marshal(src)
	Assert(c, result.TotalHits() >= int64(*numEvents),
		fmt.Sprintf("Something wrong, half the total events indexed should match the query: %s", jsonQuery))

	// query 8: find all the event that has string "honda"
	query8 := es.NewQueryStringQuery(string("honda"))
	result, err = client.Search(ctx, indexName, indexType, query8, nil, from, maxResults, sortByField, sortAsc)
	if err != nil {
		log.Fatalf("failed to search events for query: %v, err:%v", query8, err)
	}

	src, _ = query8.Source()
	jsonQuery, _ = json.Marshal(src)
	Assert(c, result.TotalHits() == 0,
		fmt.Sprintf("Something wrong, none of the event should match the query: %s", jsonQuery))
}

// indexEventsBulk performs the bulk indexing
func indexEventsBulk(ctx context.Context, client elastic.ESClient, c *C) {
	requests := make([]*elastic.BulkRequest, *numEvents)
	// add requests for the bulk operation
	for i := 0; i < *numEvents; i++ {
		event.ObjectMeta.UUID = uuid.NewV4().String()
		event.ObjectMeta.Name = event.ObjectMeta.UUID

		// change the namespace from `default` to infraNamespace
		event.ObjectMeta.Namespace = infraNamespace

		event.ObjectMeta.CreationTime.Timestamp = *creationTime
		ts, _ := types.TimestampProto(time.Now())
		event.ObjectMeta.ModTime.Timestamp = *ts

		requests[i] = &elastic.BulkRequest{
			RequestType: elastic.Index,
			Index:       indexName,
			IndexType:   indexType,
			ID:          event.ObjectMeta.UUID,
			Obj:         event,
		}
	}

	start := time.Now()

	// perform bulk operation
	bulkResp, err := client.Bulk(ctx, requests)
	if err != nil {
		log.Fatalf("failed to perform bulk indexing, err: %v", err)
	}

	// check the number of succeeded operation
	AssertEquals(c, *numEvents, len(bulkResp.Succeeded()),
		fmt.Sprintf("requests succeeded - expected: %v, got: %v", *numEvents, len(bulkResp.Succeeded())))

	// make sure there are 0 failed operation
	AssertEquals(c, 0, len(bulkResp.Failed()),
		fmt.Sprintf("requests failed - expected: %v, got: %v", 0, len(bulkResp.Failed())))

	log.Infof("total time taken for bulk indexing: %v\n", time.Since(start))
}

// indexEventsSequential indexs events in a sequential manner
func indexEventsSequential(ctx context.Context, client elastic.ESClient, c *C) {
	start := time.Now()

	// index events one by one
	for i := 1; i <= *numEvents; i++ {
		event.ObjectMeta.UUID = uuid.NewV4().String()
		event.ObjectMeta.Name = event.ObjectMeta.UUID

		event.ObjectMeta.CreationTime.Timestamp = *creationTime
		transStart := time.Now()
		ts, _ := types.TimestampProto(transStart)
		event.ObjectMeta.ModTime.Timestamp = *ts

		// log failure and continue
		if err := client.Index(ctx, indexName, indexType, event.ObjectMeta.UUID, event); err != nil {
			log.Infof("failed to index event %s err:%v", event.ObjectMeta.Name, err)
			continue
		}
	}

	log.Infof("total time taken for sequential indexing: %v\n", time.Since(start))
}

// teardown elasticsearch cluster
func teardown(ctx context.Context, client elastic.ESClient, c *C, name string) {
	// arg -preserve-events will prevent deleting elasticsearch container
	if *preserveEvents {
		return
	}

	if client != nil {
		Assert(c, client.DeleteIndex(ctx, indexName) == nil,
			fmt.Sprintf("failed to delete index: %v", indexName))
	}

	err := testutils.StopElasticsearch(name, elasticDir)
	Assert(c, err == nil || (err != nil && strings.Contains(err.Error(), "is already in progress")),
		fmt.Sprintf("failed to stop elasticsearch container, err: %v", err))
}

// setup spins a elasticsearch cluster if requested
func setup(c *C, name string, signer certs.CSRSigner, trustRoots []*x509.Certificate) {
	// construct event object to be indexed
	constructEvent()

	var err error

	// skip the setup
	if *skipESSetup {
		elasticAddr, err = testutils.GetElasticsearchAddress(name)
		Assert(c, err == nil, fmt.Sprintf("failed to get elasticsearch addr, err: %v", err))
		return
	}

	// spin up a single node elasticsearch with the given name; running separate elasticsearch for
	// each test helps to run the tests in parallel.
	elasticAddr, elasticDir, err = testutils.StartElasticsearch(name, "", signer, trustRoots)
	Assert(c, err == nil, fmt.Sprintf("failed to start elasticsearch container, err: %v", err))
}

// constructEvent helper function to create event object
func constructEvent() {
	event.TypeMeta = api.TypeMeta{
		Kind: "Event",
	}
	event.ObjectMeta = api.ObjectMeta{
		Tenant:          "default",
		Namespace:       "default",
		ResourceVersion: "2",
		UUID:            "adfadf-sadfasdf-adfsdf-sadf",
	}
	event.EventAttributes = evtsapi.EventAttributes{
		Severity: *severity,
		Type:     *eventType,
		Message:  fmt.Sprintf("%s - tenant[default]", *eventType),
		Count:    1,
		Source: &evtsapi.EventSource{
			Component: "CMD",
			NodeName:  "test",
		},
	}
}
