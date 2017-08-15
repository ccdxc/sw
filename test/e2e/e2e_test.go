package e2e_test

import (
	"os"
	"os/exec"
	. "testing"

	"golang.org/x/net/context"
	"google.golang.org/grpc"
	. "gopkg.in/check.v1"

	"github.com/pensando/sw/globals"
	"github.com/pensando/sw/orch"
	"github.com/pensando/sw/orch/simapi"
	"github.com/pensando/sw/orch/vchub/sim"
	"github.com/pensando/sw/utils/log"
	tu "github.com/pensando/sw/utils/testutils"
)

const (
	vmsPerHost       = 2
	vchContainerName = "vcHub"
	vchApiAddr       = "127.0.0.1:" + globals.VCHubAPIPort
)

type e2eSuite struct {
	name string
}

// TODO auto generate based on hostsim wired_mac
var hostMacs = []string{
	"020202020201",
	"020202020202",
	"020202020203",
}

var vcSim simapi.OrchSim

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
	// start a vc simulator
	vcSim = sim.New()
	vcURL, err := vcSim.Run("127.0.0.1:8989", hostMacs, vmsPerHost)
	if err != nil {
		log.Fatalf("vcSim.Run returned %v", err)
	}

	// start a vchub binary
	runVCH(vcURL)
}

func TestE2E(t *T) {
	if os.Getenv("E2E_TEST") == "" {
		os.Exit(0)
	}
	TestingT(t)
}

func runVCH(url string) {
	dockerPath, err := exec.LookPath("docker")
	if err != nil {
		log.Fatalf("docker not found %v", err)
	}
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
		if len(nics) == len(hostMacs) {
			return true
		}
		return false
	}, "50ms", "10s")

	// Add a NwIF
	u1 := "http://192.168.30.121:5050"
	err = vcSim.SetHostURL(hostMacs[0], u1)
	c.Assert(err, IsNil)

	nwReq := &simapi.NwIFSetReq{
		Name:      "testNwIF",
		IPAddr:    "111.11.11.11",
		Vlan:      "115",
		PortGroup: "dvportGroup-13",
	}
	resp, err := vcSim.CreateNwIF(u1, nwReq)
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
			if nwif.Status.IpAddress == nwReq.IPAddr {
				return true
			} else {
				log.Infof("%s", nwif.Status.IpAddress)
			}
		}
		return false
	}, "50ms", "10s")

	// delete the ep and verify
	del := vcSim.DeleteNwIF(u1, resp.UUID)
	c.Assert(del, NotNil)
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
}
