// +build iris

package fakehal

import (
	"context"
	"encoding/json"
	"fmt"

	irisproto "github.com/pensando/sw/nic/agent/dscagent/types/irisproto"
	"github.com/pensando/sw/venice/utils/log"
)

// ##########################n HAL methods stubed out to satisfy interface

// VrfGet stubbed out
func (h Hal) VrfGet(ctx context.Context, req *irisproto.VrfGetRequestMsg) (*irisproto.VrfGetResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got VrfGet Request:")
	fmt.Println(string(dat))

	return nil, nil
}

// SecurityProfileGet stubbed out
func (h Hal) SecurityProfileGet(ctx context.Context, req *irisproto.SecurityProfileGetRequestMsg) (*irisproto.SecurityProfileGetResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got SecurityProfileGet Request:")
	fmt.Println(string(dat))

	return nil, nil
}

// SecurityGroupPolicyCreate stubbed out
func (h Hal) SecurityGroupPolicyCreate(ctx context.Context, req *irisproto.SecurityGroupPolicyRequestMsg) (*irisproto.SecurityGroupPolicyResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got SecurityGroupPolicyCreate Request:")
	fmt.Println(string(dat))

	return nil, nil
}

// SecurityGroupPolicyUpdate stubbed out
func (h Hal) SecurityGroupPolicyUpdate(ctx context.Context, req *irisproto.SecurityGroupPolicyRequestMsg) (*irisproto.SecurityGroupPolicyResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got MirrorSession Request:")
	fmt.Println(string(dat))

	return nil, nil
}

// SecurityGroupPolicyDelete stubbed out
func (h Hal) SecurityGroupPolicyDelete(ctx context.Context, req *irisproto.SecurityGroupPolicyDeleteRequestMsg) (*irisproto.SecurityGroupPolicyDeleteResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got SecurityGroupPolicyDelete Request:")
	fmt.Println(string(dat))

	return nil, nil
}

// SecurityGroupPolicyGet stubbed out
func (h Hal) SecurityGroupPolicyGet(ctx context.Context, req *irisproto.SecurityGroupPolicyGetRequestMsg) (*irisproto.SecurityGroupPolicyGetResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got SecurityGroupPolicyGet Request:")
	fmt.Println(string(dat))

	return nil, nil
}

// SecurityGroupCreate stubbed out
func (h Hal) SecurityGroupCreate(ctx context.Context, req *irisproto.SecurityGroupRequestMsg) (*irisproto.SecurityGroupResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got SecurityGroupCreate Request:")
	fmt.Println(string(dat))

	return nil, nil
}

// SecurityGroupUpdate stubbed out
func (h Hal) SecurityGroupUpdate(ctx context.Context, req *irisproto.SecurityGroupRequestMsg) (*irisproto.SecurityGroupResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got SecurityGroupUpdate Request:")
	fmt.Println(string(dat))

	return nil, nil
}

// SecurityGroupDelete stubbed out
func (h Hal) SecurityGroupDelete(ctx context.Context, req *irisproto.SecurityGroupDeleteRequestMsg) (*irisproto.SecurityGroupDeleteResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got SecurityGroupDelete Request:")
	fmt.Println(string(dat))

	return nil, nil
}

// SecurityGroupGet stubbed out
func (h Hal) SecurityGroupGet(ctx context.Context, req *irisproto.SecurityGroupGetRequestMsg) (*irisproto.SecurityGroupGetResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got SecurityGroupGet Request:")
	fmt.Println(string(dat))

	return nil, nil
}

// SecurityPolicyGet stubbed out
func (h Hal) SecurityPolicyGet(ctx context.Context, req *irisproto.SecurityPolicyGetRequestMsg) (*irisproto.SecurityPolicyGetResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got SecurityPolicyGet Request:")
	fmt.Println(string(dat))

	return nil, nil
}

// SecurityFlowGateGet stubbed out
func (h Hal) SecurityFlowGateGet(ctx context.Context, req *irisproto.SecurityFlowGateGetRequestMsg) (*irisproto.SecurityFlowGateGetResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got SecurityFlowGateGet Request:")
	fmt.Println(string(dat))

	return nil, nil
}

// SecurityFlowGateDelete stubbed out
func (h Hal) SecurityFlowGateDelete(ctx context.Context, req *irisproto.SecurityFlowGateDeleteRequestMsg) (*irisproto.SecurityFlowGateDeleteResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got SecurityFlowGateDelete Request:")
	fmt.Println(string(dat))

	return nil, nil
}

