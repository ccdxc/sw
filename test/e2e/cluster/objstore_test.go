package cluster

import (
	"bytes"
	"context"
	"crypto/md5"
	"crypto/sha256"
	"crypto/tls"
	"encoding/hex"
	"encoding/json"
	"fmt"
	"io"
	"io/ioutil"
	"math/rand"
	"mime/multipart"
	"net/http"
	"os"
	"os/exec"
	"sort"
	"strings"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/venice/globals"

	"github.com/pensando/sw/venice/utils/log"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"github.com/pensando/sw/api/generated/objstore"
	"github.com/pensando/sw/api/generated/search"
	loginctx "github.com/pensando/sw/api/login/context"
	"github.com/pensando/sw/venice/utils/netutils"

	"github.com/pkg/errors"
)

func createBundle() (int, error) {

	venicefile := "venice.tgz"
	naplesfile := "naples_fw.tar"
	metafile := "metadata.json"
	bundlefile := "bundle.tar"
	veniceOSfile := "venice_appl_os.tgz"

	buf := createBuffer(100 * 1024)
	h := sha256.New()
	h.Write(buf)
	hash := hex.EncodeToString(h.Sum(nil))

	err := ioutil.WriteFile(venicefile, buf, 0644)
	if err != nil {
		return 0, errors.Wrap(err, "writing venice file")
	}

	err = ioutil.WriteFile(naplesfile, buf, 0644)
	if err != nil {
		return 0, errors.Wrap(err, "writing naples file")
	}

	err = ioutil.WriteFile(veniceOSfile, buf, 0644)
	if err != nil {
		return 0, errors.Wrap(err, "writing veniceOS file")
	}

	meta := map[string]map[string]string{
		"Bundle": {"Version": "1.0.0",
			"Description": "Meta File",
			"ReleaseDate": "May2019",
			"Environment": "Production",
			"Name":        "metadata.json"},
		"Venice": {"Version": "3.2.0",
			"Description": "Venice Image",
			"ReleaseDate": "May2019",
			"Name":        "venice.tgz",
			"Environment": "Production",
			"hash":        hash},
		"veniceOS": {"Version": "3.2.0",
			"Description": "Venice Image",
			"ReleaseDate": "May2019",
			"Name":        "venice_appl_os.tgz",
			"Environment": "Production",
			"hash":        hash},
		"Naples": {"Version": "4.5.1",
			"Description": "Naples Image",
			"ReleaseDate": "May2019",
			"Name":        "naples_fw.tar",
			"Environment": "Production",
			"hash":        hash}}

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
	cmd := exec.Command("tar", "-cvf", "/tmp/"+bundlefile, metafile, venicefile, naplesfile, veniceOSfile)
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
	if err := os.Remove(veniceOSfile); err != nil {
		log.Errorf("[%s] remove failed %s", veniceOSfile, err)
	}

	return 0, nil
}

