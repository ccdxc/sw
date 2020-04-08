// +build iris

package fakehal

import (
	"context"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"net"
	"time"

	"google.golang.org/grpc"

	irisproto "github.com/pensando/sw/nic/agent/dscagent/types/irisproto"
	delphi "github.com/pensando/sw/nic/delphi/gosdk"
	sysmgr "github.com/pensando/sw/nic/sysmgr/golib"
	"github.com/pensando/sw/venice/utils/log"
)

// Hal implements fake hal svc layer
type Hal struct {
	ListenURL    string
	Srv          *grpc.Server
	SysmgrClient *sysmgr.Client
}

// ########################## SysMgr Methods

// OnMountComplete informs sysmgr that mount is doine
func (h Hal) OnMountComplete() {
	h.SysmgrClient.InitDone()
}

// Name returns netagent svc name
func (h Hal) Name() string {
	return "fakehal"
}

// ########################## HAL methods used by DSCAgent

// EndpointCreate stubbed out
func (h Hal) EndpointCreate(ctx context.Context, req *irisproto.EndpointRequestMsg) (*irisproto.EndpointResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got EndpointCreate Request:")
	fmt.Println(string(dat))

	return &irisproto.EndpointResponseMsg{
		Response: []*irisproto.EndpointResponse{
			{
				ApiStatus: irisproto.ApiStatus_API_STATUS_OK,
			},
		},
	}, nil
}

// EndpointUpdate stubbed out
func (h Hal) EndpointUpdate(ctx context.Context, req *irisproto.EndpointUpdateRequestMsg) (*irisproto.EndpointUpdateResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got EndpointUpdate Request:")
	fmt.Println(string(dat))

	return &irisproto.EndpointUpdateResponseMsg{
		Response: []*irisproto.EndpointResponse{
			{
				ApiStatus: irisproto.ApiStatus_API_STATUS_OK,
			},
		},
	}, nil
}

// EndpointDelete stubbed out
func (h Hal) EndpointDelete(ctx context.Context, req *irisproto.EndpointDeleteRequestMsg) (*irisproto.EndpointDeleteResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got EndpointDelete Request:")
	fmt.Println(string(dat))

	return &irisproto.EndpointDeleteResponseMsg{
		Response: []*irisproto.EndpointDeleteResponse{
			{
				ApiStatus: irisproto.ApiStatus_API_STATUS_OK,
			},
		},
	}, nil
}

// InterfaceCreate stubbed out
func (h Hal) InterfaceCreate(ctx context.Context, req *irisproto.InterfaceRequestMsg) (*irisproto.InterfaceResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got InterfaceCreate Request:")
	fmt.Println(string(dat))

	return &irisproto.InterfaceResponseMsg{
		Response: []*irisproto.InterfaceResponse{
			{
				ApiStatus: irisproto.ApiStatus_API_STATUS_OK,
			},
		},
	}, nil
}

// InterfaceUpdate stubbed out
func (h Hal) InterfaceUpdate(ctx context.Context, req *irisproto.InterfaceRequestMsg) (*irisproto.InterfaceResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got InterfaceUpdate Request:")
	fmt.Println(string(dat))

	return &irisproto.InterfaceResponseMsg{
		Response: []*irisproto.InterfaceResponse{
			{
				ApiStatus: irisproto.ApiStatus_API_STATUS_OK,
			},
		},
	}, nil
}

// InterfaceDelete stubbed out
func (h Hal) InterfaceDelete(ctx context.Context, req *irisproto.InterfaceDeleteRequestMsg) (*irisproto.InterfaceDeleteResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got InterfaceDelete Request:")
	fmt.Println(string(dat))

	return &irisproto.InterfaceDeleteResponseMsg{
		Response: []*irisproto.InterfaceDeleteResponse{
			{
				ApiStatus: irisproto.ApiStatus_API_STATUS_OK,
			},
		},
	}, nil
}

// InterfaceGet stubbed out
func (h Hal) InterfaceGet(ctx context.Context, req *irisproto.InterfaceGetRequestMsg) (*irisproto.InterfaceGetResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got InterfaceGet Request:")
	fmt.Println(string(dat))

	return &irisproto.InterfaceGetResponseMsg{
		Response: []*irisproto.InterfaceGetResponse{
			{
				ApiStatus: irisproto.ApiStatus_API_STATUS_OK,
				Spec:      &irisproto.InterfaceSpec{},
			},
		},
	}, nil
}

