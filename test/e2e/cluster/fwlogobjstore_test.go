// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.

package cluster

import (
	"compress/gzip"
	"context"
	"crypto/tls"
	"encoding/csv"
	"encoding/json"
	"fmt"
	"strconv"
	"strings"
	"time"

	es "github.com/olivere/elastic"
	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"github.com/pensando/sw/api"
	// es "github.com/olivere/elastic"
	"github.com/pensando/sw/nic/agent/tmagent/state/fwgen/fwevent"
	types "github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/elastic"
	"github.com/pensando/sw/venice/utils/log"
	objstore "github.com/pensando/sw/venice/utils/objstore/client"
	mockresolver "github.com/pensando/sw/venice/utils/resolver/mock"
)

const (
	timeFormat             = "2006-01-02T15:04:05"
	bucketPrefix           = "default"
	metaBucketPrefix       = "meta-default"
	bucketName             = "fwlogs"
	elasticContextDeadline = 90 * time.Second
)

var _ = Describe("tests for storing firewall logs in object store", func() {

	var (
		esClient        elastic.ESClient
		fwLogClient     objstore.Client
		fwLogMetaClient objstore.Client
	)

	Context("firewall logs with object store and elastic", func() {
		BeforeEach(func() {
			validateCluster()
			if esClient == nil {
				esClient = setupElasticClient()
			}

			if fwLogClient == nil && fwLogMetaClient == nil {
				fwLogClient, fwLogMetaClient = setupFwLogsClients()
			}
		})

		It("push fwlogs", func() {
			snIf := ts.tu.APIClient.ClusterV1().DistributedServiceCard()
			snics, _ := snIf.List(context.Background(), &api.ListWatchOptions{})
			for _, snic := range snics {
				napleID := snic.Status.PrimaryMAC
				fmt.Println("Bucket name", bucketPrefix+"."+bucketName)

				// get the current list of objects in objectstore
				currentCount, err := getObjectCount(fwLogClient, "")
				Expect(err).NotTo(HaveOccurred())

				currentMetaCount, err := getObjectCount(fwLogMetaClient, "")
				Expect(err).NotTo(HaveOccurred())

				// Run the loop 3 times to make sure that the pipeline is continously transferring data from card to object store.
				for i := 0; i < 3; i++ {
					temp := 0
					tempMeta := 0

					for ip, name := range ts.tu.IPToNameMap {
						// Log added for debugging. Will remove eventually.
						By(fmt.Sprintf("ip name %s %s", ip, name))
					}

					ip := strings.Split(snic.Spec.IPConfig.IPAddress, "/")[0]

					By(fmt.Sprintf("trigger fwlog in NIC container %s %s %s %s %s", napleID,
						strings.Replace(snic.Spec.ID, "-", "", -1), snic.Spec.IPConfig.IPAddress, ip,
						ts.tu.IPToNameMap[ip]))

					st := ts.tu.LocalCommandOutput(fmt.Sprintf("docker exec %s %s",
						ts.tu.IPToNameMap[ip], fwevent.Cmd(1000, 65)))
					Expect(st == "null").Should(BeTrue())

					// fwlogs are reported to object store per min.
					// check object count
					Eventually(func() bool {
						newCount, err := getObjectCount(fwLogClient, "")
						Expect(err).NotTo(HaveOccurred())

						newMetaCount, err := getObjectCount(fwLogMetaClient, "")
						Expect(err).NotTo(HaveOccurred())

						objName, err := getLastObjectName(fwLogClient, "")
						Expect(err).NotTo(HaveOccurred())

						metaObjName, err := getLastObjectName(fwLogMetaClient, "")
						Expect(err).NotTo(HaveOccurred())

						By(fmt.Sprintf("objects len %d %d %s", newCount, currentCount, objName))
						By(fmt.Sprintf("meta objects len %d %d %s", newMetaCount, currentMetaCount, metaObjName))
						temp = newCount
						tempMeta = newMetaCount

						if newCount > currentCount && objName != "" {
							verifyDataObject(fwLogClient, objName)
						}

						if newMetaCount > currentMetaCount && metaObjName != "" {
							verifyMetaObject(fwLogMetaClient, metaObjName)
						}

						return (newCount > currentCount) && (newMetaCount > currentMetaCount)
					}, time.Second*200, time.Second*10).Should(BeTrue())

					currentCount = temp
					currentMetaCount = tempMeta
				}

				// TODO: Add number of logs in the meta of the object and also Assert on that number here.
				// Not sure how to Assert on the name of the object since its dependent on time.
				// Just do for one naples. Thats enough for e2e tests.
				break
			}
		})

		It("verify logs on elastic", func() {
			dataIndex := elastic.GetIndex(globals.FwLogs, globals.ReservedFwLogsTenantName)
			verifyIndexExistsOnElastic(esClient, dataIndex)
			verifyLogsOnElastic(esClient, fwLogClient, dataIndex)
		})

		It("veirfy firewallobjects index on elastic", func() {
			objectsIndex := elastic.GetIndex(globals.FwLogsObjects, "")
			verifyIndexExistsOnElastic(esClient, objectsIndex)
			verifyMinioObjectsOnElastic(esClient, fwLogClient, objectsIndex)
		})

		AfterEach(func() {
			validateCluster()
		})
	})
})

