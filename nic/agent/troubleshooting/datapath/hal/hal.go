// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package troubleshootingHal

import (
	"context"
	"errors"
	"fmt"
	"os"
	"sync"

	"github.com/golang/mock/gomock"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/nic/agent/troubleshooting/state/types"
	"github.com/pensando/sw/venice/ctrler/tsm/rpcserver/tsproto"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
)

const (
	halGRPCDefaultBaseURL = "localhost"
	halGRPCDefaultPort    = "50054"
)

// ErrMirrorCreate error code is returned when mirror create fails
var ErrMirrorCreate = errors.New("Mirror create failed")

// ErrFlowMonitorRuleCreate error code is returned when flow monitor rule cannot be applied.
var ErrFlowMonitorRuleCreate = errors.New("Flow monitor rule create failed")

// ErrDropMonitorRuleCreate error code is returned when drop monitor rule cannot be applied.
var ErrDropMonitorRuleCreate = errors.New("Drop monitor rule create failed")

// Kind holds the HAL Datapath kind. It could either be mock HAL or real HAL.
type Kind string

// Hal holds clients to talk to HAL gRPC server.
type Hal struct {
	client      *rpckit.RPCClient
	mockCtrl    *gomock.Controller
	MockClients mockClients
	TeleClient  halproto.TelemetryClient
}

// MockClients stores references for mockclients to be used for setting expectations
type mockClients struct {
	MockTeleClient *halproto.MockTelemetryClient
}

// Datapath contains mock and hal clients.
type Datapath struct {
	sync.Mutex
	Kind Kind
	Hal  Hal
}

// String returns string value of the datapath kind
func (k *Kind) String() string {
	return string(*k)
}

// Errorf for satisfying gomock
func (hd *Hal) Errorf(format string, args ...interface{}) {
	log.Errorf(format, args...)
}

// Fatalf for satisfying gomock
func (hd *Hal) Fatalf(format string, args ...interface{}) {
	log.Fatalf(format, args...)
}

// NewHalDatapath returns a mock hal datapath
func NewHalDatapath(kind Kind) (*Datapath, error) {
	var err error
	var hal Hal
	haldp := Datapath{}
	haldp.Kind = kind

	if haldp.Kind.String() == "hal" {
		hal.client, err = hal.createNewGRPCClient()
		if err != nil {
			return nil, err
		}
		hal.TeleClient = halproto.NewTelemetryClient(hal.client.ClientConn)
		haldp.Hal = hal
		return &haldp, nil
	}

	hal.mockCtrl = gomock.NewController(&hal)
	hal.MockClients = mockClients{
		MockTeleClient: halproto.NewMockTelemetryClient(hal.mockCtrl),
	}
	hal.TeleClient = hal.MockClients.MockTeleClient

	haldp.Hal = hal
	haldp.Hal.setExpectations()
	return &haldp, nil
}

