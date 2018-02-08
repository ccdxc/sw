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
	indexName = "mock"
	indexType = "mc"
	ctx       = context.Background()
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
	result, err := mc.Search(ctx, "dummy", indexType, mockQuery{})
	Assert(t, !elastic.IsIndexExists(err), "expected failure, index does not exists")
	Assert(t, result == nil, fmt.Sprintf("expected nil search result, got %v", result))

	// search for docs
	result, err = mc.Search(ctx, indexName, indexType, mockQuery{name: "dummy1"})
	Assert(t, err == nil, "failed to search")
	Assert(t, result.TotalHits() == 2, fmt.Sprintf("expected search result %v, got %v", 2, result.TotalHits()))

	// search for docs
	result, err = mc.Search(ctx, indexName, indexType, mockQuery{name: "dummy2"})
	Assert(t, err == nil, "failed to search")
	Assert(t, result.TotalHits() == 1, fmt.Sprintf("expected search result %v, got %v", 1, result.TotalHits()))

	// search for non-existent docs
	result, err = mc.Search(ctx, indexName, indexType, mockQuery{name: "non-existent-doc"})
	Assert(t, err == nil, "failed to search")
	Assert(t, result.TotalHits() == 0, fmt.Sprintf("expected search result %v, got %v", 0, result.TotalHits()))

	Assert(t, mc.DeleteIndex(ctx, "mock") == nil, "failed to delete index")

	// search on the deleted index
	result, err = mc.Search(ctx, indexName, indexType, mockQuery{})
	Assert(t, !elastic.IsIndexExists(err), "expected failure, index does not exists")
	Assert(t, result == nil, fmt.Sprintf("expected nil search result, got %v", result))
}

// TestESMockBulkIndex tests bulk indexing and search operation
func TestESMockBulkIndex(t *testing.T) {
	mc := newMockClient()
	Assert(t, mc.CreateIndex(ctx, indexName, "") == nil, "failed to create index")

	requests := make([]*elastic.BulkRequest, 10)
	for i := 0; i < 10; i++ {
		requests[i] = &elastic.BulkRequest{
			RequestType: "index",
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
	result, err := mc.Search(ctx, indexName, indexType, mockQuery{name: "dummy1"})
	Assert(t, err == nil, "failed to search")
	Assert(t, result.TotalHits() == 1, fmt.Sprintf("expected search result %v, got %v", 1, result.TotalHits()))

	// index some more docs (valid + invalid docs)
	requests = []*elastic.BulkRequest{}
	for i := 0; i < 10; i++ {
		// valid
		requests = append(requests, &elastic.BulkRequest{
			RequestType: "index",
			Index:       indexName,
			IndexType:   indexType,
			ID:          uuid.NewV4().String(),
			Obj:         mockObj{name: "dummy1"},
		})

		// invalid
		requests = append(requests, &elastic.BulkRequest{
			RequestType: "index",
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

	result, err = mc.Search(ctx, indexName, indexType, mockQuery{name: "dummy1"})
	Assert(t, err == nil, "failed to search")
	Assert(t, result.TotalHits() == 11, fmt.Sprintf("expected search result %v, got %v", 11, result.TotalHits()))

	Assert(t, mc.FlushIndex(ctx, indexName) == nil, "failed to flush index")

	Assert(t, mc.DeleteIndex(ctx, indexName) == nil, "failed to delete index")
}

// TestESMockSearch tests the search operation of the mockClient
func TestESMockSearch(t *testing.T) {
	mc := newMockClient()
	Assert(t, mc.CreateIndex(ctx, indexName, "") == nil, "failed to create index")

	requests := make([]*elastic.BulkRequest, 10)
	for i := 0; i < 10; i++ {
		requests[i] = &elastic.BulkRequest{
			RequestType: "index",
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
	result, err := mc.Search(ctx, indexName, indexType, mockQuery{name: "dummy1"})
	Assert(t, err == nil, "failed to search")
	Assert(t, result.TotalHits() == 10, fmt.Sprintf("expected search result %v, got %v", 11, result.TotalHits()))

	// search using invalid query
	result, err = mc.Search(ctx, indexName, indexType, nil)
	Assert(t, elastic.IsInvalidSearchQuery(err), fmt.Sprintf("expected failure, got %v", err))
	Assert(t, result == nil, fmt.Sprintf("expected nil search result, got %v", result))
}
