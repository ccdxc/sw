// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package elastic

const (
	// Index represents the index request; used in bulk requests
	Index string = "index"

	// Update represents the update request; used in bulk requests
	Update string = "update"

	// Delete represents the delete request; used in bulk requests
	Delete string = "delete"
)

// BulkRequest represents each request in the bulk operation
type BulkRequest struct {
	RequestType string      // type of the request. e.g. index, update, delete, etc.
	Index       string      // name of the index. e.g. events
	IndexType   string      // type within in the above index. e.g. event
	Obj         interface{} // obj(doc) to be indexed
	ID          string      // ID to be used while indexing the doc; this prevents ES from generating an ID
}
