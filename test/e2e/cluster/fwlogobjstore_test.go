// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.

package cluster

import (
	"compress/gzip"
	"context"
	"crypto/tls"
	"encoding/csv"
	"fmt"
	"strings"
	"time"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/tmagent/state/fwgen/fwevent"
	objstore "github.com/pensando/sw/venice/utils/objstore/client"
)

const timeFormat = "2006-01-02T15:04:05"
const bucketPrefix = "fwlogs"
const bucketName = "fwlogs"
const singleBucket = true

var _ = Describe("tests for storing firewall logs in object store", func() {
	Context("push firewall logs in object store for default tenant", func() {
		BeforeEach(func() {
			validateCluster()
		})
		It("push fwlogs", func() {
			// Setup the objstore client
			tlcConfig := &tls.Config{
				InsecureSkipVerify: true,
			}

			snIf := ts.tu.APIClient.ClusterV1().DistributedServiceCard()
			snics, _ := snIf.List(context.Background(), &api.ListWatchOptions{})
			for _, snic := range snics {
				// try for this hour and next hour
				t := time.Now()
				y, m, dt := t.Date()
				h, _, _ := t.Clock()
				napleID := snic.Status.PrimaryMAC
				timestamp := time.Date(y, m, dt, h, 0, 0, 0, time.UTC)
				var fwLogClient objstore.Client
				var err error
				if singleBucket {
					// fwlogs.fwlogs
					fwLogClient, err = objstore.NewClient(bucketPrefix,
						bucketName, ts.tu.Resolver(), objstore.WithTLSConfig(tlcConfig))
				} else {
					fwLogClient, err = objstore.NewClient(bucketPrefix,
						napleID+"-"+strings.Replace(strings.Replace(timestamp.UTC().Format(timeFormat), ":", "-", -1), "T", "t", -1),
						ts.tu.Resolver(), objstore.WithTLSConfig(tlcConfig))
				}
				Expect(err).NotTo(HaveOccurred())

				if singleBucket {
					fmt.Println("Bucket name", bucketPrefix+"."+bucketName)
				} else {
					fmt.Println("Bucket name",
						bucketPrefix+"."+napleID+"-"+
							strings.Replace(strings.Replace(timestamp.UTC().Format(timeFormat), ":", "-", -1), "T", "t", -1))
				}

				// get the current list of objects in objectstore
				objects, err := fwLogClient.ListObjects("")
				Expect(err).NotTo(HaveOccurred())
				currentCount := 0
				for _, o := range objects {
					if o != "" {
						By(fmt.Sprintf("current object key %s", o))
						currentCount++
					}
				}

				// Run the loop 3 times to make sure that the pipeline is continously transferring data from card to object store.
				for i := 0; i < 3; i++ {
					temp := 0

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
						objects, _ := fwLogClient.ListObjects("")
						newCount := 0
						objName := ""
						for _, o := range objects {
							if o != "" {
								By(fmt.Sprintf("new object key %s", o))
								newCount++
								objName = o
							}
						}

						// 2020-02-20T04:02:59_2020-02-20T03:55:05.csv.gzip
						By(fmt.Sprintf("objects len %d %d %s", newCount, currentCount, objName))
						temp = newCount

						if newCount > currentCount {
							// ListObjects will return the objects sorted in lexical date order.
							// Last object will be the most recent object.
							objReader, err := fwLogClient.GetObject(context.Background(), objName)
							Expect(err).NotTo(HaveOccurred())

							zipReader, err := gzip.NewReader(objReader)
							Expect(err).NotTo(HaveOccurred())

							rd := csv.NewReader(zipReader)
							data, err := rd.ReadAll()
							Expect(err).NotTo(HaveOccurred())
							Expect(len(data) != 0).Should(BeTrue())
							By(fmt.Sprintf("data len %d", len(data)))
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
						return newCount > currentCount
					}, time.Second*200, time.Second*10).Should(BeTrue())
					currentCount = temp
				}

				// TODO: Add number of logs in the meta of the object and also Assert on that number here.
				// Not sure how to Assert on the name of the object since its dependent on time.
				// Just do for one naples. Thats enough for e2e tests.
				break
			}
		})
		AfterEach(func() {
			validateCluster()
		})
	})
})
