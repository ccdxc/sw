package vospkg

import (
	"bytes"
	"context"
	"fmt"
	"io"
	"mime/multipart"
	"net/http"
	"net/http/httptest"
	"testing"
	"time"

	"github.com/go-martini/martini"
	minioclient "github.com/minio/minio-go"
	"github.com/pkg/errors"

	"github.com/pensando/sw/api/generated/objstore"
	"github.com/pensando/sw/venice/vos"

	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestNewHandler(t *testing.T) {
	h, err := newHTTPHandler(nil, nil)
	AssertOk(t, err, "newHTTPHandler returned error")
	Assert(t, h != nil, "handler is nil")
	ctx, cancel := context.WithCancel(context.Background())
	// should start the server and return
	h.start(ctx, "0", nil)
	cancel()
}

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
	inst := &instance{}
	inst.Init(fb)
	srv := httpHandler{client: fb, instance: inst}
	srv.uploadHandler(wr, req, "images")
	Assert(t, wr.Code == http.StatusBadRequest, "invalid response for bad request [%v]", wr.Code)
	wr = httptest.NewRecorder()
	req, err = createUploadReq("/test/", metadata, "file", "file.test", "POST", []byte{})
	AssertOk(t, err, "failed to create request (%s)", err)
	srv.uploadHandler(wr, req, "images")
	Assert(t, wr.Code == http.StatusOK, "invalid response for bad request [%v][%v]", wr.Code, wr.Body.String())
	wr = httptest.NewRecorder()
	req, err = createUploadReq("/test/", metadata, "file", "file.test", "GET", []byte{})
	AssertOk(t, err, "failed to create request (%s)", err)
	srv.uploadHandler(wr, req, "images")
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
	srv.uploadHandler(wr, req, "images")
	Assert(t, wr.Code == http.StatusOK, "invalid response for good request [%v][%v]", wr.Code, wr.Body.String())
	fb.putSize = 1024
	fb.putErr = errors.New("some error")
	fb.statObject = 0
	wr = httptest.NewRecorder()
	req, err = createUploadReq("/test/", metadata, "file", "file.test", "POST", []byte{})
	AssertOk(t, err, "failed to create request (%s)", err)
	srv.uploadHandler(wr, req, "images")
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
	srv.uploadHandler(wr, req, "images")
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
	srv.uploadHandler(wr, req, "images")
	Assert(t, wr.Code == http.StatusInternalServerError, "invalid response for good request [%v][%v]", wr.Code, wr.Body.String())
}

func TestDownloadHandler(t *testing.T) {
	wr := httptest.NewRecorder()
	fb := &mockBackend{}
	inst := &instance{}
	inst.Init(fb)
	srv := httpHandler{client: fb, instance: inst}
	var cbErr1, cbErr2 error
	var cbcalled1, cbcalled2 int
	// Add plugin funcs
	cbFunc1 := func(ctx context.Context, oper vos.ObjectOper, in *objstore.Object, client vos.BackendClient) error {
		cbcalled1++
		return cbErr1
	}
	cbFunc2 := func(ctx context.Context, oper vos.ObjectOper, in *objstore.Object, client vos.BackendClient) error {
		cbcalled2++
		return cbErr2
	}
	inst.RegisterCb("images", vos.PreOp, vos.Download, cbFunc1)
	inst.RegisterCb("images", vos.PostOp, vos.Download, cbFunc2)

	req, err := http.NewRequest("GET", "/download/image", nil)
	AssertOk(t, err, "failed to create request")

	wr = httptest.NewRecorder()
	params := martini.Params(map[string]string{"_1": ""})
	srv.downloadHandler(params, wr, req)
	Assert(t, wr.Code == http.StatusBadRequest, "expecting failure due to no path")

	wr = httptest.NewRecorder()
	fso := &fakeStoreObj{}
	fb.fObj = fso
	fb.retErr = errors.New("some error")
	req, err = http.NewRequest("GET", "/download/image/some/path", nil)
	AssertOk(t, err, "failed to create request")
	params["_1"] = "/some/path"
	srv.downloadHandler(params, wr, req)
	Assert(t, wr.Code == http.StatusNotFound, "expecting failure due to not found got [%v]", wr.Code)

	wr = httptest.NewRecorder()

	cbErr1 = fb.retErr
	fb.retErr = nil
	wr = httptest.NewRecorder()
	srv.downloadHandler(params, wr, req)
	Assert(t, wr.Code == http.StatusPreconditionFailed, "expecting failure due to not found got [%v]", wr.Code)

	cnt := 0
	testStr := []byte("aabababababababaabababababaababababababaabababababa")
	readErr := io.EOF
	fso.readFn = func(in []byte) (int, error) {
		in = testStr
		var err error
		if cnt == 3 {
			err = readErr
		}

		if cnt > 3 {
			return 0, io.EOF
		}
		cnt++
		return len(testStr), err
	}
	wr = httptest.NewRecorder()
	cbcalled1, cbcalled2, cnt = 0, 0, 0
	cbErr1 = nil
	srv.downloadHandler(params, wr, req)
	AssertOk(t, err, "DownloadFile failed")
	Assert(t, cbcalled1 == 1, "exepecting 1 call for preop got [%d]", cbcalled1)
	Assert(t, cbcalled2 == 1, "exepecting 1 call for postop got [%d]", cbcalled2)
	Assert(t, wr.Code == http.StatusOK, "expecting downlod to succeed got  [%v]", wr.Code)

	cbErr2 = errors.New("some error")
	wr = httptest.NewRecorder()
	cbcalled1, cbcalled2, cnt = 0, 0, 0
	srv.downloadHandler(params, wr, req)
}
