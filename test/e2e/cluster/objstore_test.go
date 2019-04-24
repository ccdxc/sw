package cluster

import (
	"bytes"
	"context"
	"crypto/md5"
	"crypto/tls"
	"encoding/json"
	"fmt"
	"io"
	"io/ioutil"
	"math/rand"
	"mime/multipart"
	"net/http"
	"os"
	"os/exec"

	"github.com/pensando/sw/venice/utils/log"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"github.com/pensando/sw/api/generated/objstore"
	loginctx "github.com/pensando/sw/api/login/context"
	penctx "github.com/pensando/sw/api/login/context"
	"github.com/pensando/sw/venice/utils/netutils"

	"github.com/pkg/errors"
)

func createBundle() (int, error) {

	venicefile := "venice.tgz"
	naplesfile := "naples_fw.tar"
	metafile := "metadata.json"
	bundlefile := "bundle.tar"

	buf := createBuffer(100 * 1024)

	err := ioutil.WriteFile(venicefile, buf, 0644)
	if err != nil {
		return 0, errors.Wrap(err, "writing venice file")
	}

	err = ioutil.WriteFile(naplesfile, buf, 0644)
	if err != nil {
		return 0, errors.Wrap(err, "writing naples file")
	}

	meta := map[string]map[string]string{
		"bundle": {"Version": "1.0.0",
			"Description": "Meta File",
			"ReleaseDate": "May2019",
			"Name":        "metadata.json"},

		"venice": {"Version": "3.2.0",
			"Description": "Venice Image",
			"ReleaseDate": "May2019",
			"Name":        "venice.tgz"},

		"naples": {"Version": "4.5.1",
			"Description": "Naples Image",
			"ReleaseDate": "May2019",
			"Name":        "naples_fw.tar"}}

	b, err := json.Marshal(meta)
	if err != nil {
		return 0, errors.Wrap(err, "failed to marshal meta info")
	}

	err = ioutil.WriteFile(metafile, b, 0644)
	if err != nil {
		return 0, errors.Wrap(err, "writing naples file")
	}
	if _, err = exec.LookPath("tar"); err != nil {
		log.Errorf("LookPath failed during extract err %v", err)
		return 0, errors.Wrap(err, "tar utility is not found")
	}
	cmd := exec.Command("tar", "-cvf", "/tmp/"+bundlefile, metafile, venicefile, naplesfile)
	output, err := cmd.CombinedOutput()
	if err != nil {
		log.Errorf("Failed to create bundle Image %s with output:%s errcode %v", bundlefile, string(output), err)
		return 0, errors.Wrap(err, "tar creation failed")
	}

	if err := os.Remove(metafile); err != nil {
		log.Errorf("[%s] remove failed %s", metafile, err)
	}
	if err := os.Remove(venicefile); err != nil {
		log.Errorf("[%s] remove failed %s", venicefile, err)
	}
	if err := os.Remove(naplesfile); err != nil {
		log.Errorf("[%s] remove failed %s", naplesfile, err)
	}

	return 0, nil
}

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
	uri := fmt.Sprintf("https://%s/objstore/v1/uploads/images/", ts.tu.APIGwAddr)
	req, err := http.NewRequest("POST", uri, body)
	if err != nil {
		return 0, errors.Wrap(err, "http.newRequest failed")
	}
	authzHeader, ok := loginctx.AuthzHeaderFromContext(ctx)
	if !ok {
		return 0, fmt.Errorf("no authorizaton header in context")
	}
	req.Header.Set("Authorization", authzHeader)
	req.Header.Set("Content-Type", writer.FormDataContentType())
	transport := &http.Transport{TLSClientConfig: &tls.Config{InsecureSkipVerify: true}}
	client := &http.Client{Transport: transport}
	resp, err := client.Do(req)
	if err != nil {
		return 0, errors.Wrap(err, "Sending req")
	}
	defer resp.Body.Close()
	if resp.StatusCode != http.StatusOK {
		body, _ := ioutil.ReadAll(resp.Body)
		By(fmt.Sprintf("Did not get a success on upload [%v]", string(body)))
		return 0, fmt.Errorf("failed to get upload [%v][%v]", resp.Status, string(body))
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

	It("Exercise Bundle Upload/Download", func() {
		filename := "bundle.tar"
		metadata := map[string]string{
			"Version":     "v1.3.2",
			"Environment": "production",
			"Description": "E2E bundle Image upload",
			"Releasedate": "May2018",
		}
		_, err := createBundle()
		if err != nil {
			log.Infof("Error (%+v) creating file /tmp/bundle.tar", err)
			Fail(fmt.Sprintf("Failed to create /tmp/bundle.tar"))
		}
		fileBuf, err := ioutil.ReadFile("/tmp/" + filename)
		if err != nil {
			log.Infof("Error (%+v) reading file /tmp/bundle.tar", err)
			Fail(fmt.Sprintf("Failed to read /tmp/bundle.tar"))
		}
		ctx := ts.tu.NewLoggedInContext(context.Background())
		_, err = uploadFile(ctx, filename, metadata, fileBuf)
		Expect(err).Should(BeNil(), "Failed to upload file")
	})

})
