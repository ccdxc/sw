package cluster

import (
	"bytes"
	"context"
	"crypto/md5"
	"crypto/tls"
	"fmt"
	"io"
	"math/rand"
	"mime/multipart"
	"net/http"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"github.com/pensando/sw/api/generated/objstore"
	penctx "github.com/pensando/sw/api/login/context"
	"github.com/pensando/sw/venice/utils/netutils"

	"github.com/pkg/errors"
)

func uploadFile(ctx context.Context, filename string, metadata map[string]string, content []byte) (int, error) {

	body := &bytes.Buffer{}
	writer := multipart.NewWriter(body)
	part, err := writer.CreateFormFile("file", filename)
	if err != nil {
		return 0, errors.Wrap(err, "CreateFormFile failed")
	}
	written, err := part.Write(content)
	if err != nil {
		return 0, errors.Wrap(err, "writing form")
	}

	for key, val := range metadata {
		_ = writer.WriteField(key, val)
	}
	err = writer.Close()
	if err != nil {
		return 0, errors.Wrap(err, "closing writer")
	}
	uri := fmt.Sprintf("https://%s/objstore/v1/uploads/", ts.tu.APIGwAddr)
	req, err := http.NewRequest("POST", uri, body)
	if err != nil {
		return 0, errors.Wrap(err, "http.newRequest failed")
	}
	req.Header.Set("Content-Type", writer.FormDataContentType())
	transport := &http.Transport{TLSClientConfig: &tls.Config{InsecureSkipVerify: true}}
	client := &http.Client{Transport: transport}
	_, err = client.Do(req)
	if err != nil {
		return 0, errors.Wrap(err, "Sending req")
	}
	return written, nil
}

// downloadFile dowloads file from the objstore and returns the md5 hash and total size of the downloaded content
func downloadFile(filename string) ([]byte, int, error) {
	h := md5.New()
	ctx := context.Background()
	fr, err := ts.tu.VOSClient.GetObject(ctx, filename)
	if err != nil {
		return nil, 0, err
	}
	buf := make([]byte, 1024)
	totsize := 0
	for {
		n, err := fr.Read(buf)
		if err != nil && err != io.EOF {
			return nil, 0, err
		}
		if n == 0 {
			break
		}
		totsize += n
		if _, err = h.Write(buf[:n]); err != nil {
			return nil, 0, err
		}
	}
	return h.Sum(nil), totsize, nil
}

func createBuffer(size int) []byte {
	var letters = []rune("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ")
	b := make([]rune, size)
	for i := range b {
		b[i] = letters[rand.Intn(len(letters))]
	}
	return []byte(string(b))
}

func statFile(ctx context.Context, filename string) (*objstore.Object, error) {
	restcl := netutils.NewHTTPClient()
	authzHeader, ok := penctx.AuthzHeaderFromContext(ctx)
	if !ok {
		return nil, fmt.Errorf("no authorizaton header in context")
	}
	restcl.SetHeader("Authorization", authzHeader)
	restcl.WithTLSConfig(&tls.Config{InsecureSkipVerify: true})
	uri := fmt.Sprintf("https://%s/objstore/v1/images/objects/%s", ts.tu.APIGwAddr, filename)
	By(fmt.Sprintf("fetch stat for file [%s][%s]", filename, uri))
	resp := &objstore.Object{}
	_, err := restcl.Req("GET", uri, nil, &resp)
	Expect(err).Should(BeNil(), "Failed to rest query [%v]", uri)
	return resp, nil
}

var _ = Describe("Objstore Write and read test", func() {
	BeforeEach(func() {
		// Close if there is an existing client and reestablish a client connection.
		// This temporarily worksaround the issue that the resolution of the destination URI happens
		// in the objstore client at the time of creation of the client. Test could have changed the
		// location of the objstore.
		err := ts.tu.SetupObjstoreClient()
		Expect(err).Should(BeNil(), "failed to create objstore client")
	})

	It("Exercise objstore Upload/Dowload", func() {
		filename := "e2etest.file"
		metadata := map[string]string{
			"Version":     "v1.3.2",
			"Environment": "production",
			"Description": "E2E test Image upload",
			"Releasedate": "May2018",
		}
		buf := createBuffer(100 * 1024)
		h := md5.New()
		h.Write(buf)
		hash := h.Sum(nil)
		ctx := ts.tu.NewLoggedInContext(context.Background())
		wr, err := uploadFile(ctx, filename, metadata, buf)
		Expect(err).Should(BeNil(), "Failed to upload file")
		rhash, rd, err := downloadFile(filename)
		Expect(err).Should(BeNil(), "Failed to download file")
		Expect(hash).Should(Equal(rhash), "uploaded and downloaded hashes do not match")
		Expect(rd).Should(Equal(wr), "size written does not match size read")
		resp, err := statFile(ctx, filename)
		Expect(err).Should(BeNil(), "failed to stat file")
		for k, v := range metadata {
			if v1, ok := resp.Labels[k]; !ok {
				Fail(fmt.Sprintf("did not find [%v] in labels [%+v]", k, resp))
			} else {
				if v1 != v {
					Fail(fmt.Sprintf("did not match [%v] got[%v] want[%v] in labels", k, v1, v))
				}
			}
		}
	})
})
