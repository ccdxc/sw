package equinix_test

import (
	"encoding/binary"
	"encoding/json"
	"fmt"
	"net"
	"reflect"
	"strings"
	"time"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
	"github.com/pensando/sw/iota/test/venice/iotakit/model/base"
	"github.com/pensando/sw/iota/test/venice/iotakit/model/objects"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
	uuid "github.com/satori/go.uuid"
	yaml "gopkg.in/yaml.v2"
)

var savedTenant, savedNetwork string

var _ = Describe("Network", func() {

	var startTime time.Time

	BeforeEach(func() {
		startTime = time.Now().UTC()
		// verify cluster is in good health
		Eventually(func() error {
			return ts.model.VerifyClusterStatus()
		}).Should(Succeed())
	})
	AfterEach(func() {
		Expect(ts.model.ServiceStoppedEvents(startTime, ts.model.Naples()).Len(0))
	})

	Context("Network tests", func() {
		It("Add a subnet & verify config", func() {
			tenantName, err := defaultTenantName()
			Expect(err).ShouldNot(HaveOccurred())
			log.Infof("Tenant name : %s", tenantName)

			//Create VPC config
			vpcName := "testVPC"
			vpcVni := uint32(700)
			vpc := ts.model.NewVPC(tenantName, vpcName, "0001.0102.0202", vpcVni, "")
			Expect(vpc.Commit()).Should(Succeed())

			//Validate using Get command
			veniceVpc, err := ts.model.GetVPC(vpcName, tenantName)
			Expect(err).ShouldNot(HaveOccurred())
			Expect(veniceVpc.Obj.Name == vpcName).Should(BeTrue())

			//Create a subnet
			nwName := "testNetwork"
			nwVni := 0x80000 | vpcVni
			log.Infof("NW Vni %v", nwVni)
			nwp := &base.NetworkParams{
				nwName,
				"10.1.2.0/24",
				"10.1.2.1",
				nwVni,
				vpcName,
				tenantName,
			}
			nwc := ts.model.NewNetwork(nwp)
			Expect(nwc.Commit()).Should(Succeed())

			//Verify network config in Venice

			nw, err := ts.model.GetNetwork(tenantName, nwName)
			Expect(err).ShouldNot(HaveOccurred())
			Expect(nw.VeniceNetwork.Name == nwName).Should(BeTrue())

			// get all host nw interfaces
			nwIfs := AttachNwInterfaceToSubnet(tenantName, nwName)
			uuid := nw.VeniceNetwork.GetUUID()
			log.Infof("Network object UUID %s", uuid)

			// Wait for Naples to finish configuring
			time.Sleep(20 * time.Second)
			//Verify state in Naples
			nwList := ts.model.Networks(tenantName)
			verifyNetAgentNwState(nwList)
			verifyPDSNwState(nwList)

			// detach the interface to the subnet
			DetachNwInterfaceFromSubnet(tenantName, nwName, nwIfs)
			Expect(nwc.Delete()).Should(Succeed())
			Expect(vpc.Delete()).Should(Succeed())

			//Verify state in Naples
			nwList = ts.model.Networks(tenantName)
			verifyNetAgentNwState(nwList)
			verifyPDSNwState(nwList)
		})

		It("Change subnet prefix & verify config", func() {
			tenantName, err := defaultTenantName()
			Expect(err).ShouldNot(HaveOccurred())
			log.Infof("Tenant name : %s", tenantName)

			//Create VPC config
			vpcName := "testVPC"
			vpcVni := uint32(700)
			vpc := ts.model.NewVPC(tenantName, vpcName, "0001.0102.0202", vpcVni, "")
			Expect(vpc.Commit()).Should(Succeed())

			//Create a subnet
			nwName := "testNetwork"
			nwVni := 0x80000 | vpcVni
			log.Infof("NW Vni %v", nwVni)
			nwp := &base.NetworkParams{
				nwName,
				"10.100.2.0/24",
				"10.100.2.1",
				nwVni,
				vpcName,
				tenantName,
			}
			nwc := ts.model.NewNetwork(nwp)
			Expect(nwc.Commit()).Should(Succeed())

			//Verify network config in Venice
			nw, err := ts.model.GetNetwork(tenantName, nwName)
			Expect(err).ShouldNot(HaveOccurred())
			Expect(nw.VeniceNetwork.Name == nwName).Should(BeTrue())

			uuid := nw.VeniceNetwork.GetUUID()
			log.Infof("Network object UUID %s", uuid)

			// get all host nw interfaces
			nwIfs := AttachNwInterfaceToSubnet(tenantName, nwName)

			newPrefix := "10.100.0.0/16"
			snc := nwc.Subnets()
			snc[0].UpdateIPv4Subnet(newPrefix)
			log.Infof("Updating nw prefix from %s to %s", nwp.Ip, newPrefix)
			Expect(nwc.Commit()).Should(Succeed())

			nw, err = ts.model.GetNetwork(tenantName, nwName)
			Expect(err).ShouldNot(HaveOccurred())
			Expect(nw.VeniceNetwork.Spec.GetIPv4Subnet() == newPrefix).Should(BeTrue())

			// Wait for Naples to finish configuring
			time.Sleep(10 * time.Second)

			nwList := ts.model.Networks(tenantName)
			verifyNetAgentNwState(nwList)
			verifyPDSNwState(nwList)

			// detach the interface to the subnet
			DetachNwInterfaceFromSubnet(tenantName, nwName, nwIfs)
			Expect(nwc.Delete()).Should(Succeed())
			Expect(vpc.Delete()).Should(Succeed())

			nwList = ts.model.Networks(tenantName)
			verifyNetAgentNwState(nwList)
			verifyPDSNwState(nwList)
		})

		It("Change Gateway IP & verify config", func() {
			//Get existing tenant and network
			nwc, err := getNetworkCollection()
			Expect(err).ShouldNot(HaveOccurred())

			nwc = nwc.Any(1)
			nw := nwc.Subnets()[0]

			log.Infof("Network : %+v", nw)
			tenantName := nw.VeniceNetwork.Tenant
			nwName := nw.VeniceNetwork.Name

			//update Gateway IP
			snc := nwc.Subnets()
			oldGw := snc[0].VeniceNetwork.Spec.GetIPv4Gateway()

			ipBytes := strings.Split(oldGw, ".")
			//lastByte, _ := strconv.Atoi(ipBytes[3])
			newGw := fmt.Sprintf("%s.%s.%s.250", ipBytes[0], ipBytes[1], ipBytes[2])
			//newGw := fmt.Sprintf("%s.%s.%s.%v", ipBytes[0], ipBytes[1], ipBytes[2],
			//	(lastByte+)%256)

			snc[0].UpdateIPv4Gateway(newGw)
			log.Infof("Gateway IP change from %s to %s", oldGw, newGw)
			Expect(nwc.Commit()).Should(Succeed())

			nw, err = ts.model.GetNetwork(tenantName, nwName)
			Expect(err).ShouldNot(HaveOccurred())
			Expect(nw.VeniceNetwork.Spec.GetIPv4Gateway() == newGw).Should(BeTrue())

			// Wait for Naples to finish configuring
			time.Sleep(10 * time.Second)
			nwList := ts.model.Networks(tenantName)
			verifyNetAgentNwState(nwList)
			verifyPDSNwState(nwList)

			//Revert to original
			snc[0].UpdateIPv4Gateway(oldGw)
			log.Infof("Gateway IP change from %s to %s", newGw, oldGw)
			Expect(nwc.Commit()).Should(Succeed())

			nw, err = ts.model.GetNetwork(tenantName, nwName)
			Expect(err).ShouldNot(HaveOccurred())
			Expect(nw.VeniceNetwork.Spec.GetIPv4Gateway() == oldGw).Should(BeTrue())

			// Wait for Naples to finish configuring
			time.Sleep(10 * time.Second)
			nwList = ts.model.Networks(tenantName)
			verifyNetAgentNwState(nwList)
			verifyPDSNwState(nwList)
		})

		It("Change Vni & verify it's not allowed", func() {
			//Get existing tenant and network
			nwc, err := getNetworkCollection()
			Expect(err).ShouldNot(HaveOccurred())

			nwc = nwc.Any(1)
			nw := nwc.Subnets()[0]

			log.Infof("Network : %+v", nw)
			tenantName := nw.VeniceNetwork.Tenant
			nwName := nw.VeniceNetwork.Name

			//update Vni
			snc := nwc.Subnets()
			nwVni := snc[0].VeniceNetwork.Spec.VxlanVNI
			tempVni := nwVni + 10
			snc[0].VeniceNetwork.Spec.VxlanVNI = tempVni
			log.Infof("Trying Vni change from %v to %v", nwVni, tempVni)
			Expect(nwc.Commit()).ShouldNot(Succeed())

			nw, err = ts.model.GetNetwork(tenantName, nwName)
			Expect(err).ShouldNot(HaveOccurred())
			Expect(nw.VeniceNetwork.Spec.GetVxlanVNI() == nwVni).Should(BeTrue())

			// Wait for Naples to finish configuring
			time.Sleep(10 * time.Second)
			nwList := ts.model.Networks(tenantName)
			verifyNetAgentNwState(nwList)
			verifyPDSNwState(nwList)
		})

		It("Change VPC for subnet & verify it's not allowed", func() {
			//Get existing tenant and network
			nwc, err := getNetworkCollection()
			Expect(err).ShouldNot(HaveOccurred())

			nwc = nwc.Any(1)
			nw := nwc.Subnets()[0]

			log.Infof("Network : %+v", nw)
			tenantName := nw.VeniceNetwork.Tenant
			nwName := nw.VeniceNetwork.Name

			tempVpcName := "testVPC"
			tempVpc := ts.model.NewVPC(tenantName, tempVpcName, "0001.0102.0202", uint32(700), "")
			Expect(tempVpc.Commit()).Should(Succeed())

			snc := nwc.Subnets()
			vpc := snc[0].VeniceNetwork.Spec.VirtualRouter
			snc[0].VeniceNetwork.Spec.VirtualRouter = tempVpcName
			log.Infof("Trying to change subnet VPC from %s to %s", vpc, tempVpcName)
			Expect(nwc.Commit()).ShouldNot(Succeed())

			nw, err = ts.model.GetNetwork(tenantName, nwName)
			Expect(err).ShouldNot(HaveOccurred())
			Expect(nw.VeniceNetwork.Spec.GetVirtualRouter() == vpc).Should(BeTrue())

			// Wait for Naples to finish configuring
			time.Sleep(10 * time.Second)
			nwList := ts.model.Networks(tenantName)
			verifyNetAgentNwState(nwList)
			verifyPDSNwState(nwList)
			Expect(tempVpc.Delete()).Should(Succeed())
		})

		It("Change RT & verify config", func() {
			//Get existing tenant and network
			nwc, err := getNetworkCollection()
			Expect(err).ShouldNot(HaveOccurred())

			nwc = nwc.Any(1)
			nw := nwc.Subnets()[0]

			log.Infof("Network : %+v", nw)
			tenantName := nw.VeniceNetwork.Tenant
			nwName := nw.VeniceNetwork.Name

			uuid := nw.VeniceNetwork.GetUUID()
			log.Infof("Network object UUID %s", uuid)

			//Change its RT and commit
			exportRTs := nw.VeniceNetwork.Spec.RouteImportExport.ExportRTs
			importRTs := nw.VeniceNetwork.Spec.RouteImportExport.ImportRTs

			//Update RT value
			offset := uint32(10)
			exportRTs[0].AssignedValue += offset
			importRTs[0].AssignedValue += offset
			log.Infof("Export RT new assigned value %v", exportRTs[0].AssignedValue)
			log.Infof("Import RT new assigned value %v", importRTs[0].AssignedValue)
			Expect(nwc.Commit()).Should(Succeed())

			nwVenice, err := ts.model.GetNetwork(tenantName, nwName)
			Expect(err).ShouldNot(HaveOccurred())
			exportAssignedVal := nwVenice.VeniceNetwork.Spec.RouteImportExport.ExportRTs[0].GetAssignedValue()
			importAssignedVal := nwVenice.VeniceNetwork.Spec.RouteImportExport.ImportRTs[0].GetAssignedValue()
			Expect(exportAssignedVal == exportRTs[0].AssignedValue).Should(BeTrue())
			Expect(importAssignedVal == importRTs[0].AssignedValue).Should(BeTrue())

			// Wait for Naples to finish configuring
			time.Sleep(10 * time.Second)

			verifyNetAgentNwState(nwc)
			verifyPDSNwState(nwc)

			//Restore RT value
			exportRTs[0].AssignedValue -= offset
			importRTs[0].AssignedValue -= offset
			Expect(nwc.Commit()).Should(Succeed())
			verifyNetAgentNwState(nwc)
			verifyPDSNwState(nwc)
		})

	})
})