// L2SegmentCreate stubbed out
func (h Hal) L2SegmentCreate(ctx context.Context, req *irisproto.L2SegmentRequestMsg) (*irisproto.L2SegmentResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got L2SegmentCreate Request:")
	fmt.Println(string(dat))

	return &irisproto.L2SegmentResponseMsg{
		Response: []*irisproto.L2SegmentResponse{
			{
				ApiStatus: irisproto.ApiStatus_API_STATUS_OK,
			},
		},
	}, nil
}

// L2SegmentUpdate stubbed out
func (h Hal) L2SegmentUpdate(ctx context.Context, req *irisproto.L2SegmentRequestMsg) (*irisproto.L2SegmentResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got L2SegmentUpdate Request:")
	fmt.Println(string(dat))

	return &irisproto.L2SegmentResponseMsg{
		Response: []*irisproto.L2SegmentResponse{
			{
				ApiStatus: irisproto.ApiStatus_API_STATUS_OK,
			},
		},
	}, nil
}

// L2SegmentDelete stubbed out
func (h Hal) L2SegmentDelete(ctx context.Context, req *irisproto.L2SegmentDeleteRequestMsg) (*irisproto.L2SegmentDeleteResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got L2SegmentDelete Request:")
	fmt.Println(string(dat))

	return &irisproto.L2SegmentDeleteResponseMsg{
		Response: []*irisproto.L2SegmentDeleteResponse{
			{
				ApiStatus: irisproto.ApiStatus_API_STATUS_OK,
			},
		},
	}, nil
}

// SecurityPolicyCreate stubbed out
func (h Hal) SecurityPolicyCreate(ctx context.Context, req *irisproto.SecurityPolicyRequestMsg) (*irisproto.SecurityPolicyResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got SecurityPolicyCreate Request:")
	fmt.Println(string(dat))

	return &irisproto.SecurityPolicyResponseMsg{
		Response: []*irisproto.SecurityPolicyResponse{
			{
				ApiStatus: irisproto.ApiStatus_API_STATUS_OK,
			},
		},
	}, nil
}

// SecurityPolicyUpdate stubbed out
func (h Hal) SecurityPolicyUpdate(ctx context.Context, req *irisproto.SecurityPolicyRequestMsg) (*irisproto.SecurityPolicyResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got SecurityPolicyUpdate Request:")
	fmt.Println(string(dat))

	return &irisproto.SecurityPolicyResponseMsg{
		Response: []*irisproto.SecurityPolicyResponse{
			{
				ApiStatus: irisproto.ApiStatus_API_STATUS_OK,
			},
		},
	}, nil
}

// SecurityPolicyDelete stubbed out
func (h Hal) SecurityPolicyDelete(ctx context.Context, req *irisproto.SecurityPolicyDeleteRequestMsg) (*irisproto.SecurityPolicyDeleteResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got SecurityPolicyDelete Request:")
	fmt.Println(string(dat))

	return &irisproto.SecurityPolicyDeleteResponseMsg{
		Response: []*irisproto.SecurityPolicyDeleteResponse{
			{
				ApiStatus: irisproto.ApiStatus_API_STATUS_OK,
			},
		},
	}, nil
}

// VrfCreate stubbed out
func (h Hal) VrfCreate(ctx context.Context, req *irisproto.VrfRequestMsg) (*irisproto.VrfResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got VrfCreate Request:")
	fmt.Println(string(dat))

	return &irisproto.VrfResponseMsg{
		Response: []*irisproto.VrfResponse{
			{
				ApiStatus: irisproto.ApiStatus_API_STATUS_OK,
			},
		},
	}, nil
}

// VrfUpdate stubbed out
func (h Hal) VrfUpdate(ctx context.Context, req *irisproto.VrfRequestMsg) (*irisproto.VrfResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got VrfUpdate Request:")
	fmt.Println(string(dat))

	return &irisproto.VrfResponseMsg{
		Response: []*irisproto.VrfResponse{
			{
				ApiStatus: irisproto.ApiStatus_API_STATUS_OK,
			},
		},
	}, nil
}

