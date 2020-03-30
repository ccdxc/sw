package main

import (
	"context"
	"errors"
	"fmt"
	"math"
	"net"
	"os"
	"os/exec"
	"strings"
	"sync"
	"testing"
	"time"

	"github.com/golang/mock/gomock"
	"google.golang.org/grpc"
	. "gopkg.in/check.v1"

	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
	. "github.com/pensando/sw/venice/utils/testutils"
)

type Kind string

// DB holds all the state information.
type DB struct {
	EndpointDB       map[string]*halproto.EndpointRequestMsg
	EndpointUpdateDB map[string]*halproto.EndpointUpdateRequestMsg
	EndpointDelDB    map[string]*halproto.EndpointDeleteRequestMsg
	SgDB             map[string]*halproto.SecurityGroupRequestMsg
	TenantDB         map[string]*halproto.VrfRequestMsg
	TenantDelDB      map[string]*halproto.VrfDeleteRequestMsg
	SgPolicyDB       map[string]*halproto.SecurityPolicyRequestMsg
	InterfaceDB      map[string]*halproto.InterfaceRequestMsg
	LifDB            map[string]*halproto.LifRequestMsg
	InterfaceDelDB   map[string]*halproto.InterfaceDeleteRequestMsg
	LifDelDB         map[string]*halproto.LifDeleteRequestMsg
}

type Datapath struct {
	sync.Mutex
	Kind Kind
	Hal  Hal
	DB   DB
}

// MockClients stores references for mockclients to be used for setting expectations
type mockClients struct {
	//MockEpclient       *halproto.MockEndpointClient
	//MockIfclient       *halproto.MockInterfaceClient
	//MockL2Segclient    *halproto.MockL2SegmentClient
	//MockNetClient      *halproto.MockNetworkClient
	//MockLbclient       *halproto.MockL4LbClient
	//MockSgclient       *halproto.MockNwSecurityClient
	//MockSessclient     *halproto.MockSessionClient
	//MockTnclient       *halproto.MockVrfClient
	//MockNatClient      *halproto.MockNatClient
	//MockIPSecClient    *halproto.MockIpsecClient
	//MockTCPProxyClient *halproto.MockTcpProxyClient
	//MockPortClient     *halproto.MockPortClient
	//MockSystemClient   *halproto.MockSystemClient
	//MockEventClient    *halproto.MockEventClient
}

type Hal struct {
	client   *grpc.ClientConn
	mockCtrl *gomock.Controller
	//StateAPI             types.CtrlerIntf
	MockClients mockClients
	Epclient    halproto.EndpointClient
	Ifclient    halproto.InterfaceClient
	L2SegClient halproto.L2SegmentClient
	Netclient   halproto.NetworkClient
	//Lbclient             halproto.L4LbClient
	Sgclient halproto.NwSecurityClient
	//Sessclient           halproto.SessionClient
	Tnclient  halproto.VrfClient
	Natclient halproto.NatClient
	//IPSecclient          halproto.IpsecClient
	PortClient halproto.PortClient
	//TCPProxyPolicyClient halproto.TcpProxyClient
	//SystemClient         halproto.SystemClient
	EventClient halproto.EventClient
}

// Errorf for satisfying gomock
func (hd *Hal) Errorf(format string, args ...interface{}) {
	log.Errorf(format, args...)
}

// Fatalf for satisfying gomock
func (hd *Hal) Fatalf(format string, args ...interface{}) {
	log.Fatalf(format, args...)
}

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
	dp          *Datapath
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
	dp, err := NewHalDatapath("hal")
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
	AssertEquals(c, true, strings.Contains(resp, "Designated"), fmt.Sprintf("halctl returned: %v", resp))
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
	AssertEquals(c, true, strings.Contains(resp, "Enic-4"), fmt.Sprintf("halctl returned: %v", resp))
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
	AssertEquals(c, true, strings.Contains(resp, "Uplink"), fmt.Sprintf("halctl returned: %v", resp))
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
	AssertEquals(c, true, strings.Contains(resp, "Interface"), fmt.Sprintf("halctl returned: %v", resp))
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

