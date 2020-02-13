// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.

package cluster

import (
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
const bucketPrefix = "default.fwlogs"

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
				fwLogClient, err := objstore.NewClient(bucketPrefix,
					napleID+"-"+strings.Replace(strings.Replace(timestamp.UTC().Format(timeFormat), ":", "-", -1), "T", "t", -1),
					ts.tu.Resolver(), objstore.WithTLSConfig(tlcConfig))
				Expect(err).NotTo(HaveOccurred())
				fmt.Println("Bucket name",
					bucketPrefix+"."+napleID+"-"+
						strings.Replace(strings.Replace(timestamp.UTC().Format(timeFormat), ":", "-", -1), "T", "t", -1))

				// get the current list of objects in objectstore
				objects, err := fwLogClient.ListObjects(time.Now().Format("2006-01-02"))
				Expect(err).NotTo(HaveOccurred())
				currentCount := len(objects)

				// Run the loop 3 times to make sure that the pipeline is continously transferring data from card to object store.
				for i := 0; i < 3; i++ {
					temp := 0
					// check object count
					Eventually(func() bool {
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
						// Sleeping for a minute because the fwlogs are reported to object store per min.
						time.Sleep(100 * time.Second)
						objects, _ := fwLogClient.ListObjects("")
						By(fmt.Sprintf("objects len %d %d", len(objects), currentCount))
						temp = len(objects)

						// ListObjects will return the objects sorted in lexical date order.
						// Last object will be the most recent object.
						objReader, err := fwLogClient.GetObject(context.Background(), objects[len(objects)-1])
						Expect(err).NotTo(HaveOccurred())

						rd := csv.NewReader(objReader)
						data, err := rd.ReadAll()
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
						Expect(logCount != 0).Should(BeTrue())
						return len(objects) > currentCount
					}).Should(BeTrue())
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
