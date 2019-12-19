package main

import (
	"context"
	"fmt"
	"net"
	"os/exec"
	"strings"
	"testing"

	"google.golang.org/grpc"
	. "gopkg.in/check.v1"

	"os"

	"github.com/pensando/sw/nic/agent/netagent/datapath"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
	. "github.com/pensando/sw/venice/utils/testutils"
)

const (
	halCtlBinaryName = "halctl"
	// Needed to ensure that halctl binary is installed in local $GOPATH
	selfPkgName = "github.com/pensando/sw/nic/agent/cmd/halctl"
)

// test vectors for halctl

// valid short flag vrf get cmd
var getVrfCmdValidShort = []string{"show", "vrf", "spec"}

var getL2SegmentCmdValidShort = []string{"show", "l2seg", "spec"}

var getIfCmdValidShort = []string{"show", "interface"}

var getIfUplinkCmdValidShort = []string{"show", "interface", "uplink"}

var getIfUplinkPcCmdValidShort = []string{"show", "interface", "uplink-pc"}

var getIfEnicCmdValidShort = []string{"show", "interface", "enic"}

var getIfTunnelCmdValidShort = []string{"show", "interface", "tunnel"}

var getIfCPUCmdValidShort = []string{"show", "interface", "cpu"}

// veniceIntegSuite is the state of integ test
type halCtlSuite struct {
	halListener netutils.TestListenAddr
	dp          *datapath.Datapath
	mockSrv     *grpc.Server
}

type mockServer struct{}

func (m *mockServer) VrfCreate(context.Context, *halproto.VrfRequestMsg) (*halproto.VrfResponseMsg, error) {
	return nil, nil
}
func (m *mockServer) VrfUpdate(context.Context, *halproto.VrfRequestMsg) (*halproto.VrfResponseMsg, error) {
	return nil, nil
}
func (m *mockServer) VrfDelete(context.Context, *halproto.VrfDeleteRequestMsg) (*halproto.VrfDeleteResponseMsg, error) {
	return nil, nil
}

func (m *mockServer) VrfGet(context.Context, *halproto.VrfGetRequestMsg) (*halproto.VrfGetResponseMsg, error) {
	resp := &halproto.VrfGetResponseMsg{
		Response: []*halproto.VrfGetResponse{
			{
				ApiStatus: halproto.ApiStatus_API_STATUS_OK,
				Spec: &halproto.VrfSpec{
					KeyOrHandle: &halproto.VrfKeyHandle{
						KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
							VrfId: 1,
						},
					},
				},
			},
			{
				ApiStatus: halproto.ApiStatus_API_STATUS_OK,
				Spec: &halproto.VrfSpec{
					KeyOrHandle: &halproto.VrfKeyHandle{
						KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
							VrfId: 2,
						},
					},
				},
			},
		},
	}
	return resp, nil
}

func (m *mockServer) L2SegmentCreate(context.Context, *halproto.L2SegmentRequestMsg) (*halproto.L2SegmentResponseMsg, error) {
	return nil, nil
}
func (m *mockServer) L2SegmentUpdate(context.Context, *halproto.L2SegmentRequestMsg) (*halproto.L2SegmentResponseMsg, error) {
	return nil, nil
}
func (m *mockServer) L2SegmentDelete(context.Context, *halproto.L2SegmentDeleteRequestMsg) (*halproto.L2SegmentDeleteResponseMsg, error) {
	return nil, nil
}
func (m *mockServer) L2SegmentGet(context.Context, *halproto.L2SegmentGetRequestMsg) (*halproto.L2SegmentGetResponseMsg, error) {
	resp := &halproto.L2SegmentGetResponseMsg{
		Response: []*halproto.L2SegmentGetResponse{
			{
				ApiStatus: halproto.ApiStatus_API_STATUS_OK,
				Spec: &halproto.L2SegmentSpec{
					KeyOrHandle: &halproto.L2SegmentKeyHandle{
						KeyOrHandle: &halproto.L2SegmentKeyHandle_SegmentId{
							SegmentId: 1,
						},
					},
					VrfKeyHandle: &halproto.VrfKeyHandle{
						KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
							VrfId: 1,
						},
					},
				},
			},
			{
				ApiStatus: halproto.ApiStatus_API_STATUS_OK,
				Spec: &halproto.L2SegmentSpec{
					KeyOrHandle: &halproto.L2SegmentKeyHandle{
						KeyOrHandle: &halproto.L2SegmentKeyHandle_SegmentId{
							SegmentId: 2,
						},
					},
					VrfKeyHandle: &halproto.VrfKeyHandle{
						KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
							VrfId: 1,
						},
					},
				},
			},
			{
				ApiStatus: halproto.ApiStatus_API_STATUS_OK,
				Spec: &halproto.L2SegmentSpec{
					KeyOrHandle: &halproto.L2SegmentKeyHandle{
						KeyOrHandle: &halproto.L2SegmentKeyHandle_SegmentId{
							SegmentId: 3,
						},
					},
					VrfKeyHandle: &halproto.VrfKeyHandle{
						KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
							VrfId: 2,
						},
					},
				},
			},
		},
	}
	return resp, nil
}