func AttachNwInterfaceToSubnet(tenant, nw string) *objects.NetworkInterfaceCollection {
	// get all host nw interfaces
	filter := fmt.Sprintf("spec.type=host-pf")
	nwIfs, err := ts.model.ListNetworkInterfacesByFilter(filter)
	Expect(err).ShouldNot(HaveOccurred())
	Expect(nwIfs).ShouldNot(BeNil())
	Expect(len(nwIfs.Interfaces) != 0).Should(BeTrue())

	//First save existing network to revert to
	savedNetwork = nwIfs.Interfaces[0].Spec.AttachNetwork
	savedTenant = nwIfs.Interfaces[0].Spec.AttachTenant
	// attach the first interface to the subnet
	nwIfs.Interfaces[0].Spec.AttachNetwork = nw
	nwIfs.Interfaces[0].Spec.AttachTenant = tenant
	log.Infof("Updating nwif: %s | spec: %v | status: %v", nwIfs.Interfaces[0].Name, nwIfs.Interfaces[0].Spec, nwIfs.Interfaces[0].Status)
	Expect(nwIfs.Commit()).Should(Succeed())
	return nwIfs
}

func DetachNwInterfaceFromSubnet(tenant, nw string, nwIfs *objects.NetworkInterfaceCollection) {

	for _, n := range nwIfs.Interfaces {
		if n.Spec.AttachNetwork == nw && n.Spec.AttachTenant == tenant {
			n.Spec.AttachNetwork = savedNetwork
			n.Spec.AttachTenant = savedTenant
			break
		}
	}
	Expect(nwIfs.Commit()).Should(Succeed())
}