// VrfDelete stubbed out
func (h Hal) VrfDelete(ctx context.Context, req *irisproto.VrfDeleteRequestMsg) (*irisproto.VrfDeleteResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got VrfDelete Request:")
	fmt.Println(string(dat))

	return &irisproto.VrfDeleteResponseMsg{
		Response: []*irisproto.VrfDeleteResponse{
			{
				ApiStatus: irisproto.ApiStatus_API_STATUS_OK,
			},
		},
	}, nil
}

// EventListen stubbed out
func (h Hal) EventListen(stream irisproto.Event_EventListenServer) error {
	evtResp := &irisproto.EventResponse{
		ApiStatus: irisproto.ApiStatus_API_STATUS_OK,
		EventId:   0,
		EventInfo: &irisproto.EventResponse_LifEvent{
			LifEvent: &irisproto.LifGetResponse{
				ApiStatus: irisproto.ApiStatus_API_STATUS_OK,
				Spec: &irisproto.LifSpec{
					KeyOrHandle: &irisproto.LifKeyHandle{
						KeyOrHandle: &irisproto.LifKeyHandle_LifId{
							LifId: 1,
						},
					},
					Name: "portal-to-narnia",
				},
				Status: &irisproto.LifStatus{
					LifStatus: irisproto.IfStatus_IF_STATUS_UP,
				},
			},
		},
	}

	if err := stream.Send(evtResp); err != nil {
		log.Errorf("Streaming Lif Err: %v", err)
		return err
	}
	return nil
}

// LifGet stubbed out
func (h Hal) LifGet(ctx context.Context, req *irisproto.LifGetRequestMsg) (*irisproto.LifGetResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got LifGet Request:")
	fmt.Println(string(dat))

	return &irisproto.LifGetResponseMsg{
		Response: []*irisproto.LifGetResponse{
			{
				ApiStatus: irisproto.ApiStatus_API_STATUS_OK,
				Spec: &irisproto.LifSpec{
					KeyOrHandle: &irisproto.LifKeyHandle{
						KeyOrHandle: &irisproto.LifKeyHandle_LifId{
							LifId: 1,
						},
					},
					Name: "eth0",
				},
				Status: &irisproto.LifStatus{
					LifStatus: irisproto.IfStatus_IF_STATUS_UP,
				},
			},
			{
				ApiStatus: irisproto.ApiStatus_API_STATUS_OK,
				Spec: &irisproto.LifSpec{
					KeyOrHandle: &irisproto.LifKeyHandle{
						KeyOrHandle: &irisproto.LifKeyHandle_LifId{
							LifId: 2,
						},
					},
					Name: "eth1",
				},
				Status: &irisproto.LifStatus{
					LifStatus: irisproto.IfStatus_IF_STATUS_UP,
				},
			},
		},
	}, nil
}

// PortInfoGet stubbed out
func (h Hal) PortInfoGet(ctx context.Context, req *irisproto.PortInfoGetRequestMsg) (*irisproto.PortInfoGetResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got PortInfoGet Request:")
	fmt.Println(string(dat))

	return &irisproto.PortInfoGetResponseMsg{
		Response: []*irisproto.PortInfoGetResponse{
			{
				ApiStatus: irisproto.ApiStatus_API_STATUS_OK,
				Spec: &irisproto.PortInfoSpec{
					KeyOrHandle: &irisproto.PortKeyHandle{
						KeyOrHandle: &irisproto.PortKeyHandle_PortId{
							PortId: 1,
						},
					},
					PortType:   irisproto.PortType_PORT_TYPE_ETH,
					NumLanes:   4,
					AdminState: irisproto.PortAdminState_PORT_ADMIN_STATE_UP,
				},
			},
			{
				ApiStatus: irisproto.ApiStatus_API_STATUS_OK,
				Spec: &irisproto.PortInfoSpec{
					KeyOrHandle: &irisproto.PortKeyHandle{
						KeyOrHandle: &irisproto.PortKeyHandle_PortId{
							PortId: 5,
						},
					},
					PortType:   irisproto.PortType_PORT_TYPE_ETH,
					NumLanes:   4,
					AdminState: irisproto.PortAdminState_PORT_ADMIN_STATE_UP,
				},
			},
			{
				ApiStatus: irisproto.ApiStatus_API_STATUS_OK,
				Spec: &irisproto.PortInfoSpec{
					KeyOrHandle: &irisproto.PortKeyHandle{
						KeyOrHandle: &irisproto.PortKeyHandle_PortId{
							PortId: 9,
						},
					},
					PortType:   irisproto.PortType_PORT_TYPE_MGMT,
					NumLanes:   4,
					AdminState: irisproto.PortAdminState_PORT_ADMIN_STATE_UP,
				},
			},
		},
	}, nil
}