func (m *mockServer) LifCreate(context.Context, *halproto.LifRequestMsg) (*halproto.LifResponseMsg, error) {
	return nil, nil
}

func (m *mockServer) LifUpdate(context.Context, *halproto.LifRequestMsg) (*halproto.LifResponseMsg, error) {
	return nil, nil
}

func (m *mockServer) LifDelete(context.Context, *halproto.LifDeleteRequestMsg) (*halproto.LifDeleteResponseMsg, error) {
	return nil, nil
}

func (m *mockServer) LifGet(context.Context, *halproto.LifGetRequestMsg) (*halproto.LifGetResponseMsg, error) {
	return nil, nil
}

func (m *mockServer) LifGetQState(context.Context, *halproto.GetQStateRequestMsg) (*halproto.GetQStateResponseMsg, error) {
	return nil, nil
}

func (m *mockServer) LifSetQState(context.Context, *halproto.SetQStateRequestMsg) (*halproto.SetQStateResponseMsg, error) {
	return nil, nil
}

func (m *mockServer) AddL2SegmentOnUplink(context.Context, *halproto.InterfaceL2SegmentRequestMsg) (*halproto.InterfaceL2SegmentResponseMsg, error) {
	return nil, nil
}

func (m *mockServer) DelL2SegmentOnUplink(context.Context, *halproto.InterfaceL2SegmentRequestMsg) (*halproto.InterfaceL2SegmentResponseMsg, error) {
	return nil, nil
}

func (m *mockServer) InterfaceCreate(context.Context, *halproto.InterfaceRequestMsg) (*halproto.InterfaceResponseMsg, error) {
	return nil, nil
}
func (m *mockServer) InterfaceUpdate(context.Context, *halproto.InterfaceRequestMsg) (*halproto.InterfaceResponseMsg, error) {
	return nil, nil
}
func (m *mockServer) InterfaceDelete(context.Context, *halproto.InterfaceDeleteRequestMsg) (*halproto.InterfaceDeleteResponseMsg, error) {
	return nil, nil
}
func (m *mockServer) InterfaceGet(context.Context, *halproto.InterfaceGetRequestMsg) (*halproto.InterfaceGetResponseMsg, error) {
	resp := &halproto.InterfaceGetResponseMsg{
		Response: []*halproto.InterfaceGetResponse{
			{
				ApiStatus: halproto.ApiStatus_API_STATUS_OK,
				Spec: &halproto.InterfaceSpec{
					KeyOrHandle: &halproto.InterfaceKeyHandle{
						KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
							InterfaceId: 1,
						},
					},
				},
			},
			{
				ApiStatus: halproto.ApiStatus_API_STATUS_OK,
				Spec: &halproto.InterfaceSpec{
					KeyOrHandle: &halproto.InterfaceKeyHandle{
						KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
							InterfaceId: 2,
						},
					},
					Type: halproto.IfType_IF_TYPE_UPLINK,
				},
			},
			{
				ApiStatus: halproto.ApiStatus_API_STATUS_OK,
				Spec: &halproto.InterfaceSpec{
					KeyOrHandle: &halproto.InterfaceKeyHandle{
						KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
							InterfaceId: 3,
						},
					},
					Type: halproto.IfType_IF_TYPE_UPLINK_PC,
				},
			},
			{
				ApiStatus: halproto.ApiStatus_API_STATUS_OK,
				Spec: &halproto.InterfaceSpec{
					KeyOrHandle: &halproto.InterfaceKeyHandle{
						KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
							InterfaceId: 4,
						},
					},
					Type: halproto.IfType_IF_TYPE_ENIC,
				},
			},
			{
				ApiStatus: halproto.ApiStatus_API_STATUS_OK,
				Spec: &halproto.InterfaceSpec{
					KeyOrHandle: &halproto.InterfaceKeyHandle{
						KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
							InterfaceId: 5,
						},
					},
					Type: halproto.IfType_IF_TYPE_TUNNEL,
				},
			},
			{
				ApiStatus: halproto.ApiStatus_API_STATUS_OK,
				Spec: &halproto.InterfaceSpec{
					KeyOrHandle: &halproto.InterfaceKeyHandle{
						KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
							InterfaceId: 6,
						},
					},
					Type: halproto.IfType_IF_TYPE_CPU,
				},
			},
		},
	}
	return resp, nil
}