type pdsSubnet struct {
	spec struct {
		uuid string
		//vpc  string
		ip  string
		gw  string
		vni uint32
	}
}

func normalizeVeniceNetworkObj(obj *objects.NetworkCollection) map[string]pdsSubnet {
	cfg := make(map[string]pdsSubnet)

	for _, s := range obj.Subnets() {
		var sn pdsSubnet
		sn.spec.uuid = s.VeniceNetwork.GetUUID()
		sn.spec.ip = s.VeniceNetwork.Spec.GetIPv4Subnet()
		sn.spec.gw = s.VeniceNetwork.Spec.GetIPv4Gateway()
		sn.spec.vni = s.VeniceNetwork.Spec.GetVxlanVNI()
		cfg[sn.spec.uuid] = sn
	}
	return cfg
}

func ipNumtoStr(ipLong uint32) string {
	ipByte := make([]byte, 4)
	binary.BigEndian.PutUint32(ipByte, ipLong)
	ip := net.IP(ipByte)
	return ip.String()
}

func normalizePDSSubnetObj(subnets []*objects.Network, node *objects.Naples, isHWNode bool) map[string]pdsSubnet {

	m := make(map[string]pdsSubnet)

	for _, s := range subnets {
		sn_uuid := s.VeniceNetwork.GetUUID()
		naples := []*objects.Naples{node}
		var cmdOut []string
		if isHWNode {
			cmd := fmt.Sprintf("/nic/bin/pdsctl show subnet --id %s --yaml", sn_uuid)
			cmdOut, _ = ts.model.RunNaplesCommand(&objects.NaplesCollection{Nodes: naples}, cmd)
		} else {
			cmd := fmt.Sprintf("/naples/nic/bin/pdsctl show subnet --id %s --yaml", sn_uuid)
			cmdOut, _ = ts.model.RunFakeNaplesCommand(&objects.NaplesCollection{FakeNodes: naples}, cmd)
			log.Infof("Cmd %s CmdOut %s", cmd, cmdOut)
		}
		var data Subnet
		err := yaml.Unmarshal([]byte(cmdOut[0]), &data)
		if err != nil {
			return nil
		}
		id, _ := uuid.FromBytes(data.Spec.Id)

		var p pdsSubnet
		p.spec.uuid = id.String()

		gwStr := ipNumtoStr(data.Spec.IPV4prefix.Addr)
		prefixLen := data.Spec.IPV4prefix.Len
		ipv4Addr := net.ParseIP(gwStr)
		ipv4Mask := net.CIDRMask(int(prefixLen), 32)

		p.spec.ip = fmt.Sprintf("%s/%v", ipv4Addr.Mask(ipv4Mask), data.Spec.IPV4prefix.Len)
		//p.spec.ip = fmt.Sprintf("%s/%v", ipNumtoStr(data.Spec.IPV4prefix.Addr), data.Spec.IPV4prefix.Len)
		p.spec.gw = ipNumtoStr(data.Spec.IPv4Gateway)
		p.spec.vni = data.Spec.FabricEncap.Value.Val.Vnid

		m[p.spec.uuid] = p
	}

	return m
}