//######## Test Helper Functions ###############

func NewHalDatapath(kind Kind) (*Datapath, error) {
	var err error
	var hal Hal
	haldp := Datapath{}
	haldp.Kind = kind

	db := DB{EndpointDB: make(map[string]*halproto.EndpointRequestMsg),
		EndpointUpdateDB: make(map[string]*halproto.EndpointUpdateRequestMsg),
		EndpointDelDB:    make(map[string]*halproto.EndpointDeleteRequestMsg),
		SgDB:             make(map[string]*halproto.SecurityGroupRequestMsg),
		TenantDB:         make(map[string]*halproto.VrfRequestMsg),
		TenantDelDB:      make(map[string]*halproto.VrfDeleteRequestMsg),
		SgPolicyDB:       make(map[string]*halproto.SecurityPolicyRequestMsg),
		InterfaceDB:      make(map[string]*halproto.InterfaceRequestMsg),
		LifDB:            make(map[string]*halproto.LifRequestMsg),
		InterfaceDelDB:   make(map[string]*halproto.InterfaceDeleteRequestMsg),
		LifDelDB:         make(map[string]*halproto.LifDeleteRequestMsg),
	}
	haldp.DB = db
	if haldp.Kind.String() == "hal" {
		hal.client, err = createNewGRPCClient()
		if err != nil {
			return nil, err
		}
		hal.Epclient = halproto.NewEndpointClient(hal.client)
		hal.Ifclient = halproto.NewInterfaceClient(hal.client)
		hal.L2SegClient = halproto.NewL2SegmentClient(hal.client)
		hal.Netclient = halproto.NewNetworkClient(hal.client)
		//hal.Lbclient = halproto.NewL4LbClient(hal.client)
		hal.Sgclient = halproto.NewNwSecurityClient(hal.client)
		//hal.Sessclient = halproto.NewSessionClient(hal.client)
		hal.Tnclient = halproto.NewVrfClient(hal.client)
		hal.Natclient = halproto.NewNatClient(hal.client)
		//hal.IPSecclient = halproto.NewIpsecClient(hal.client)
		//hal.TCPProxyPolicyClient = halproto.NewTcpProxyClient(hal.client)
		hal.PortClient = halproto.NewPortClient(hal.client)
		//hal.SystemClient = halproto.NewSystemClient(hal.client)
		hal.EventClient = halproto.NewEventClient(hal.client)
		haldp.Hal = hal
		return &haldp, nil
	}
	hal.mockCtrl = gomock.NewController(&hal)
	hal.MockClients = mockClients{
		//MockEpclient:       halproto.NewMockEndpointClient(hal.mockCtrl),
		//MockIfclient:       halproto.NewMockInterfaceClient(hal.mockCtrl),
		//MockL2Segclient:    halproto.NewMockL2SegmentClient(hal.mockCtrl),
		//MockNetClient:      halproto.NewMockNetworkClient(hal.mockCtrl),
		//MockLbclient:       halproto.NewMockL4LbClient(hal.mockCtrl),
		//MockSgclient:       halproto.NewMockNwSecurityClient(hal.mockCtrl),
		//MockSessclient:     halproto.NewMockSessionClient(hal.mockCtrl),
		//MockTnclient:       halproto.NewMockVrfClient(hal.mockCtrl),
		//MockNatClient:      halproto.NewMockNatClient(hal.mockCtrl),
		//MockIPSecClient:    halproto.NewMockIpsecClient(hal.mockCtrl),
		//MockTCPProxyClient: halproto.NewMockTcpProxyClient(hal.mockCtrl),
		//MockPortClient:     halproto.NewMockPortClient(hal.mockCtrl),
		//MockSystemClient:   halproto.NewMockSystemClient(hal.mockCtrl),
		//MockEventClient:    halproto.NewMockEventClient(hal.mockCtrl),
	}

	//hal.Epclient = hal.MockClients.MockEpclient
	//hal.Ifclient = hal.MockClients.MockIfclient
	//hal.L2SegClient = hal.MockClients.MockL2Segclient
	//hal.Netclient = hal.MockClients.MockNetClient
	//hal.Lbclient = hal.MockClients.MockLbclient
	//hal.Sgclient = hal.MockClients.MockSgclient
	//hal.Sessclient = hal.MockClients.MockSessclient
	//hal.Tnclient = hal.MockClients.MockTnclient
	//hal.Natclient = hal.MockClients.MockNatClient
	//hal.IPSecclient = hal.MockClients.MockIPSecClient
	//hal.TCPProxyPolicyClient = hal.MockClients.MockTCPProxyClient
	//hal.PortClient = hal.MockClients.MockPortClient
	//hal.SystemClient = hal.MockClients.MockSystemClient
	haldp.Hal = hal
	haldp.Hal.setExpectations()
	return &haldp, nil
}

