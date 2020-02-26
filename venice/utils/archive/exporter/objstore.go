package exporter

import (
	"compress/gzip"
	"context"
	"fmt"
	"io"
	"path/filepath"
	"strconv"
	"strings"
	"time"

	"github.com/pensando/sw/api/generated/monitoring"
	objstoreapi "github.com/pensando/sw/api/generated/objstore"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/archive"
	"github.com/pensando/sw/venice/utils/log"
	objstore "github.com/pensando/sw/venice/utils/objstore/client"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

const (
	// VOSTargetKey is the name of the key that stores name of object
	VOSTargetKey = "vostarget"
)

// Option fills the optional params for object store exporter
type Option func(*objstoreExporter)

type objstoreExporter struct {
	rslvr          resolver.Interface
	logger         log.Logger
	objstoreClient objstore.Client
	bucket         string
}

// WithObjectstoreClient configures exporter with an object store client
func WithObjectstoreClient(client objstore.Client) Option {
	return func(e *objstoreExporter) {
		e.objstoreClient = client
	}
}

func (o *objstoreExporter) Export(ctx context.Context, reader archive.TransformReader, meta map[string]string) (string, int64, error) {
	target, ok := meta[VOSTargetKey]
	if !ok {
		target = strings.Replace(strconv.FormatInt(time.Now().Unix(), 10), " ", "-", -1)
	}
	target = fmt.Sprintf("%s.%s", target, "gz")
	r, w := io.Pipe()
	go func() {
		gzipWriter, _ := gzip.NewWriterLevel(w, gzip.DefaultCompression) // error will be nil if level is correct
		_, err := io.Copy(gzipWriter, reader)
		if err != nil {
			o.logger.ErrorLog("method", "Export", "msg", "error copying data to gzip writer", "error", err)
			gzipWriter.Close()
			w.CloseWithError(err)
			return
		}
		gzipWriter.Close()
		w.Close()
	}()
	defer r.Close()
	size, err := o.objstoreClient.PutObject(ctx, target, r, meta)
	if err != nil {
		o.logger.ErrorLog("method", "Export", "msg", "failed to create object in object store", "error", err)
		return "", size, err
	}
	o.logger.InfoLog("method", "Export", "msg", fmt.Sprintf("created object [%s] of size [%d] in bucket [%s] ", target, size, o.bucket))
	return fmt.Sprintf("/objstore/v1/downloads/%s/%s", o.bucket, target), size, err
}

func (o *objstoreExporter) Delete(req *monitoring.ArchiveRequest) error {
	target := filepath.Base(req.Status.URI) // returns . for empty URI
	if target != "" && target != "/" && target != "." && target != ".." {
		objURI := fmt.Sprintf("/objstore/v1/%s/objects/%s", o.bucket, target)
		o.logger.InfoLog("method", "Delete", "msg", fmt.Sprintf("deleting object %s", objURI))
		return o.objstoreClient.RemoveObject(objURI)
	}
	o.logger.ErrorLog("method", "Delete", "msg", fmt.Sprintf("incorrect object path [%s] in archive request", req.Status.URI))
	return fmt.Errorf("incorrect object path [%s] in archive request", req.Status.URI)
}

// NewObjstoreExporter returns an Exporter backed by object store. Data is compressed using gzip before saving to object store
func NewObjstoreExporter(reqType, tenant string, rslvr resolver.Interface, logger log.Logger, options ...Option) (archive.Exporter, error) {
	var bucket string
	switch reqType {
	case monitoring.ArchiveRequestSpec_Event.String():
		bucket = objstoreapi.Buckets_events.String()
	case monitoring.ArchiveRequestSpec_AuditEvent.String():
		bucket = objstoreapi.Buckets_auditevents.String()
	case monitoring.ArchiveRequestSpec_FwLog.String():
		bucket = objstoreapi.Buckets_fwlogs.String()
	}
	expter := &objstoreExporter{
		rslvr:  rslvr,
		logger: logger,
		bucket: bucket,
	}
	for _, o := range options {
		o(expter)
	}
	if expter.objstoreClient == nil {
		tlsp, err := rpckit.GetDefaultTLSProvider(globals.Vos)
		if err != nil {
			return nil, err
		}
		tlsc, err := tlsp.GetClientTLSConfig(globals.Vos)
		if err != nil {
			return nil, err
		}
		tlsc.ServerName = globals.Vos

		objstoreClient, err := objstore.NewClient(tenant, bucket, rslvr, objstore.WithTLSConfig(tlsc))
		if err != nil {
			return nil, err
		}
		expter.objstoreClient = objstoreClient
	}
	return expter, nil
}
