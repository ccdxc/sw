package techsupport_test

import (
	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
)

var _ = Describe("techsupport tests", func() {
	BeforeEach(func() {
		// verify cluster is in good health
		Eventually(func() error {
			return ts.model.Action().VerifyClusterStatus()
		}).Should(Succeed())
	})
	AfterEach(func() {
		ts.tb.AfterTestCommon()
	})

	Context("Iota Techsupport tests", func() {

		It("Perform Techsupport", func() {
           techsupport := &monitoring.TechSupportRequest{}
           var percent = 1 //need to get from global params
           if ts.scaleData { 
                var names []string
                naples := ts.model.Naples().Names()
                var nc = len(naples) * (percent / 100)
                for n := 0; n < nc; n++ {
                    names = append(names, naples[n])
                }
                nodes := ts.model.VeniceNodes()
                for _,ip := range nodes.GenVeniceIPs() {
                    names = append(names,ip)
                }
                techsupport = &monitoring.TechSupportRequest{
                    TypeMeta: api.TypeMeta{
                        Kind: "TechSupportRequest",
                    },
                    ObjectMeta: api.ObjectMeta{
                        Name:"techsupport-test",
                    },
                    Spec: monitoring.TechSupportRequestSpec{
                        Verbosity: 1,
                        NodeSelector: &monitoring.TechSupportRequestSpec_NodeSelectorSpec{
                            Names: names,
                        },
                    },
                }
            } else {
    			techsupport = &monitoring.TechSupportRequest{
    				TypeMeta: api.TypeMeta{
	    				Kind: "TechSupportRequest",
		    		},
			    	ObjectMeta: api.ObjectMeta{
				    	Name: "techsupport-test",
    				},
	    			Spec: monitoring.TechSupportRequestSpec{
		    			NodeSelector: &monitoring.TechSupportRequestSpec_NodeSelectorSpec{Names:[]string{"00ae.cd88.0011"}},
    				},
	    		}
            }

			err := ts.model.Action().PerformTechsupport(techsupport)
			Expect(err).ShouldNot(HaveOccurred())

			// verify techsupport is successful
			Eventually(func() error {
				return ts.model.Action().VerifyTechsupportStatus(techsupport.Name)
			}).Should(Succeed())
			return

		})
	})
})
