// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package mock

import (
	"context"
	"encoding/json"
	"fmt"
	"net/http"
	"strings"
	"testing"

	es "gopkg.in/olivere/elastic.v5"

	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/elastic"
	tu "github.com/pensando/sw/venice/utils/testutils"
)

// TestMockElasticServer tests the elastic functionalities using mock server
func TestMockElasticServer(t *testing.T) {
	// start mock elastic server
	mes := NewElasticServer()

	mes.Start()
	defer mes.Stop()

	// create a new elastic client using the mock server address
	client, err := elastic.NewClient(mes.GetElasticURL(), nil)
	tu.AssertOk(t, err, "failed to create client")

	ctx := context.Background()
	indexName := "events"
	indexType := "event"

	// create index
	err = client.CreateIndex(ctx, indexName, "settings")
	tu.AssertOk(t, err, "failed to create index")

	// creating the same index again fails
	err = client.CreateIndex(ctx, indexName, "settings")
	tu.Assert(t, elastic.IsIndexExists(err), "expected failure, index exists already")

	version, err := client.Version()
	tu.Assert(t, err == nil && !utils.IsEmpty(version), "failed to get elasticsearch version")

	tu.AssertOk(t, client.Ping(ctx), "failed to ping elastic cluster")

	// index doc `id1`
	data := `{"test":"data"}`
	err = client.Index(ctx, indexName, indexType, "id1", data)
	tu.AssertOk(t, err, "failed to perform index operation")

	// test bulk operation
	_, err = client.Bulk(ctx, []*elastic.BulkRequest{
		&elastic.BulkRequest{RequestType: "index", Index: indexName, IndexType: indexType, Obj: "{}", ID: "dummy1"},
		&elastic.BulkRequest{RequestType: "index", Index: indexName, IndexType: indexType, Obj: "{}", ID: "dummy2"},
	})
	tu.AssertOk(t, err, "failed to perform bulk operation")

	// search should return the docs matching the string `test`
	resp, err := client.Search(ctx, indexName, indexType, es.NewRawStringQuery(`{"match_all":"test"}`))
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
	resp, err = client.Search(ctx, indexName, indexType, es.NewRawStringQuery(`{"match_all":"test1"}`))
	tu.AssertOk(t, err, "failed to perform search")
	doc, err = json.Marshal(&resp.Hits.Hits[0].Source)
	tu.AssertOk(t, err, "failed to doc from search result")
	totalHits = resp.TotalHits()
	tu.Assert(t, totalHits == 1, fmt.Sprintf("expected %v hits, got %v", 1, totalHits))
	tu.Assert(t, string(doc) == data, fmt.Sprintf("expected doc %v, got %v", data, string(doc)))

	// query to match docs containing string `test`
	resp, err = client.Search(ctx, indexName, indexType, es.NewRawStringQuery(`{"match_all":"test"}`))
	tu.AssertOk(t, err, "failed to perform search")
	totalHits = resp.TotalHits()
	tu.Assert(t, totalHits == 2, fmt.Sprintf("expected %v hits, got %v", 2, totalHits))

	/// index one more document
	err = client.Index(ctx, indexName, indexType, "id3", `{}`)
	tu.AssertOk(t, err, "failed to perform index operation")

	// query to match all the docs in the given index
	resp, err = client.Search(ctx, indexName, indexType, es.NewRawStringQuery(`{"match_all":""}`))
	tu.AssertOk(t, err, "failed to perform search")
	totalHits = resp.TotalHits()
	tu.Assert(t, totalHits == 3, fmt.Sprintf("expected %v hits, got %v", 3, totalHits))

	// set default HTTP status on the server and make sure the calls get back the same
	// simulate internal server error
	mes.SetDefaultStatusCode(http.StatusInternalServerError)
	err = client.Index(ctx, indexName, indexType, "id4", `{"test":"data"}`)
	tu.Assert(t, strings.Contains(err.Error(), "Internal Server Error"), "expected internal server error")
	_, err = client.Search(ctx, indexName, indexType, es.NewRawStringQuery(`{"match_all":"test"}`))
	tu.Assert(t, strings.Contains(err.Error(), "Internal Server Error"), "expected internal server error")

	// reset/clear the HTTP status and make sure the calls succeeded
	mes.ClearDefaultStatusCode()
	err = client.Index(ctx, indexName, indexType, "id4", `{"test":"data"}`)
	tu.AssertOk(t, err, "failed to perform index operation")
	tu.AssertOk(t, client.Ping(ctx), "failed to ping elastic cluster")

	// -ve case; search for random strings
	resp, err = client.Search(ctx, indexName, indexType, es.NewRawStringQuery(`{"match_all":"4adf232"}`))
	tu.AssertOk(t, err, "failed to perform search")
	totalHits = resp.TotalHits()
	tu.Assert(t, totalHits == 0, fmt.Sprintf("expected %v hits, got %v", 0, totalHits))
	//resp.Hits.Hits - contains the list of matched documents
	tu.Assert(t, len(resp.Hits.Hits) == 0, fmt.Sprintf("expected %v docs, got %v", 0, len(resp.Hits.Hits)))
}