// PortCreate stubbed out
func (h Hal) PortCreate(ctx context.Context, req *irisproto.PortRequestMsg) (*irisproto.PortResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got PortCreate Request:")
	fmt.Println(string(dat))

	return &irisproto.PortResponseMsg{
		Response: []*irisproto.PortResponse{
			{
				ApiStatus: irisproto.ApiStatus_API_STATUS_OK,
			},
		},
	}, nil
}

// PortUpdate stubbed out
func (h Hal) PortUpdate(ctx context.Context, req *irisproto.PortRequestMsg) (*irisproto.PortResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got PortUpdate Request:")
	fmt.Println(string(dat))

	return &irisproto.PortResponseMsg{
		Response: []*irisproto.PortResponse{
			{
				ApiStatus: irisproto.ApiStatus_API_STATUS_OK,
			},
		},
	}, nil
}

// PortDelete stubbed out
func (h Hal) PortDelete(ctx context.Context, req *irisproto.PortDeleteRequestMsg) (*irisproto.PortDeleteResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got PortDelete Request:")
	fmt.Println(string(dat))

	return &irisproto.PortDeleteResponseMsg{
		Response: []*irisproto.PortDeleteResponse{
			{
				ApiStatus: irisproto.ApiStatus_API_STATUS_OK,
			},
		},
	}, nil
}

const (
	ifTypeShift       = 28
	ifSlotShift       = 24
	ifParentPortShift = 16
	ifTypeMask        = 0xF
)

// PortGet stubbed out
func (h Hal) PortGet(ctx context.Context, req *irisproto.PortGetRequestMsg) (*irisproto.PortGetResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got PortGet Request:")
	fmt.Println(string(dat))

	return &irisproto.PortGetResponseMsg{
		Response: []*irisproto.PortGetResponse{
			{
				Spec: &irisproto.PortSpec{
					PortType: irisproto.PortType_PORT_TYPE_ETH,
					KeyOrHandle: &irisproto.PortKeyHandle{
						KeyOrHandle: &irisproto.PortKeyHandle_PortId{
							PortId: 1,
						},
					},
				},
				Status: &irisproto.PortStatus{
					IfIndex: 1 << ifTypeShift,
					LinkStatus: &irisproto.PortLinkStatus{
						OperState: irisproto.PortOperState_PORT_OPER_STATUS_UP,
					},
				},
				ApiStatus: irisproto.ApiStatus_API_STATUS_OK,
			},
			{
				Spec: &irisproto.PortSpec{
					PortType: irisproto.PortType_PORT_TYPE_ETH,
					KeyOrHandle: &irisproto.PortKeyHandle{
						KeyOrHandle: &irisproto.PortKeyHandle_PortId{
							PortId: 2,
						},
					},
				},
				Status: &irisproto.PortStatus{
					IfIndex: 1 << ifTypeShift,
					LinkStatus: &irisproto.PortLinkStatus{
						OperState: irisproto.PortOperState_PORT_OPER_STATUS_UP,
					},
				},
				ApiStatus: irisproto.ApiStatus_API_STATUS_OK,
			},
		},
	}, nil
}

// StartAacsServer stubbed out
func (h Hal) StartAacsServer(ctx context.Context, req *irisproto.AacsRequestMsg) (*irisproto.Empty, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got StartAacsServer Request:")
	fmt.Println(string(dat))

	return &irisproto.Empty{}, nil
}

// StopAacsServer stubbed out
func (h Hal) StopAacsServer(ctx context.Context, req *irisproto.Empty) (*irisproto.Empty, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got StopAacsServer Request:")
	fmt.Println(string(dat))

	return &irisproto.Empty{}, nil
}