// L2SegmentGet stubbed out
func (h Hal) L2SegmentGet(ctx context.Context, req *irisproto.L2SegmentGetRequestMsg) (*irisproto.L2SegmentGetResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got L2SegmentGet Request:")
	fmt.Println(string(dat))

	return nil, nil
}

// LifCreate stubbed out
func (h Hal) LifCreate(ctx context.Context, req *irisproto.LifRequestMsg) (*irisproto.LifResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got LifCreate Request:")
	fmt.Println(string(dat))

	return nil, nil
}

// LifUpdate stubbed out
func (h Hal) LifUpdate(ctx context.Context, req *irisproto.LifRequestMsg) (*irisproto.LifResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got LifCreate Request:")
	fmt.Println(string(dat))

	return nil, nil
}

// LifDelete stubbed out
func (h Hal) LifDelete(ctx context.Context, req *irisproto.LifDeleteRequestMsg) (*irisproto.LifDeleteResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got LifCreate Request:")
	fmt.Println(string(dat))

	return nil, nil
}

// LifGetQState stubbed out
func (h Hal) LifGetQState(ctx context.Context, req *irisproto.GetQStateRequestMsg) (*irisproto.GetQStateResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got LifGetQState Request:")
	fmt.Println(string(dat))

	return nil, nil
}

// LifSetQState stubbed out
func (h Hal) LifSetQState(ctx context.Context, req *irisproto.SetQStateRequestMsg) (*irisproto.SetQStateResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got LifSetQState Request:")
	fmt.Println(string(dat))

	return nil, nil
}

// AddL2SegmentOnUplink stubbed out
func (h Hal) AddL2SegmentOnUplink(ctx context.Context, req *irisproto.InterfaceL2SegmentRequestMsg) (*irisproto.InterfaceL2SegmentResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got AddL2SegmentOnUplink Request:")
	fmt.Println(string(dat))

	return nil, nil
}

// DelL2SegmentOnUplink stubbed out
func (h Hal) DelL2SegmentOnUplink(ctx context.Context, req *irisproto.InterfaceL2SegmentRequestMsg) (*irisproto.InterfaceL2SegmentResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got DelL2SegmentOnUplink Request:")
	fmt.Println(string(dat))

	return nil, nil
}

// EndpointGet stubbed out
func (h Hal) EndpointGet(ctx context.Context, req *irisproto.EndpointGetRequestMsg) (*irisproto.EndpointGetResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got EndpointGet Request:")
	fmt.Println(string(dat))

	return nil, nil
}

// FilterCreate stubbed out
func (h Hal) FilterCreate(ctx context.Context, req *irisproto.FilterRequestMsg) (*irisproto.FilterResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got FilterCreate Request:")
	fmt.Println(string(dat))

	return nil, nil
}

// FilterDelete stubbed out
func (h Hal) FilterDelete(ctx context.Context, req *irisproto.FilterDeleteRequestMsg) (*irisproto.FilterDeleteResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got FilterDelete Request:")
	fmt.Println(string(dat))

	return nil, nil
}

// FilterGet stubbed out
func (h Hal) FilterGet(ctx context.Context, req *irisproto.FilterGetRequestMsg) (*irisproto.FilterGetResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got FilterGet Request:")
	fmt.Println(string(dat))

	return nil, nil
}

// CollectorUpdate Stubbed out
func (h Hal) CollectorUpdate(ctx context.Context, req *irisproto.CollectorRequestMsg) (*irisproto.CollectorResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got CollectorUpdate Request:")
	fmt.Println(string(dat))

	return nil, nil
}

// CollectorGet Stubbed out
func (h Hal) CollectorGet(ctx context.Context, req *irisproto.CollectorGetRequestMsg) (*irisproto.CollectorGetResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got CollectorGet Request:")
	fmt.Println(string(dat))

	return nil, nil
}

// FlowMonitorRuleUpdate Stubbed out
func (h Hal) FlowMonitorRuleUpdate(ctx context.Context, req *irisproto.FlowMonitorRuleRequestMsg) (*irisproto.FlowMonitorRuleResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got FlowMonitorRuleUpdate Request:")
	fmt.Println(string(dat))

	return nil, nil
}

// FlowMonitorRuleGet Stubbed out
func (h Hal) FlowMonitorRuleGet(ctx context.Context, req *irisproto.FlowMonitorRuleGetRequestMsg) (*irisproto.FlowMonitorRuleGetResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got FlowMonitorRuleGet Request:")
	fmt.Println(string(dat))

	return nil, nil
}

// DropMonitorRuleCreate Stubbed out
func (h Hal) DropMonitorRuleCreate(ctx context.Context, req *irisproto.DropMonitorRuleRequestMsg) (*irisproto.DropMonitorRuleResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got DropMonitorRuleCreate Request:")
	fmt.Println(string(dat))

	return nil, nil
}

