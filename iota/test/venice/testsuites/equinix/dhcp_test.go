package equinix_test

import (
	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/iota/test/venice/iotakit/model/objects"
	"github.com/pensando/sw/venice/utils/log"

	uuid "github.com/satori/go.uuid"
	yaml "gopkg.in/yaml.v2"

	"time"
)

type Subnet struct {
	TypeMeta string `yaml:"typemeta"`
	ObjMeta  string `yaml:"objmeta"`
	Spec     struct {
		Id         []byte `yaml:"id"`
		VPCId      []byte `yaml:"vpcid"`
		IPV4prefix struct {
			Addr uint32 `yaml:"addr"`
			Len  uint32 `yaml:"len"`
		} `yaml:"v4prefix"`
		IPv4Gateway uint32 `yaml:"ipv4virtualrouterip"`
		FabricEncap struct {
			Type  int32 `yaml:"type"`
			Value struct {
				Val struct {
					Vnid uint32 `yaml:"vnid"`
				} `yaml:"val"`
			} `yaml:"value"`
		} `yaml:"fabricencap"`
		DHCPPolicyId [][]byte `yaml:"dhcppolicyid"`
	} `yaml:"spec"`
	Status struct {
		HwId uint32 `yaml:"hwid"`
	} `yaml:"status"`
	Stats struct {
	} `yaml:"stats"`
}

