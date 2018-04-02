// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package veniceinteg

import (
	"context"
	"encoding/json"
	"flag"
	"fmt"
	"os/exec"
	"strings"
	"testing"
	"time"

	"github.com/gogo/protobuf/types"
	uuid "github.com/satori/go.uuid"
	es "gopkg.in/olivere/elastic.v5"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/elastic"
	mapper "github.com/pensando/sw/venice/utils/elastic/mapper"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

const (
	elasticURL   = "http://127.0.0.1:9200"
	registryURL  = "registry.test.pensando.io:5000"
	elasticImage = "elasticsearch/elasticsearch:6.2.2"
)

// sub-objects (e.g. ObjectMeta) needs to be nested only if they are slices. If not, it need
// not be nested, as a result all the fields will be flattened thus, searching becomes easy.
// e.g. `ObjectMeta.Namespace` can be queried directly without having to go through nested query.

var (
	tenantName     = "ford"
	indexName      = elastic.GetIndex(globals.Events, tenantName)
	indexType      = elastic.GetDocType(globals.Events)
	from           = int32(0)
	maxResults     = int32(10)
	infraNamespace = "infra"

	// test command line args
	preserveEvents = flag.Bool("preserve-events", false, "Preserve events?")
	skipESSetup    = flag.Bool("skip-es-setup", false, "Skip elasticsearc setup?")
	numEvents      = flag.Int("num-events", 5000, "Number of events to be indexed")
	eventType      = flag.String("event-type", "NodeJoined", "Event type of the events")
	severity       = flag.String("severity", "INFO", "Severity of the events")

	cTime           = time.Now()
	creationTime, _ = types.TimestampProto(cTime)
	event           = events.Event{}
)

func TestElastic(t *testing.T) {
	var esClient elastic.ESClient
	var err error

	//context must be passed for each elastic call
	ctx := context.Background()

	setup(t)
	defer teardown(ctx, esClient, t)

	// Create a client
	AssertEventually(t,
		func() (bool, interface{}) {
			esClient, err = elastic.NewClient(elasticURL, log.GetNewLogger(log.GetDefaultConfig("events")))
			if err != nil {
				t.Logf("error creating client: %v", err)
				return false, nil
			}
			return true, nil
		}, "failed to create elastic client", "20ms", "2m")

	ping(ctx, esClient, t)

	// Getting the ES version number is quite common, so there's a shortcut
	esversion, err := esClient.Version()
	if err != nil {
		t.Fatal("failed to get elasticsearch version")
	}
	t.Logf("Elasticsearch version %s", esversion)

	// Generate Elastic mapping and settings for event
	eventObj := events.Event{
		EventAttributes: events.EventAttributes{
			// Need to make sure pointer fields are valid to
			// generate right mappings using reflect
			ObjectRef: &api.ObjectRef{},
			Source:    &events.EventSource{},
		},
	}
	mapping, err := mapper.ElasticMapper(eventObj,
		indexType,
		mapper.WithShardCount(1),
		mapper.WithReplicaCount(0))
	AssertOk(t, err, "Failed to generate elastic mapping for events")

	// Generate JSON string for the mapping
	configs, err := mapping.JSONString()
	AssertOk(t, err, "Failed to get JSONString from elastic mapper")

	// Create elastic index with event mapping and settings
	if err := esClient.CreateIndex(ctx, indexName, configs); err != nil && !elastic.IsIndexExists(err) {
		t.Fatalf("failed to create index: %v, %v", err, elastic.IsIndexExists(err))
	}

	// index events
	indexEventsSequential(ctx, esClient, t)
	indexEventsBulk(ctx, esClient, t)
	// index a nil document
	Assert(t, esClient.Index(ctx, indexName, indexType, "dummy-id", nil) != nil,
		"Cannot index a nil document; expected failure")

	// search events
	searchEvents(ctx, esClient, t)

	// TODO: update events
}

// searchEvents runs a couple of queries on the elastic cluster
func searchEvents(ctx context.Context, client elastic.ESClient, t *testing.T) {
	t.Logf("performing search queries on the index: %v", indexName)
	Assert(t, client.FlushIndex(ctx, indexName) == nil,
		"Flush operation failed, cannot perform search")

	// Query 1: date range; search for events with events.ObjectMeta.ModTime falls in last 60 seconds
	// this is a query on the nested object events.ObjectMeta.ModTime
	now := time.Now()
	query1 := es.NewRangeQuery("meta.mod-time").Gte(now.Add(-30 * time.Second)).Lte(now)
	result, err := client.Search(ctx, indexName, indexType, query1, nil, from, maxResults)
	if err != nil {
		t.Fatalf("failed to search events for query: %v, err:%v", query1, err)
	}

	src, _ := query1.Source()
	jsonQuery, _ := json.Marshal(src)
	Assert(t, result.TotalHits() > 0,
		fmt.Sprintf("Something wrong, atleast few events should match the query: %s", jsonQuery))

	// Query 2: term query; look for events with severity == event.Severity
	// term queries are used for keyword searches (exact values)
	// whereas, match queries are full_text searches
	query2 := es.NewTermQuery("severity", event.Severity)
	result, err = client.Search(ctx, indexName, indexType, query2, nil, from, maxResults)
	if err != nil {
		t.Fatalf("failed to search events for query: %v, err:%v", query2, err)
	}

	src, _ = query2.Source()
	jsonQuery, _ = json.Marshal(src)
	Assert(t, result.TotalHits() > 0,
		fmt.Sprintf("Something wrong, atleast few events should match the query: %s", jsonQuery))

	// Query 3: match all the events;
	// creationTime is the same for all the events indexed during this run.
	query3 := es.NewMatchQuery("meta.creation-time", cTime)
	result, err = client.Search(ctx, indexName, indexType, query3, nil, from, maxResults)
	if err != nil {
		t.Fatalf("failed to search events for query: %v, err:%v", query3, err)
	}

	src, _ = query3.Source()
	jsonQuery, _ = json.Marshal(src)
	Assert(t, result.TotalHits() == int64(2*(*numEvents)), // sequential + bulk indexing (2*numEvents)
		fmt.Sprintf("Something wrong, all the events should match the query: %s", jsonQuery))

	// Query 4: combine queries 1 & 2;
	// look for severity == event.Severity events within the given 30 seconds
	query4 := es.NewBoolQuery().Must(query1, query2)
	result, err = client.Search(ctx, indexName, indexType, query4, nil, from, maxResults)
	if err != nil {
		t.Fatalf("failed to search events for query: %v, err:%v", query4, err)
	}

	src, _ = query4.Source()
	jsonQuery, _ = json.Marshal(src)
	Assert(t, result.TotalHits() > 0,
		fmt.Sprintf("Something wrong, atleast few events should match the query: %s", jsonQuery))

	// query 5: combine 2 & 3;
	// search for events with severity == event.Severity with the creation time of this run
	query5 := es.NewBoolQuery().Must(query2, query3)
	result, err = client.Search(ctx, indexName, indexType, query5, nil, from, maxResults)
	if err != nil {
		t.Fatalf("failed to search events for query: %v, err:%v", query5, err)
	}

	src, _ = query5.Source()
	jsonQuery, _ = json.Marshal(src)
	Assert(t, result.TotalHits() == int64(2*(*numEvents)),
		fmt.Sprintf("Something wrong, all the events should match the query: %s", jsonQuery))

	// full-text search queries

	// query 6: find all the event that has string defined in `infraNamespace`;
	// atleast half the total events should match this query which is `numEvents`
	query6 := es.NewQueryStringQuery(infraNamespace)
	result, err = client.Search(ctx, indexName, indexType, query6, nil, from, maxResults)
	if err != nil {
		t.Fatalf("failed to search events for query: %v, err:%v", query6, err)
	}

	src, _ = query6.Source()
	jsonQuery, _ = json.Marshal(src)
	Assert(t, result.TotalHits() >= int64(*numEvents),
		fmt.Sprintf("Something wrong, half the total events indexed should match the query: %s", jsonQuery))

	// query 7: find events by `ObjectMeta.Namespace: string`
	// atleast half the total events should match this query which is `numEvents`
	query7 := es.NewQueryStringQuery(fmt.Sprintf("%s:%s", "meta.namespace", infraNamespace))
	result, err = client.Search(ctx, indexName, indexType, query7, nil, from, maxResults)
	if err != nil {
		t.Fatalf("failed to search events for query: %v, err:%v", query7, err)
	}

	src, _ = query7.Source()
	jsonQuery, _ = json.Marshal(src)
	Assert(t, result.TotalHits() >= int64(*numEvents),
		fmt.Sprintf("Something wrong, half the total events indexed should match the query: %s", jsonQuery))

	// query 8: find all the event that has string "honda"
	query8 := es.NewQueryStringQuery(string("honda"))
	result, err = client.Search(ctx, indexName, indexType, query8, nil, from, maxResults)
	if err != nil {
		t.Fatalf("failed to search events for query: %v, err:%v", query8, err)
	}

	src, _ = query8.Source()
	jsonQuery, _ = json.Marshal(src)
	Assert(t, result.TotalHits() == 0,
		fmt.Sprintf("Something wrong, none of the event should match the query: %s", jsonQuery))
}

// indexEventsBulk performs the bulk indexing
func indexEventsBulk(ctx context.Context, client elastic.ESClient, t *testing.T) {
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
			RequestType: "index",
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
		t.Fatalf("failed to perform bulk indexing, err: %v", err)
	}

	// check the number of succeeded operation
	AssertEquals(t, *numEvents, len(bulkResp.Succeeded()),
		fmt.Sprintf("requests succeeded - expected: %v, got: %v", *numEvents, len(bulkResp.Succeeded())))

	// make sure there are 0 failed operation
	AssertEquals(t, 0, len(bulkResp.Failed()),
		fmt.Sprintf("requests failed - expected: %v, got: %v", 0, len(bulkResp.Failed())))

	t.Logf("total time taken for bulk indexing: %v\n", time.Since(start))
}