func (hd *Hal) setExpectations() {
	//hd.MockClients.MockEpclient.EXPECT().EndpointCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//hd.MockClients.MockEpclient.EXPECT().EndpointUpdate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//hd.MockClients.MockEpclient.EXPECT().EndpointDelete(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//
	//hd.MockClients.MockIfclient.EXPECT().InterfaceGet(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//hd.MockClients.MockIfclient.EXPECT().AddL2SegmentOnUplink(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//hd.MockClients.MockIfclient.EXPECT().InterfaceCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//hd.MockClients.MockIfclient.EXPECT().InterfaceUpdate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//hd.MockClients.MockIfclient.EXPECT().InterfaceDelete(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//
	//hd.MockClients.MockIfclient.EXPECT().LifGet(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//hd.MockClients.MockIfclient.EXPECT().LifCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//hd.MockClients.MockIfclient.EXPECT().LifUpdate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//hd.MockClients.MockIfclient.EXPECT().LifDelete(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//
	//hd.MockClients.MockL2Segclient.EXPECT().L2SegmentCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//hd.MockClients.MockL2Segclient.EXPECT().L2SegmentUpdate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//hd.MockClients.MockL2Segclient.EXPECT().L2SegmentDelete(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//
	//hd.MockClients.MockNetClient.EXPECT().NetworkCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//hd.MockClients.MockNetClient.EXPECT().NetworkUpdate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//hd.MockClients.MockNetClient.EXPECT().NetworkDelete(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//
	//hd.MockClients.MockNetClient.EXPECT().RouteCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//hd.MockClients.MockNetClient.EXPECT().RouteUpdate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//hd.MockClients.MockNetClient.EXPECT().RouteDelete(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//
	//hd.MockClients.MockNetClient.EXPECT().NexthopCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//hd.MockClients.MockNetClient.EXPECT().NexthopUpdate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//hd.MockClients.MockNetClient.EXPECT().NexthopDelete(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//
	//hd.MockClients.MockSgclient.EXPECT().SecurityGroupCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//hd.MockClients.MockSgclient.EXPECT().SecurityGroupUpdate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//hd.MockClients.MockSgclient.EXPECT().SecurityGroupDelete(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//
	//hd.MockClients.MockSgclient.EXPECT().SecurityGroupPolicyCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//hd.MockClients.MockSgclient.EXPECT().SecurityGroupPolicyUpdate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//hd.MockClients.MockSgclient.EXPECT().SecurityGroupPolicyDelete(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//
	//hd.MockClients.MockSgclient.EXPECT().SecurityProfileCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//hd.MockClients.MockSgclient.EXPECT().SecurityProfileUpdate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//hd.MockClients.MockSgclient.EXPECT().SecurityProfileDelete(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//
	//hd.MockClients.MockSgclient.EXPECT().SecurityPolicyCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//hd.MockClients.MockSgclient.EXPECT().SecurityPolicyUpdate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//hd.MockClients.MockSgclient.EXPECT().SecurityPolicyDelete(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//
	//hd.MockClients.MockTnclient.EXPECT().VrfCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//hd.MockClients.MockTnclient.EXPECT().VrfUpdate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//hd.MockClients.MockTnclient.EXPECT().VrfDelete(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//
	//hd.MockClients.MockNatClient.EXPECT().NatPoolCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//hd.MockClients.MockNatClient.EXPECT().NatPoolUpdate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//hd.MockClients.MockNatClient.EXPECT().NatPoolDelete(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//
	//hd.MockClients.MockNatClient.EXPECT().NatPolicyCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//hd.MockClients.MockNatClient.EXPECT().NatPolicyUpdate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//hd.MockClients.MockNatClient.EXPECT().NatPolicyDelete(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//
	//hd.MockClients.MockNatClient.EXPECT().NatMappingCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//hd.MockClients.MockNatClient.EXPECT().NatMappingDelete(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//
	//hd.MockClients.MockIPSecClient.EXPECT().IpsecRuleCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//hd.MockClients.MockIPSecClient.EXPECT().IpsecRuleUpdate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//hd.MockClients.MockIPSecClient.EXPECT().IpsecRuleDelete(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//
	//hd.MockClients.MockIPSecClient.EXPECT().IpsecSAEncryptCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//hd.MockClients.MockIPSecClient.EXPECT().IpsecSAEncryptUpdate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//hd.MockClients.MockIPSecClient.EXPECT().IpsecSAEncryptDelete(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//
	//hd.MockClients.MockIPSecClient.EXPECT().IpsecSADecryptCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//hd.MockClients.MockIPSecClient.EXPECT().IpsecSADecryptUpdate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//hd.MockClients.MockIPSecClient.EXPECT().IpsecSADecryptDelete(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//
	//hd.MockClients.MockTCPProxyClient.EXPECT().TcpProxyRuleCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//hd.MockClients.MockTCPProxyClient.EXPECT().TcpProxyRuleUpdate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//hd.MockClients.MockTCPProxyClient.EXPECT().TcpProxyRuleDelete(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//
	//hd.MockClients.MockPortClient.EXPECT().PortCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//hd.MockClients.MockPortClient.EXPECT().PortUpdate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//hd.MockClients.MockPortClient.EXPECT().PortDelete(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//hd.MockClients.MockPortClient.EXPECT().PortInfoGet(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//
	//hd.MockClients.MockSystemClient.EXPECT().SystemUUIDGet(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	//
	//hd.MockClients.MockEventClient.EXPECT().EventListen(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)

}