// Hook up gocheck into the "go test" runner.
func TestHalCtl(t *testing.T) {
	// integ test suite
	var sts = &halCtlSuite{}

	var _ = Suite(sts)
	TestingT(t)
}

func (h *halCtlSuite) SetUpSuite(c *C) {
	lisErr := h.halListener.GetAvailablePort()
	c.Assert(lisErr, IsNil)
	os.Setenv("HAL_GRPC_PORT", fmt.Sprintf("%d", h.halListener.Port))
	log.Infof("Current HAL Port: %d", h.halListener.Port)
	lis, err := net.Listen("tcp", h.halListener.ListenURL.String())
	c.Assert(err, IsNil)
	go func(lis net.Listener) {
		h.mockSrv = grpc.NewServer()
		halproto.RegisterVrfServer(h.mockSrv, &mockServer{})
		halproto.RegisterL2SegmentServer(h.mockSrv, &mockServer{})
		halproto.RegisterInterfaceServer(h.mockSrv, &mockServer{})
		h.mockSrv.Serve(lis)
	}(lis)
	dp, err := datapath.NewHalDatapath("hal")
	c.Assert(err, IsNil)
	h.dp = dp
	_, err = exec.Command("go", "install", selfPkgName).CombinedOutput()
	c.Assert(err, IsNil)
}

func (h *halCtlSuite) SetUpTest(c *C) {
	log.Infof("============================= %s starting ==========================", c.TestName())
}

func (h *halCtlSuite) TearDownTest(c *C) {
	log.Infof("============================= %s completed ==========================", c.TestName())

}

func (h *halCtlSuite) TearDownSuite(c *C) {
	h.mockSrv.Stop()
}

// basic test to get all Vrfs
func (h *halCtlSuite) TestVrfGet(c *C) {
	var err error
	var resp string
	req := &halproto.VrfGetRequest{
		KeyOrHandle: &halproto.VrfKeyHandle{
			KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
				VrfId: uint64(1),
			},
		},
	}
	vrfGetReqMsg := &halproto.VrfGetRequestMsg{
		Request: []*halproto.VrfGetRequest{req},
	}

	AssertEventually(c, func() (bool, interface{}) {
		resp, err = h.getVrfs(vrfGetReqMsg)
		return err == nil, nil
	}, "Failed to get VRFs")
	AssertEquals(c, true, strings.Contains(resp, "1    0"), fmt.Sprintf("halctl returned: %v", resp))
	// AssertEquals(c, true, strings.Contains(resp, "vrfid: 1"), fmt.Sprintf("halctl returned: %v", resp))
}

// basic test to get all l2 segments
func (h *halCtlSuite) TestL2SegmentGet(c *C) {
	var err error
	var resp string
	req := &halproto.L2SegmentGetRequest{
		KeyOrHandle: &halproto.L2SegmentKeyHandle{
			KeyOrHandle: &halproto.L2SegmentKeyHandle_SegmentId{
				SegmentId: uint64(1),
			},
		},
	}
	l2SegGetReqMsg := &halproto.L2SegmentGetRequestMsg{
		Request: []*halproto.L2SegmentGetRequest{req},
	}

	AssertEventually(c, func() (bool, interface{}) {
		resp, err = h.getL2Segments(l2SegGetReqMsg)
		return err == nil, nil
	}, "Failed to get L2Segments")
	AssertEquals(c, true, strings.Contains(resp, "1       Mgmt"), fmt.Sprintf("halctl returned: %v", resp))
	//AssertEquals(c, true, strings.Contains(resp, "segmentid: 1"), fmt.Sprintf("halctl returned: %v", resp))
}

// basic test to get all l2 interfaces
func (h *halCtlSuite) TestInterfaceGet(c *C) {
	var err error
	var resp string
	req := &halproto.InterfaceGetRequest{
		KeyOrHandle: &halproto.InterfaceKeyHandle{
			KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
				InterfaceId: uint64(1),
			},
		},
	}
	ifGetReqMsg := &halproto.InterfaceGetRequestMsg{
		Request: []*halproto.InterfaceGetRequest{req},
	}

	AssertEventually(c, func() (bool, interface{}) {
		resp, err = h.getInterfaces(ifGetReqMsg)
		return err == nil, nil
	}, "Failed to get Interfaces")
	AssertEquals(c, true, strings.Contains(resp, "1                     0"), fmt.Sprintf("halctl returned: %v", resp))
	//AssertEquals(c, true, strings.Contains(resp, "segmentid: 1"), fmt.Sprintf("halctl returned: %v", resp))
}