// indexEventsSequential indexs events in a sequential manner
func indexEventsSequential(ctx context.Context, client elastic.ESClient, t *testing.T) {
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
			t.Logf("failed to index event %s err:%v", event.ObjectMeta.Name, err)
			continue
		}
	}

	t.Logf("total time taken for sequential indexing: %v\n", time.Since(start))
}

// ping - helper function to test the connection with elasticsearch
func ping(ctx context.Context, client elastic.ESClient, t *testing.T) {
	// ping the elastic server to get version details
	AssertEventually(t,
		func() (bool, interface{}) {
			if err := client.Ping(ctx); err != nil {
				return false, nil
			}
			return true, nil
		}, "failed to ping elastic cluster")

}

// teardown elasticsearch cluster
func teardown(ctx context.Context, client elastic.ESClient, t *testing.T) {
	// arg -preserve-events will prevent deleting elasticsearch container
	if *preserveEvents {
		return
	}

	if client != nil {
		Assert(t, client.DeleteIndex(ctx, indexName) == nil,
			fmt.Sprintf("failed to delete index: %v", indexName))
	}

	err := setupElasticsearch(t, "stop")
	Assert(t, err == nil, fmt.Sprintf("failed to stop elasticsearch container, err: %v", err))
}

// setup spins a elasticsearch cluster if requested
func setup(t *testing.T) {
	// construct event object to be indexed
	constructEvent()

	// skip the setup
	if *skipESSetup {
		return
	}

	// spin up a single node elasticsearch
	err := setupElasticsearch(t, "start")
	Assert(t, err == nil, fmt.Sprintf("failed to start elasticsearch container, err: %v", err))
}