func verifyDataObject(c objstore.Client, objName string) {
	data, err := getObjectAndParseCsv(c, objName)
	Expect(err).NotTo(HaveOccurred())
	Expect(len(data) != 0).Should(BeTrue())
	logCount := 0
	for i := 1; i < len(data); i++ {
		// look for row 1, row 0 is header in CSV format
		line := data[i]
		logCount++
		Expect(line[0] != "").Should(BeTrue())
		Expect(line[1] != "").Should(BeTrue())
		Expect(line[2] != "").Should(BeTrue())
		Expect(line[3] != "").Should(BeTrue())
		Expect(line[4] != "").Should(BeTrue())
		Expect(line[5] != "").Should(BeTrue())
		Expect(line[6] != "").Should(BeTrue())
		Expect(line[7] != "").Should(BeTrue())
		Expect(line[8] != "").Should(BeTrue())
		Expect(line[9] != "").Should(BeTrue())
		Expect(line[10] != "").Should(BeTrue())
		Expect(line[11] != "").Should(BeTrue())
		Expect(line[12] != "").Should(BeTrue())
		Expect(line[13] != "").Should(BeTrue())
	}
	// Expect(logCount != 0).Should(BeTrue())
}

func verifyMetaObject(c objstore.Client, objName string) {
	// ListObjects will return the objects sorted in lexical date order.
	// Last object will be the most recent object.
	data, err := getObjectAndParseCsv(c, objName)
	Expect(err).NotTo(HaveOccurred())
	Expect(len(data) != 0).Should(BeTrue())
	logcount := 0
	for i := 1; i < len(data); i++ {
		// look for row 1, row 0 is header in CSV format
		line := data[i]
		// fmt.Println("**** metaObject ****", line)
		Expect(line[0] != "").Should(BeTrue())
		Expect(line[1] != "").Should(BeTrue())
		logcount++
	}
	// Expect(logcount != 0).Should(BeTrue())
}

func getObjectCount(c objstore.Client, objectKeyPrefix string) (int, error) {
	objects, err := c.ListObjects(objectKeyPrefix)
	if err != nil {
		return 0, err
	}
	currentCount := 0
	for _, o := range objects {
		if o != "" {
			By(fmt.Sprintf("object key %s", o))
			currentCount++
		}
	}
	return currentCount, nil
}

func getLastObjectName(c objstore.Client, objectKeyPrefix string) (string, error) {
	objects, err := c.ListObjects(objectKeyPrefix)
	if err != nil {
		return "", err
	}
	name := ""
	for _, o := range objects {
		if o != "" {
			name = o
		}
	}
	return name, nil
}