func (hd *Hal) setExpectations() {
	hd.MockClients.MockTeleClient.EXPECT().MirrorSessionCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockTeleClient.EXPECT().MirrorSessionUpdate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockTeleClient.EXPECT().MirrorSessionDelete(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
}

func (hd *Hal) createNewGRPCClient() (*rpckit.RPCClient, error) {
	halPort := os.Getenv("HAL_GRPC_PORT")
	if halPort == "" {
		halPort = halGRPCDefaultPort
	}
	srvURL := halGRPCDefaultBaseURL + ":" + halPort
	// create a grpc client
	// ToDo Use AgentID for mysvcName
	rpcClient, err := rpckit.NewRPCClient("hal", srvURL, rpckit.WithTLSProvider(nil))
	if err != nil {
		log.Errorf("Creating gRPC Client failed on HAL Troubleshooting API Datapath. Server URL: %s", srvURL)
		return nil, err
	}

	return rpcClient, err
}

// objectKey returns object key from meta
func objectKey(ometa *api.ObjectMeta) string {
	return fmt.Sprintf("%s|%s", ometa.Tenant, ometa.Name)
}

// SetAgent sets the agent for this datapath
func (hd *Datapath) SetAgent(ag types.TsDatapathIntf) error {
	return nil
}

// This function deletes partially updated flow/drop rules and mirror session during mirror session create.
func handleMirrorSessionCreateFailure(hd *Datapath, db *types.MirrorDB,
	mirrorSessionReqs []*halproto.MirrorSessionRequestMsg,
	flowRuleReqs []*halproto.FlowMonitorRuleRequestMsg,
	dropRuleReqs []*halproto.DropMonitorRuleRequestMsg) {

	for _, halConsumedDropRule := range dropRuleReqs {
		ReqMsg := halproto.DropMonitorRuleDeleteRequestMsg{}
		for _, ruleSpec := range halConsumedDropRule.Request {
			dropDeleteRule := halproto.DropMonitorRuleDeleteRequest{
				Meta: &halproto.ObjectMeta{
					VrfId: 0, // TODO: Map Tenant-Name to Vrf
				},
				KeyOrHandle: &halproto.DropMonitorRuleKeyHandle{
					KeyOrHandle: &halproto.DropMonitorRuleKeyHandle_DropmonitorruleId{
						DropmonitorruleId: ruleSpec.KeyOrHandle.GetDropmonitorruleId(),
					},
				},
			}
			ReqMsg.Request = append(ReqMsg.Request, &dropDeleteRule)
			//clean up locally maintained map for this rule.
			dropRule := db.DropRuleIDToDropRule[ruleSpec.KeyOrHandle.GetDropmonitorruleId()]
			delete(db.DropMonitorRules, dropRule)
			delete(db.DropRuleIDToDropRule, ruleSpec.KeyOrHandle.GetDropmonitorruleId())
		}
		hd.Hal.TeleClient.DropMonitorRuleDelete(context.Background(), &ReqMsg)
	}
	for _, halConsumedFlowRule := range flowRuleReqs {
		FlowReqMsg := halproto.FlowMonitorRuleDeleteRequestMsg{}
		for _, ruleSpec := range halConsumedFlowRule.Request {
			flowDeleteRule := halproto.FlowMonitorRuleDeleteRequest{
				Meta: &halproto.ObjectMeta{
					VrfId: 0, // TODO: Map Tenant-Name to Vrf
				},
				KeyOrHandle: &halproto.FlowMonitorRuleKeyHandle{
					KeyOrHandle: &halproto.FlowMonitorRuleKeyHandle_FlowmonitorruleId{
						FlowmonitorruleId: ruleSpec.KeyOrHandle.GetFlowmonitorruleId(),
					},
				},
			}
			FlowReqMsg.Request = append(FlowReqMsg.Request, &flowDeleteRule)
			//clean up locally maintained map for this rule.
			flowRule := db.FlowMonitorRuleIDToFlowMonitorRule[ruleSpec.KeyOrHandle.GetFlowmonitorruleId()]
			delete(db.FlowMonitorRules, flowRule)
			delete(db.FlowMonitorRuleIDToFlowMonitorRule, ruleSpec.KeyOrHandle.GetFlowmonitorruleId())
		}
		hd.Hal.TeleClient.FlowMonitorRuleDelete(context.Background(), &FlowReqMsg)
	}

	for _, halConsumedMirrorSession := range mirrorSessionReqs {
		MirrorReqMsg := halproto.MirrorSessionDeleteRequestMsg{}
		for _, mirrorSess := range halConsumedMirrorSession.Request {
			mirrorDelReq := halproto.MirrorSessionDeleteRequest{
				Meta: &halproto.ObjectMeta{
					VrfId: 0, // TODO: Map Tenant-Name to Vrf
				},
				KeyOrHandle: &halproto.MirrorSessionKeyHandle{
					KeyOrHandle: &halproto.MirrorSessionKeyHandle_MirrorsessionId{
						MirrorsessionId: mirrorSess.KeyOrHandle.GetMirrorsessionId(),
					},
				},
			}
			MirrorReqMsg.Request = append(MirrorReqMsg.Request, &mirrorDelReq)
		}
		hd.Hal.TeleClient.MirrorSessionDelete(context.Background(), &MirrorReqMsg)
	}

}

// CreatePacketCaptureSession creates a mirror session
func (hd *Datapath) CreatePacketCaptureSession(mirrorSessionName string, db *types.MirrorDB, mirrorReqMsg *halproto.MirrorSessionRequestMsg, flowRuleReqMsgList []*halproto.FlowMonitorRuleRequestMsg, dropRuleReqMsgList []*halproto.DropMonitorRuleRequestMsg) error {

	var err error

	log.Infof("Packet capture session create request sent to Hal {%+v}", mirrorReqMsg)

	// Maintains list of RequestMessages for each of MirrorCreate, FlowRuleCreate, DropRuleCreate.
	// These lists are used to cleanup rules and/or mirror session in case of any failures.
	var halConsumedMirrorSessions []*halproto.MirrorSessionRequestMsg
	var halConsumedFlowRules []*halproto.FlowMonitorRuleRequestMsg
	var halConsumedDropRules []*halproto.DropMonitorRuleRequestMsg

	if mirrorReqMsg != nil {
		resp, err := hd.Hal.TeleClient.MirrorSessionCreate(context.Background(), mirrorReqMsg)
		if err != nil {
			log.Errorf("Error creating mirror session. Err: %v", err)
		}
		if hd.Kind == "hal" {
			if resp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
				log.Errorf("HAL returned non OK status when creating mirror session. %v",
					resp.Response[0].ApiStatus)
				err = ErrMirrorCreate
			} else {
				halConsumedMirrorSessions = append(halConsumedMirrorSessions, mirrorReqMsg)
				db.PktMirrorSessions[mirrorSessionName].Handle = resp.Response[0].Status.Handle
				db.PktMirrorSessions[mirrorSessionName].Created = true
			}
		}
	}

	if err == nil && len(flowRuleReqMsgList) > 0 {
		for _, flowRuleReqMsg := range flowRuleReqMsgList {
			resp, err := hd.Hal.TeleClient.FlowMonitorRuleCreate(context.Background(), flowRuleReqMsg)
			if err != nil {
				log.Errorf("Error creating flow monitor rule. Err: %v", err)
			}
			if err == nil && hd.Kind == "hal" {
				for _, Response := range resp.Response {
					if Response.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
						log.Errorf("Flow monitor rule error. HAL returned Err. %v",
							Response.ApiStatus)
						err = ErrFlowMonitorRuleCreate
					} else {
						halConsumedFlowRules = append(halConsumedFlowRules, flowRuleReqMsg)
					}
				}
			}
		}
	}

	if err == nil && len(dropRuleReqMsgList) > 0 {
		for _, dropRuleReqMsg := range dropRuleReqMsgList {
			resp, err := hd.Hal.TeleClient.DropMonitorRuleCreate(context.Background(), dropRuleReqMsg)
			if err != nil {
				log.Errorf("Error creating drop monitor rule. Err: %v", err)
			}
			if err == nil && hd.Kind == "hal" {
				for _, Response := range resp.Response {
					if Response.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
						log.Errorf("Drop monitor rule error. HAL returned Err. %v",
							Response.ApiStatus)
						err = ErrDropMonitorRuleCreate
					} else {
						halConsumedDropRules = append(halConsumedDropRules, dropRuleReqMsg)
					}
				}
			}
		}
	}

	if err == nil {
		if len(flowRuleReqMsgList) > 0 {
			// Since all FlowRules are successfully processed by HAL,
			// create FlowRule to FlowStatus map
			for _, flowRuleReqMsg := range flowRuleReqMsgList {
				for _, ruleSpec := range flowRuleReqMsg.Request {
					flowRule := db.FlowMonitorRuleIDToFlowMonitorRule[ruleSpec.KeyOrHandle.
						GetFlowmonitorruleId()]
					db.FlowMonitorRules[flowRule] = &types.MonitorRuleStatus{
						RuleID:  ruleSpec.KeyOrHandle.GetFlowmonitorruleId(),
						Applied: true,
						//Handle:Need a way to map DoprMonitorRuleStatus to ruleID
						//so that handle returned by HAL corresponds to appropriate
						//DropRule. For now ignore handle value returned by HAL
					}
				}
			}
		}
		if len(dropRuleReqMsgList) > 0 {
			// Since all dropRules are successfully processed by HAL,
			// create dropRule to dropStatus map
			for _, dropRuleReqMsg := range dropRuleReqMsgList {
				for _, ruleSpec := range dropRuleReqMsg.Request {
					dropRule := db.DropRuleIDToDropRule[ruleSpec.KeyOrHandle.
						GetDropmonitorruleId()]
					db.DropMonitorRules[dropRule] = &types.MonitorRuleStatus{
						RuleID:  ruleSpec.KeyOrHandle.GetDropmonitorruleId(),
						Applied: true,
						//Handle:Need a way to map DoprMonitorRuleStatus to ruleID
						//so that handle returned by HAL corresponds to appropriate
						//DropRule. For now ignore handle value returned by HAL
					}
				}
			}
		}
	} else {
		handleMirrorSessionCreateFailure(hd, db, halConsumedMirrorSessions,
			halConsumedFlowRules, halConsumedDropRules)
	}

	return err
}

// UpdatePacketCaptureSession updates a mirror session
func (hd *Datapath) UpdatePacketCaptureSession(mirrorSession *tsproto.MirrorSession) error {
	var err error
	log.Infof("Update packet capture session request being sent to Hal {%+v}", mirrorSession)

	return err
}

// DeletePacketCaptureSession deletes mirror session
func (hd *Datapath) DeletePacketCaptureSession(mirrorSession *tsproto.MirrorSession) error {
	var err error
	log.Infof("Delete packet capture session request being sent to Hal {%+v}", mirrorSession)

	return err
}
