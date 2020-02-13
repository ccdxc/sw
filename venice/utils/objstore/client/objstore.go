// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package objstore

import (
	"bytes"
	"context"
	"crypto/tls"
	"fmt"
	"io"
	"io/ioutil"
	"math/rand"
	"path/filepath"
	"strconv"
	"strings"
	"time"

	vlog "github.com/pensando/sw/venice/utils/log"

	"github.com/pensando/sw/venice/globals"
	minio "github.com/pensando/sw/venice/utils/objstore/minio/client"
	"github.com/pensando/sw/venice/utils/resolver"
)

const (
	maxRetry       = 3
	inProgressFile = ".inprogress"
	connectErr     = "connect:"
)

var objsLog = vlog.WithContext("pkg", "objstore")

type objStoreBackend interface {
	// PutObject uploads object into the object store
	PutObject(ctx context.Context, objectName string, reader io.Reader, userMeta map[string]string) (int64, error)

	// PutObjectOfSize uploads object of size to the object store
	PutObjectOfSize(ctx context.Context, objectName string, reader io.Reader, size int64, userMeta map[string]string) (int64, error)

	// PutObjectExplicit will override the default service name given at time of initializing the client with the given
	// service name.
	// In terms of MinIO, the given serviceName will become the MinIO's bucket name
	PutObjectExplicit(ctx context.Context, serviceName string, objectName string, reader io.Reader, metaData map[string]string) (int64, error)

	// GetObject gets the object from object store
	GetObject(ctx context.Context, objectName string) (io.ReadCloser, error)

	// StatObject returns object information
	StatObject(objectName string) (*minio.ObjectStats, error)

	// ListObjects returns object names with the prefix
	ListObjects(prefix string) ([]string, error)

	// RemoveObjects removes objects with the prefix
	RemoveObjects(prefix string) error

	// RemoveObject removes one object
	RemoveObject(path string) error
}

// object store back-end details
type client struct {
	accessID       string
	secretKey      string
	connRetries    int
	tlsConfig      *tls.Config
	bucketName     string
	client         objStoreBackend
	resolverClient resolver.Interface
}

// Option provides optional parameters to the client constructor
type Option func(c *client)

// WithTLSConfig provides a custom TLS configuration for the client to use when
// contacting the backend. Can be used to provide custom trust roots, server names,
// cipher configs, etc.
func WithTLSConfig(tlsConfig *tls.Config) Option {
	return func(c *client) {
		c.tlsConfig = tlsConfig
	}
}

// WithConnectRetries sets the number of connect retries for the client.
func WithConnectRetries(count int) Option {
	return func(c *client) {
		c.connRetries = count
	}
}

// NewClient creates a new client to the Venice object store
// tenant name and service name is used to form the bucket as "tenantName.serviceName"
func NewClient(tenantName, serviceName string, resolver resolver.Interface, opts ...Option) (Client, error) {

	// TODO: validate bucket name
	// TODO: get access id & secret key from object store

	// Initialize client object.
	c := &client{
		accessID:       "miniokey",
		secretKey:      "minio0523",
		bucketName:     fmt.Sprintf("%s.%s", tenantName, serviceName),
		resolverClient: resolver,
		connRetries:    maxRetry,
	}

	for _, o := range opts {
		o(c)
	}

	if err := c.connect(); err != nil {
		return nil, err
	}

	return c, nil
}

// connect() resolves object store name to URL and connects
func (c *client) connect() error {
	// resolve name
	addr, err := c.getObjStoreAddr()
	if err != nil {
		return err
	}

	// Fisher-Yates shuffle
	rand.Seed(time.Now().UnixNano())
	rand.Shuffle(len(addr), func(i, j int) {
		addr[i], addr[j] = addr[j], addr[i]
	})

	objsLog.Infof("{%s} urls %+v ", globals.VosMinio, addr)

	// connect and check
	for _, url := range addr {
		for i := 0; i < c.connRetries; i++ {
			objsLog.Infof("connecting to {%s} %s", globals.VosMinio, url)
			mc, err := minio.NewClient(url, c.accessID, c.secretKey, c.tlsConfig, c.bucketName)
			if err != nil {
				objsLog.Warnf("failed to create client to %s, %s", url, err)
				time.Sleep(time.Second * 1)
				continue
			}

			// update client
			c.client = mc
			return nil
		}
	}

	return fmt.Errorf("failed to connect to any of the object stores %+v", addr)
}

// getObjStoreAddr get object store URLs from the resolver
func (c *client) getObjStoreAddr() ([]string, error) {
	for i := 0; i < maxRetry; i++ {
		addr := c.resolverClient.GetURLs(globals.VosMinio)
		if len(addr) > 0 {
			return addr, nil
		}
		time.Sleep(time.Second * 1)
		objsLog.Warnf("failed to get {%s} url, retrying", globals.VosMinio)
	}

	return []string{}, fmt.Errorf("failed to get object store url from resolver")
}

