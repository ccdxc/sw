// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package mock

import (
	"context"
	"fmt"
	"testing"

	uuid "github.com/satori/go.uuid"

	"github.com/pensando/sw/venice/utils/elastic"
	. "github.com/pensando/sw/venice/utils/testutils"
)

var (
	indexName   = "mock"
	indexType   = "mc"
	ctx         = context.Background()
	from        = int32(0)
	maxResults  = int32(10)
	sortByField = ""
	sortAsc     = true
)

// TestESMockIndex tests the index creation and deletion operation.
func TestESMockIndex(t *testing.T) {
	mc := newMockClient()
	Assert(t, mc.CreateIndex(ctx, indexName, "") == nil, "failed to create index")
	Assert(t, mc.CreateIndex(ctx, indexName, "") != nil, "exected failure, index already exists")
	Assert(t, mc.DeleteIndex(ctx, indexName) == nil, "failed to delete index")
	Assert(t, mc.DeleteIndex(ctx, indexName) == nil, "failed to delete index")
}

// TestESMockDocIndex tests document indexing and search operation
func TestESMockDocIndex(t *testing.T) {
	mc := newMockClient()
	Assert(t, mc.CreateIndex(ctx, indexName, "") == nil, "failed to create index")
	Assert(t, mc.Index(ctx, indexName, indexType, uuid.NewV4().String(), mockObj{name: "dummy1"}) == nil,
		"failed to index document")
	Assert(t, mc.Index(ctx, indexName, indexType, uuid.NewV4().String(), mockObj{name: "dummy1"}) == nil,
		"failed to index document")
	Assert(t, mc.Index(ctx, indexName, indexType, uuid.NewV4().String(), mockObj{name: "dummy2"}) == nil,
		"failed to index document")

	// index invalid doc
	Assert(t, mc.Index(ctx, indexName, indexType, uuid.NewV4().String(), nil) != nil,
		"expected failure, invalid doc")

	// search on non-existing index
	result, err := mc.Search(ctx, "dummy", indexType, mockQuery{}, mockAggregation{}, from, maxResults, sortByField, sortAsc)
	Assert(t, !elastic.IsIndexExists(err), "expected failure, index does not exists")
	Assert(t, result == nil, fmt.Sprintf("expected nil search result, got %v", result))

	// search for docs
	result, err = mc.Search(ctx, indexName, indexType, mockQuery{name: "dummy1"}, mockAggregation{}, from, maxResults, sortByField, sortAsc)
	Assert(t, err == nil, "failed to search")
	Assert(t, result.TotalHits() == 2, fmt.Sprintf("expected search result %v, got %v", 2, result.TotalHits()))

	// search for docs
	result, err = mc.Search(ctx, indexName, indexType, mockQuery{name: "dummy2"}, mockAggregation{}, from, maxResults, sortByField, sortAsc)
	Assert(t, err == nil, "failed to search")
	Assert(t, result.TotalHits() == 1, fmt.Sprintf("expected search result %v, got %v", 1, result.TotalHits()))

	// search for non-existent docs
	result, err = mc.Search(ctx, indexName, indexType, mockQuery{name: "non-existent-doc"}, mockAggregation{}, from, maxResults, sortByField, sortAsc)
	Assert(t, err == nil, "failed to search")
	Assert(t, result.TotalHits() == 0, fmt.Sprintf("expected search result %v, got %v", 0, result.TotalHits()))

	Assert(t, mc.DeleteIndex(ctx, "mock") == nil, "failed to delete index")

	// search on the deleted index
	result, err = mc.Search(ctx, indexName, indexType, mockQuery{}, mockAggregation{}, from, maxResults, sortByField, sortAsc)
	Assert(t, !elastic.IsIndexExists(err), "expected failure, index does not exists")
	Assert(t, result == nil, fmt.Sprintf("expected nil search result, got %v", result))
}

