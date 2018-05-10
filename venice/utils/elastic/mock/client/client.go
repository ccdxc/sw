// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package mock

import (
	"context"
	"fmt"

	es "gopkg.in/olivere/elastic.v5"

	"github.com/pensando/sw/venice/utils/elastic"
)

// mockObj
type mockObj struct {
	name string
}

// mockQuery
type mockQuery struct {
	name string
}

// mockAggregation
type mockAggregation struct {
}

// MockClient for elastic
type mockClient struct {
	indexes map[string]bool // map of indexes available
	docs    map[string]map[string]mockObj
}

// newMockClient returns the new mock client
func newMockClient() elastic.ESClient {
	return &mockClient{
		indexes: map[string]bool{},
		docs:    map[string]map[string]mockObj{},
	}
}

// IsClusterHealthy - mock implementation
func (e *mockClient) IsClusterHealthy(ctx context.Context) (bool, error) {
	return true, nil
}

// Version - mock implementation
func (e *mockClient) Version() (string, error) {
	return "", nil
}

// CreateIndexTemplate creates the tempalte using mockClient
func (e *mockClient) CreateIndexTemplate(ctx context.Context, template, settings string) error {
	return nil
}

// DeleteIndexTemplate deletes the tempalte using mockClient
func (e *mockClient) DeleteIndexTemplate(ctx context.Context, tempplate string) error {
	return nil
}

// CreateIndex creates the index using mockClient
func (e *mockClient) CreateIndex(ctx context.Context, index, settings string) error {
	if _, ok := e.indexes[index]; ok {
		return elastic.NewError(elastic.ErrIndexExists, "")
	}

	e.indexes[index] = true
	e.docs[index] = map[string]mockObj{}
	return nil
}

// DeleteIndex deletes the index using mockClient
func (e *mockClient) DeleteIndex(ctx context.Context, index string) error {
	delete(e.indexes, index)
	delete(e.docs, index)
	return nil
}

// FlushIndex mock implementation of flush operation
func (e *mockClient) FlushIndex(ctx context.Context, index string) error {
	return nil
}

//Index indexes the given mockObj using mockClient
func (e *mockClient) Index(ctx context.Context, index, iType, ID string, obj interface{}) error {
	if _, ok := e.indexes[index]; !ok {
		return elastic.NewError(elastic.ErrIndexNotExist, "")
	}

	if mc, ok := obj.(mockObj); ok {
		e.docs[index][ID] = mc
		return nil
	}

	return fmt.Errorf("failed to index given object")
}

//Bulk performs the bulk operation using mockClient
func (e *mockClient) Bulk(ctx context.Context, objs []*elastic.BulkRequest) (*es.BulkResponse, error) {
	var error bool
	var response es.BulkResponse

	for _, obj := range objs {
		switch obj.RequestType {
		case "index":
			mc, objOk := obj.Obj.(mockObj)
			if _, indexFound := e.indexes[obj.Index]; indexFound && objOk {
				e.docs[obj.Index][obj.ID] = mc
				response.Items = append(response.Items,
					map[string]*es.BulkResponseItem{"index": &es.BulkResponseItem{
						Index: obj.Index, Type: obj.IndexType, Id: obj.ID},
					})
			} else {
				error = true
			}
			// TODO: handle other request types
		}
	}

	if error {
		return &response, elastic.NewError(elastic.ErrBulkRequestFailed, "")
	}

	return &response, nil
}

//Delete removes the given mockObj using mockClient
func (e *mockClient) Delete(ctx context.Context, index, iType, ID string) error {
	if _, ok := e.indexes[index]; !ok {
		return elastic.NewError(elastic.ErrIndexNotExist, "")
	}

	delete(e.docs[index], ID)
	return nil
}

// Search - mock implementation of search operation
func (e *mockClient) Search(ctx context.Context, index, iType string, query interface{}, aggregation interface{}, from, size int32, sortBy string) (*es.SearchResult, error) {
	var totalHits int64

	if _, ok := e.indexes[index]; !ok {
		return nil, elastic.NewError(elastic.ErrIndexNotExist, "")
	}

	mcQuery, ok := query.(mockQuery)
	if !ok {
		return nil, elastic.NewError(elastic.ErrInvalidSearchQuery, "")
	}

	if len(e.docs[index]) != 0 {
		for _, obj := range e.docs[index] { // all objects belonging to this index
			if obj.name == mcQuery.name {
				totalHits++
			}
		}
	}

	return &es.SearchResult{Hits: &es.SearchHits{TotalHits: totalHits}}, nil
}

// Close - mock client implementation
func (e *mockClient) Close() error {
	e.indexes = nil
	e.docs = nil
	return nil
}

// GetResetCount - mock client implementation
func (e *mockClient) GetResetCount() int {
	return 0
}
