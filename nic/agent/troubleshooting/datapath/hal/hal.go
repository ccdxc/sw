// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package troubleshootingHal

import (
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"os"
	"sync"

	"github.com/golang/mock/gomock"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/dscagent/types/irisproto"
	"github.com/pensando/sw/nic/agent/troubleshooting/state/types"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
)

const (
	halMaxMirrorSession   = 8
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
	//MockTeleClient *halproto.MockTelemetryClient
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
func NewHalDatapath() (*Datapath, error) {
	var err error
	var hal Hal
	haldp := Datapath{}

	hal.client, err = hal.createNewGRPCClient()
	if err != nil {
		return nil, err
	}
	hal.TeleClient = halproto.NewTelemetryClient(hal.client.ClientConn)
	haldp.Hal = hal
	return &haldp, nil

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
func handleMirrorSessionCreateFailure(hd *Datapath, vrfID uint64, db *types.MirrorDB,
	mirrorSessionReqs []*halproto.MirrorSessionRequestMsg,
	flowRuleReqs []*halproto.FlowMonitorRuleRequestMsg,
	dropRuleReqs []*halproto.DropMonitorRuleRequestMsg) {

	for _, halConsumedDropRule := range dropRuleReqs {
		ReqMsg := halproto.DropMonitorRuleDeleteRequestMsg{}
		for _, ruleSpec := range halConsumedDropRule.Request {
			dropDeleteRule := halproto.DropMonitorRuleDeleteRequest{
				KeyOrHandle: &halproto.DropMonitorRuleKeyHandle{
					KeyOrHandle: &halproto.DropMonitorRuleKeyHandle_DropmonitorruleId{
						DropmonitorruleId: ruleSpec.KeyOrHandle.GetDropmonitorruleId(),
					},
				},
			}
			ReqMsg.Request = append(ReqMsg.Request, &dropDeleteRule)
		}
		hd.Hal.TeleClient.DropMonitorRuleDelete(context.Background(), &ReqMsg)
	}
	for _, halConsumedFlowRule := range flowRuleReqs {
		FlowReqMsg := halproto.FlowMonitorRuleDeleteRequestMsg{}
		for _, ruleSpec := range halConsumedFlowRule.Request {
			flowDeleteRule := halproto.FlowMonitorRuleDeleteRequest{
				KeyOrHandle: &halproto.FlowMonitorRuleKeyHandle{
					KeyOrHandle: &halproto.FlowMonitorRuleKeyHandle_FlowmonitorruleId{
						FlowmonitorruleId: ruleSpec.KeyOrHandle.GetFlowmonitorruleId(),
					},
				},
			}
			FlowReqMsg.Request = append(FlowReqMsg.Request, &flowDeleteRule)
		}
		hd.Hal.TeleClient.FlowMonitorRuleDelete(context.Background(), &FlowReqMsg)
	}

	for _, halConsumedMirrorSession := range mirrorSessionReqs {
		MirrorReqMsg := halproto.MirrorSessionDeleteRequestMsg{}
		for _, mirrorSess := range halConsumedMirrorSession.Request {
			mirrorDelReq := halproto.MirrorSessionDeleteRequest{
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

func checkIDContainment(IDs []uint64, ID uint64) (bool, int) {
	for i, v := range IDs {
		if v == ID {
			return true, i
		}
	}
	return false, 0
}

// createUpdateMirorSession is used to create new  mirror session or update exisitng session.
func (hd *Datapath) createUpdateMirrorSession(mirrorSessionKey string, vrfID uint64, update bool, db *types.MirrorDB,
	mirrorReqMsg *halproto.MirrorSessionRequestMsg,
	flowRuleReqMsgList []*halproto.FlowMonitorRuleRequestMsg,
	dropRuleReqMsgList []*halproto.DropMonitorRuleRequestMsg) ([]uint64, []uint64, error) {

	var err error

	// Maintains list of RequestMessages for each of MirrorCreate, FlowRuleCreate, DropRuleCreate.
	// These lists are used to cleanup rules and/or mirror session in case of any failures.
	var halConsumedMirrorSessions []*halproto.MirrorSessionRequestMsg
	var halConsumedFlowRules []*halproto.FlowMonitorRuleRequestMsg
	var halConsumedDropRules []*halproto.DropMonitorRuleRequestMsg
	var purgedFlowRuleIDs []uint64
	var purgedDropRuleIDs []uint64

	mirrorSessID, ok := db.MirrorSessionNameToID[mirrorSessionKey]
	if !ok {
		return nil, nil, ErrMirrorCreate
	}
	mirrorSessObj, ok := db.MirrorSessionIDToObj[mirrorSessID]
	if !ok {
		return nil, nil, ErrMirrorCreate
	}
	if mirrorReqMsg != nil {
		if update {
			// delete and create again
			delReq := &halproto.MirrorSessionDeleteRequestMsg{}

			// build del request
			for _, msg := range mirrorReqMsg.Request {
				delReq.Request = append(delReq.Request, &halproto.MirrorSessionDeleteRequest{
					KeyOrHandle: msg.KeyOrHandle,
				})
			}

			log.Infof("delete mirror session %+v", mirrorReqMsg)
			_, err = hd.Hal.TeleClient.MirrorSessionDelete(context.Background(), delReq)
			if err != nil {
				log.Errorf("Error deleting mirror session. Err: %v, req: %+v", err, delReq)
				return []uint64{}, []uint64{}, err
			}
		}

		var resp *halproto.MirrorSessionResponseMsg
		b, _ := json.MarshalIndent(mirrorReqMsg, "", "   ")
		fmt.Println("############### MIRROR SESSION REQUEST MESSAGE ############")
		fmt.Println(string(b))
		fmt.Println("############### MIRROR SESSION REQUEST MESSAGE ############")
		resp, err = hd.Hal.TeleClient.MirrorSessionCreate(context.Background(), mirrorReqMsg)
		if err != nil {
			log.Errorf("Error creating mirror session. Err: %v, req:%+v", err, mirrorReqMsg)
		}
		if err == nil && hd.Kind.String() == "hal" {
			if resp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
				log.Errorf("HAL returned non OK status when creating mirror session. %+v, req: %+v",
					resp.Response[0].ApiStatus, mirrorReqMsg)
				err = ErrMirrorCreate
			} else {
				halConsumedMirrorSessions = append(halConsumedMirrorSessions, mirrorReqMsg)
				mirrorSessObj.Handle = resp.Response[0].Status.Handle
				mirrorSessObj.Created = true
			}
		}
	}

	if err == nil && len(flowRuleReqMsgList) > 0 {
		var resp *halproto.FlowMonitorRuleResponseMsg
		for _, flowRuleReqMsg := range flowRuleReqMsgList {
			resp, err = hd.Hal.TeleClient.FlowMonitorRuleCreate(context.Background(), flowRuleReqMsg)
			if err != nil {
				log.Errorf("Error creating flow monitor rule. Err: %v, req: %+v", err, flowRuleReqMsg)
			}
			if err == nil && hd.Kind.String() == "hal" {
				for _, Response := range resp.Response {
					if Response.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
						log.Errorf("Flow monitor rule create error. HAL returned Err. %v, req: %+v",
							Response.ApiStatus, flowRuleReqMsg)
						err = ErrFlowMonitorRuleCreate
					} else {
						halConsumedFlowRules = append(halConsumedFlowRules, flowRuleReqMsg)
					}
				}
			}
		}
	}

	if err == nil && len(dropRuleReqMsgList) > 0 {
		var resp *halproto.DropMonitorRuleResponseMsg
		for _, dropRuleReqMsg := range dropRuleReqMsgList {
			resp, err = hd.Hal.TeleClient.DropMonitorRuleCreate(context.Background(), dropRuleReqMsg)
			if err != nil {
				log.Errorf("Error creating drop monitor rule. Err: %v, req: %+v", err, dropRuleReqMsg)
			}
			if err == nil && hd.Kind.String() == "hal" {
				for _, Response := range resp.Response {
					if Response.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
						log.Errorf("Drop monitor rule create error. HAL returned Err. %v, req: %+v",
							Response.ApiStatus, dropRuleReqMsg)
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
			oldFlowRuleIDs := mirrorSessObj.FlowMonitorRuleIDs
			mirrorSessObj.FlowMonitorRuleIDs = nil
			// Since all FlowRules are successfully processed by HAL,
			// create FlowRule to FlowStatus map
			for _, flowRuleReqMsg := range flowRuleReqMsgList {
				for _, ruleSpec := range flowRuleReqMsg.Request {
					ruleID := ruleSpec.KeyOrHandle.GetFlowmonitorruleId()
					flowRuleObj := db.FlowMonitorRuleIDToObj[ruleID]
					//Add mirrorSession Key to the rule if this rule is not associated with mirrorsession
					if len(flowRuleObj.MirrorSessionIDs) >= halMaxMirrorSession {
						// Assert
					}
					contains, _ := checkIDContainment(flowRuleObj.MirrorSessionIDs, mirrorSessID)
					if !contains {
						flowRuleObj.MirrorSessionIDs = append(flowRuleObj.MirrorSessionIDs, mirrorSessID)
						db.FlowMonitorRuleIDToObj[ruleID] = flowRuleObj
					}
					mirrorSessObj.FlowMonitorRuleIDs = append(mirrorSessObj.FlowMonitorRuleIDs, ruleID)
				}
			}
			//In case of mirror session update, if previous flow rules are dropped, for those rules
			// we need to delete rule  or remove mirrorSession from the rule if the rule has
			// list of active mirrorSessions. Prepare a list of purged RuleIDs and return.
			if update {
				for _, f := range oldFlowRuleIDs {
					present, _ := checkIDContainment(mirrorSessObj.FlowMonitorRuleIDs, f)
					if !present {
						//oldRuleID is not part of mirrorSession anymore.
						purgedFlowRuleIDs = append(purgedFlowRuleIDs, f)
					}
				}
			}
		}
		if len(dropRuleReqMsgList) > 0 {
			oldDropRuleIDs := mirrorSessObj.DropMonitorRuleIDs
			mirrorSessObj.DropMonitorRuleIDs = nil
			// Since all dropRules are successfully processed by HAL,
			// create dropRule to dropStatus map
			for _, dropRuleReqMsg := range dropRuleReqMsgList {
				for _, ruleSpec := range dropRuleReqMsg.Request {
					ruleID := ruleSpec.KeyOrHandle.GetDropmonitorruleId()
					dropRuleObj := db.DropRuleIDToObj[ruleID]
					//Add mirrorSession key to the rule if this rule is not associated with mirrorsession
					if len(dropRuleObj.MirrorSessionIDs) >= halMaxMirrorSession {
						// Assert
					}
					contains, _ := checkIDContainment(dropRuleObj.MirrorSessionIDs, mirrorSessID)
					if !contains {
						dropRuleObj.MirrorSessionIDs = append(dropRuleObj.MirrorSessionIDs, mirrorSessID)
						db.DropRuleIDToObj[ruleID] = dropRuleObj
					}
					mirrorSessObj.DropMonitorRuleIDs = append(mirrorSessObj.DropMonitorRuleIDs, ruleID)
				}
			}
			//In case of mirror session update, if previous drop rules are dropped, for those rules
			// we need to delete rule  or remove mirrorSession from the rule if the rule has
			// list of active mirrorSessions. Prepare a list of purged RuleIDs and return.
			if update {
				for _, d := range oldDropRuleIDs {
					present, _ := checkIDContainment(mirrorSessObj.DropMonitorRuleIDs, d)
					if !present {
						//oldRuleID is not part of mirrorSession anymore.
						purgedDropRuleIDs = append(purgedDropRuleIDs, d)
					}
				}
			}
		}
	} else {
		handleMirrorSessionCreateFailure(hd, vrfID, db, halConsumedMirrorSessions,
			halConsumedFlowRules, halConsumedDropRules)
	}

	db.MirrorSessionIDToObj[mirrorSessID] = mirrorSessObj

	return purgedFlowRuleIDs, purgedDropRuleIDs, err
}

// CreatePacketCaptureSession creates a mirror session
func (hd *Datapath) CreatePacketCaptureSession(mirrorSessionKey string, vrfID uint64, db *types.MirrorDB,
	mirrorReqMsg *halproto.MirrorSessionRequestMsg,
	flowRuleReqMsgList []*halproto.FlowMonitorRuleRequestMsg,
	dropRuleReqMsgList []*halproto.DropMonitorRuleRequestMsg) error {
	log.Infof("Packet capture session create request sent to Hal {%+v}, {%+v}, {%+v}", mirrorReqMsg,
		flowRuleReqMsgList, dropRuleReqMsgList)
	_, _, err := hd.createUpdateMirrorSession(mirrorSessionKey, vrfID, false, db, mirrorReqMsg,
		flowRuleReqMsgList, dropRuleReqMsgList)
	return err
}

// UpdatePacketCaptureSession updates a mirror session
func (hd *Datapath) UpdatePacketCaptureSession(mirrorSessionKey string, vrfID uint64, db *types.MirrorDB,
	mirrorReqMsg *halproto.MirrorSessionRequestMsg,
	flowRuleReqMsgList []*halproto.FlowMonitorRuleRequestMsg,
	dropRuleReqMsgList []*halproto.DropMonitorRuleRequestMsg) ([]uint64, []uint64, error) {
	log.Infof("Update packet capture session request being sent to Hal {%+v}", mirrorSessionKey)
	return hd.createUpdateMirrorSession(mirrorSessionKey, vrfID, true, db, mirrorReqMsg,
		flowRuleReqMsgList, dropRuleReqMsgList)
}

// DeletePacketCaptureSession deletes mirror session
func (hd *Datapath) DeletePacketCaptureSession(mirrorSessionKey string, mirrorDeleteReqMsg *halproto.MirrorSessionDeleteRequestMsg) error {
	log.Debugf("Delete packet capture session request being sent to Hal {%+v}", mirrorSessionKey)
	if mirrorDeleteReqMsg == nil {
		return nil
	}

	resp, err := hd.Hal.TeleClient.MirrorSessionDelete(context.Background(), mirrorDeleteReqMsg)
	if err != nil {
		log.Errorf("Error deleting  mirror session. Err: %v, req:%+v", err, mirrorDeleteReqMsg)
		return err
	}

	if hd.Kind.String() == "hal" {
		if resp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL returned non OK status when deleting mirror session. %v, req: %+v",
				resp.Response[0].ApiStatus, mirrorDeleteReqMsg)
			err = ErrMirrorCreate
		}
	}

	return err
}

// UpdateFlowMonitorRule updates changes to existing FlowMonitor rule
func (hd *Datapath) UpdateFlowMonitorRule(flowRuleReqMsgList []*halproto.FlowMonitorRuleRequestMsg) error {
	for _, flowRuleReqMsg := range flowRuleReqMsgList {
		resp, err := hd.Hal.TeleClient.FlowMonitorRuleCreate(context.Background(), flowRuleReqMsg)
		if err != nil {
			log.Errorf("Error creating flow monitor rule. Err: %v, req:%+v", err, flowRuleReqMsg)
		}
		if err == nil && hd.Kind.String() == "hal" {
			for _, Response := range resp.Response {
				if Response.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
					log.Errorf("Flow monitor rule error. HAL returned Err. %v",
						Response.ApiStatus)
				}
			}
		}
	}
	return nil
}

// DeleteFlowMonitorRule updates removes FlowMonitor rule
func (hd *Datapath) DeleteFlowMonitorRule(flowRuleDeleteReqMsgList []*halproto.FlowMonitorRuleDeleteRequestMsg) error {
	for _, flowRuleDeleteReqMsg := range flowRuleDeleteReqMsgList {
		resp, err := hd.Hal.TeleClient.FlowMonitorRuleDelete(context.Background(), flowRuleDeleteReqMsg)
		if err != nil {
			log.Errorf("Error deleting flow monitor rule. Err: %v, req: %+v", err, flowRuleDeleteReqMsg)
		}
		if err == nil && hd.Kind.String() == "hal" {
			for _, Response := range resp.Response {
				if Response.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
					log.Errorf("Flow monitor rule delete error. HAL returned Err. %v, req: %+v",
						Response.ApiStatus, flowRuleDeleteReqMsg)
				}
			}
		}
	}
	return nil
}

// UpdateDropMonitorRule updates changes to existing DropMonitor rule
func (hd *Datapath) UpdateDropMonitorRule(dropRuleReqMsgList []*halproto.DropMonitorRuleRequestMsg) error {
	for _, dropRuleReqMsg := range dropRuleReqMsgList {
		resp, err := hd.Hal.TeleClient.DropMonitorRuleCreate(context.Background(), dropRuleReqMsg)
		if err != nil {
			log.Errorf("Error creating drop monitor rule. Err: %v, req: %+v", err, dropRuleReqMsg)
		}
		if err == nil && hd.Kind.String() == "hal" {
			for _, Response := range resp.Response {
				if Response.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
					log.Errorf("Drop monitor rule error. HAL returned Err. %v, req: %+v",
						Response.ApiStatus, dropRuleReqMsg)
				}
			}
		}
	}
	return nil
}

// DeleteDropMonitorRule updates removes DropMonitor rule
func (hd *Datapath) DeleteDropMonitorRule(dropRuleDeleteReqMsgList []*halproto.DropMonitorRuleDeleteRequestMsg) error {
	for _, dropRuleDeleteReqMsg := range dropRuleDeleteReqMsgList {
		resp, err := hd.Hal.TeleClient.DropMonitorRuleDelete(context.Background(), dropRuleDeleteReqMsg)
		if err != nil {
			log.Errorf("Error deleting drop monitor rule. Err: %v, req: %+v", err, dropRuleDeleteReqMsg)
		}
		if err == nil && hd.Kind.String() == "hal" {
			for _, Response := range resp.Response {
				if Response.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
					log.Errorf("Drop monitor rule delete error. HAL returned Err. %v, req: %+v",
						Response.ApiStatus, dropRuleDeleteReqMsg)
				}
			}
		}
	}
	return nil
}
