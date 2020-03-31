package statemgr

import (
	"fmt"
	"strings"
	"sync"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/ctkit"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/ref"
	"github.com/pensando/sw/venice/utils/runtime"
)

//SmFlowExportPolicyInterface is statemanagers struct for export policy
type SmFlowExportPolicyInterface struct {
	featureMgrBase
	sync.Mutex
	sm *Statemgr
}

// FlowExportPolicyState keep state for flow export policy
type FlowExportPolicyState struct {
	FlowExportPolicy *ctkit.FlowExportPolicy `json:"-"`
	stateMgr         *SmFlowExportPolicyInterface
}

// NewFlowExportPolicyState creates new app state object
func NewFlowExportPolicyState(fe *ctkit.FlowExportPolicy, stateMgr *SmFlowExportPolicyInterface) (*FlowExportPolicyState, error) {
	fes := &FlowExportPolicyState{
		FlowExportPolicy: fe,
		stateMgr:         stateMgr,
	}
	fe.HandlerCtx = fes

	return fes, nil
}

// FlowExportPolicyStateFromObj conerts from memdb object to network state
func FlowExportPolicyStateFromObj(obj runtime.Object) (*FlowExportPolicyState, error) {
	switch obj.(type) {
	case *ctkit.FlowExportPolicy:
		sgobj := obj.(*ctkit.FlowExportPolicy)
		switch sgobj.HandlerCtx.(type) {
		case *FlowExportPolicyState:
			sgs := sgobj.HandlerCtx.(*FlowExportPolicyState)
			return sgs, nil
		default:
			return nil, ErrIncorrectObjectType
		}

	default:
		return nil, ErrIncorrectObjectType
	}
}

var smgrFlowExportPolicyInterface *SmFlowExportPolicyInterface

// CompleteRegistration is the callback function statemgr calls after init is done
func (smm *SmFlowExportPolicyInterface) CompleteRegistration() {
	log.Infof("Got CompleteRegistration for smgrFlowExportPolicyInterface %v", smm)
	smm.sm.SetFlowExportPolicyReactor(smgrFlowExportPolicyInterface)
}

func initSmFlowExportPolicyInterface() {
	mgr := MustGetStatemgr()
	smgrFlowExportPolicyInterface = &SmFlowExportPolicyInterface{
		sm: mgr,
	}
	mgr.Register("statemgrFlowExportPolicy", smgrFlowExportPolicyInterface)
}

// FindFlowExportPolicy finds mirror session state
func (smm *SmFlowExportPolicyInterface) FindFlowExportPolicy(tenant, name string) (*FlowExportPolicyState, error) {
	// find the object
	obj, err := smm.sm.FindObject("FlowExportPolicy", tenant, "default", name)
	if err != nil {
		return nil, err
	}

	return FlowExportPolicyStateFromObj(obj)
}