// PutObject uploads an object to object store
// on connect error, this function loops through all available object store back-ends
// PUT request header is limited to 8 KB in size.
// Within the PUT request header, the user-defined metadata is limited to 2 KB in size.
// The size of user-defined metadata is measured by taking the sum of the number of bytes in the
// UTF-8 encoding of each key and value.
func (c *client) PutObject(ctx context.Context, objectName string, reader io.Reader, userMeta map[string]string) (int64, error) {
	for retry := 0; retry < maxRetry; retry++ {
		n, err := c.client.PutObject(ctx, objectName, reader, userMeta)
		if err == nil {
			return n, err
		}

		if !strings.Contains(err.Error(), connectErr) {
			return 0, err
		}
		if err := c.connect(); err != nil {
			return 0, err
		}
	}

	return 0, fmt.Errorf("maximum retries exceeded to upload %s", objectName)
}

// PutObjectOfSize uploads object of "size" to object store
func (c *client) PutObjectOfSize(ctx context.Context, objectName string, reader io.Reader, size int64, userMeta map[string]string) (int64, error) {
	for retry := 0; retry < maxRetry; retry++ {
		n, err := c.client.PutObjectOfSize(ctx, objectName, reader, size, userMeta)
		if err == nil {
			return n, err
		}

		if !strings.Contains(err.Error(), connectErr) {
			return 0, err
		}
		if err := c.connect(); err != nil {
			return 0, err
		}
	}

	return 0, fmt.Errorf("maximum retries exceeded to upload %s", objectName)
}

// PutObjectExplicit uploads an object to object store under the given bucket name (i.e. serviceName)
func (c *client) PutObjectExplicit(ctx context.Context,
	serviceName string, objectName string, reader io.Reader, metaData map[string]string) (int64, error) {
	for retry := 0; retry < maxRetry; retry++ {
		n, err := c.client.PutObjectExplicit(ctx, serviceName, objectName, reader, metaData)
		if err == nil {
			return n, err
		}

		if !strings.Contains(err.Error(), connectErr) {
			return 0, err
		}
		if err := c.connect(); err != nil {
			return 0, err
		}
	}

	return 0, fmt.Errorf("maximum retries exceeded to upload %s under bucket %s", objectName, serviceName)
}

type streamObj struct {
	ctx        context.Context
	client     *client
	objectName string
	offset     int
	metaData   map[string]string
}

// Write uploads an object to object store
// object name is formed from the object name and the current offset
func (s *streamObj) Write(b []byte) (int, error) {
	buff := bytes.NewBuffer(b)
	objName := objNameToStreamObjName(s.objectName, s.offset)

	n, err := s.client.PutObject(s.ctx, objName, buff, s.metaData)
	if err != nil {
		return 0, fmt.Errorf("failed to upload object %s, %s", objName, err)
	}

	if _, err := s.client.setInProgress(s.ctx, s.objectName, s.offset+1); err != nil {
		return 0, fmt.Errorf("failed to upload progress file for %s, %s", s.objectName, err)
	}
	s.offset++

	return int(n), nil
}

// Close closes the stream and removes in-progress flag
func (s *streamObj) Close() error {
	return s.client.clearInProgress(s.objectName)
}

func objNameToStreamObjName(objName string, offset int) string {
	return fmt.Sprintf("%s/%s.%d", objName, filepath.Base(objName), offset)
}

func objNameToInProgress(objName string) string {
	return fmt.Sprintf("%s/%s", objName, inProgressFile)
}

// PutStreamObject creates in-progress file and returns a writer to the caller
// and every write() uploads a new object
func (c *client) PutStreamObject(ctx context.Context, objectName string, metaData map[string]string) (io.WriteCloser, error) {
	stObj := &streamObj{client: c, metaData: metaData, objectName: objectName, ctx: ctx}

	// check if we need to advance the offset
	offset, err := c.getInProgress(ctx, objectName)
	if err == nil {
		if offset != 0 {
			objsLog.Infof("putStreamObj change offset to %d for %s", offset, objectName)
			stObj.offset = offset
		}
	} else {
		if _, err := c.setInProgress(ctx, objectName, 0); err != nil {
			return nil, fmt.Errorf("failed to upload progress file for %s, error: %s", objectName, err)
		}
	}

	return stObj, nil
}

// GetObject downloads an object from the object store
// caller should close() after read()
func (c *client) GetObject(ctx context.Context, objectName string) (io.ReadCloser, error) {
	for retry := 0; retry < maxRetry; retry++ {
		reader, err := c.client.GetObject(ctx, objectName)
		if err == nil {
			return reader, err
		}
		if !strings.Contains(err.Error(), connectErr) {
			return nil, err
		}

		if err := c.connect(); err != nil {
			return nil, err
		}
	}

	return nil, fmt.Errorf("maximum retries exceeded to get %s", objectName)
}