// SecurityProfileCreate stubbed out
func (h Hal) SecurityProfileCreate(ctx context.Context, req *irisproto.SecurityProfileRequestMsg) (*irisproto.SecurityProfileResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got SecurityProfileCreate Request:")
	fmt.Println(string(dat))

	return &irisproto.SecurityProfileResponseMsg{
		Response: []*irisproto.SecurityProfileResponse{
			{
				ApiStatus: irisproto.ApiStatus_API_STATUS_OK,
			},
		},
	}, nil
}

// SecurityProfileUpdate stubbed out
func (h Hal) SecurityProfileUpdate(ctx context.Context, req *irisproto.SecurityProfileRequestMsg) (*irisproto.SecurityProfileResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got SecurityProfileUpdate Request:")
	fmt.Println(string(dat))

	return &irisproto.SecurityProfileResponseMsg{
		Response: []*irisproto.SecurityProfileResponse{
			{
				ApiStatus: irisproto.ApiStatus_API_STATUS_OK,
			},
		},
	}, nil
}

// SecurityProfileDelete stubbed out
func (h Hal) SecurityProfileDelete(ctx context.Context, req *irisproto.SecurityProfileDeleteRequestMsg) (*irisproto.SecurityProfileDeleteResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got SecurityProfileDelete Request:")
	fmt.Println(string(dat))

	return &irisproto.SecurityProfileDeleteResponseMsg{
		Response: []*irisproto.SecurityProfileDeleteResponse{
			{
				ApiStatus: irisproto.ApiStatus_API_STATUS_OK,
			},
		},
	}, nil
}

// MirrorSessionCreate stubbed out
func (h Hal) MirrorSessionCreate(ctx context.Context, req *irisproto.MirrorSessionRequestMsg) (*irisproto.MirrorSessionResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got MirrorSessionCreate Request:")
	fmt.Println(string(dat))
	return &irisproto.MirrorSessionResponseMsg{
		Response: []*irisproto.MirrorSessionResponse{
			{
				ApiStatus: irisproto.ApiStatus_API_STATUS_OK,
				Status: &irisproto.MirrorSessionStatus{
					Handle:      req.Request[0].KeyOrHandle.GetMirrorsessionId(),
					ActiveFlows: 0,
				},
			},
		},
	}, nil
}

// FlowMonitorRuleCreate stubbed out
func (h Hal) FlowMonitorRuleCreate(ctx context.Context, req *irisproto.FlowMonitorRuleRequestMsg) (*irisproto.FlowMonitorRuleResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got FlowMonitorRuleCreate Request:")
	fmt.Println(string(dat))

	return &irisproto.FlowMonitorRuleResponseMsg{
		Response: []*irisproto.FlowMonitorRuleResponse{
			{
				ApiStatus: irisproto.ApiStatus_API_STATUS_OK,
			},
		},
	}, nil
}

// CollectorCreate Stubbed out
func (h Hal) CollectorCreate(ctx context.Context, req *irisproto.CollectorRequestMsg) (*irisproto.CollectorResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got CollectorCreate Request:")
	fmt.Println(string(dat))

	return &irisproto.CollectorResponseMsg{
		Response: []*irisproto.CollectorResponse{
			{
				ApiStatus: irisproto.ApiStatus_API_STATUS_OK,
			},
		},
	}, nil
}

// FlowMonitorRuleDelete Stubbed out
func (h Hal) FlowMonitorRuleDelete(ctx context.Context, req *irisproto.FlowMonitorRuleDeleteRequestMsg) (*irisproto.FlowMonitorRuleDeleteResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got FlowMonitorRuleDelete Request:")
	fmt.Println(string(dat))

	return &irisproto.FlowMonitorRuleDeleteResponseMsg{
		Response: []*irisproto.FlowMonitorRuleDeleteResponse{
			{
				ApiStatus: irisproto.ApiStatus_API_STATUS_OK,
			},
		},
	}, nil
}

// MirrorSessionDelete Stubbed out
func (h Hal) MirrorSessionDelete(ctx context.Context, req *irisproto.MirrorSessionDeleteRequestMsg) (*irisproto.MirrorSessionDeleteResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got MirrorSessionDelete Request:")
	fmt.Println(string(dat))

	return &irisproto.MirrorSessionDeleteResponseMsg{
		Response: []*irisproto.MirrorSessionDeleteResponse{
			{
				ApiStatus: irisproto.ApiStatus_API_STATUS_OK,
			},
		},
	}, nil
}