// basic test to get all uplinks
func (h *halCtlSuite) TestUplinksGet(c *C) {
	var err error
	var resp string
	req := &halproto.InterfaceGetRequest{
		KeyOrHandle: &halproto.InterfaceKeyHandle{
			KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
				InterfaceId: uint64(1),
			},
		},
	}
	ifGetReqMsg := &halproto.InterfaceGetRequestMsg{
		Request: []*halproto.InterfaceGetRequest{req},
	}

	AssertEventually(c, func() (bool, interface{}) {
		resp, err = h.getUplinks(ifGetReqMsg)
		return err == nil, nil
	}, "Failed to get Uplinks")
	AssertEquals(c, true, strings.Contains(resp, "uplink-2"), fmt.Sprintf("halctl returned: %v", resp))
	//AssertEquals(c, true, strings.Contains(resp, "segmentid: 1"), fmt.Sprintf("halctl returned: %v", resp))
}

// basic test to get all uplinkPCs
func (h *halCtlSuite) TestUplinkPCsGet(c *C) {
	var err error
	var resp string
	req := &halproto.InterfaceGetRequest{
		KeyOrHandle: &halproto.InterfaceKeyHandle{
			KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
				InterfaceId: uint64(1),
			},
		},
	}
	ifGetReqMsg := &halproto.InterfaceGetRequestMsg{
		Request: []*halproto.InterfaceGetRequest{req},
	}

	AssertEventually(c, func() (bool, interface{}) {
		resp, err = h.getUplinkPCs(ifGetReqMsg)
		return err == nil, nil
	}, "Failed to get Uplinks")
	AssertEquals(c, true, strings.Contains(resp, "uplinkpc-3"), fmt.Sprintf("halctl returned: %v", resp))
	//AssertEquals(c, true, strings.Contains(resp, "segmentid: 1"), fmt.Sprintf("halctl returned: %v", resp))
}

// basic test to get all enics
func (h *halCtlSuite) TestEnicsGet(c *C) {
	var err error
	var resp string
	req := &halproto.InterfaceGetRequest{
		KeyOrHandle: &halproto.InterfaceKeyHandle{
			KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
				InterfaceId: uint64(1),
			},
		},
	}
	ifGetReqMsg := &halproto.InterfaceGetRequestMsg{
		Request: []*halproto.InterfaceGetRequest{req},
	}

	AssertEventually(c, func() (bool, interface{}) {
		resp, err = h.getEnics(ifGetReqMsg)
		return err == nil, nil
	}, "Failed to get Enics")
	AssertEquals(c, true, strings.Contains(resp, "enic-4"), fmt.Sprintf("halctl returned: %v", resp))
}

/*
// basic test to get all tunnels
func (h *halCtlSuite) TestTunnelsGet(c *C) {
	var err error
	var resp string
	req := &halproto.InterfaceGetRequest{
		KeyOrHandle: &halproto.InterfaceKeyHandle{
			KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
				InterfaceId: uint64(1),
			},
		},
	}
	ifGetReqMsg := &halproto.InterfaceGetRequestMsg{
		Request: []*halproto.InterfaceGetRequest{req},
	}

	AssertEventually(c, func() (bool, interface{}) {
		resp, err = h.getTunnels(ifGetReqMsg)
		return err == nil, nil
	}, "Failed to get Tunnels")
	AssertEquals(c, true, strings.Contains(resp, "5         0"), fmt.Sprintf("halctl returned: %v", resp))
}
*/

// basic test to get all cpu
func (h *halCtlSuite) TestCPUGet(c *C) {
	var err error
	var resp string
	req := &halproto.InterfaceGetRequest{
		KeyOrHandle: &halproto.InterfaceKeyHandle{
			KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
				InterfaceId: uint64(1),
			},
		},
	}
	ifGetReqMsg := &halproto.InterfaceGetRequestMsg{
		Request: []*halproto.InterfaceGetRequest{req},
	}

	AssertEventually(c, func() (bool, interface{}) {
		resp, err = h.getCPU(ifGetReqMsg)
		return err == nil, nil
	}, "Failed to get CPU")
	AssertEquals(c, true, strings.Contains(resp, "cpu-6"), fmt.Sprintf("halctl returned: %v", resp))
}

