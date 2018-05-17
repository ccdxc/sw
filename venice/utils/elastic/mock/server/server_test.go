// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package mock

import (
	"context"
	"encoding/json"
	"fmt"
	"net/http"
	"strings"
	"testing"
	"time"

	es "gopkg.in/olivere/elastic.v5"

	"github.com/pensando/sw/api"
	types "github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/elastic"
	"github.com/pensando/sw/venice/utils/log"
	mockresolver "github.com/pensando/sw/venice/utils/resolver/mock"
	tu "github.com/pensando/sw/venice/utils/testutils"
)

var (
	from       = int32(0)
	maxResults = int32(10)
	indexName  = "events"
	indexType  = "event"
	sortBy     = ""
)

// TestMockElasticServer tests the elastic functionalities using mock server
func TestMockElasticServer(t *testing.T) {
	// start mock elastic server
	mes := NewElasticServer()

	mes.Start()
	defer mes.Stop()

	// create a new elastic client using the mock server address
	client, err := elastic.NewClient(mes.GetElasticURL(), nil,
		log.GetNewLogger(log.GetDefaultConfig("elasticsearch-mock-server")))
	tu.AssertOk(t, err, "failed to create client")

	ctx := context.Background()

	// create index
	err = client.CreateIndex(ctx, indexName, "settings")
	tu.AssertOk(t, err, "failed to create index")

	// creating the same index again fails
	err = client.CreateIndex(ctx, indexName, "settings")
	tu.Assert(t, elastic.IsIndexExists(err), "expected failure, index exists already")

	version, err := client.Version()
	tu.Assert(t, err == nil && !utils.IsEmpty(version), "failed to get elasticsearch version")

	healthy, err := client.IsClusterHealthy(ctx)
	tu.AssertOk(t, err, "failed to check the elasticsearch cluster health")
	tu.Assert(t, healthy, "elasticsearch cluster not healthy")

	// index doc `id1`
	data := `{"test":"data"}`
	err = client.Index(ctx, indexName, indexType, "id1", data)
	tu.AssertOk(t, err, "failed to perform index operation")

	// test bulk operation
	_, err = client.Bulk(ctx, []*elastic.BulkRequest{
		&elastic.BulkRequest{RequestType: elastic.Index, Index: indexName, IndexType: indexType, Obj: "{}", ID: "dummy1"},
		&elastic.BulkRequest{RequestType: elastic.Index, Index: indexName, IndexType: indexType, Obj: "{}", ID: "dummy2"},
	})
	tu.AssertOk(t, err, "failed to perform bulk operation")

	// search should return the docs matching the string `test`
	resp, err := client.Search(ctx, indexName, indexType, es.NewRawStringQuery(`{"match_all":"test"}`), nil, from, maxResults, sortBy)
	tu.AssertOk(t, err, "failed to perform search")
	doc, err := json.Marshal(&resp.Hits.Hits[0].Source)
	tu.AssertOk(t, err, "failed to doc from search result")
	totalHits := resp.TotalHits()
	tu.Assert(t, totalHits == 1, fmt.Sprintf("expected %v hits, got %v", 1, totalHits))
	// check the document as well
	tu.Assert(t, string(doc) == data, fmt.Sprintf("expected doc %v, got %v", data, string(doc)))

	// index one more document
	data = `{"test1":"data"}`
	err = client.Index(ctx, indexName, indexType, "id2", data)
	tu.AssertOk(t, err, "failed to perform index operation")

	// this search should return the docs matching the string `test1`
	resp, err = client.Search(ctx, indexName, indexType, es.NewRawStringQuery(`{"match_all":"test1"}`), nil, from, maxResults, sortBy)
	tu.AssertOk(t, err, "failed to perform search")
	doc, err = json.Marshal(&resp.Hits.Hits[0].Source)
	tu.AssertOk(t, err, "failed to doc from search result")
	totalHits = resp.TotalHits()
	tu.Assert(t, totalHits == 1, fmt.Sprintf("expected %v hits, got %v", 1, totalHits))
	tu.Assert(t, string(doc) == data, fmt.Sprintf("expected doc %v, got %v", data, string(doc)))

	// query to match docs containing string `test`
	resp, err = client.Search(ctx, indexName, indexType, es.NewRawStringQuery(`{"match_all":"test"}`), nil, from, maxResults, sortBy)
	tu.AssertOk(t, err, "failed to perform search")
	totalHits = resp.TotalHits()
	tu.Assert(t, totalHits == 2, fmt.Sprintf("expected %v hits, got %v", 2, totalHits))

	/// index one more document
	err = client.Index(ctx, indexName, indexType, "id3", `{}`)
	tu.AssertOk(t, err, "failed to perform index operation")

	// query to match all the docs in the given index
	resp, err = client.Search(ctx, indexName, indexType, es.NewRawStringQuery(`{"match_all":""}`), nil, from, maxResults, sortBy)
	tu.AssertOk(t, err, "failed to perform search")
	totalHits = resp.TotalHits()
	tu.Assert(t, totalHits == 3, fmt.Sprintf("expected %v hits, got %v", 3, totalHits))

	// set default HTTP status on the server and make sure the calls get back the same
	// simulate internal server error
	mes.SetDefaultStatusCode(http.StatusInternalServerError)
	err = client.Index(ctx, indexName, indexType, "id4", `{"test":"data"}`)
	tu.Assert(t, strings.Contains(err.Error(), "Internal Server Error"), "expected internal server error")
	_, err = client.Search(ctx, indexName, indexType, es.NewRawStringQuery(`{"match_all":"test"}`), nil, from, maxResults, sortBy)
	tu.Assert(t, strings.Contains(err.Error(), "Internal Server Error"), "expected internal server error")

	// reset/clear the HTTP status and make sure the calls succeeded
	mes.ClearDefaultStatusCode()
	err = client.Index(ctx, indexName, indexType, "id4", `{"test":"data"}`)
	tu.AssertOk(t, err, "failed to perform index operation")

	healthy, err = client.IsClusterHealthy(ctx)
	tu.AssertOk(t, err, "failed to check elasticsearch cluster health")
	tu.Assert(t, healthy, "elasticsearch cluster not healthy")

	// -ve case; search for random strings
	resp, err = client.Search(ctx, indexName, indexType, es.NewRawStringQuery(`{"match_all":"4adf232"}`), nil, from, maxResults, sortBy)
	tu.AssertOk(t, err, "failed to perform search")
	totalHits = resp.TotalHits()
	tu.Assert(t, totalHits == 0, fmt.Sprintf("expected %v hits, got %v", 0, totalHits))
	//resp.Hits.Hits - contains the list of matched documents
	tu.Assert(t, len(resp.Hits.Hits) == 0, fmt.Sprintf("expected %v docs, got %v", 0, len(resp.Hits.Hits)))
}

