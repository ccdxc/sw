// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package veniceinteg

import (
	"context"
	"encoding/json"
	"fmt"
	"os"
	"os/exec"
	"strconv"
	"strings"
	"testing"
	"time"

	"github.com/gogo/protobuf/types"
	uuid "github.com/satori/go.uuid"
	es "gopkg.in/olivere/elastic.v5"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/elastic"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

const (
	elasticURL   = "http://127.0.0.1:9200"
	registryURL  = "registry.test.pensando.io:5000"
	elasticImage = "elasticsearch/elasticsearch:6.2.2"
)

var (
	numEvents = 5000
	indexName = "events"
	indexType = "event"

	// create an index and map the fields
	eventsIndex = `{
		"settings": {
			"number_of_shards":1,
			"number_of_replicas":0
		},
		"mappings": {
			"event": {
				"properties": {
					"Kind": {"type": "keyword"},
					"Severity": {"type": "keyword"},
					"Component": {"type": "keyword"},
					"EventType": {"type": "keyword"},
					"Message": {"type": "text"},
					"Count": {"type": "integer"},
					"Meta": {
						"type" : "nested",
						"properties": {
							"Name": {"type": "text"},
							"UUID": {"type": "text"},
							"Tenant": {"type": "text"},
							"Namespace": {"type": "text"},
							"CreationTime": {"type": "date"},
							"ModTime": {"type": "date"},
							"ResourceVersion": {"type": "short"}
						}
					}
				}
			}
		}
	}`

	cTime           = time.Now()
	creationTime, _ = types.TimestampProto(cTime)
	event           = DEvent{
		Kind: "Node",
		Meta: api.ObjectMeta{
			Tenant:          "default",
			Namespace:       "default",
			ResourceVersion: "2",
			UUID:            "adfadf-sadfasdf-adfsdf-sadf",
		},
		Component: "CMD",
		Severity:  "INFO",
		EventType: "NodeJoined",
		Message:   "Node[node2] joined cluster[default]",
		Count:     1,
	}
)

// dummy event object
// TODO: change this to actual event object once the event protos are done
type DEvent struct {
	Kind      string
	Severity  string
	Component string
	Meta      api.ObjectMeta
	EventType string
	Message   string
	Count     int
}

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

	if err := esClient.CreateIndex(ctx, indexName, eventsIndex); err != nil && !elastic.IsIndexExists(err) {
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

	// Query 1: date range; search for events with events.Meta.ModTime falls in last 30 seconds
	// this is a query on the nested object events.Meta.ModTime
	now := time.Now()

	query1 := es.NewNestedQuery("Meta", es.NewRangeQuery("Meta.ModTime").
		Gte(now.Add(-30*time.Second)).Lte(now))
	result, err := client.Search(ctx, indexName, indexType, query1)
	if err != nil {
		t.Fatalf("failed to search events for query: %v, err:%v", query1, err)
	}

	src, _ := query1.Source()
	jsonQuery, _ := json.Marshal(src)
	Assert(t, result.TotalHits() > 0,
		fmt.Sprintf("Something wrong, atleast few events should match the query: %s", jsonQuery))

	// Query 2: term query; look for events with Severity == event.Severity
	// term queries are used for keyword searches (exact values)
	// whereas, match queries are full_text searches
	query2 := es.NewTermQuery("Severity", event.Severity)
	result, err = client.Search(ctx, indexName, indexType, query2)
	if err != nil {
		t.Fatalf("failed to search events for query: %v, err:%v", query2, err)
	}

	src, _ = query2.Source()
	jsonQuery, _ = json.Marshal(src)
	Assert(t, result.TotalHits() > 0,
		fmt.Sprintf("Something wrong, atleast few events should match the query: %s", jsonQuery))

	// Query 3: match all the events;
	// creationTime is the same for all the events indexed during this run.
	query3 := es.NewNestedQuery("Meta", es.NewMatchQuery("Meta.CreationTime", cTime))
	result, err = client.Search(ctx, indexName, indexType, query3)
	if err != nil {
		t.Fatalf("failed to search events for query: %v, err:%v", query3, err)
	}

	src, _ = query3.Source()
	jsonQuery, _ = json.Marshal(src)
	Assert(t, result.TotalHits() == int64(2*numEvents), // sequential + bulk indexing (2*numEvents)
		fmt.Sprintf("Something wrong, all the events should match the query: %s", jsonQuery))

	// Query 4: combine queries 1 & 2;
	// look for Severity == event.Severity events within the given 30 seconds
	query4 := es.NewBoolQuery().Must(query1, query2)
	result, err = client.Search(ctx, indexName, indexType, query4)
	if err != nil {
		t.Fatalf("failed to search events for query: %v, err:%v", query4, err)
	}

	src, _ = query4.Source()
	jsonQuery, _ = json.Marshal(src)
	Assert(t, result.TotalHits() > 0,
		fmt.Sprintf("Something wrong, atleast few events should match the query: %s", jsonQuery))

	// query 5: combine 2 & 3;
	// search for events with Severity == event.Severity with the creation time of this run
	query5 := es.NewBoolQuery().Must(query2, query3)
	result, err = client.Search(ctx, indexName, indexType, query5)
	if err != nil {
		t.Fatalf("failed to search events for query: %v, err:%v", query5, err)
	}

	src, _ = query5.Source()
	jsonQuery, _ = json.Marshal(src)
	Assert(t, result.TotalHits() == int64(2*numEvents),
		fmt.Sprintf("Something wrong, all the events should match the query: %s", jsonQuery))

}

// indexEventsBulk performs the bulk indexing
func indexEventsBulk(ctx context.Context, client elastic.ESClient, t *testing.T) {
	requests := make([]*elastic.BulkRequest, numEvents)
	// add requests for the bulk operation
	for i := 0; i < numEvents; i++ {
		event.Meta.UUID = uuid.NewV4().String()
		event.Meta.Name = event.Meta.UUID

		event.Meta.CreationTime.Timestamp = *creationTime
		ts, _ := types.TimestampProto(time.Now())
		event.Meta.ModTime.Timestamp = *ts

		requests[i] = &elastic.BulkRequest{
			RequestType: "index",
			Index:       indexName,
			IndexType:   indexType,
			ID:          event.Meta.UUID,
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
	AssertEquals(t, numEvents, len(bulkResp.Succeeded()),
		fmt.Sprintf("requests succeeded - expected: %v, got: %v", numEvents, len(bulkResp.Succeeded())))

	// make sure there are 0 failed operation
	AssertEquals(t, 0, len(bulkResp.Failed()),
		fmt.Sprintf("requests failed - expected: %v, got: %v", 0, len(bulkResp.Failed())))

	t.Logf("total time taken for bulk indexing: %v\n", time.Since(start))
}

// indexEventsSequential indexs events in a sequential manner
func indexEventsSequential(ctx context.Context, client elastic.ESClient, t *testing.T) {
	start := time.Now()

	// index events one by one
	for i := 1; i <= numEvents; i++ {
		event.Meta.UUID = uuid.NewV4().String()
		event.Meta.Name = event.Meta.UUID

		event.Meta.CreationTime.Timestamp = *creationTime
		transStart := time.Now()
		ts, _ := types.TimestampProto(transStart)
		event.Meta.ModTime.Timestamp = *ts

		// log failure and continue
		if err := client.Index(ctx, indexName, indexType, event.Meta.UUID, event); err != nil {
			t.Logf("failed to index event %s", event.Meta.Name)
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
	// PRESERVE_EVENTS will prevent deleting elasticsearch container
	if len(strings.TrimSpace(os.Getenv("PRESERVE_EVENTS"))) != 0 {
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
	// construct event object based on ENV if there are any
	constructEvent()

	// spin up a new 1 node elastic cluster
	if len(strings.TrimSpace(os.Getenv("SKIP_ELASTIC_SETUP"))) != 0 {
		return
	}

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

	return nil
}

// constructEvent helper function to easily index multiple kinds without changing the code.
// this is a dummy event
func constructEvent() {
	nEvents := os.Getenv("NUM_EVENTS")
	if len(strings.TrimSpace(nEvents)) != 0 {
		temp, _ := strconv.Atoi(nEvents)
		if temp > 0 {
			numEvents = temp
		}
	}

	kind := os.Getenv("KIND")
	if len(strings.TrimSpace(kind)) != 0 {
		event.Kind = kind
		event.EventType = fmt.Sprintf("%sXXX", kind)
		event.Message = fmt.Sprintf("%s[default] XXXed tenant[default]", kind)
	}

	severity := os.Getenv("SEVERITY")
	if len(strings.TrimSpace(severity)) != 0 {
		event.Severity = severity
	}
}
