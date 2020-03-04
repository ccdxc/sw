// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.

package cluster_test

import (
	"fmt"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/iota/test/venice/iotakit/model/objects"
	"github.com/pensando/sw/venice/utils/log"
)

var _ = Describe("venice profile test", func() {
	BeforeEach(func() {})
	AfterEach(func() {})

	Context("tags:type=basic; Basic Profile create", func() {
		It("tags:sanity=false should be able to create/delete profile", func() {
			dscProfile := cluster.DSCProfile{
				TypeMeta: api.TypeMeta{Kind: "DSCProfile"},
				ObjectMeta: api.ObjectMeta{
					Name:      "defaultInsersionEnforced1",
					Namespace: "",
					Tenant:    "",
				},
				Spec: cluster.DSCProfileSpec{
					FwdMode:    "INSERTION",
					PolicyMode: "ENFORCED",
				},
			}
			Expect(ts.model.ConfigClient().CreateDscProfile(&dscProfile)).ShouldNot(HaveOccurred())
			halcmd := fmt.Sprintf("/nic/bin/halctl show system mode | /bin/grep 'FWD_MODE_MICROSEG'")
			ts.model.ForEachNaples(func(nc *objects.NaplesCollection) error {
				if out, err := ts.model.RunNaplesCommand(nc, halcmd); err == nil {
					log.Info(out)
					if len(out) == 0 {
						return fmt.Errorf("mode change failed")
					}
				} else {
					return fmt.Errorf("Failed to execute hal")
				}
				return nil
			})

			halcmd = fmt.Sprintf("/nic/bin/halctl show system mode | /bin/grep 'POLICY_MODE_ENFORCED'")
			ts.model.ForEachNaples(func(nc *objects.NaplesCollection) error {
				if out, err := ts.model.RunNaplesCommand(nc, halcmd); err == nil {
					log.Info(out)
					if len(out) == 0 {
						return fmt.Errorf("mode change failed")
					}
				} else {
					return fmt.Errorf("Failed to execute hal")
				}
				return nil
			})
			//Expect(ts.model.ConfigClient().DeleteDscProfile(&dscProfile)).ShouldNot(HaveOccurred())

		})

	})

})