func (c *client) checkInProgress(objectName string) bool {
	// stat progress file
	pgfile := objNameToInProgress(objectName)
	_, err := c.StatObject(pgfile)
	if err == nil {
		return true
	}
	return false
}

func (c *client) setInProgress(ctx context.Context, objectName string, offset int) (int64, error) {
	//  progress file
	pgfile := objNameToInProgress(objectName)
	buff := bytes.NewBufferString(fmt.Sprintf("%d", offset))
	return c.PutObject(ctx, pgfile, buff, nil)
}

func (c *client) getInProgress(ctx context.Context, objectName string) (int, error) {
	pgfile := objNameToInProgress(objectName)
	pgRead, err := c.GetObject(ctx, pgfile)
	if err != nil {
		return 0, err
	}

	b, err := ioutil.ReadAll(pgRead)
	if err != nil {
		return 0, fmt.Errorf("failed to read existing %s, error: %s", pgfile, err)
	}
	offset, err := strconv.Atoi(string(b))
	if err != nil {
		return 0, fmt.Errorf("failed to convert %s, error: %s", string(b), err)
	}
	return offset, nil
}

func (c *client) clearInProgress(objectName string) error {
	pgfile := objNameToInProgress(objectName)
	return c.RemoveObjects(pgfile)
}

// GetStreamObjectAtOffset downloads one object at offset from the object store
// caller should close() after read()
// returns:
// 	object found  --> returns object reader
// 	object not found and PutStreamObject() is in progress --> waits till context is canceled
// 	object is not found and PutStreamObject() is not in progress ---> error
func (c *client) GetStreamObjectAtOffset(ctx context.Context, objectName string, offset int) (io.ReadCloser, error) {
	stObjName := objNameToStreamObjName(objectName, offset)

	for ctx.Err() == nil {
		r, err := c.GetObject(ctx, stObjName)
		if err == nil {
			return r, nil
		}
		// object not found, retry if the upload is in progress
		if c.checkInProgress(objectName) != true {
			return nil, fmt.Errorf("failed to find stream object %s", stObjName)
		}

		time.Sleep(time.Millisecond * 500)
	}

	return nil, fmt.Errorf("context deadline exceeded to get %s, err %s", stObjName, ctx.Err())
}

// StatObject returns information about the object
func (c *client) StatObject(objectName string) (*ObjectStats, error) {

	pgfile := objNameToInProgress(objectName)
	stname := objNameToStreamObjName(objectName, 0)

	objList, err := c.ListObjects(objectName)
	if err != nil {
		return nil, err
	}

	// check for stream object
	for _, f := range objList {
		if f == pgfile {
			objectName = pgfile
		}

		if f == stname {
			objectName = stname
			break
		}
	}

	for retry := 0; retry < maxRetry; retry++ {
		obj, err := c.client.StatObject(objectName)
		if err == nil {
			return &ObjectStats{Size: obj.Size, LastModified: obj.LastModified, ContentType: obj.ContentType, MetaData: obj.UserMeta}, err
		}

		if !strings.Contains(err.Error(), connectErr) {
			return nil, err
		}

		if err := c.connect(); err != nil {
			return nil, err
		}
	}

	return nil, fmt.Errorf("maximum retries exceeded to stat %s", objectName)
}

// ListObject lists all objects with the given prefix
func (c *client) ListObjects(prefix string) ([]string, error) {
	for retry := 0; retry < maxRetry; retry++ {
		objs, err := c.client.ListObjects(prefix)
		if err == nil {
			return objs, err
		}

		if !strings.Contains(err.Error(), connectErr) {
			return nil, err
		}

		if err := c.connect(); err != nil {
			return nil, err
		}

	}

	return nil, fmt.Errorf("maximum retries exceeded to list %s", prefix)
}

// RemoveObjects deletes all objects with the given prefix
func (c *client) RemoveObjects(prefix string) error {
	for retry := 0; retry < maxRetry; retry++ {
		err := c.client.RemoveObjects(prefix)
		if err == nil {
			return nil
		}

		if !strings.Contains(err.Error(), connectErr) {
			return err
		}

		if err := c.connect(); err != nil {
			return err
		}
	}

	return fmt.Errorf("maximum retries exceeded to remove %s", prefix)
}

// RemoveObject removes one object
func (c *client) RemoveObject(path string) error {
	for retry := 0; retry < maxRetry; retry++ {
		err := c.client.RemoveObject(path)
		if err == nil {
			return nil
		}

		if !strings.Contains(err.Error(), connectErr) {
			return err
		}

		if err := c.connect(); err != nil {
			return err
		}
	}
	return fmt.Errorf("maximum retries exceeded to remove %s", path)
}
