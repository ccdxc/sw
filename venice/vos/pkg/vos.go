package vos

import (
	"context"
	"crypto/tls"
	"fmt"
	"io"
	"net/http"
	"os"
	"strings"
	"time"

	minioclient "github.com/minio/minio-go"
	minio "github.com/minio/minio/cmd"
	"github.com/pkg/errors"

	"github.com/pensando/sw/api/generated/objstore"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
)

const (
	minioKey        = "miniokey"
	minioSecret     = "minio0523"
	defaultLocation = "default"
)

const (
	metaPrefix       = "X-Amz-Meta-"
	metaCreationTime = "Creation-Time"
	metaFileName     = "file"
	metaContentType  = "content-type"
)

// backendClient is an interface representing the backend minio service.
type backendClient interface {
	BucketExists(bucketName string) (bool, error)
	MakeBucket(bucketName string, location string) error
	RemoveBucket(bucketName string) error

	PutObject(bucketName, objectName string, reader io.Reader, objectSize int64, opts minioclient.PutObjectOptions) (n int64, err error)
	RemoveObject(bucketName, objectName string) error
	ListObjectsV2(bucketName, objectPrefix string, recursive bool, doneCh <-chan struct{}) <-chan minioclient.ObjectInfo
	StatObject(bucketName, objectName string, opts minioclient.StatObjectOptions) (minioclient.ObjectInfo, error)
}

func createBuckets(client backendClient) error {
	log.Infof("creating default buckets in minio")
	for _, n := range objstore.Buckets_name {
		name := "default." + strings.ToLower(n)
		ok, err := client.BucketExists(strings.ToLower(name))
		if err != nil {
			return errors.Wrap(err, "client error")
		}
		if !ok {
			err = client.MakeBucket(strings.ToLower(name), defaultLocation)
			if err != nil {
				return errors.Wrap(err, fmt.Sprintf("MakeBucket operation[%s]", name))
			}
		}
	}
	return nil
}

// New creaate an instance of obj store
func New(ctx context.Context, args []string) error {
	os.Setenv("MINIO_ACCESS_KEY", minioKey)
	os.Setenv("MINIO_SECRET_KEY", minioSecret)
	log.Infof("minio env: %+v", os.Environ())
	log.Infof("minio args:  %+v", args)
	go minio.Main(args)

	time.Sleep(2 * time.Second)
	url := "localhost:" + globals.VosMinioPort
	log.Infof("connecting to minio at [%v]", url)

	client, err := minioclient.New(url, minioKey, minioSecret, true)
	if err != nil {
		log.Errorf("Failed to create client (%s)", err)
		return errors.Wrap(err, "Failed to create Client")
	}
	defTr := http.DefaultTransport.(*http.Transport)
	defTr.TLSClientConfig = &tls.Config{InsecureSkipVerify: true}
	client.SetCustomTransport(defTr)

	grpcBackend, err := newGrpcServer(client)
	if err != nil {
		return errors.Wrap(err, "failed to start grpc listener")
	}

	httpBackend, err := newHTTPHandler(client)
	if err != nil {
		return errors.Wrap(err, "failed to start http listener")
	}

	err = createBuckets(client)
	if err != nil {
		return errors.Wrap(err, "create buckets")
	}
	grpcBackend.start(ctx)
	httpBackend.start(ctx)
	log.Infof("Initialization complete")
	<-ctx.Done()
	return nil
}
