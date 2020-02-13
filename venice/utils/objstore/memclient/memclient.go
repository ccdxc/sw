package memclient

import (
	"bytes"
	"context"
	"io"
	"io/ioutil"
	"strings"
	"sync"
	"time"

	"github.com/pkg/errors"

	objstore "github.com/pensando/sw/venice/utils/objstore/client"
)

type memBuffer struct {
	*bytes.Buffer
}

func (m *memBuffer) Close() error {
	return nil
}

type memObj struct {
	modified time.Time
	buf      memBuffer
	meta     map[string]string
}
type memClient struct {
	sync.Mutex
	store map[string]memObj
}

// PutObject uploads an object to the object store
func (m *memClient) PutObject(ctx context.Context, objectName string, reader io.Reader, metaData map[string]string) (int64, error) {
	defer m.Unlock()
	m.Lock()
	o, ok := m.store[objectName]
	if ok {
		o.buf.Reset()
	} else {
		o.buf = memBuffer{bytes.NewBuffer(nil)}
		m.store[objectName] = o
	}
	o.meta = metaData
	o.modified = time.Now()
	return io.Copy(o.buf, reader)
}

// PutObjectOfSize uploads object of "size' to object store
func (m *memClient) PutObjectOfSize(ctx context.Context, objectName string, reader io.Reader, size int64, metaData map[string]string) (int64, error) {
	return 0, errors.New("unimplemented")
}

// PutObjectExplicit uploads an object to object store under the given bucket name (i.e. serviceName)
func (m *memClient) PutObjectExplicit(ctx context.Context,
	serviceName string, objectName string, reader io.Reader, metaData map[string]string) (int64, error) {
	return 0, errors.New("unimplemented")
}

// PutStreamObject uploads stream of objects to the object store
// each write() uploads a new object
// caller must close() after write() to close the stream
func (m *memClient) PutStreamObject(ctx context.Context, objectName string, metaData map[string]string) (io.WriteCloser, error) {
	return nil, errors.New("unimplemented")
}

// GetObject gets the object from the object store
// the caller must close() the reader after read()
func (m *memClient) GetObject(ctx context.Context, objectName string) (io.ReadCloser, error) {
	defer m.Unlock()
	m.Lock()
	o, ok := m.store[objectName]
	if ok {
		return ioutil.NopCloser(bytes.NewReader(o.buf.Bytes())), nil
		// return &o.buf, nil
	}
	return nil, nil
}

// GetStreamObjectAtOffset reads an object uploaded by PutStreamObject()
func (m *memClient) GetStreamObjectAtOffset(ctx context.Context, objectName string, offset int) (io.ReadCloser, error) {
	return nil, errors.New("unimplemented")
}

// StatObject returns object information
func (m *memClient) StatObject(objectName string) (*objstore.ObjectStats, error) {
	defer m.Unlock()
	m.Lock()
	if o, ok := m.store[objectName]; ok {
		return &objstore.ObjectStats{
			LastModified: o.modified,
			Size:         int64(o.buf.Len()),
			MetaData:     o.meta,
		}, nil
	}

	return nil, errors.New("not found")
}

// ListObjects lists all objects with the given prefix
func (m *memClient) ListObjects(prefix string) ([]string, error) {
	defer m.Unlock()
	m.Lock()
	ret := []string{}
	for k := range m.store {
		ret = append(ret, k)
	}
	return ret, nil
}

// RemoveObjects removes all objects with the given prefix
// this function walks through all the objects with the given prefix and deletes one object at a time
// status is returned at the end of the walk with details of the failed objects, if any
func (m *memClient) RemoveObjects(prefix string) error {
	defer m.Unlock()
	m.Lock()
	var delKeys []string
	for k := range m.store {
		if strings.HasPrefix(k, prefix) {
			delKeys = append(delKeys, k)
		}
	}
	for i := range delKeys {
		delete(m.store, delKeys[i])
	}
	return nil
}

// RemoveObjects one object with the given path
// this function walks through all the objects with the given prefix and deletes one object at a time
// status is returned at the end of the walk with details of the failed objects, if any
func (m *memClient) RemoveObject(path string) error {
	defer m.Unlock()
	m.Lock()
	delete(m.store, path)

	return nil
}

// NewMemObjstore returns a new MemObjstore object
func NewMemObjstore() objstore.Client {
	return &memClient{store: make(map[string]memObj)}
}
