package mirror_test

import (
	//"errors"

	"context"
	"fmt"
	"time"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
	"github.com/pensando/sw/api/labels"
	"github.com/pensando/sw/iota/test/venice/iotakit/model/objects"
)

var _ = Describe("Interface mirror tests", func() {
	var startTime time.Time
	dataPathEnabled := true
	BeforeEach(func() {
		startTime = time.Now().UTC()
		// verify cluster is in good health
		Eventually(func() error {
			return ts.model.VerifyClusterStatus()
		}).Should(Succeed())
	})
	AfterEach(func() {
		//Expect No Service is stopped
		Expect(ts.model.ServiceStoppedEvents(startTime, ts.model.Naples()).Len(0))
	})
	Context("Uplink mirror tests", func() {

		runDataPath := func(workloadPairs *objects.WorkloadPairCollection,
			veniceCollector *objects.VeniceNodeCollection) error {
			ctx, cancel := context.WithCancel(context.Background())
			tcpdumpDone := make(chan error)
			var output int
			go func() {
				var err error
				output, err = veniceCollector.GetGRETCPDumpCount(ctx)
				tcpdumpDone <- err

			}()

			//Sleep for a while so that tcpdump starts
			time.Sleep(2 * time.Second)
			ts.model.PingPairs(workloadPairs)
			cancel()
			err := <-tcpdumpDone

			if err != nil {
				return fmt.Errorf("Error running command.")
			}

			if output == 0 {
				return fmt.Errorf("Did not receive mirror packets")
			}

			return nil
		}

		verifyDatapathCollection := func(workloadPairs *objects.WorkloadPairCollection,
			veniceCollector *objects.VeniceNodeCollection) error {
			if !dataPathEnabled {
				return nil
			}
			return runDataPath(workloadPairs, veniceCollector)
		}

		verifyNoDatapathCollection := func(workloadPairs *objects.WorkloadPairCollection,
			veniceCollector *objects.VeniceNodeCollection) error {
			if !dataPathEnabled {
				return nil
			}
			err := runDataPath(workloadPairs, veniceCollector)
			if err != nil {
				return nil
			}
			return fmt.Errorf("Collector still found packets..")
		}

		It("Mirror all uplink traffic to collector", func() {
			veniceCollector := ts.model.VeniceNodes().Leader()

			intfCollection := ts.model.NetworkInterfaces().Uplinks()
			Expect(intfCollection.Len() != 0).Should(BeTrue())

			label := map[string]string{
				"env": "production",
			}
			otherLabel := map[string]string{
				"env1": "production1",
			}

			Expect(intfCollection.AddLabel(label).Commit()).ShouldNot(HaveOccurred())

			selector := labels.SelectorFromSet(labels.Set(label))
			otherSelector := labels.SelectorFromSet(labels.Set(otherLabel))

			// add permit rules for workload pairs
			workloadPairs := ts.model.WorkloadPairs().WithinNetwork().Any(1)
			msc := ts.model.NewMirrorSession("lif-mirror").SetInterfaceSelector(selector)
			msc.AddVeniceCollector(veniceCollector, "udp/4545", 1)
			Expect(msc.Commit()).Should(Succeed())

			Eventually(func() error {
				return intfCollection.VerifyMirrors([]string{"default/default/lif-mirror"})
			}).Should(Succeed())

			Eventually(func() error {
				return verifyDatapathCollection(workloadPairs, veniceCollector)
			}).Should(Succeed())

			msc.SetInterfaceSelector(otherSelector)
			Expect(msc.Commit()).Should(Succeed())

			Eventually(func() error {
				return intfCollection.VerifyNoMirrors([]string{"default/default/lif-mirror"})
			}).Should(Succeed())

			Eventually(func() error {
				return verifyNoDatapathCollection(workloadPairs, veniceCollector)
			}).Should(Succeed())

			//Delete mirror session once done
			Expect(msc.Delete()).Should(Succeed())
			//remove labels once done
			Expect(intfCollection.AddLabel(nil).Commit()).ShouldNot(HaveOccurred())
		})

		It("Mirror uplink with change in collector", func() {
			veniceCollector := ts.model.VeniceNodes().Leader()
			otherVeniceCollector := ts.model.VeniceNodes().NonLeaders().Any(1)

			Expect(len(otherVeniceCollector.Nodes) != 0)

			intfCollection := ts.model.NetworkInterfaces().Uplinks()
			Expect(intfCollection.Len() != 0).Should(BeTrue())

			label := map[string]string{
				"env": "production",
			}
			Expect(intfCollection.AddLabel(label).Commit()).ShouldNot(HaveOccurred())

			selector := labels.SelectorFromSet(labels.Set(label))

			// add permit rules for workload pairs
			workloadPairs := ts.model.WorkloadPairs().WithinNetwork().Any(1)
			msc := ts.model.NewMirrorSession("lif-mirror").SetInterfaceSelector(selector)
			msc.AddVeniceCollector(veniceCollector, "udp/4545", 1)
			Expect(msc.Commit()).Should(Succeed())

			Eventually(func() error {
				return intfCollection.VerifyMirrors([]string{"default/default/lif-mirror"})
			}).Should(Succeed())

			Eventually(func() error {
				return verifyDatapathCollection(workloadPairs, veniceCollector)
			}).Should(Succeed())

			msc.ClearCollectors()
			msc.AddVeniceCollector(otherVeniceCollector, "udp/4545", 1)
			Expect(msc.Commit()).Should(Succeed())

			Eventually(func() error {
				return intfCollection.VerifyMirrors([]string{"default/default/lif-mirror"})
			}).Should(Succeed())

			Eventually(func() error {
				return verifyDatapathCollection(workloadPairs, otherVeniceCollector)
			}).Should(Succeed())

			Eventually(func() error {
				return verifyNoDatapathCollection(workloadPairs, veniceCollector)
			}).Should(Succeed())

			//Delete mirror session once done
			Expect(msc.Delete()).Should(Succeed())
			//remove labels once done
			Expect(intfCollection.AddLabel(nil).Commit()).ShouldNot(HaveOccurred())

		})

		It("Mirror uplink with change in mirror session", func() {
			veniceCollector := ts.model.VeniceNodes().Leader()
			otherVeniceCollector := ts.model.VeniceNodes().NonLeaders().Any(1)

			intfCollection := ts.model.NetworkInterfaces().Uplinks()
			Expect(intfCollection.Len() != 0).Should(BeTrue())

			label := map[string]string{
				"env": "production",
			}
			Expect(intfCollection.AddLabel(label).Commit()).ShouldNot(HaveOccurred())

			selector := labels.SelectorFromSet(labels.Set(label))

			// add permit rules for workload pairs
			workloadPairs := ts.model.WorkloadPairs().WithinNetwork().Any(1)
			msc := ts.model.NewMirrorSession("lif-mirror").SetInterfaceSelector(selector)
			msc.AddVeniceCollector(veniceCollector, "udp/4545", 1)
			Expect(msc.Commit()).Should(Succeed())

			Eventually(func() error {
				return intfCollection.VerifyMirrors([]string{"default/default/lif-mirror"})
			}).Should(Succeed())

			Eventually(func() error {
				return verifyDatapathCollection(workloadPairs, veniceCollector)
			}).Should(Succeed())

			Expect(msc.Delete()).Should(Succeed())

			Eventually(func() error {
				return intfCollection.VerifyNoMirrors([]string{"default/default/lif-mirror"})
			}).Should(Succeed())

			Eventually(func() error {
				return verifyNoDatapathCollection(workloadPairs, veniceCollector)
			}).Should(Succeed())

			newMsc := ts.model.NewMirrorSession("lif-mirror-new").SetInterfaceSelector(selector)
			newMsc.AddVeniceCollector(otherVeniceCollector, "udp/4545", 1)
			Expect(newMsc.Commit()).Should(Succeed())

			Eventually(func() error {
				return intfCollection.VerifyMirrors([]string{"default/default/lif-mirror-new"})
			}).Should(Succeed())

			Eventually(func() error {
				return verifyDatapathCollection(workloadPairs, otherVeniceCollector)
			}).Should(Succeed())

			//Delete mirror session once done
			Expect(newMsc.Delete()).Should(Succeed())
			//remove labels once done
			Expect(intfCollection.AddLabel(nil).Commit()).ShouldNot(HaveOccurred())
		})

		It("Mirror uplink with interface label added later", func() {
			veniceCollector := ts.model.VeniceNodes().Leader()

			intfCollection := ts.model.NetworkInterfaces().Uplinks()
			Expect(intfCollection.Len() != 0).Should(BeTrue())

			label := map[string]string{
				"env": "production",
			}

			selector := labels.SelectorFromSet(labels.Set(label))

			// add permit rules for workload pairs
			workloadPairs := ts.model.WorkloadPairs().WithinNetwork().Any(1)
			msc := ts.model.NewMirrorSession("lif-mirror").SetInterfaceSelector(selector)
			msc.AddVeniceCollector(veniceCollector, "udp/4545", 1)
			Expect(msc.Commit()).Should(Succeed())

			Eventually(func() error {
				return intfCollection.VerifyNoMirrors([]string{"default/default/lif-mirror"})
			}).Should(Succeed())

			Eventually(func() error {
				return verifyNoDatapathCollection(workloadPairs, veniceCollector)
			}).Should(Succeed())

			Expect(intfCollection.AddLabel(label).Commit()).ShouldNot(HaveOccurred())

			Eventually(func() error {
				return intfCollection.VerifyMirrors([]string{"default/default/lif-mirror"})
			}).Should(Succeed())

			Eventually(func() error {
				return verifyDatapathCollection(workloadPairs, veniceCollector)
			}).Should(Succeed())

			//remove labels once done
			Expect(intfCollection.AddLabel(nil).Commit()).ShouldNot(HaveOccurred())
			//Delete mirror session once done
			Expect(msc.Delete()).Should(Succeed())
		})

		It("Mirror uplink with interface label removed later", func() {
			veniceCollector := ts.model.VeniceNodes().Leader()

			intfCollection := ts.model.NetworkInterfaces().Uplinks()
			Expect(intfCollection.Len() != 0).Should(BeTrue())

			label := map[string]string{
				"env": "production",
			}
			Expect(intfCollection.AddLabel(label).Commit()).ShouldNot(HaveOccurred())

			selector := labels.SelectorFromSet(labels.Set(label))

			// add permit rules for workload pairs
			workloadPairs := ts.model.WorkloadPairs().WithinNetwork().Any(1)
			msc := ts.model.NewMirrorSession("lif-mirror").SetInterfaceSelector(selector)
			msc.AddVeniceCollector(veniceCollector, "udp/4545", 1)
			Expect(msc.Commit()).Should(Succeed())

			Eventually(func() error {
				return intfCollection.VerifyMirrors([]string{"default/default/lif-mirror"})
			}).Should(Succeed())

			Eventually(func() error {
				return verifyDatapathCollection(workloadPairs, veniceCollector)
			}).Should(Succeed())

			Expect(intfCollection.AddLabel(nil).Commit()).ShouldNot(HaveOccurred())

			Eventually(func() error {
				return intfCollection.VerifyNoMirrors([]string{"default/default/lif-mirror"})
			}).Should(Succeed())

			Eventually(func() error {
				return verifyNoDatapathCollection(workloadPairs, veniceCollector)
			}).Should(Succeed())

			//Add Back label and see whether it works
			Expect(intfCollection.AddLabel(label).Commit()).ShouldNot(HaveOccurred())

			Eventually(func() error {
				return intfCollection.VerifyMirrors([]string{"default/default/lif-mirror"})
			}).Should(Succeed())

			Eventually(func() error {
				return verifyDatapathCollection(workloadPairs, veniceCollector)
			}).Should(Succeed())

			//remove labels once done
			Expect(intfCollection.AddLabel(nil).Commit()).ShouldNot(HaveOccurred())
			//Delete mirror session once done
			Expect(msc.Delete()).Should(Succeed())

		})

		It("Mirror uplink with add/delete collector", func() {
			veniceCollector := ts.model.VeniceNodes().Leader()
			otherVeniceCollector := ts.model.VeniceNodes().NonLeaders().Any(1)

			intfCollection := ts.model.NetworkInterfaces().Uplinks()
			Expect(intfCollection.Len() != 0).Should(BeTrue())

			label := map[string]string{
				"env": "production",
			}
			Expect(intfCollection.AddLabel(label).Commit()).ShouldNot(HaveOccurred())

			selector := labels.SelectorFromSet(labels.Set(label))

			// add permit rules for workload pairs
			workloadPairs := ts.model.WorkloadPairs().WithinNetwork().Any(1)
			msc := ts.model.NewMirrorSession("lif-mirror").SetInterfaceSelector(selector)
			msc.AddVeniceCollector(veniceCollector, "udp/4545", 1)
			Expect(msc.Commit()).Should(Succeed())

			Eventually(func() error {
				return intfCollection.VerifyMirrors([]string{"default/default/lif-mirror"})
			}).Should(Succeed())

			Eventually(func() error {
				return verifyDatapathCollection(workloadPairs, veniceCollector)
			}).Should(Succeed())

			msc.AddVeniceCollector(otherVeniceCollector, "udp/4545", 1)
			Expect(msc.Commit()).Should(Succeed())

			Eventually(func() error {
				return intfCollection.VerifyMirrors([]string{"default/default/lif-mirror"})
			}).Should(Succeed())

			Eventually(func() error {
				return verifyDatapathCollection(workloadPairs, otherVeniceCollector)
			}).Should(Succeed())

			Eventually(func() error {
				return verifyDatapathCollection(workloadPairs, veniceCollector)
			}).Should(Succeed())

			//Remove the other collector
			msc.ClearCollectors()
			msc.AddVeniceCollector(otherVeniceCollector, "udp/4545", 1)
			Expect(msc.Commit()).Should(Succeed())

			Eventually(func() error {
				return verifyNoDatapathCollection(workloadPairs, veniceCollector)
			}).Should(Succeed())

			Eventually(func() error {
				return verifyDatapathCollection(workloadPairs, otherVeniceCollector)
			}).Should(Succeed())

			//remove labels once done
			Expect(intfCollection.AddLabel(nil).Commit()).ShouldNot(HaveOccurred())
			//Delete mirror session once done
			Expect(msc.Delete()).Should(Succeed())
		})

		It("Multiple Mirror session for the same interfaces", func() {
			veniceCollector := ts.model.VeniceNodes().Leader()
			otherVeniceCollector := ts.model.VeniceNodes().NonLeaders().Any(1)

			intfCollection := ts.model.NetworkInterfaces().Uplinks()
			Expect(intfCollection.Len() != 0).Should(BeTrue())

			label := map[string]string{
				"env": "production",
			}
			Expect(intfCollection.AddLabel(label).Commit()).ShouldNot(HaveOccurred())

			selector := labels.SelectorFromSet(labels.Set(label))

			// add permit rules for workload pairs
			workloadPairs := ts.model.WorkloadPairs().WithinNetwork().Any(1)
			msc := ts.model.NewMirrorSession("lif-mirror").SetInterfaceSelector(selector)
			msc.AddVeniceCollector(veniceCollector, "udp/4545", 1)
			Expect(msc.Commit()).Should(Succeed())

			Eventually(func() error {
				return intfCollection.VerifyMirrors([]string{"default/default/lif-mirror"})
			}).Should(Succeed())

			Eventually(func() error {
				return verifyDatapathCollection(workloadPairs, veniceCollector)
			}).Should(Succeed())

			//Add one more mirror session
			newMsc := ts.model.NewMirrorSession("lif-mirror-new").SetInterfaceSelector(selector)
			newMsc.AddVeniceCollector(otherVeniceCollector, "udp/4545", 1)
			Expect(newMsc.Commit()).Should(Succeed())

			Eventually(func() error {
				return intfCollection.VerifyMirrors([]string{"default/default/lif-mirror", "default/default/lif-mirror-new"})
			}).Should(Succeed())

			Eventually(func() error {
				return verifyDatapathCollection(workloadPairs, veniceCollector)
			}).Should(Succeed())

			Eventually(func() error {
				return verifyDatapathCollection(workloadPairs, otherVeniceCollector)
			}).Should(Succeed())

			//Delete the first mirror now
			Expect(msc.Delete()).Should(Succeed())

			Eventually(func() error {
				return intfCollection.VerifyMirrors([]string{"default/default/lif-mirror-new"})
			}).Should(Succeed())

			Eventually(func() error {
				return verifyNoDatapathCollection(workloadPairs, veniceCollector)
			}).Should(Succeed())

			Eventually(func() error {
				return verifyDatapathCollection(workloadPairs, otherVeniceCollector)
			}).Should(Succeed())

			Expect(newMsc.Delete()).Should(Succeed())

			Eventually(func() error {
				return intfCollection.VerifyNoMirrors([]string{"default/default/lif-mirror", "default/default/lif-mirror-new"})
			}).Should(Succeed())

			Eventually(func() error {
				return verifyNoDatapathCollection(workloadPairs, veniceCollector)
			}).Should(Succeed())

			Eventually(func() error {
				return verifyNoDatapathCollection(workloadPairs, otherVeniceCollector)
			}).Should(Succeed())

			//remove labels once done
			Expect(intfCollection.AddLabel(nil).Commit()).ShouldNot(HaveOccurred())
		})

		It("Multiple Mirror session for the same interfaces del/add labels", func() {
			veniceCollector := ts.model.VeniceNodes().Leader()
			otherVeniceCollector := ts.model.VeniceNodes().NonLeaders().Any(1)

			intfCollection := ts.model.NetworkInterfaces().Uplinks()
			Expect(intfCollection.Len() != 0).Should(BeTrue())

			label := map[string]string{
				"env": "production",
			}
			Expect(intfCollection.AddLabel(label).Commit()).ShouldNot(HaveOccurred())

			selector := labels.SelectorFromSet(labels.Set(label))

			// add permit rules for workload pairs
			workloadPairs := ts.model.WorkloadPairs().WithinNetwork().Any(1)
			msc := ts.model.NewMirrorSession("lif-mirror").SetInterfaceSelector(selector)
			msc.AddVeniceCollector(veniceCollector, "udp/4545", 1)
			Expect(msc.Commit()).Should(Succeed())

			Eventually(func() error {
				return intfCollection.VerifyMirrors([]string{"default/default/lif-mirror"})
			}).Should(Succeed())

			Eventually(func() error {
				return verifyDatapathCollection(workloadPairs, veniceCollector)
			}).Should(Succeed())

			//Add one more mirror session
			newMsc := ts.model.NewMirrorSession("lif-mirror-new").SetInterfaceSelector(selector)
			newMsc.AddVeniceCollector(otherVeniceCollector, "udp/4545", 1)
			Expect(newMsc.Commit()).Should(Succeed())

			Eventually(func() error {
				return intfCollection.VerifyMirrors([]string{"default/default/lif-mirror", "default/default/lif-mirror-new"})
			}).Should(Succeed())

			Eventually(func() error {
				return verifyDatapathCollection(workloadPairs, veniceCollector)
			}).Should(Succeed())

			Eventually(func() error {
				return verifyDatapathCollection(workloadPairs, otherVeniceCollector)
			}).Should(Succeed())

			//remove from the interfaces
			Expect(intfCollection.AddLabel(nil).Commit()).ShouldNot(HaveOccurred())
			Eventually(func() error {
				return intfCollection.VerifyNoMirrors([]string{"default/default/lif-mirror", "default/default/lif-mirror-new"})
			}).Should(Succeed())

			Eventually(func() error {
				return verifyNoDatapathCollection(workloadPairs, veniceCollector)
			}).Should(Succeed())

			Eventually(func() error {
				return verifyNoDatapathCollection(workloadPairs, otherVeniceCollector)
			}).Should(Succeed())

			Expect(intfCollection.AddLabel(label).Commit()).ShouldNot(HaveOccurred())

			Eventually(func() error {
				return intfCollection.VerifyMirrors([]string{"default/default/lif-mirror", "default/default/lif-mirror-new"})
			}).Should(Succeed())

			Eventually(func() error {
				return verifyDatapathCollection(workloadPairs, veniceCollector)
			}).Should(Succeed())

			Eventually(func() error {
				return verifyDatapathCollection(workloadPairs, otherVeniceCollector)
			}).Should(Succeed())

			//Delete the first mirror now
			Expect(msc.Delete()).Should(Succeed())

			Eventually(func() error {
				return intfCollection.VerifyMirrors([]string{"default/default/lif-mirror-new"})
			}).Should(Succeed())

			Eventually(func() error {
				return verifyNoDatapathCollection(workloadPairs, veniceCollector)
			}).Should(Succeed())

			Eventually(func() error {
				return verifyDatapathCollection(workloadPairs, otherVeniceCollector)
			}).Should(Succeed())

			Expect(newMsc.Delete()).Should(Succeed())

			Eventually(func() error {
				return intfCollection.VerifyNoMirrors([]string{"default/default/lif-mirror", "default/default/lif-mirror-new"})
			}).Should(Succeed())

			Eventually(func() error {
				return verifyNoDatapathCollection(workloadPairs, veniceCollector)
			}).Should(Succeed())

			Eventually(func() error {
				return verifyNoDatapathCollection(workloadPairs, otherVeniceCollector)
			}).Should(Succeed())

			//remove labels once done
			Expect(intfCollection.AddLabel(nil).Commit()).ShouldNot(HaveOccurred())
		})
	})
})