var _ = Describe("IPAM Tests", func() {

	var tenant string = "customer0"
	var defaultIpam string
	var serverip string = "20.20.3.1"
	var customIpam string = "dhcp_relay_1"
	var vpcName string
	var vpcc *objects.VpcObjCollection

	BeforeEach(func() {
		// verify cluster is in good health
		Eventually(func() error {
			return ts.model.VerifyClusterStatus()
		}).Should(Succeed())

		// get IPAM policy params of default tenant VRF
		vt, err := objects.TenantVPCCollection(tenant, ts.model.ConfigClient(), ts.model.Testbed())
		Expect(err).Should(Succeed())
		vpcc = vt
		vpcName = vpcc.Objs[0].Obj.Name
		defaultIpam = vpcc.Objs[0].Obj.Spec.DefaultIPAMPolicy
		ip, err := objects.GetIPAMPolicy(ts.model.ConfigClient(), defaultIpam, tenant)
		Expect(err).Should(Succeed())
		serverip = ip.PolicyObj.Spec.DHCPRelay.Servers[0].IPAddress

		// create a Custom IPAM policy
		createIPAMPolicy(customIpam, "", tenant, serverip)

	})
	AfterEach(func() {
		// get back to default policy on VPC
		Expect(vpcc.SetIPAM(defaultIpam)).Should(Succeed())

		// delete Custom IPAM policy
		deleteIPAMPolicy(customIpam, tenant)

	})

	Context("IPAM Tests", func() {

		It("Default IPAM Policy on VPC", func() {

			// apply customIpam to VPC- this verifies update IPAM on VPC case also
			Expect(vpcc.SetIPAM(customIpam)).Should(Succeed())

			// get network from vpc
			nwc, err := GetNetworkCollectionFromVPC(vpcName, tenant)
			Expect(err).Should(Succeed())
			selNetwork := nwc.Any(1)

			// verify ipam on subnet
			// TODO: Enable and fix this once pdsctl show dhcp starts showing dhcpPolicy created
			// verifyIPAMonSubnet(selNetwork.Subnets()[0].Name, customIpam, tenant)

			if ts.tb.HasNaplesHW() {
				// get workload pair and validate datapath
				wpc := ts.model.WorkloadPairs().OnNetwork(selNetwork.Subnets()[0]).Any(1)
				verifyIPAMDataPath(wpc)
			}
		})

		It("Override/Remove IPAM policy on Subnet", func() {

			// get network from vpc
			nwc, err := GetNetworkCollectionFromVPC(vpcName, tenant)
			Expect(err).Should(Succeed())

			// override customIpam on the subnet
			selNetwork := nwc.Any(1)
			Expect(selNetwork.SetIPAMOnNetwork(selNetwork.Subnets()[0], customIpam)).Should(Succeed())

			// verify ipam on subnet
			// TODO: Enable and fix this once pdsctl show dhcp starts showing dhcpPolicy created
			// verifyIPAMonSubnet(selNetwork.Subnets()[0].Name, customIpam, tenant)

			if ts.tb.HasNaplesHW() {
				// get workload pair and validate datapath
				wpc := ts.model.WorkloadPairs().OnNetwork(selNetwork.Subnets()[0]).Any(1)
				verifyIPAMDataPath(wpc)
			}

			// remove ipam on subnet - it should point to default IPAM policy on VPC
			Expect(selNetwork.SetIPAMOnNetwork(selNetwork.Subnets()[0], "")).Should(Succeed())

			// verify ipam on subnet
			// TODO: Enable and fix this once pdsctl show dhcp starts showing dhcpPolicy created
			// verifyIPAMonSubnet(selNetwork.Subnets()[0].Name, defaultIpam, tenant)

			if ts.tb.HasNaplesHW() {
				// get workload pair and validate datapath
				wpc := ts.model.WorkloadPairs().OnNetwork(selNetwork.Subnets()[0]).Any(1)
				verifyIPAMDataPath(wpc)
			}
		})

		It("Multiple IPAM per naples", func() {

			// get networks from vpc
			nwc, err := GetNetworkCollectionFromVPC(vpcName, tenant)
			Expect(err).Should(Succeed())

			if len(nwc.Subnets()) < 2 {
				Skip("Not enough subnets to verify multiple IPAMs per naples")
			}

			selNetwork := nwc.Any(2)
			// override customIpam on one of the subnets
			Expect(selNetwork.SetIPAMOnNetwork(selNetwork.Subnets()[0], customIpam)).Should(Succeed())

			// verify ipam on subnets
			// TODO: Enable and fix this once pdsctl show dhcp starts showing dhcpPolicy created
			// verifyIPAMonSubnet(selNetwork.Subnets()[0].Name, customIpam, tenant)
			// verifyIPAMonSubnet(selNetwork.Subnets()[1].Name, defaultIpam, tenant)

			if ts.tb.HasNaplesHW() {
				// get workload pair and validate datapath - customIpam
				wpc := ts.model.WorkloadPairs().OnNetwork(selNetwork.Subnets()[0]).Any(1)
				verifyIPAMDataPath(wpc)

				// get workload pair and validate datapath - defaultIpam
				wpc = ts.model.WorkloadPairs().OnNetwork(selNetwork.Subnets()[1]).Any(1)
				verifyIPAMDataPath(wpc)
			}

			// remove ipam on subnet - it should point to default IPAM policy on VPC
			Expect(selNetwork.SetIPAMOnNetwork(selNetwork.Subnets()[0], "")).Should(Succeed())

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

		It("Change Servers in IPAM Policy", func() {

			// create a test-policy with random dhcp server
			createIPAMPolicy("test-policy", "", tenant, "51.1.1.1")

			// apply "test-policy" Ipam to tenant VPC
			Expect(vpcc.SetIPAM("test-policy")).Should(Succeed())

			// get network from vpc
			nwc, err := GetNetworkCollectionFromVPC(vpcName, tenant)
			Expect(err).Should(Succeed())
			selNetwork := nwc.Any(1)

			// verify ipam on subnet
			// TODO: Enable and fix this once pdsctl show dhcp starts showing dhcpPolicy created
			// verifyIPAMonSubnet(selNetwork.Subnets()[0].Name, "test-policy", tenant)
			var wpc *objects.WorkloadPairCollection
			if ts.tb.HasNaplesHW() {
				// get workload pair and validate datapath
				wpc = ts.model.WorkloadPairs().OnNetwork(selNetwork.Subnets()[0]).Any(1)
				verifyIPAMDataPathFail(wpc)
			}

			// update test-policy
			createIPAMPolicy("test-policy", "", tenant, serverip)
			if ts.tb.HasNaplesHW() {
				// get workload pair and validate datapath
				verifyIPAMDataPath(wpc)
			}

			// get back to default policy on VPC
			Expect(vpcc.SetIPAM(defaultIpam)).Should(Succeed())

			// delete test-policy
			deleteIPAMPolicy("test-policy", tenant)
		})

		It("Remove IPAM policy on VPC", func() {

			// remove IPAM policy on tenant vpc
			Expect(vpcc.SetIPAM("")).Should(Succeed())

			// get network from vpc
			nwc, err := GetNetworkCollectionFromVPC(vpcName, tenant)
			Expect(err).Should(Succeed())
			selNetwork := nwc.Any(1)

			// verify ipam on subnet
			verifyNoIPAMonSubnet(selNetwork.Subnets()[0].Name, tenant)

			// ping should fail as there no ipam policy configured to get DHCP ip for host
			var wpc *objects.WorkloadPairCollection
			if ts.tb.HasNaplesHW() {
				wpc = ts.model.WorkloadPairs().OnNetwork(selNetwork.Subnets()[0]).Any(1)
				verifyIPAMDataPathFail(wpc)
			}

			// get back to default policy on VPC
			Expect(vpcc.SetIPAM(defaultIpam)).Should(Succeed())
			if ts.tb.HasNaplesHW() {
				verifyIPAMDataPath(wpc)
			}
		})

	})
})

func createIPAMPolicy(ipam, vpc, tenant string, serverip string) {
	ipc := ts.model.NewIPAMPolicy(ipam, tenant, vpc, serverip)
	Expect(ipc.Commit()).Should(Succeed())
	// validate policy
	validateIPAMPolicy(ipam, vpc, tenant, serverip)
}

func deleteIPAMPolicy(ipam, tenant string) {
	ipc := objects.NewIPAMPolicyCollection(ts.model.ConfigClient(), ts.model.Testbed())
	ip, err := objects.GetIPAMPolicy(ts.model.ConfigClient(), ipam, tenant)
	Expect(err).ShouldNot(HaveOccurred())
	ipc.PolicyObjs = append(ipc.PolicyObjs, ip)
	Expect(ipc.Delete()).Should(Succeed())
}

func GetNetworkCollectionFromVPC(vpc, tenant string) (*objects.NetworkCollection, error) {

	numSubnets := 0

	ten, err := ts.model.ConfigClient().ListTenant()
	if err != nil {
		return nil, err
	}

	numSubnets = ts.model.Networks(ten[0].Name).Count()

	return objects.VpcNetworkCollection(tenant, vpc, numSubnets, ts.model.ConfigClient())
}

func verifyIPAMonSubnet(subnet, ipam, tenant string) error {

	var data Subnet

	// get network
	veniceNw, err := ts.model.GetNetwork(tenant, subnet)
	Expect(err).ShouldNot(HaveOccurred())
	nw_uuid := veniceNw.VeniceNetwork.GetUUID()

	// get ipam
	veniceIpam, err := objects.GetIPAMPolicy(ts.model.ConfigClient(), ipam, tenant)
	Expect(err).ShouldNot(HaveOccurred())
	ipam_uuid := veniceIpam.PolicyObj.GetUUID()

	var matchUUID = func(s string) bool {
		err = yaml.Unmarshal([]byte(s), &data)
		Expect(err).ShouldNot(HaveOccurred())
		uid, _ := uuid.FromBytes(data.Spec.DHCPPolicyId[0])
		log.Infof("IPAM UUID (venice):%v && Subnet UUID(pdsctl) :%v", ipam_uuid, uid.String())
		return ipam_uuid == uid.String()
	}

	// wait for Naples to finish configuring
	time.Sleep(5 * time.Second)

	// fetch network from pdsctl. network should have updated IPAM Policy
	ts.model.ForEachFakeNaples(func(nc *objects.NaplesCollection) error {
		cmd := "/naples/nic/bin/pdsctl show subnet --id " + nw_uuid + " --yaml"
		cmdOut, err := ts.model.RunFakeNaplesBackgroundCommand(nc, cmd)
		Expect(err).ShouldNot(HaveOccurred())
		cmdResp, _ := cmdOut.([]*iota.Command)
		for _, cmdLine := range cmdResp {
			Expect(matchUUID(cmdLine.Stdout)).Should(BeTrue())
		}
		return nil
	})
	ts.model.ForEachNaples(func(nc *objects.NaplesCollection) error {
		cmd := "/nic/bin/pdsctl show subnet --id " + nw_uuid + " --yaml"
		cmdOut, err := ts.model.RunNaplesCommand(nc, cmd)
		Expect(err).ShouldNot(HaveOccurred())
		for _, cmdLine := range cmdOut {
			Expect(matchUUID(cmdLine)).Should(BeTrue())
		}
		return nil
	})
	return nil
}

func verifyNoIPAMonSubnet(subnet, tenant string) error {

	var data Subnet

	// get network
	veniceNw, err := ts.model.GetNetwork(tenant, subnet)
	Expect(err).ShouldNot(HaveOccurred())
	nw_uuid := veniceNw.VeniceNetwork.GetUUID()

	// wait for Naples to finish configuring
	time.Sleep(5 * time.Second)

	var allZeroByteArray = func(b []byte) bool {
		for _, v := range b {
			if v != 0 {
				return false
			}
		}
		return true
	}

	// fetch network from pdsctl. network should have updated IPAM Policy
	ts.model.ForEachFakeNaples(func(nc *objects.NaplesCollection) error {
		cmd := "/naples/nic/bin/pdsctl show subnet --id " + nw_uuid + " --yaml"
		cmdOut, err := ts.model.RunFakeNaplesBackgroundCommand(nc, cmd)
		Expect(err).ShouldNot(HaveOccurred())
		cmdResp, _ := cmdOut.([]*iota.Command)
		for _, cmdLine := range cmdResp {
			err = yaml.Unmarshal([]byte(cmdLine.Stdout), &data)
			Expect(err).ShouldNot(HaveOccurred())
			Expect(allZeroByteArray(data.Spec.DHCPPolicyId[0])).Should(BeTrue())
		}
		return nil
	})
	ts.model.ForEachNaples(func(nc *objects.NaplesCollection) error {
		cmd := "/nic/bin/pdsctl show subnet --id " + nw_uuid + " --yaml"
		cmdOut, err := ts.model.RunNaplesCommand(nc, cmd)
		Expect(err).ShouldNot(HaveOccurred())
		for _, cmdLine := range cmdOut {
			err = yaml.Unmarshal([]byte(cmdLine), &data)
			Expect(err).ShouldNot(HaveOccurred())
			Expect(allZeroByteArray(data.Spec.DHCPPolicyId[0])).Should(BeTrue())
		}
		return nil
	})
	return nil
}

func validateIPAMPolicy(name, vrf, tenant string, ip string) {
	obj, err := objects.GetIPAMPolicy(ts.model.ConfigClient(), name, tenant)
	// for now, validate by getting from venice by name
	// can add netagent validation if reqd
	Expect(err).ShouldNot(HaveOccurred())

	match := false
	for _, s := range obj.PolicyObj.Spec.DHCPRelay.Servers {
		if s.IPAddress == ip {
			match = true
			break
		}
	}
	Expect(match == true).Should(BeTrue())
}

func verifyIPAMDataPath(wpc *objects.WorkloadPairCollection) {
	Expect(len(wpc.Pairs) != 0).Should(BeTrue())

	// Get dynamic IP for workloadPair
	err := wpc.WorkloadPairGetDynamicIps(ts.model.Testbed())
	Expect(err).Should(Succeed())

	Eventually(func() error {
		return ts.model.PingPairs(wpc)
	}).Should(Succeed())
}

func verifyIPAMDataPathFail(wpc *objects.WorkloadPairCollection) {
	Expect(len(wpc.Pairs) != 0).Should(BeTrue())

	// Get dynamic IP for workloadPair
	err := wpc.WorkloadPairGetDynamicIps(ts.model.Testbed())
	Expect(err).ShouldNot(Succeed())
}