// setupElasticsearch helper function to start/stop elasticsearch container
func setupElasticsearch(t *testing.T, action string) error {
	var cmd []string
	switch action {
	case "start":
		t.Log("starting elasticsearch container")

		// set max_map_count; this is a must requirement to run elasticsearch
		// https://www.elastic.co/guide/en/elasticsearch/reference/current/vm-max-map-count.html
		if out, err := exec.Command("sysctl", "-w", "vm.max_map_count=262144").CombinedOutput(); err != nil {
			t.Logf("failed to set max_map_count %s", out)
		}

		cmd = []string{
			"run", "--rm", "-d", "-p", "9200:9200", "--name=pen-test-elasticsearch",
			"-e", "cluster.name=pen-test-elasticcluster", "-e", "xpack.security.enabled=false",
			"-e", "ES_JAVA_OPTS=-Xms512m -Xmx512m",
			fmt.Sprintf("%s/%s", registryURL, elasticImage)}
	case "stop":
		t.Log("stopping elasticsearch container")
		cmd = []string{"rm", "-f", "pen-test-elasticsearch"}
	default:
		return fmt.Errorf("requested action not supported: %v", action)
	}

	// run the command
	if out, err := exec.Command("docker", cmd...).CombinedOutput(); err != nil &&
		!strings.Contains(string(out), "No such container") {
		return fmt.Errorf("%s, err: %v", out, err)
	}

	// buffer to let the container go way or start completely
	time.Sleep(2 * time.Second)

	return nil
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
	event.EventAttributes = events.EventAttributes{
		Severity: *severity,
		Type:     *eventType,
		Message:  fmt.Sprintf("%s - tenant[default]", *eventType),
		Count:    1,
		Source: &events.EventSource{
			Component: "CMD",
			NodeName:  "test",
		},
	}
}
