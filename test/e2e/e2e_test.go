package e2e_test

import (
	"fmt"
	"os"
	"os/exec"
	. "testing"

	"golang.org/x/net/context"
	"google.golang.org/grpc"
	. "gopkg.in/check.v1"

	"github.com/pensando/sw/globals"
	"github.com/pensando/sw/orch"
	"github.com/pensando/sw/orch/simapi"
	"github.com/pensando/sw/utils/log"
	n "github.com/pensando/sw/utils/netutils"
	tu "github.com/pensando/sw/utils/testutils"
)

const (
	vmsPerHost         = 2
	vchContainerName   = "vcHub"
	vcSimContainerName = "vcSim"
	vchApiAddr         = "127.0.0.1:" + globals.VCHubAPIPort
	defVCSimURL        = "http://127.0.0.1:18086"
	defSoapURL         = "http://user:pass@127.0.0.1:8989/sdk"
	hostSimURL         = "http://192.168.30.121:5050"
	hostMac            = "02:02:02:02:02:01"
)

type e2eSuite struct {
	name string
}

var sts = &e2eSuite{}
var _ = Suite(sts)

// TestMain is the entry point for e2e tests
func TestMain(m *M) {
	if os.Getenv("E2E_TEST") == "" {
		os.Exit(0)
	}
	os.Exit(m.Run())
}

func (s *e2eSuite) SetUpSuite(c *C) {
	dockerPath, err := exec.LookPath("docker")
	if err != nil {
		log.Fatalf("docker not found %v", err)
	}

	runVCSim(dockerPath)

	// start a vchub binary
	runVCH(dockerPath, defSoapURL)
}

func TestE2E(t *T) {
	if os.Getenv("E2E_TEST") == "" {
		os.Exit(0)
	}
	TestingT(t)
}

func runVCSim(dockerPath string) {
	out, err := exec.Command(dockerPath, "run", "--net=host", "--name", vcSimContainerName, "-d", "pen-vcsim", "-hostsim-urls", hostSimURL, "-snic-list", hostMac).CombinedOutput()
	if err != nil {
		log.Infof("runVCSim: %v, %s", err, string(out))
	}

}

func runVCH(dockerPath, url string) {
	out, err := exec.Command(dockerPath, "run", "--net=host", "--name", vchContainerName, "-d", "pen-vchub", "-vcenter-list", url).CombinedOutput()
	if err != nil {
		log.Infof("runVCH: %v, %s", err, string(out))
	}
}

// TestVCHBasic tests basic vchub functionality
func (s *e2eSuite) TestVCHBasic(c *C) {
	// Open a vch api client
	var opts []grpc.DialOption
	opts = append(opts, grpc.WithInsecure())
	conn, err := grpc.Dial(vchApiAddr, opts...)
	c.Assert(err, IsNil)

	vcHubClient := orch.NewOrchApiClient(conn)

	tu.AssertEventually(c, func() bool {
		filter := &orch.Filter{}
		nicList, err := vcHubClient.ListSmartNICs(context.Background(), filter)
		if err != nil {
			return false
		}
		nics := nicList.GetItems()
		if len(nics) == 1 {
			for _, nic := range nics {
				log.Infof("nic: %+v", nic.Status)
			}
			return true
		}
		return false
	}, "50ms", "10s")

	// Add a NwIF

	nwReq := &simapi.NwIFSetReq{
		Name:      "testNwIF",
		IPAddr:    "111.11.11.11",
		Vlan:      "115",
		PortGroup: "dvportGroup-13",
		SmartNIC:  hostMac,
	}
	resp := &simapi.NwIFSetResp{}
	err = n.HTTPPost(defVCSimURL+"/nwifs/create", nwReq, resp)
	c.Assert(err, IsNil)
	log.Infof("Response: %+v", resp)

	tu.AssertEventually(c, func() bool {
		filter := &orch.Filter{}
		ifList, err := vcHubClient.ListNwIFs(context.Background(), filter)
		if err != nil {
			return false
		}
		nwIfs := ifList.GetItems()
		for _, nwif := range nwIfs {
			if nwif.Status.IpAddress == nwReq.IPAddr && nwif.Status.SmartNIC_ID == hostMac {
				log.Infof("%+v", nwif.Status)
				return true
			} else {
				log.Infof("%s", nwif.Status.IpAddress)
			}
		}
		return false
	}, "50ms", "10s")

	// delete the ep and verify
	r := &simapi.NwIFSetReq{}
	del := &simapi.NwIFDelResp{}
	u1 := fmt.Sprintf("%s/nwifs/%s/delete", defVCSimURL, resp.UUID)
	err = n.HTTPPost(u1, r, del)
	c.Assert(err, IsNil)

	tu.AssertEventually(c, func() bool {
		filter := &orch.Filter{}
		ifList, err := vcHubClient.ListNwIFs(context.Background(), filter)
		if err != nil {
			return false
		}
		nwIfs := ifList.GetItems()
		found := false
		for _, nwif := range nwIfs {
			if nwif.Status.IpAddress == nwReq.IPAddr {
				found = true
				break
			}
		}

		return !found
	}, "50ms", "10s")
}

func (s *e2eSuite) TearDownSuite(c *C) {
	dockerPath, _ := exec.LookPath("docker")
	exec.Command(dockerPath, "rm", "-f", vchContainerName).CombinedOutput()
	exec.Command(dockerPath, "rm", "-f", vcSimContainerName).CombinedOutput()
}
