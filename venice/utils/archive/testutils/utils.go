package testutils

import (
	"bytes"
	"compress/gzip"
	"context"
	"errors"
	"fmt"
	"io"
	"strings"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/utils/archive"
	"github.com/pensando/sw/venice/utils/testutils"
)

// CreateArchiveRequest creates an archive request
func CreateArchiveRequest(ctx context.Context, apicl apiclient.Services, req *monitoring.ArchiveRequest) (*monitoring.ArchiveRequest, error) {
	var err error
	var createdReq *monitoring.ArchiveRequest
	if !testutils.CheckEventually(func() (bool, interface{}) {
		createdReq, err = apicl.MonitoringV1().ArchiveRequest().Create(ctx, req)
		if err != nil {
			return false, err
		}
		return true, createdReq
	}, "100ms", "20s") {
		return nil, err
	}
	return createdReq, nil
}

// MustCreateArchiveRequest raises a panic if archive request creation fails
func MustCreateArchiveRequest(ctx context.Context, apicl apiclient.Services, req *monitoring.ArchiveRequest) (*monitoring.ArchiveRequest, error) {
	createdReq, err := CreateArchiveRequest(ctx, apicl, req)
	if err != nil {
		panic(fmt.Sprintf("error creating archive request %v", err))
	}
	return createdReq, err
}

// DeleteArchiveRequest creates an archive request
func DeleteArchiveRequest(ctx context.Context, apicl apiclient.Services, meta *api.ObjectMeta) (*monitoring.ArchiveRequest, error) {
	var err error
	var deletedReq *monitoring.ArchiveRequest
	if !testutils.CheckEventually(func() (bool, interface{}) {
		deletedReq, err = apicl.MonitoringV1().ArchiveRequest().Delete(ctx, meta)
		if err != nil {
			return false, err
		}
		return true, deletedReq
	}, "100ms", "20s") {
		return nil, err
	}
	return deletedReq, nil
}

// MustDeleteArchiveRequest raises a panic if archive request creation fails
func MustDeleteArchiveRequest(ctx context.Context, apicl apiclient.Services, meta *api.ObjectMeta) (*monitoring.ArchiveRequest, error) {
	deletedReq, err := DeleteArchiveRequest(ctx, apicl, meta)
	if err != nil {
		panic(fmt.Sprintf("error deleting archive request %v", err))
	}
	return deletedReq, err
}

// ExtractArchive un-compresses gzip file in memory and returns a string
func ExtractArchive(r io.ReadCloser) (string, error) {
	var b bytes.Buffer
	_, err := io.Copy(&b, r)
	if err != nil {
		return "", err
	}
	// gzip reader needs ByteReader interface
	gr, err := gzip.NewReader(&b)
	if err != nil {
		return "", err
	}
	var unzippedBuf bytes.Buffer
	_, err = io.Copy(&unzippedBuf, gr)
	if err != nil {
		return "", err
	}
	return unzippedBuf.String(), nil
}

// ExtractObjectNameFromURI gets object name from URI
func ExtractObjectNameFromURI(uri string) (string, error) {
	toks := strings.Split(uri, "/")
	if len(toks) <= 1 {
		return "", errors.New("unexpected number of tokens")
	}
	return toks[len(toks)-1], nil
}

// PrintJobs prints archive requests in the running jobs
func PrintJobs(jobs []archive.Job) string {
	var buf bytes.Buffer
	for _, j := range jobs {
		buf.WriteString(fmt.Sprintf("%#v\n", *j.GetArchiveRequest()))
	}
	return buf.String()
}
