package vos

import (
	"bytes"
	"fmt"
	"mime/multipart"
	"net/http"
	"net/http/httptest"
	"testing"
	"time"

	minioclient "github.com/minio/minio-go"
	"github.com/pkg/errors"

	. "github.com/pensando/sw/venice/utils/testutils"
)

func createUploadReq(uri string, params map[string]string, pname, fname, method string, content []byte) (*http.Request, error) {
	body := &bytes.Buffer{}
	writer := multipart.NewWriter(body)
	part, err := writer.CreateFormFile(pname, fname)
	if err != nil {
		return nil, err
	}
	part.Write(content)

	for key, val := range params {
		_ = writer.WriteField(key, val)
	}
	err = writer.Close()
	if err != nil {
		return nil, err
	}

	req, err := http.NewRequest(method, uri, body)
	req.Header.Set("Content-Type", writer.FormDataContentType())
	return req, err
}

func TestUploadHandler(t *testing.T) {
	metadata := map[string]string{
		"test1": "One",
		"test2": "Two",
	}

	req, err := createUploadReq("/test/", metadata, "badname", "file.test", "POST", []byte{})
	AssertOk(t, err, "failted to create request (%s)", err)
	wr := httptest.NewRecorder()
	fb := &mockBackend{}
	srv := httpHandler{client: fb}
	srv.uploadHandler(wr, req)
	Assert(t, wr.Code == http.StatusBadRequest, "invalid response for bad request [%v]", wr.Code)
	wr = httptest.NewRecorder()
	req, err = createUploadReq("/test/", metadata, "file", "file.test", "POST", []byte{})
	AssertOk(t, err, "failed to create request (%s)", err)
	srv.uploadHandler(wr, req)
	Assert(t, wr.Code == http.StatusOK, "invalid response for bad request [%v][%v]", wr.Code, wr.Body.String())
	wr = httptest.NewRecorder()
	req, err = createUploadReq("/test/", metadata, "file", "file.test", "GET", []byte{})
	AssertOk(t, err, "failed to create request (%s)", err)
	srv.uploadHandler(wr, req)
	Assert(t, wr.Code == http.StatusMethodNotAllowed, "invalid response for bad request [%v][%v]", wr.Code, wr.Body.String())
	httphdr := http.Header{}
	fb.statObject = 0
	tn := time.Now().Format(time.RFC3339Nano)
	for k, v := range metadata {
		httphdr[metaPrefix+k] = []string{v}
	}
	httphdr[metaPrefix+metaCreationTime] = []string{tn}
	sObjs := []*minioclient.ObjectInfo{
		nil,
		{
			ETag:     "abcdef",
			Key:      "file1",
			Metadata: httphdr,
			Size:     1024,
		},
	}
	fb.statObject = 0
	fb.statFunc = func(bucketName, objectName string, opts minioclient.StatObjectOptions) (minioclient.ObjectInfo, error) {
		if fb.statObject <= len(sObjs) {
			if sObjs[fb.statObject-1] != nil {
				return *sObjs[fb.statObject-1], nil
			}
			return minioclient.ObjectInfo{}, errors.New("not found")
		}
		return minioclient.ObjectInfo{}, fmt.Errorf("out of range [%d]", fb.statObject)
	}
	fb.putSize = 1024
	fb.putErr = nil
	fb.statObject = 0
	wr = httptest.NewRecorder()
	req, err = createUploadReq("/test/", metadata, "file", "file.test", "POST", []byte{})
	AssertOk(t, err, "failed to create request (%s)", err)
	srv.uploadHandler(wr, req)
	Assert(t, wr.Code == http.StatusOK, "invalid response for good request [%v][%v]", wr.Code, wr.Body.String())
	fb.putSize = 1024
	fb.putErr = errors.New("some error")
	fb.statObject = 0
	wr = httptest.NewRecorder()
	req, err = createUploadReq("/test/", metadata, "file", "file.test", "POST", []byte{})
	AssertOk(t, err, "failed to create request (%s)", err)
	srv.uploadHandler(wr, req)
	Assert(t, wr.Code == http.StatusInternalServerError, "invalid response for good request [%v][%v]", wr.Code, wr.Body.String())
	sObjs = []*minioclient.ObjectInfo{
		nil,
		nil,
	}
	fb.putErr = nil
	fb.statObject = 0
	wr = httptest.NewRecorder()
	req, err = createUploadReq("/test/", metadata, "file", "file.test", "POST", []byte{})
	AssertOk(t, err, "failed to create request (%s)", err)
	srv.uploadHandler(wr, req)
	Assert(t, wr.Code == http.StatusInternalServerError, "invalid response for good request [%v][%v]", wr.Code, wr.Body.String())

	fb.statObject = 0
	fb.putSize = 2048
	sObjs = []*minioclient.ObjectInfo{
		nil,
		{
			ETag:     "abcdef",
			Key:      "file1",
			Metadata: httphdr,
			Size:     1024,
		},
	}
	wr = httptest.NewRecorder()
	req, err = createUploadReq("/test/", metadata, "file", "file.test", "POST", []byte{})
	AssertOk(t, err, "failed to create request (%s)", err)
	srv.uploadHandler(wr, req)
	Assert(t, wr.Code == http.StatusInternalServerError, "invalid response for good request [%v][%v]", wr.Code, wr.Body.String())
}