func createNewGRPCClient() (*grpc.ClientConn, error) {
	// create a grpc client
	// ToDo Use AgentID for mysvcName
	return waitForHAL()
}

func waitForHAL() (rpcClient *grpc.ClientConn, err error) {

	halUP := make(chan bool, 1)
	ticker := time.NewTicker(time.Millisecond * 500)
	timeout := time.After(time.Minute * 10)
	rpcClient, err = isHalConnected()
	if err == nil {
		log.Info("HAL Connected on 1st tick")
		return
	}

	for {
		select {
		case <-ticker.C:
			rpcClient, err = isHalConnected()
			if err != nil {
				halUP <- true
			}
		case <-halUP:
			log.Info("Agent is connected to HAL")
			return
		case <-timeout:
			log.Errorf("Agent could not connect to HAL. Err: %v", err)
			return nil, errors.New("hal not available")
		}
	}
}

func isHalConnected() (*grpc.ClientConn, error) {
	halPort := os.Getenv("HAL_GRPC_PORT")
	if halPort == "" {
		halPort = "50054"
	}
	halURL := fmt.Sprintf("127.0.0.1:%s", halPort)
	log.Debugf("Trying to connect to HAL at %s", halURL)
	var grpcOpts []grpc.DialOption
	grpcOpts = append(grpcOpts, grpc.WithMaxMsgSize(math.MaxInt32-1))
	grpcOpts = append(grpcOpts, grpc.WithInsecure())
	return grpc.Dial(halURL, grpcOpts...)
}

// String returns string value of the datapath kind
func (k *Kind) String() string {
	return string(*k)
}
