package indexer

// Interface is an interface for Indexing functionality
type Interface interface {

	// Start service
	Start() error

	// Stop service
	Stop()

	// GetObjectCount return count of total objects indexed
	GetObjectCount() uint64

	// CreateIndex creates the given index with the given settings in SearchDB.
	CreateIndex(index, settings string) error

	// DeleteIndex deletes the given index from SearchDB.
	DeleteIndex(index string) error

	// FlushIndex flushes the given index. This ensures all the writes are flushed to the shard.
	FlushIndex(index string) error

	// Index indexes a single document (obj) on the given `index` and
	// type `docType` with the given ID.
	Index(index, docType, ID string, obj interface{}) error

	// Delete deletes a single document (obj) from the given `index` and
	// type `docType` with the given ID.
	Delete(index, docType, ID string) error

	// Bulk performs the bulk request against SearchDB. Each of the request
	// in bulk operation can be heterogeneous.
	// Length of IDs and objects should match and there is one corelation
	// between elements in IDs and objects slice.
	Bulk(index, docType string, IDs []string, objects []interface{}) error
}