func getObjectAndParseCsv(c objstore.Client, objName string) ([][]string, error) {
	objReader, err := c.GetObject(context.Background(), objName)
	if err != nil {
		return nil, err
	}
	defer objReader.Close()

	zipReader, err := gzip.NewReader(objReader)
	if err != nil {
		return nil, err
	}
	defer zipReader.Close()

	rd := csv.NewReader(zipReader)
	data, err := rd.ReadAll()
	if err != nil {
		return nil, err
	}

	By(fmt.Sprintf("data len %d, data %s", len(data), data))
	return data, nil
}

func setupFwLogsClients() (objstore.Client, objstore.Client) {
	// Setup the objstore client
	tlcConfig := &tls.Config{
		InsecureSkipVerify: true,
	}

	// fwlogs.fwlogs
	fwLogClient, err := objstore.NewClient(bucketPrefix,
		bucketName, ts.tu.Resolver(), objstore.WithTLSConfig(tlcConfig))
	Expect(err).NotTo(HaveOccurred())

	fwLogMetaClient, err := objstore.NewClient(metaBucketPrefix,
		bucketName, ts.tu.Resolver(), objstore.WithTLSConfig(tlcConfig))
	Expect(err).NotTo(HaveOccurred())

	return fwLogClient, fwLogMetaClient
}

func setupElasticClient() elastic.ESClient {
	mr := mockresolver.New()

	// e2e starts only quorum venice nodes
	for _, name := range ts.tu.QuorumNodes {
		si := &types.ServiceInstance{
			TypeMeta: api.TypeMeta{
				Kind: "ServiceInstance",
			},
			ObjectMeta: api.ObjectMeta{
				Name: name,
			},
			Service: globals.ElasticSearch,
			URL:     fmt.Sprintf("%s:%s", ts.tu.NameToIPMap[name], globals.ElasticsearchRESTPort),
		}
		mr.AddServiceInstance(si)
	}

	logConfig := log.GetDefaultConfig("objstore_fwlogs_test")
	var esClient elastic.ESClient
	Eventually(func() error {
		var err error
		esClient, err = elastic.NewAuthenticatedClient("", mr, log.GetNewLogger(logConfig))
		return err
	}, 60, 1).Should(BeNil(), "failed to initialize elastic client")
	return esClient
}

func verifyLogsOnElastic(c elastic.ESClient, fwLogClient objstore.Client, indexName string) {
	// Get the last object from Minio.
	// Get one log line from it, get  the source and destip from that line.
	// Query elastic for the ip pair and verify that the log is present in elastic.
	objName, err := getLastObjectName(fwLogClient, "")
	Expect(err).NotTo(HaveOccurred())

	data, err := getObjectAndParseCsv(fwLogClient, objName)
	Expect(err).NotTo(HaveOccurred())
	Expect(len(data) != 0).Should(BeTrue())
	notFound := 0
	for i := 1; i < len(data); i++ {
		line := data[i]
		sip := line[2]
		dip := line[3]
		proto := line[7]

		By(fmt.Sprintf("querying elastic for indexName %s, sip %s, dip %s, csvLine %s", indexName, sip, dip, line))

		// construct the query
		query := es.NewBoolQuery().
			Must(es.NewMatchQuery("sip", sip)).
			Must(es.NewMatchQuery("dip", dip)).
			Must(es.NewMatchQuery("proto", proto))

		// execute query
		result, err := c.Search(context.Background(),
			"*firewalllogs*", // index
			"",               // skip the index type
			query,            // query to be executed
			nil,              // no aggregation
			0,                // from
			maxResults,       // to
			"creationTs",     // sorting is required
			true)             // sort in desc order

		Expect(err).NotTo(HaveOccurred())

		// fmt.Println("**** hits in elastic ****", len(result.Hits.Hits))

		for _, res := range result.Hits.Hits {
			var obj map[string]interface{}
			err := json.Unmarshal(*res.Source, &obj)
			Expect(err).NotTo(HaveOccurred())
			compareAndVerifyLogLineInCsvAndElastic(line, obj)
		}

		if len(result.Hits.Hits) == 0 {
			notFound++
		}
		Expect(len(result.Hits.Hits) != 0).Should(BeTrue())
	}

	// fmt.Println("**** total records not found in elastic ****", notFound)
}