// TestESMockBulkIndex tests bulk indexing and search operation
func TestESMockBulkIndex(t *testing.T) {
	mc := newMockClient()
	defer mc.Close()
	Assert(t, mc.CreateIndex(ctx, indexName, "") == nil, "failed to create index")

	requests := make([]*elastic.BulkRequest, 10)
	for i := 0; i < 10; i++ {
		requests[i] = &elastic.BulkRequest{
			RequestType: elastic.Index,
			Index:       indexName,
			IndexType:   indexType,
			ID:          uuid.NewV4().String(),
			Obj:         mockObj{name: fmt.Sprintf("dummy%v", i)},
		}
	}

	// test bulk operation
	resp, err := mc.Bulk(ctx, requests)
	Assert(t, err == nil, "failed to perform bulk operation")
	Assert(t, len(resp.Indexed()) == 10, fmt.Sprintf("expected %v, got %v", 10, len(resp.Indexed())))

	// search
	result, err := mc.Search(ctx, indexName, indexType, mockQuery{name: "dummy1"}, mockAggregation{}, from, maxResults, sortByField, sortAsc)
	Assert(t, err == nil, "failed to search")
	Assert(t, result.TotalHits() == 1, fmt.Sprintf("expected search result %v, got %v", 1, result.TotalHits()))

	// index some more docs (valid + invalid docs)
	requests = []*elastic.BulkRequest{}
	for i := 0; i < 10; i++ {
		// valid
		requests = append(requests, &elastic.BulkRequest{
			RequestType: elastic.Index,
			Index:       indexName,
			IndexType:   indexType,
			ID:          uuid.NewV4().String(),
			Obj:         mockObj{name: "dummy1"},
		})

		// invalid
		requests = append(requests, &elastic.BulkRequest{
			RequestType: elastic.Index,
			Index:       indexName,
			IndexType:   indexType,
			ID:          uuid.NewV4().String(),
			Obj:         nil,
		})
	}

	resp, err = mc.Bulk(ctx, requests)
	Assert(t, elastic.IsBulkRequestFailed(err), fmt.Sprintf("expected failure, got %v", err))
	// only 10 of them are valid requests
	Assert(t, len(resp.Indexed()) == 10, fmt.Sprintf("expected %v, got %v", 10, len(resp.Indexed())))

	result, err = mc.Search(ctx, indexName, indexType, mockQuery{name: "dummy1"}, mockAggregation{}, from, maxResults, sortByField, sortAsc)
	Assert(t, err == nil, "failed to search")
	Assert(t, result.TotalHits() == 11, fmt.Sprintf("expected search result %v, got %v", 11, result.TotalHits()))

	Assert(t, mc.FlushIndex(ctx, indexName) == nil, "failed to flush index")

	Assert(t, mc.DeleteIndex(ctx, indexName) == nil, "failed to delete index")
}

// TestESMockSearch tests the search operation of the mockClient
func TestESMockSearch(t *testing.T) {
	mc := newMockClient()
	defer mc.Close()
	Assert(t, mc.CreateIndex(ctx, indexName, "") == nil, "failed to create index")

	requests := make([]*elastic.BulkRequest, 10)
	for i := 0; i < 10; i++ {
		requests[i] = &elastic.BulkRequest{
			RequestType: elastic.Index,
			Index:       indexName,
			IndexType:   indexType,
			ID:          uuid.NewV4().String(),
			Obj:         mockObj{name: "dummy1"},
		}
	}

	// test bulk operation
	resp, err := mc.Bulk(ctx, requests)
	Assert(t, err == nil, "failed to perform bulk operation")
	Assert(t, len(resp.Indexed()) == 10, fmt.Sprintf("expected %v, got %v", 10, len(resp.Indexed())))

	// search documents
	result, err := mc.Search(ctx, indexName, indexType, mockQuery{name: "dummy1"}, mockAggregation{}, from, maxResults, sortByField, sortAsc)
	Assert(t, err == nil, "failed to search")
	Assert(t, result.TotalHits() == 10, fmt.Sprintf("expected search result %v, got %v", 10, result.TotalHits()))

	// search using invalid query
	result, err = mc.Search(ctx, indexName, indexType, nil, mockAggregation{}, from, maxResults, sortByField, sortAsc)
	Assert(t, elastic.IsInvalidSearchQuery(err), fmt.Sprintf("expected failure, got %v", err))
	Assert(t, result == nil, fmt.Sprintf("expected nil search result, got %v", result))
}

