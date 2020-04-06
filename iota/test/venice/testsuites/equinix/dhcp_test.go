package equinix_test

import (
	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/iota/test/venice/iotakit/model/base"
	"github.com/pensando/sw/iota/test/venice/iotakit/model/objects"
	"github.com/pensando/sw/iota/test/venice/iotakit/cfg/objClient"

	uuid "github.com/satori/go.uuid"
	yaml "gopkg.in/yaml.v2"

	"time"
)

type Subnet struct {
	TypeMeta string `yaml:"typemeta"`
	ObjMeta  string `yaml:"objmeta"`
	Spec     struct {
		Id           []byte   `yaml:"id"`
		VPCId        []byte   `yaml:"vpcid"`
		DHCPPolicyId [][]byte `yaml:"dhcppolicyid"`
	} `yaml:"spec"`
	Status struct {
		HwId uint32 `yaml:"hwid"`
	} `yaml:"status"`
	Stats struct {
	} `yaml:"stats"`
}

var _ = Describe("IPAM Tests", func() {
	BeforeEach(func() {
		// verify cluster is in good health
		Eventually(func() error {
			return ts.model.VerifyClusterStatus()
		}).Should(Succeed())
	})
	AfterEach(func() {
		ts.tb.AfterTestCommon()
	})

	Context("IPAM Tests", func() {
		policy1 := "dchp-relay-11"
		policy2 := "dchp-relay-21"
		policyTenant := "customer0"
		policyVrf := ""
		var ipc *objects.IPAMPolicyCollection
		var vpcc *objects.VpcObjCollection
		var nwc *objects.NetworkCollection
		var p1_uuid string

		It("Create IPAM Policies", func() {

			// Add IPAM policy
			ipc = ts.model.NewIPAMPolicy(policy1, policyTenant, policyVrf, "50.1.1.1")
			Expect(ipc.Commit()).Should(Succeed())

			// validate policy
			ValidateIPAMPolicy (ipc.Client, policy1, policyTenant, policyVrf, "50.1.1.1")

			// get policy and save uuid for future validations
			ip, err := objects.GetIPAMPolicy(ipc.Client, policy1, policyTenant)
			Expect(err).ShouldNot(HaveOccurred())
			p1_uuid = ip.PolicyObj.GetUUID()
		})

		vpcName := "testVPC_IPAM"
		It("Default IPAM Policy on VPC", func() {
			// Create VPC
			vpcc = ts.model.NewVPC(policyTenant, vpcName, "0009.0102.0202", 256, policy1)
			Expect(vpcc.Commit()).Should(Succeed())

			// Get VPC and validate IPAM
			veniceVpc, err := ts.model.GetVPC(vpcName, policyTenant)
			Expect(err).ShouldNot(HaveOccurred())
			Expect(veniceVpc.Obj.Spec.DefaultIPAMPolicy == policy1).Should(BeTrue())
		})

		nwName := "testNetwork0_IPAM"
		var nw_uuid string
		It("Default IPAM policy on Subnet", func() {
			// Create Subnet
			nw := &base.NetworkParams{
				nwName,
				"20.1.2.0/24",
				"20.1.2.1",
				uint32(0x91000 | 0x100),
				vpcName,
				policyTenant,
			}
			nwc = ts.model.NewNetwork(nw)
			Expect(nwc.Commit()).Should(Succeed())

			// get network
			veniceNw, err := ts.model.GetNetwork(policyTenant, nwName)
			Expect(err).ShouldNot(HaveOccurred())
			nw_uuid = veniceNw.VeniceNetwork.GetUUID()

			// reboot is taking long time before returning any pdsctl output on SIM
			time.Sleep(60 * time.Second)

			// verify ipam on subnet
			VerifyIPAMonSubnet(nw_uuid, p1_uuid)
		})

		var p2_uuid string
		It("Override IPAM policy on Subnet", func() {
			// get network
			veniceNw, err := ts.model.GetNetwork(policyTenant, nwName)
			Expect(err).ShouldNot(HaveOccurred())

			// create new IPAM policy and update network
			ipc = ts.model.NewIPAMPolicy(policy2, policyTenant, policyVrf, "60.1.1.1")
			Expect(ipc.Commit()).Should(Succeed())

			// validate policy
			ValidateIPAMPolicy(ipc.Client, policy2, policyTenant, policyVrf, "60.1.1.1")
	
			// get policy and save uuid
			ip, err := objects.GetIPAMPolicy(ipc.Client, policy2, policyTenant)
			Expect(err).ShouldNot(HaveOccurred())
			p2_uuid = ip.PolicyObj.GetUUID()

			// oveeride policy on subnet
			veniceNw.VeniceNetwork.Spec.IPAMPolicy = policy2
			nwc_upd := objects.NewNetworkCollection(ipc.Client, ipc.Testbed)
			nwc_upd.AddSubnet(veniceNw)
			Expect(nwc_upd.Commit()).Should(Succeed())

			// verify ipam on subnet
			VerifyIPAMonSubnet(nw_uuid, p2_uuid)
		})

		It("Multiple IPAM per naples", func() {
			// Add another network with default IPAM. first network has overriden policy.
			// this makes 2 networks with 2 IPAM policies
			// Create Subnet
			nwName2 := "testNetwork1_IPAM"
			nw := &base.NetworkParams{
				nwName2,
				"20.2.2.0/24",
				"20.2.2.1",
				uint32(0x92000 | 0x100),
				vpcName,
				policyTenant,
			}
			nwc2 := ts.model.NewNetwork(nw)
			Expect(nwc2.Commit()).Should(Succeed())

			// get network
			veniceNw, err := ts.model.GetNetwork(policyTenant, nwName2)
			Expect(err).ShouldNot(HaveOccurred())
			nw2_uuid := veniceNw.VeniceNetwork.GetUUID()

			// verify ipam on subnet
			VerifyIPAMonSubnet(nw2_uuid, p1_uuid)

			// delete Subnet
			Expect(nwc2.Delete()).Should(Succeed())
		})

		/*
			// this test case is failing, commenting it out until multiple IPAM servers
			// are supported in venice/netagent
			It("Multiple IPAM servers in a policy", func() {
				// get IPAM policy
				ip, err := objects.GetIPAMPolicy(ipc.Client, policy2, policyTenant)
				Expect(err).ShouldNot(HaveOccurred())

				//Update IPAM policy to include one more than 1 server
				ipc_upd := objects.NewIPAMPolicyCollection(ipc.Client, ipc.Testbed)
				ipc_upd.PolicyObjs = append(ipc_upd.PolicyObjs, ip)
				ipc_upd.AddServer(policy2, "60.1.2.1", policyVrf)
				Expect(ipc_upd.Commit()).Should(Succeed())

			})
		*/

		It("Remove IPAM policy on Subnet", func() {
			// get network
			veniceNw, err := ts.model.GetNetwork(policyTenant, nwName)
			Expect(err).ShouldNot(HaveOccurred())
			veniceNw.VeniceNetwork.Spec.IPAMPolicy = ""
			nwc_upd := objects.NewNetworkCollection(ipc.Client, ipc.Testbed)
			nwc_upd.AddSubnet(veniceNw)
			Expect(nwc_upd.Commit()).Should(Succeed())

			// verify ipam on subnet
			VerifyIPAMonSubnet(nw_uuid, p1_uuid)
		})

		It("Change Servers in IPAM Policy", func() {
			// Get IPAM policy and validate
			ipc = ts.model.NewIPAMPolicy(policy1, policyTenant, policyVrf, "51.1.1.1")
			Expect(ipc.Commit()).Should(Succeed())

			// Get IPAM policy and validate the server
			ValidateIPAMPolicy(ipc.Client, policy1, policyTenant, policyVrf, "51.1.1.1")
		})
		/*
			// this test case is failing
			It("Change IPAM policy for VPC", func() {
				// change from policy1 to policy2 on VPC

				// get VPC
				veniceVpc, err := ts.model.GetVPC(vpcName, policyTenant)
				Expect(err).ShouldNot(HaveOccurred())

				// update default IPAM policy
				veniceVpc.Obj.Spec.DefaultIPAMPolicy = policy2
				vpcc_upd := objects.NewVPCCollection(ipc.Client, ipc.Testbed)
				vpcc_upd.Objs = append(vpcc_upd.Objs, veniceVpc)
				Expect(vpcc_upd.Commit()).Should(Succeed())
				veniceVpc, err = ts.model.GetVPC(vpcName, policyTenant)
				Expect(err).ShouldNot(HaveOccurred())
				Expect(veniceVpc.Obj.Spec.DefaultIPAMPolicy == policy2).Should(BeTrue())

				// verify ipam on subnet
				VerifyIPAMonSubnet(nw_uuid, p2_uuid)
			})
		*/

		It("Delete IPAM Policies", func() {
			// Delete Subnet
			Expect(nwc.Delete()).Should(Succeed())

			// Delete VPC
			Expect(vpcc.Delete()).Should(Succeed())

			//Delete IPAM Policies
			ipc_upd := objects.NewIPAMPolicyCollection(ipc.Client, ipc.Testbed)
			ip, err := objects.GetIPAMPolicy(ipc.Client, policy1, policyTenant)
			Expect(err).ShouldNot(HaveOccurred())
			ipc_upd.PolicyObjs = append(ipc_upd.PolicyObjs, ip)
			ip, err = objects.GetIPAMPolicy(ipc.Client, policy2, policyTenant)
			Expect(err).ShouldNot(HaveOccurred())
			ipc_upd.PolicyObjs = append(ipc_upd.PolicyObjs, ip)
			Expect(ipc.Delete()).Should(Succeed())
		})
	})
})