func verifyPDSNwState(obj *objects.NetworkCollection) {
	vSub := normalizeVeniceNetworkObj(obj)

	nodes := ts.model.Naples().FakeNodes
	for _, node := range nodes {
		pdsSub := normalizePDSSubnetObj(obj.Subnets(), node, false)
		log.Infof("Venice obj %+v", vSub)
		log.Infof("PDS obj %+v", pdsSub)
		Expect(reflect.DeepEqual(vSub, pdsSub)).Should(BeTrue())
	}

	nodes = ts.model.Naples().Nodes
	for i, node := range nodes {
		pdsSub := normalizePDSSubnetObj(obj.Subnets(), node, true)
		log.Infof("%d Venice obj %+v", i+1, vSub)
		log.Infof("%d PDS obj %+v", i+1, pdsSub)
		Expect(reflect.DeepEqual(vSub, pdsSub)).Should(BeTrue())
	}
}

//Netagent verification helpers
type netAgentSubnet struct {
	meta struct {
		name   string
		tenant string
		uuid   string
	}
	spec struct {
		vpc               string
		ip                string
		gw                string
		vni               uint32
		routeImportExport string
		ipamPolicy        string
	}
}

func normalizeVeniceForNA(obj *objects.NetworkCollection) map[string]netAgentSubnet {
	cfg := make(map[string]netAgentSubnet)

	for _, s := range obj.Subnets() {
		var sn netAgentSubnet
		sn.meta.name = s.VeniceNetwork.GetName()
		sn.meta.tenant = s.VeniceNetwork.GetTenant()
		sn.meta.uuid = s.VeniceNetwork.GetUUID()

		sn.spec.vpc = s.VeniceNetwork.Spec.GetVirtualRouter()
		sn.spec.ip = s.VeniceNetwork.Spec.GetIPv4Subnet()
		sn.spec.gw = s.VeniceNetwork.Spec.GetIPv4Gateway()
		sn.spec.vni = s.VeniceNetwork.Spec.GetVxlanVNI()
		//if this doesn't work use RDSpec.clone() and keep a copy
		sn.spec.routeImportExport = s.VeniceNetwork.Spec.RouteImportExport.String()
		sn.spec.ipamPolicy = s.VeniceNetwork.Spec.GetIPAMPolicy()

		//log.Infof("Venice RD Spec : %s", sn.spec.routeImportExport)

		cfg[sn.meta.uuid] = sn
	}
	return cfg
}