/*
func (h *halCtlSuite) TestVRFGetCornerCases(c *C) {
	// test long version of the flags
	b, err := exec.Command(halCtlBinaryName, getVrfCmdValidLong...).CombinedOutput()
	c.Assert(err, IsNil)
	AssertEquals(c, true, strings.Contains(string(b), "vrfid: 1"), fmt.Sprintf("halctl returned: %v", string(b)))

	// test invalid commands
	b, err = exec.Command(halCtlBinaryName, getVrfCmdInvalidMissingRequiredKey...).CombinedOutput()
	c.Assert(err, IsNil)
	AssertEquals(c, true, strings.Contains(string(b), "Error"), fmt.Sprintf("halctl returned: %v", string(b)))

}

func (h *halCtlSuite) TestL2SegmentGetCornerCases(c *C) {
	// test long version of the flags
	b, err := exec.Command(halCtlBinaryName, getL2SegmentCmdValidLong...).CombinedOutput()
	c.Assert(err, IsNil)
	AssertEquals(c, true, strings.Contains(string(b), "segmentid: 1"), fmt.Sprintf("halctl returned: %v", string(b)))

	// test invalid commands
	b, err = exec.Command(halCtlBinaryName, getL2SegmentCmdInvalidMissingRequiredKey...).CombinedOutput()
	c.Assert(err, IsNil)
	AssertEquals(c, true, strings.Contains(string(b), `Error: Required flag(s) "vrf-id" have/has not been set`), fmt.Sprintf("halctl returned: %v", string(b)))

}

func (h *halCtlSuite) TestCommandSuggestions(c *C) {
	// Top level command suggestions. Expect get to be present when doing halctl
	b, err := exec.Command(halCtlBinaryName).CombinedOutput()
	c.Assert(err, IsNil)
	AssertEquals(c, true, strings.Contains(string(b), "get"), fmt.Sprintf("halctl returned: %v", string(b)))

	// First level command suggestions. Expect vrf and l2segment to be present when doing halctl get
	b, err = exec.Command(halCtlBinaryName, "get").CombinedOutput()
	c.Assert(err, IsNil)
	AssertEquals(c, true, strings.Contains(string(b), "vrf"), fmt.Sprintf("halctl returned: %v", string(b)))
	AssertEquals(c, true, strings.Contains(string(b), "l2segment"), fmt.Sprintf("halctl returned: %v", string(b)))
}

func (h *halCtlSuite) TestInvalidFlags(c *C) {
	// Top level command suggestions. Expect get to be present when doing halctl
	b, err := exec.Command(halCtlBinaryName, invalidFlagCmd...).CombinedOutput()
	c.Assert(err, IsNil)
	AssertEquals(c, true, strings.Contains(string(b), "Error: unknown flag: --foo"), fmt.Sprintf("halctl returned: %v", string(b)))

}
*/
func (h *halCtlSuite) getVrfs(v *halproto.VrfGetRequestMsg) (string, error) {
	b, err := exec.Command(halCtlBinaryName, getVrfCmdValidShort...).CombinedOutput()
	return string(b), err
}

func (h *halCtlSuite) getL2Segments(v *halproto.L2SegmentGetRequestMsg) (string, error) {
	b, err := exec.Command(halCtlBinaryName, getL2SegmentCmdValidShort...).CombinedOutput()
	return string(b), err
}

func (h *halCtlSuite) getInterfaces(v *halproto.InterfaceGetRequestMsg) (string, error) {
	b, err := exec.Command(halCtlBinaryName, getIfCmdValidShort...).CombinedOutput()
	return string(b), err
}

func (h *halCtlSuite) getUplinks(v *halproto.InterfaceGetRequestMsg) (string, error) {
	b, err := exec.Command(halCtlBinaryName, getIfUplinkCmdValidShort...).CombinedOutput()
	return string(b), err
}

func (h *halCtlSuite) getUplinkPCs(v *halproto.InterfaceGetRequestMsg) (string, error) {
	b, err := exec.Command(halCtlBinaryName, getIfUplinkPcCmdValidShort...).CombinedOutput()
	return string(b), err
}

func (h *halCtlSuite) getEnics(v *halproto.InterfaceGetRequestMsg) (string, error) {
	b, err := exec.Command(halCtlBinaryName, getIfEnicCmdValidShort...).CombinedOutput()
	return string(b), err
}

func (h *halCtlSuite) getCPU(v *halproto.InterfaceGetRequestMsg) (string, error) {
	b, err := exec.Command(halCtlBinaryName, getIfCPUCmdValidShort...).CombinedOutput()
	return string(b), err
}

/*
func (h *halCtlSuite) getTunnels(v *halproto.InterfaceGetRequestMsg) (string, error) {
	b, err := exec.Command(halCtlBinaryName, getIfTunnelCmdValidShort...).CombinedOutput()
	return string(b), err
}
*/