func convertFlowExportPolicy(fePolicy *monitoring.FlowExportPolicy) *netproto.FlowExportPolicy {
	var (
		matchRules                 []netproto.MatchRule
		srcAddresses, dstAddresses []string
		exports                    []netproto.ExportConfig
	)

	flowExportPolicy := &netproto.FlowExportPolicy{
		TypeMeta:   fePolicy.TypeMeta,
		ObjectMeta: fePolicy.ObjectMeta,
	}

	for _, r := range fePolicy.Spec.MatchRules {
		var protoPorts []*netproto.ProtoPort
		if r.AppProtoSel != nil {
			for _, pp := range r.AppProtoSel.ProtoPorts {
				var protoPort netproto.ProtoPort
				components := strings.Split(pp, "/")
				switch len(components) {
				case 1:
					protoPort.Protocol = components[0]
				case 2:
					protoPort.Protocol = components[0]
					protoPort.Port = components[1]
				case 3:
					protoPort.Protocol = components[0]
					protoPort.Port = fmt.Sprintf("%s/%s", components[1], components[2])
				default:
					continue
				}
				protoPorts = append(protoPorts, &protoPort)
			}
		}

		if r.Src != nil && r.Src.IPAddresses != nil {
			srcAddresses = r.Src.IPAddresses
		}

		if r.Dst != nil && r.Dst.IPAddresses != nil {
			dstAddresses = r.Dst.IPAddresses
		}

		m := netproto.MatchRule{
			Src: &netproto.MatchSelector{
				Addresses: srcAddresses,
			},
			Dst: &netproto.MatchSelector{
				Addresses:  dstAddresses,
				ProtoPorts: protoPorts,
			},
		}
		matchRules = append(matchRules, m)
	}

	for _, exp := range fePolicy.Spec.Exports {
		var protoPort netproto.ProtoPort
		components := strings.Split(exp.Transport, "/")
		switch len(components) {
		case 1:
			protoPort.Protocol = components[0]
		case 2:
			protoPort.Protocol = components[0]
			protoPort.Port = components[1]
		case 3:
			protoPort.Protocol = components[0]
			protoPort.Port = fmt.Sprintf("%s/%s", components[1], components[2])
		default:
			continue
		}
		e := netproto.ExportConfig{
			Destination: exp.Destination,
			Gateway:     exp.Gateway,
			Transport:   &protoPort,
		}

		exports = append(exports, e)
	}

	flowExportPolicy.Spec = netproto.FlowExportPolicySpec{
		VrfName:          fePolicy.Spec.VrfName,
		Interval:         fePolicy.Spec.Interval,
		TemplateInterval: fePolicy.Spec.TemplateInterval,
		Format:           fePolicy.Spec.Format,
		Exports:          exports,
		MatchRules:       matchRules,
	}

	return flowExportPolicy
}

//OnFlowExportPolicyCreate mirror session create handle
func (smm *SmFlowExportPolicyInterface) OnFlowExportPolicyCreate(obj *ctkit.FlowExportPolicy) error {

	log.Infof("Creating flow export policy : %+v", obj)

	fes, err := NewFlowExportPolicyState(obj, smm)
	if err != nil {
		log.Errorf("Error flow export policy r %+v. Err: %v", obj, err)
		return err
	}

	return smm.sm.mbus.AddObjectWithReferences(obj.FlowExportPolicy.MakeKey("monitoring"),
		convertFlowExportPolicy(&fes.FlowExportPolicy.FlowExportPolicy), references(&obj.FlowExportPolicy))
}

//OnFlowExportPolicyUpdate mirror session create handle
func (smm *SmFlowExportPolicyInterface) OnFlowExportPolicyUpdate(obj *ctkit.FlowExportPolicy, newObj *monitoring.FlowExportPolicy) error {

	log.Infof("Got flow export update for %#v", newObj.ObjectMeta)

	// see if anything changed
	_, ok := ref.ObjDiff(obj.FlowExportPolicy.Spec, newObj.Spec)
	if (obj.FlowExportPolicy.GenerationID == newObj.GenerationID) && !ok {
		//mss.MirrorSession.ObjectMeta = nmirror.ObjectMeta
		return nil
	}

	_, err := FlowExportPolicyStateFromObj(obj)
	if err != nil {
		log.Errorf("Error finding mirror object for delete %v", err)
		return err
	}

	// update old state
	obj.FlowExportPolicy.Spec = newObj.Spec
	obj.FlowExportPolicy.ObjectMeta = newObj.ObjectMeta
	return smm.sm.mbus.UpdateObjectWithReferences(obj.FlowExportPolicy.MakeKey("monitoring"),
		convertFlowExportPolicy(&obj.FlowExportPolicy), references(newObj))
}

//OnFlowExportPolicyDelete mirror session create handle
func (smm *SmFlowExportPolicyInterface) OnFlowExportPolicyDelete(obj *ctkit.FlowExportPolicy) error {
	return smm.sm.mbus.DeleteObjectWithReferences(obj.FlowExportPolicy.MakeKey("monitoring"),
		convertFlowExportPolicy(&obj.FlowExportPolicy), references(&obj.FlowExportPolicy))
}

//GetFlowExportPolicyWatchOptions Get watch options
func (smm *SmFlowExportPolicyInterface) GetFlowExportPolicyWatchOptions() *api.ListWatchOptions {
	opts := &api.ListWatchOptions{}
	opts.FieldChangeSelector = []string{"ObjectMeta", "Spec"}
	return opts
}

func init() {
	initSmFlowExportPolicyInterface()
}