// CollectorDelete Stubbed out
func (h Hal) CollectorDelete(ctx context.Context, req *irisproto.CollectorDeleteRequestMsg) (*irisproto.CollectorDeleteResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got CollectorDelete Request:")
	fmt.Println(string(dat))

	return &irisproto.CollectorDeleteResponseMsg{
		Response: []*irisproto.CollectorDeleteResponse{
			{
				ApiStatus: irisproto.ApiStatus_API_STATUS_OK,
			},
		},
	}, nil
}

// SysSpecUpdate Stubbed out
func (h Hal) SysSpecUpdate(ctx context.Context, req *irisproto.SysSpec) (*irisproto.SysSpecResponse, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got SysSpecUpdate Request:")
	fmt.Println(string(dat))

	return &irisproto.SysSpecResponse{
		ApiStatus: irisproto.ApiStatus_API_STATUS_OK,
	}, nil
}

// SysSpecGet Stubbed out
func (h Hal) SysSpecGet(ctx context.Context, req *irisproto.SysSpecGetRequest) (*irisproto.SysSpecGetResponse, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got SysSpecGet Request:")
	fmt.Println(string(dat))

	return &irisproto.SysSpecGetResponse{
		ApiStatus: irisproto.ApiStatus_API_STATUS_OK,
	}, nil
}

// SystemGet Stubbed out
func (h Hal) SystemGet(ctx context.Context, req *irisproto.SystemGetRequest) (*irisproto.SystemResponse, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got SystemGet Request:")
	fmt.Println(string(dat))

	return &irisproto.SystemResponse{
		ApiStatus: irisproto.ApiStatus_API_STATUS_OK,
	}, nil
}

// SystemUUIDGet Stubbed out
func (h Hal) SystemUUIDGet(ctx context.Context, req *irisproto.Empty) (*irisproto.SystemResponse, error) {

	var dat map[string]interface{}
	byt, err := ioutil.ReadFile("/tmp/fru.json")
	if err != nil {
		log.Errorf("Failed to read contents of fru.json")
		return &irisproto.SystemResponse{
			ApiStatus: irisproto.ApiStatus_API_STATUS_ERR,
		}, nil
	}
	if err := json.Unmarshal(byt, &dat); err != nil {
		log.Errorf("Failed to unmarshal fru.json.")
		return &irisproto.SystemResponse{
			ApiStatus: irisproto.ApiStatus_API_STATUS_ERR,
		}, nil
	}

	return &irisproto.SystemResponse{
		Uuid:      dat["mac-address"].(string),
		ApiStatus: irisproto.ApiStatus_API_STATUS_OK,
	}, nil
}

// Stop stops the Fake HAL
func (h Hal) Stop() {
	log.Infof("Stopping HAL at: %s", h.ListenURL)
	if h.Srv != nil {
		h.Srv.Stop()
	}
}

// NewFakeHalServer returns a fake hal server on a specified port
func NewFakeHalServer(listenURL string) *Hal {
	lis, err := net.Listen("tcp", listenURL)
	if err != nil {
		log.Fatalf("failed to listen: %v", err)
	}

	f := Hal{
		ListenURL: listenURL,
		Srv:       grpc.NewServer(),
	}

	delphiClient, err := delphi.NewClient(&f)
	if err != nil {
		log.Infof("Failed to connect to delphi client. Ignoring this in FakeHAL")
	}
	f.SysmgrClient = sysmgr.NewClient(delphiClient, "fakehal")
	go delphiClient.Run()

	irisproto.RegisterEndpointServer(f.Srv, &f)
	irisproto.RegisterInterfaceServer(f.Srv, &f)
	irisproto.RegisterL2SegmentServer(f.Srv, &f)
	irisproto.RegisterNwSecurityServer(f.Srv, &f)
	irisproto.RegisterVrfServer(f.Srv, &f)
	irisproto.RegisterPortServer(f.Srv, &f)
	irisproto.RegisterEventServer(f.Srv, &f)
	irisproto.RegisterTelemetryServer(f.Srv, &f)
	irisproto.RegisterSystemServer(f.Srv, &f)

	go f.Srv.Serve(lis)

	time.Sleep(time.Second * 2)
	log.Infof("Fake HAL Started at: %s", f.ListenURL)
	return &f
}