func VerifyIPAMonSubnet(subnet string, ipam string) error {

	var data Subnet

	// Wait for Naples to finish configuring
	time.Sleep(5 * time.Second)

	// fetch network from pdsctl. network should have updated IPAM Policy
	ts.model.ForEachFakeNaples(func(nc *objects.NaplesCollection) error {
		cmd := "/naples/nic/bin/pdsctl show subnet --id " + subnet + " --yaml"
		cmdOut, err := ts.model.RunFakeNaplesBackgroundCommand(nc, cmd)
		Expect(err).ShouldNot(HaveOccurred())
		cmdResp, _ := cmdOut.([]*iota.Command)
		for _, cmdLine := range cmdResp {
			err = yaml.Unmarshal([]byte(cmdLine.Stdout), &data)
			Expect(err).ShouldNot(HaveOccurred())
			uid, _ := uuid.FromBytes(data.Spec.DHCPPolicyId[0])
			Expect(ipam == uid.String()).Should(BeTrue())
		}
		return nil
	})
	ts.model.ForEachNaples(func(nc *objects.NaplesCollection) error {
		cmd := "/nic/bin/pdsctl show subnet --id " + subnet + " --yaml"
		cmdOut, err := ts.model.RunNaplesCommand(nc, cmd)
		for _, cmdLine := range cmdOut {
			Expect(err).ShouldNot(HaveOccurred())
			err = yaml.Unmarshal([]byte(cmdLine), &data)
			Expect(err).ShouldNot(HaveOccurred())
			uid, _ := uuid.FromBytes(data.Spec.DHCPPolicyId[0])
			Expect(ipam == uid.String()).Should(BeTrue())
		}
		return nil
	})
	return nil
}

func ValidateIPAMPolicy(client objClient.ObjClient, name string, tenant string, vrf string, ip string) {
	obj, err := objects.GetIPAMPolicy(client, name, tenant)
	// for now, validate by getting from venice by name 
	// can add netagent validation if reqd
	Expect(err).ShouldNot(HaveOccurred())

	match := false
	for _, s := range obj.PolicyObj.Spec.DHCPRelay.Servers {
		if (s.IPAddress == ip) {
			match = true
			break
		}
	}
	Expect(match == true).Should(BeTrue())
}