func normalizeNASubnetObj(subnets []*objects.Network, node *objects.Naples, isHWNode bool) map[string]netAgentSubnet {

	var err error
	m := make(map[string]netAgentSubnet)

	naples := []*objects.Naples{node}
	var cmdOut []string
	cmd := "curl localhost:9007/api/networks/"
	if isHWNode {
		cmdOut, err = ts.model.RunNaplesCommand(&objects.NaplesCollection{Nodes: naples}, cmd)
	} else {
		cmdOut, err = ts.model.RunFakeNaplesCommand(&objects.NaplesCollection{FakeNodes: naples}, cmd)
	}

	//log.Infof("Cmd : %s CmdOut : %v, isHWNode %v", cmd, cmdOut, isHWNode)
	if err != nil {
		log.Infof("NetAgent command returned err %s", err.Error())
		return nil
	}

	var nwData []netproto.Network
	err = json.Unmarshal([]byte(cmdOut[0]), &nwData)
	if err != nil {
		return nil
	}

	for _, s := range subnets {
		name := s.VeniceNetwork.GetName()
		for _, datum := range nwData {
			if name != datum.GetName() {
				continue
			}
			var n netAgentSubnet
			n.meta.name = datum.GetName()
			n.meta.tenant = datum.GetTenant()
			n.meta.uuid = datum.GetUUID()

			n.spec.vpc = datum.Spec.GetVrfName()

			//In netagent, o/p is gw_ip/prefix
			gwStr := ipNumtoStr(datum.Spec.GetV4Address()[0].Address.GetV4Address())
			prefixLen := datum.Spec.GetV4Address()[0].GetPrefixLen()
			ipv4Addr := net.ParseIP(gwStr)
			ipv4Mask := net.CIDRMask(int(prefixLen), 32)

			n.spec.ip = fmt.Sprintf("%s/%v", ipv4Addr.Mask(ipv4Mask), prefixLen)
			n.spec.gw = gwStr
			n.spec.vni = datum.Spec.GetVxLANVNI()
			n.spec.routeImportExport = datum.Spec.RouteImportExport.String()
			//log.Infof("NetAgent RD spec : %s", n.spec.routeImportExport)
			n.spec.ipamPolicy = datum.Spec.GetIPAMPolicy()

			m[n.meta.uuid] = n
		}
	}

	return m
}

func verifyNetAgentNwState(obj *objects.NetworkCollection) {
	vSub := normalizeVeniceForNA(obj)

	nodes := ts.model.Naples().FakeNodes
	for _, node := range nodes {
		naSub := normalizeNASubnetObj(obj.Subnets(), node, false)
		Expect(reflect.DeepEqual(vSub, naSub)).Should(BeTrue())
	}

	nodes = ts.model.Naples().Nodes
	for i, node := range nodes {
		naSub := normalizeNASubnetObj(obj.Subnets(), node, true)
		log.Infof("%d Venice state : %+v", i+1, vSub)
		log.Infof("%d NetAgent state %+v", i+1, naSub)
		Expect(reflect.DeepEqual(vSub, naSub)).Should(BeTrue())
	}
}
