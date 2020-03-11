// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package minio

import (
	"context"
	"crypto/tls"
	"fmt"
	"io"
	"net/http"
	"strings"
	"time"

	"github.com/minio/minio-go"
)

// Client is the object store handle
type Client struct {
	bucketName string
	location   string
	client     *minio.Client
}

// ObjectStats returned from stats API
type ObjectStats struct {
	LastModified time.Time         // Date and time the object was last modified.
	Size         int64             // Size in bytes of the object.
	ContentType  string            // A standard MIME type describing the format of the object data.
	UserMeta     map[string]string // user metadata
}

const (
	amzPrefix     = "x-amz-"
	amzMetaPrefix = "x-amz-meta-"
)

// NewClient creates a new client to minio
func NewClient(url, accessID, secretKey string, tlsConfig *tls.Config, bucketName string) (*Client, error) {
	mc, err := minio.New(url, accessID, secretKey, (tlsConfig != nil))
	if err != nil {
		return nil, fmt.Errorf("minio client, err:%s", err)
	}

	if tlsConfig != nil {
		tr := &http.Transport{
			TLSClientConfig:    tlsConfig,
			DisableCompression: true,
		}
		mc.SetCustomTransport(tr)
	}

	// Current minio-go client does not support ListBucketsWithContext, upgrade it.
	// This code may leak the goroutine internally started by ListBuckets.
	// Once minio-go client is upgraded, remove this code.
	output := make(chan error)
	go func() {
		if _, err = mc.ListBuckets(); err != nil {
			output <- fmt.Errorf("list buckets err: %s", err)
		}
		output <- nil
	}()

	ctxNew, cancel := context.WithTimeout(context.Background(), time.Second*30)
	defer cancel()

	select {
	case <-ctxNew.Done():
		return nil, fmt.Errorf("connect: listbuckets context timed out")
	case err := <-output:
		if err != nil {
			return nil, err
		}
	}

	c := &Client{
		bucketName: bucketName,
		client:     mc,
		location:   "us-east-1",
	}

	return c, nil
}

// PutObject uploads an object to object store
// metadata shouldn't be used for storing large data
func (c *Client) PutObject(ctx context.Context, objectName string, reader io.Reader, userMeta map[string]string) (int64, error) {
	// check bucket
	s, err := c.client.BucketExists(c.bucketName)
	if err != nil {
		return 0, fmt.Errorf("failed to get bucket details, err:%s", err)
	}

	if !s {
		if err := c.client.MakeBucket(c.bucketName, c.location); err != nil {
			return 0, err
		}
	}

	// update the metadata
	metaData := map[string]string{}
	for k, v := range userMeta {
		// store system meta as is
		if strings.HasPrefix(strings.ToLower(k), amzPrefix) {
			metaData[k] = v
		} else {
			metaData[amzMetaPrefix+k] = v
		}
	}

	n, err := c.client.PutObjectWithContext(ctx, c.bucketName, objectName, reader, -1, minio.PutObjectOptions{UserMetadata: metaData})
	return n, err
}

// PutObjectOfSize - puts an object of "size" in the objectstore
func (c *Client) PutObjectOfSize(ctx context.Context, objectName string, reader io.Reader, size int64, userMeta map[string]string) (int64, error) {
	// check bucket
	s, err := c.client.BucketExists(c.bucketName)
	if err != nil {
		return 0, fmt.Errorf("failed to get bucket details, err:%s", err)
	}

	if !s {
		if err := c.client.MakeBucket(c.bucketName, c.location); err != nil {
			return 0, err
		}
	}

	// update the metadata
	metaData := map[string]string{}
	for k, v := range userMeta {
		// store system meta as is
		if strings.HasPrefix(strings.ToLower(k), amzPrefix) {
			metaData[k] = v
		} else {
			metaData[amzMetaPrefix+k] = v
		}
	}

	n, err := c.client.PutObjectWithContext(ctx, c.bucketName, objectName, reader, size, minio.PutObjectOptions{UserMetadata: metaData})
	return n, err
}

// PutObjectExplicit uploads an object to object store under the given bucket name (i.e. serviceName)
// metadata shouldn't be used for storing large data
func (c *Client) PutObjectExplicit(ctx context.Context,
	serviceName string, objectName string, reader io.Reader, size int64, userMeta map[string]string) (int64, error) {
	// check bucket
	s, err := c.client.BucketExists(serviceName)
	if err != nil {
		return 0, fmt.Errorf("failed to get bucket details, err:%s", err)
	}

	if !s {
		if err := c.client.MakeBucket(serviceName, c.location); err != nil {
			return 0, err
		}
	}

	// update the metadata
	metaData := map[string]string{}
	for k, v := range userMeta {
		// store system meta as is
		if strings.HasPrefix(strings.ToLower(k), amzPrefix) {
			metaData[k] = v
		} else {
			metaData[amzMetaPrefix+k] = v
		}
	}

	n, err := c.client.PutObjectWithContext(ctx, serviceName, objectName, reader, size, minio.PutObjectOptions{UserMetadata: metaData})
	return n, err
}

// GetObject gets the object from object store, caller should close() after readall()
func (c *Client) GetObject(ctx context.Context, objectName string) (io.ReadCloser, error) {
	_, err := c.client.StatObject(c.bucketName, objectName, minio.StatObjectOptions{})
	if err != nil {
		return nil, fmt.Errorf("stat object, err:%s", err)
	}

	objReader, err := c.client.GetObjectWithContext(ctx, c.bucketName, objectName, minio.GetObjectOptions{})
	if err != nil {
		return nil, fmt.Errorf("get object, err:%s", err)
	}
	return objReader, nil
}

// StatObject returns information about the object
func (c *Client) StatObject(objectName string) (*ObjectStats, error) {
	objInfo, err := c.client.StatObject(c.bucketName, objectName, minio.StatObjectOptions{})
	if err != nil {
		return nil, err
	}

	userMeta := make(map[string]string)
	for k, v := range objInfo.Metadata {
		if strings.HasPrefix(strings.ToLower(k), amzMetaPrefix) {
			k = k[len(amzMetaPrefix):]
			userMeta[k] = strings.Join(v, "")
		}
	}

	return &ObjectStats{Size: objInfo.Size, LastModified: objInfo.LastModified, ContentType: objInfo.ContentType,
		UserMeta: userMeta}, nil
}

// ListObjects lists all objects with the given prefix
func (c *Client) ListObjects(prefix string) ([]string, error) {
	objList := []string{}

	if _, err := c.client.ListBuckets(); err != nil {
		return objList, fmt.Errorf("list buckets, err: %s", err)
	}

	doneCh := make(chan struct{})
	defer close(doneCh)

	objChan := c.client.ListObjects(c.bucketName, prefix, true, doneCh)
	for objInfo := range objChan {
		objList = append(objList, objInfo.Key)
	}

	return objList, nil
}

// RemoveObjects deletes all objects with the given prefix
func (c *Client) RemoveObjects(prefix string) error {
	errMsg := []string{}
	objList, err := c.ListObjects(prefix)
	if err != nil {
		return err
	}

	for _, obj := range objList {
		if err = c.client.RemoveObject(c.bucketName, obj); err != nil {
			errMsg = append(errMsg, fmt.Sprintf("%s: %s", obj, err))
		}
	}

	if len(errMsg) > 0 {
		return fmt.Errorf("%s", strings.Join(errMsg, ","))
	}

	return nil
}

// RemoveObject deletes one object
func (c *Client) RemoveObject(name string) error {
	return c.client.RemoveObject(c.bucketName, name)
}