// TestESMockBulk tests the bulk operation with index, update and delete requests
func TestESMockBulk(t *testing.T) {
	mc := newMockClient()
	defer mc.Close()
	Assert(t, mc.CreateIndex(ctx, indexName, "") == nil, "failed to create index")

	totalEvents := 10
	requests := make([]*elastic.BulkRequest, totalEvents)
	for i := 0; i < 10; i++ {
		requests[i] = &elastic.BulkRequest{
			RequestType: elastic.Index,
			Index:       indexName,
			IndexType:   indexType,
			ID:          uuid.NewV4().String(),
			Obj:         mockObj{name: "dummy1 - index"},
		}
	}

	resp, err := mc.Bulk(ctx, requests)
	Assert(t, err == nil, "failed to perform bulk operation")
	Assert(t, len(resp.Indexed()) == totalEvents, fmt.Sprintf("expected %v, got %v", totalEvents, len(resp.Indexed())))

	// search documents
	result, err := mc.Search(ctx, indexName, indexType, mockQuery{name: "dummy1 - index"}, mockAggregation{}, from, maxResults, sortByField, sortAsc)
	Assert(t, err == nil, "failed to search")
	Assert(t, int(result.TotalHits()) == totalEvents, fmt.Sprintf("expected search result %v, got %v", totalEvents, result.TotalHits()))

	// update/delete
	for i := 0; i < len(requests); i += 2 {
		// update req `i`
		requests[i].RequestType = elastic.Update
		requests[i].Obj = mockObj{name: "dummy1 - updated"}

		// delete req `i+1`
		requests[i+1].RequestType = elastic.Delete
	}

	resp, err = mc.Bulk(ctx, requests)
	Assert(t, err == nil, "failed to perform bulk operation")
	Assert(t, len(resp.Updated()) == totalEvents/2, fmt.Sprintf("expected %v, got %v", totalEvents/2, len(resp.Updated())))
	Assert(t, len(resp.Deleted()) == totalEvents/2, fmt.Sprintf("expected %v, got %v", totalEvents/2, len(resp.Deleted())))

	// search old docs and it should match 0 events
	result, err = mc.Search(ctx, indexName, indexType, mockQuery{name: "dummy1 - index"}, mockAggregation{}, from, maxResults, sortByField, sortAsc)
	Assert(t, err == nil, "failed to search")
	Assert(t, result.TotalHits() == 0, fmt.Sprintf("expected search result %v, got %v", 0, result.TotalHits()))

	// search updated docs
	result, err = mc.Search(ctx, indexName, indexType, mockQuery{name: "dummy1 - updated"}, mockAggregation{}, from, maxResults, sortByField, sortAsc)
	Assert(t, err == nil, "failed to search")
	Assert(t, int(result.TotalHits()) == totalEvents/2, fmt.Sprintf("expected search result %v, got %v", totalEvents/2, result.TotalHits()))
}

func TestESMockFunctions(t *testing.T) {
	mc := newMockClient()
	defer mc.Close()
	Assert(t, mc.CreateIndex(ctx, indexName, "") == nil, "failed to create index")

	_, err := mc.GetIndicesStats(ctx, []string{indexName})
	AssertOk(t, err, "failed get indices stats")

	_, err = mc.GetSearchShards(ctx, []string{indexName})
	AssertOk(t, err, "failed get search shards")

	_, err = mc.GetNodesInfo(ctx, []string{"test"})
	AssertOk(t, err, "failed get node info")

	Assert(t, mc.GetRawClient() == nil, "raw client is not nil")
}
