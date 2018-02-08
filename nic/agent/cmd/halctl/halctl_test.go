package main

import (
	"context"
	"fmt"
	"net"
	"testing"

	"google.golang.org/grpc"
	. "gopkg.in/check.v1"

	"os/exec"

	"strings"

	"github.com/pensando/sw/nic/agent/netagent/datapath"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

const (
	testGRPCPort     = "50054"
	halCtlBinaryName = "halctl"
	// Needed to ensure that halctl binary is installed in local $GOPATH
	selfPkgName = "github.com/pensando/sw/nic/agent/cmd/halctl"
)

// test vectors for halctl

// valid short flag vrf get cmd
var getVrfCmdValidShort = []string{"get", "vrf", "-k", "1"}

// valid long flag vrf get cmd
var getVrfCmdValidLong = []string{"get", "vrf", "--vrf-id", "1"}

// missing required --vrf-id flag
var getVrfCmdInvalidMissingRequiredKey = []string{"get", "vrf"}

// valid short flag l2segment get cmd
var getL2SegmentCmdValidShort = []string{"get", "l2segment", "-k", "1", "-v", "1"}

// valid short flag l2segment get cmd
var getL2SegmentCmdValidLong = []string{"get", "l2segment", "--l2segment-id", "1", "--vrf-id", "1"}

// missing --vrf-id flag for l2segment gets
var getL2SegmentCmdInvalidMissingRequiredKey = []string{"get", "l2segment", "-k", "1"}

// invalid flag command
var invalidFlagCmd = []string{"get", "vrf", "--foo", "42"}

// veniceIntegSuite is the state of integ test
type halCtlSuite struct {
	dp      *datapath.HalDatapath
	mockSrv *grpc.Server
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
func (h *halCtlSuite) runMockHALServer(c *C) {
	lis, err := net.Listen("tcp", fmt.Sprintf("localhost:%v", testGRPCPort))
	c.Assert(err, IsNil)
	h.mockSrv = grpc.NewServer()
	halproto.RegisterVrfServer(h.mockSrv, &mockServer{})
	halproto.RegisterL2SegmentServer(h.mockSrv, &mockServer{})
	h.mockSrv.Serve(lis)
}

// Hook up gocheck into the "go test" runner.
func TestHalCtl(t *testing.T) {
	// integ test suite
	var sts = &halCtlSuite{}

	var _ = Suite(sts)
	TestingT(t)
}

func (h *halCtlSuite) SetUpSuite(c *C) {
	// Start mock serves which has stubbed out CRUDS with spoof data
	go h.runMockHALServer(c)
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
	AssertEquals(c, true, strings.Contains(resp, "vrfid: 1"), fmt.Sprintf("halctl returned: %v", resp))
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
	AssertEquals(c, true, strings.Contains(resp, "segmentid: 1"), fmt.Sprintf("halctl returned: %v", resp))
}

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

func (h *halCtlSuite) getVrfs(v *halproto.VrfGetRequestMsg) (string, error) {
	b, err := exec.Command(halCtlBinaryName, getVrfCmdValidShort...).CombinedOutput()
	return string(b), err
}

func (h *halCtlSuite) getL2Segments(v *halproto.L2SegmentGetRequestMsg) (string, error) {
	b, err := exec.Command(halCtlBinaryName, getL2SegmentCmdValidShort...).CombinedOutput()
	return string(b), err
}
