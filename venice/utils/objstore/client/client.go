// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package objstore

import (
	"context"
	"io"
	"time"
)

// Client is the object store client to put/get objects
type Client interface {
	// PutObject uploads an object to the object store
	PutObject(ctx context.Context, objectName string, reader io.Reader, metaData map[string]string) (int64, error)

	// PutObjectOfSize uploads object of "size' to object store
	PutObjectOfSize(ctx context.Context, objectName string, reader io.Reader, size int64, metaData map[string]string) (int64, error)

	// PutStreamObject uploads stream of objects to the object store
	// each write() uploads a new object
	// caller must close() after write() to close the stream
	PutStreamObject(ctx context.Context, objectName string, metaData map[string]string) (io.WriteCloser, error)

	// GetObject gets the object from the object store
	// the caller must close() the reader after read()
	GetObject(ctx context.Context, objectName string) (io.ReadCloser, error)

	// GetStreamObjectAtOffset reads an object uploaded by PutStreamObject()
	// caller must close() after read()
	// returns:
	// 	object found  --> returns object reader
	// 	object not found and PutStreamObject() is in progress --> waits till context is canceled
	// 	object is not found and PutStreamObject() is not in progress ---> returns error
	GetStreamObjectAtOffset(ctx context.Context, objectName string, offset int) (io.ReadCloser, error)

	// StatObject returns object information
	StatObject(objectName string) (*ObjectStats, error)

	// ListObjects lists all objects with the given prefix
	ListObjects(prefix string) ([]string, error)

	// RemoveObjects removes all objects with the given prefix
	// this function walks through all the objects with the given prefix and deletes one object at a time
	// status is returned at the end of the walk with details of the failed objects, if any
	RemoveObjects(prefix string) error

	// RemoveObject one object with the given path
	RemoveObject(path string) error
}

// ObjectStats is the object information returned from stats API
type ObjectStats struct {
	LastModified time.Time         // Date and time the object was last modified.
	Size         int64             // Size in bytes of the object.
	ContentType  string            // A standard MIME type describing the format of the object data.
	MetaData     map[string]string // user metadata
}