func uploadFile(ctx context.Context, bucket, filename string, metadata map[string]string, content []byte) (int, error) {

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
	uri := fmt.Sprintf("https://%s/objstore/v1/uploads/%s/", ts.tu.APIGwAddr, bucket)
	By(fmt.Sprintf("upload URI [%v]", uri))
	req, err := http.NewRequest("POST", uri, body)
	if err != nil {
		return 0, errors.Wrap(err, "http.newRequest failed")
	}
	authzHeader, ok := loginctx.AuthzHeaderFromContext(ctx)
	if !ok {
		return 0, fmt.Errorf("no authorization header in context")
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

// downloadFile downloads file from the objstore and returns the md5 hash and total size of the downloaded content
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
	defer restcl.CloseIdleConnections()
	authzHeader, ok := loginctx.AuthzHeaderFromContext(ctx)
	if !ok {
		return nil, fmt.Errorf("no authorization header in context")
	}
	restcl.SetHeader("Authorization", authzHeader)
	restcl.WithTLSConfig(&tls.Config{InsecureSkipVerify: true})
	restcl.DisableKeepAlives()
	uri := fmt.Sprintf("https://%s/objstore/v1/images/objects/%s", ts.tu.APIGwAddr, filename)
	By(fmt.Sprintf("fetch stat for file [%s][%s]", filename, uri))
	resp := &objstore.Object{}
	_, err := restcl.Req("GET", uri, nil, &resp)
	Expect(err).Should(BeNil(), "Failed to rest query [%v]", uri)
	return resp, nil
}

func getObjstoreSearchCount() (int64, error) {
	query := &search.SearchRequest{
		Query: &search.SearchQuery{
			Categories: []string{"Objstore"},
		},
		Mode: search.SearchRequest_Full.String(),
	}
	resp := &search.SearchResponse{}
	err := ts.tu.Search(ts.loggedInCtx, query, resp)
	if err != nil {
		return 0, err
	}
	return resp.ActualHits, nil
}

func testObjCUDOps(testSearch bool) func() {
	// There is a known issue where after reboot some minio servers may not get
	// object events (VS-340)
	// For now, we don't test spyglass indexing during VOS restart test
	return func() {
		filename := "bundle.tar"
		version := "1.0.0"
		metadata := map[string]string{
			"Version":     version,
			"Environment": "production",
			"Description": "E2E test Image upload",
			"Releasedate": "May2018",
		}

		var startingSearchCount int64
		var err error

		// Getting current number of search entries for objstore
		if testSearch {
			startingSearchCount, err = getObjstoreSearchCount()
			Expect(err).To(BeNil())
		}

		_, err = createBundle()
		if err != nil {
			log.Infof("Error (%+v) creating file /tmp/bundle.tar", err)
			Fail(fmt.Sprintf("Failed to create /tmp/bundle.tar"))
		}

		fileBuf, err := ioutil.ReadFile("/tmp/" + filename)
		if err != nil {
			log.Infof("Error (%+v) reading file /tmp/bundle.tar", err)
			Fail(fmt.Sprintf("Failed to read /tmp/bundle.tar"))
		}

		h := md5.New()
		h.Write(fileBuf)

		ctx := ts.tu.MustGetLoggedInContext(context.Background())
		apigwAddr := ts.tu.ClusterVIP + ":" + globals.APIGwRESTPort
		restClient, err := apiclient.NewRestAPIClient(apigwAddr)
		Expect(err).To(BeNil())

		// Use of Eventually is needed for all operations here because success of one objstore operation does not
		//  guarantee the next one will succeed, because the call may end up on a node that is rebooting.
		Eventually(func() error {
			_, err = uploadFile(ctx, "images", filename, metadata, fileBuf)
			return err
		}, 90, 1).Should(BeNil(), fmt.Sprintf("failed to upload file (%s)", err))

		// File should be searchable
		if testSearch {
			Eventually(func() error {
				count, err := getObjstoreSearchCount()
				if err != nil {
					return err
				}
				newEntries := count - startingSearchCount
				if newEntries <= 0 {
					return fmt.Errorf("Failed to find uploaded file, started with %v hits, got %v new hits", startingSearchCount, newEntries)
				}
				return nil
			}, 90, 1).Should(BeNil(), fmt.Sprintf("failed to find file in spyglass (%s)", err))
		}

		Eventually(func() error {
			_, _, err = downloadFile(version)
			return err
		}, 90, 1).Should(BeNil(), fmt.Sprintf("failed to download file (%s)", err))
		Eventually(func() error {
			_, err = statFile(ctx, version)
			return err
		}, 90, 1).Should(BeNil(), fmt.Sprintf("failed to stat file(%s)", err))

		objMeta := api.ObjectMeta{
			Name:      version,
			Tenant:    "default",
			Namespace: "images",
		}
		Eventually(func() error {
			_, err = restClient.ObjstoreV1().Object().Get(ctx, &objMeta)
			return err
		}, 90, 1).Should(BeNil(), fmt.Sprintf("failed to Get object (%s)", err))

		Eventually(func() error {
			_, err = restClient.ObjstoreV1().Object().Delete(ctx, &objMeta)
			return err
		}, 90, 1).Should(BeNil(), fmt.Sprintf("failed to Delete object (%s)", err))

		_, err = restClient.ObjstoreV1().Object().Get(ctx, &objMeta)
		Expect(err).ShouldNot(BeNil())

		if testSearch {
			// Search count should be back to starting amount
			Eventually(func() error {
				count, err := getObjstoreSearchCount()
				if err != nil {
					return err
				}
				if count != startingSearchCount {
					return fmt.Errorf("Objstore counts should have gone back down. started with %v hits, got %v hits", startingSearchCount, count)
				}
				return nil
			}, 90, 1).Should(BeNil(), fmt.Sprintf("file failed to be deleted in spyglass (%s)", err))
		}
	}
}

var _ = Describe("objstore write and read test", func() {
	getVosNodes := func() []string {
		var ret []string
		out := strings.Split(ts.tu.LocalCommandOutput("kubectl get pods -o wide --no-headers | grep pen-vos "), "\n")
		for _, line := range out {
			fields := strings.Fields(line)
			if len(fields) == 9 {
				ret = append(ret, fields[6])
			}
		}
		sort.Strings(ret)
		return ret
	}
	nodeid := 0
	var vosNodes []string

	killVosServerOnNode := func(node string) {
		ip := ts.tu.NameToIPMap[node]
		ts.tu.KillContainerOnNodeByName(ip, "pen-vos")
	}

	BeforeEach(func() {
		if ts.tu.NumQuorumNodes < 3 {
			Skip("No in distributed mode, skipping cluster tests")
			return
		}
		validateCluster()
		Eventually(func() string {
			vosNodes = getVosNodes()
			if len(vosNodes) != ts.tu.NumQuorumNodes {
				return fmt.Sprintf("expecting %d vos nodes, %d active", ts.tu.NumQuorumNodes, len(vosNodes))
			}
			return ""
		}, 30, 1).Should(BeEmpty(), fmt.Sprintf("Did not find Vos nodes on all Quorum nodes, found (%d)", len(vosNodes)))
		// Close if there is an existing client and reestablish a client connection.
		// This temporarily worksaround the issue that the resolution of the destination URI happens
		// in the objstore client at the time of creation of the client. Test could have changed the
		// location of the objstore.
		err := ts.tu.SetupObjstoreClient()
		Expect(err).Should(BeNil(), "failed to create objstore client")
	})

	It("Exercise objstore Upload/Download", func() {
		Skip("Skip this test until we fix #VS-340")
		testObjCUDOps(true)
	})

	It("Restart VOS backends and check CUD operations", func() {
		for nodeid < ts.tu.NumQuorumNodes {
			// restart
			By(fmt.Sprintf("Trying CUD ops on Object store after restart of node id [%d]", nodeid))
			Expect(len(vosNodes)).To(BeNumerically(">", nodeid), "invalid numnber of vos nodes")
			killVosServerOnNode(vosNodes[nodeid])
			Eventually(func() string {
				vosNodes = getVosNodes()
				if len(vosNodes) != ts.tu.NumQuorumNodes {
					return fmt.Sprintf("expecting %d vos nodes, %d active", ts.tu.NumQuorumNodes, len(vosNodes))
				}
				return ""
			}, 30, 1).Should(BeEmpty(), fmt.Sprintf("Did not find Vos nodes on all Quorum nodes, found (%d)", len(vosNodes)))
			testObjCUDOps(false)()
			nodeid++
		}
	})
	AfterEach(func() {
		validateCluster()
	})

})