func verifyMinioObjectsOnElastic(c elastic.ESClient, fwLogClient objstore.Client, indexName string) {
	objects, err := fwLogClient.ListObjects("")
	Expect(err).NotTo(HaveOccurred())
	for _, o := range objects {
		if o != "" {
			// construct the query
			query := es.NewBoolQuery()
			query = query.Must(es.NewMatchQuery("key", o))

			// execute query
			result, err := c.Search(context.Background(),
				indexName,    // index
				"",           // skip the index type
				query,        // query to be executed
				nil,          // no aggregation
				0,            // from
				maxResults,   // to
				"creationTs", // sorting is required
				true)         // sort in desc order

			Expect(err).NotTo(HaveOccurred())

			for _, res := range result.Hits.Hits {
				var obj map[string]interface{}
				err := json.Unmarshal(*res.Source, &obj)
				Expect(err).NotTo(HaveOccurred())
			}

			// fmt.Println("**** hits in elastic ****", len(result.Hits.Hits))
			Expect(len(result.Hits.Hits) != 0).Should(BeTrue())
		}
	}
}

func verifyIndexExistsOnElastic(c elastic.ESClient, indexName string) {
	es := c.GetRawClient()
	ctxWithDeadline, cancel := context.WithDeadline(context.Background(), time.Now().Add(elasticContextDeadline))
	defer cancel()
	exists, err := es.IndexExists(indexName).Do(ctxWithDeadline)
	Expect(err).NotTo(HaveOccurred())
	Expect(exists).Should(BeTrue())
}

func compareAndVerifyLogLineInCsvAndElastic(csvLine []string, elasticMap map[string]interface{}) {

	// fmt.Println("**** csv data ****", csvLine)
	// fmt.Println("**** elastic data ****", elasticMap)

	// "sport", "dport",
	// "proto", "act", "dir", "ruleid",
	// "sessionid", "sessionstate",
	// "icmptype", "icmpid", "icmpcode"
	Expect(csvLine[2] == elasticMap["sip"].(string)).Should(BeTrue())
	Expect(csvLine[3] == elasticMap["dip"].(string)).Should(BeTrue())

	sport, err := strconv.ParseUint(csvLine[5], 10, 64)
	Expect(err).NotTo(HaveOccurred())
	Expect(float64(sport) == elasticMap["sport"].(float64)).Should(BeTrue())

	dport, err := strconv.ParseUint(csvLine[6], 10, 64)
	Expect(err).NotTo(HaveOccurred())
	Expect(float64(dport) == elasticMap["dport"].(float64)).Should(BeTrue())

	Expect(csvLine[7] == elasticMap["proto"].(string)).Should(BeTrue())
	Expect(csvLine[8] == elasticMap["action"].(string)).Should(BeTrue())
	Expect(csvLine[9] == elasticMap["dir"].(string)).Should(BeTrue())

	ruleID, err := strconv.ParseUint(csvLine[10], 10, 64)
	Expect(err).NotTo(HaveOccurred())
	Expect(float64(ruleID) == elasticMap["ruleid"].(float64)).Should(BeTrue())

	sessionID, err := strconv.ParseUint(csvLine[11], 10, 64)
	Expect(err).NotTo(HaveOccurred())
	Expect(float64(sessionID) == elasticMap["sessid"].(float64)).Should(BeTrue())

	Expect(csvLine[12] == elasticMap["flowaction"].(string)).Should(BeTrue())

	icmpType, err := strconv.ParseUint(csvLine[13], 10, 64)
	Expect(err).NotTo(HaveOccurred())
	Expect(float64(icmpType) == elasticMap["icmptype"].(float64)).Should(BeTrue())

	icmpID, err := strconv.ParseUint(csvLine[14], 10, 64)
	Expect(err).NotTo(HaveOccurred())
	Expect(float64(icmpID) == elasticMap["icmpid"].(float64)).Should(BeTrue())

	icmpCode, err := strconv.ParseUint(csvLine[15], 10, 64)
	Expect(err).NotTo(HaveOccurred())
	Expect(float64(icmpCode) == elasticMap["icmpcode"].(float64)).Should(BeTrue())
}