// DropMonitorRuleUpdate Stubbed out
func (h Hal) DropMonitorRuleUpdate(ctx context.Context, req *irisproto.DropMonitorRuleRequestMsg) (*irisproto.DropMonitorRuleResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got DropMonitorRuleUpdate Request:")
	fmt.Println(string(dat))

	return nil, nil
}

// DropMonitorRuleDelete Stubbed out
func (h Hal) DropMonitorRuleDelete(ctx context.Context, req *irisproto.DropMonitorRuleDeleteRequestMsg) (*irisproto.DropMonitorRuleDeleteResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got DropMonitorRuleDelete Request:")
	fmt.Println(string(dat))

	return nil, nil
}

// DropMonitorRuleGet Stubbed out
func (h Hal) DropMonitorRuleGet(ctx context.Context, req *irisproto.DropMonitorRuleGetRequestMsg) (*irisproto.DropMonitorRuleGetResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got DropMonitorRuleGet Request:")
	fmt.Println(string(dat))

	return nil, nil
}

// MirrorSessionGet Stubbed out
func (h Hal) MirrorSessionGet(ctx context.Context, req *irisproto.MirrorSessionGetRequestMsg) (*irisproto.MirrorSessionGetResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got MirrorSessionGet Request:")
	fmt.Println(string(dat))

	return nil, nil
}

// APIStatsGet Stubbed out
func (h Hal) APIStatsGet(ctx context.Context, req *irisproto.Empty) (*irisproto.ApiStatsResponse, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got APIStatsGet Request:")
	fmt.Println(string(dat))

	return nil, nil
}

// ClearIngressDropStats Stubbed out
func (h Hal) ClearIngressDropStats(ctx context.Context, req *irisproto.Empty) (*irisproto.Empty, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got ClearIngressDropStats Request:")
	fmt.Println(string(dat))

	return nil, nil
}

// ClearEgressDropStats Stubbed out
func (h Hal) ClearEgressDropStats(ctx context.Context, req *irisproto.Empty) (*irisproto.Empty, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got ClearEgressDropStats Request:")
	fmt.Println(string(dat))

	return nil, nil
}

// ClearPbDropStats Stubbed out
func (h Hal) ClearPbDropStats(ctx context.Context, req *irisproto.Empty) (*irisproto.Empty, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got ClearPbDropStats Request:")
	fmt.Println(string(dat))

	return nil, nil
}

// ClearFteStats Stubbed out
func (h Hal) ClearFteStats(ctx context.Context, req *irisproto.Empty) (*irisproto.Empty, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got ClearFteStats Request:")
	fmt.Println(string(dat))

	return nil, nil
}

// ClearFteTxRxStats Stubbed out
func (h Hal) ClearFteTxRxStats(ctx context.Context, req *irisproto.Empty) (*irisproto.Empty, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got ClearFteTxRxStats Request:")
	fmt.Println(string(dat))

	return nil, nil
}

// ClearTableStats Stubbed out
func (h Hal) ClearTableStats(ctx context.Context, req *irisproto.Empty) (*irisproto.Empty, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got ClearTableStats Request:")
	fmt.Println(string(dat))

	return nil, nil
}

// ClearPbStats Stubbed out
func (h Hal) ClearPbStats(ctx context.Context, req *irisproto.Empty) (*irisproto.Empty, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got ClearPbStats Request:")
	fmt.Println(string(dat))

	return nil, nil
}

// FeatureProfileGet Stubbed out
func (h Hal) FeatureProfileGet(ctx context.Context, req *irisproto.Empty) (*irisproto.FeatureProfileResponse, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got FeatureProfileGet Request:")
	fmt.Println(string(dat))

	return nil, nil
}

// MicroSegUpdate Stubbed out
func (h Hal) MicroSegUpdate(ctx context.Context, req *irisproto.MicroSegUpdateRequestMsg) (*irisproto.MicroSegUpdateResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got MicroSegUpdate Request:")
	fmt.Println(string(dat))

	return nil, nil
}

// MicroSegStatusUpdate Stubbed out
func (h Hal) MicroSegStatusUpdate(ctx context.Context, req *irisproto.MicroSegRequestMsg) (*irisproto.MicroSegResponseMsg, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got MicroSegStatusUpdate Request:")
	fmt.Println(string(dat))

	return nil, nil
}

// SysSpecCreate Stubbed out
func (h Hal) SysSpecCreate(ctx context.Context, req *irisproto.SysSpec) (*irisproto.SysSpecResponse, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got SysSpecCreate Request:")
	fmt.Println(string(dat))

	return nil, nil
}