// TestMockElasticServerRestart tests the behavior of the client during elasticsearch restarts
func TestMockElasticServerRestart(t *testing.T) {
	// start mock elastic server
	mes := NewElasticServer()
	mes.Start()

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
		URL:     mes.GetElasticURL(),
	}

	// add mock elastic service to mock resolver
	mr.AddServiceInstance(si)

	// create a new elastic client using the mock server address
	client, err := elastic.NewClient("", mr, log.GetNewLogger(log.GetDefaultConfig(t.Name())))
	tu.AssertOk(t, err, "failed to create client")

	ctx := context.Background()

	err = client.CreateIndex(ctx, indexName, "settings")
	tu.AssertOk(t, err, "failed to create index")

	err = client.Index(ctx, indexName, indexType, "id3", `{}`)
	tu.AssertOk(t, err, "failed to perform index operation")

	stopServerRestarts := make(chan struct{}, 1)

	// restart the server every 60ms
	go func() {
		defer mes.Stop()
		for {
			select {
			case <-stopServerRestarts:
				return
			default:
				mes.Stop()
				mr.DeleteServiceInstance(si)

				time.Sleep(20 * time.Millisecond)

				mes = NewElasticServer()
				mes.Start()

				si.URL = mes.GetElasticURL()
				mr.AddServiceInstance(si)

				time.Sleep(1*time.Second + 100*time.Millisecond)
			}
		}
	}()

	for i := 0; i < 10; i++ {
		//try the query now!

		// create index
		err = client.CreateIndex(ctx, indexName, "settings")
		tu.Assert(t, err == nil || elastic.IsIndexExists(err), "expected successful creation of index or ErrIndexExists")

		err = client.CreateIndex(ctx, indexName, "settings")
		tu.Assert(t, err == nil || elastic.IsIndexExists(err), "expected successful creation of index or ErrIndexExists")

		// index a document
		err = client.Index(ctx, indexName, indexType, fmt.Sprintf("test%d", i), `{}`)
		tu.Assert(t, err == nil || elastic.IsIndexNotExists(err), "failed to perform index operation")

		// bulk index documents
		_, err = client.Bulk(ctx, []*elastic.BulkRequest{
			&elastic.BulkRequest{RequestType: elastic.Index, Index: indexName, IndexType: indexType, Obj: "{}", ID: "dummy1"},
			&elastic.BulkRequest{RequestType: elastic.Index, Index: indexName, IndexType: indexType, Obj: "{}", ID: "dummy2"},
		})
		tu.AssertOk(t, err, "failed to perform bulk operation")
	}

	close(stopServerRestarts)

	tu.Assert(t, client.GetResetCount() > 0, "client never reset?? something went wrong")
}
