// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"errors"
	"fmt"
	"sync"

	"github.com/pensando/sw/nic/agent/protos/netproto"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/ctkit"
	"github.com/pensando/sw/api/generated/monitoring"
	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/api/labels"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/memdb"
	"github.com/pensando/sw/venice/utils/ref"
	"github.com/pensando/sw/venice/utils/runtime"
)

// MirrorSessionStateFromObj conerts from memdb object to network state
func MirrorSessionStateFromObj(obj runtime.Object) (*MirrorSessionState, error) {
	switch obj.(type) {
	case *ctkit.MirrorSession:
		sgobj := obj.(*ctkit.MirrorSession)
		switch sgobj.HandlerCtx.(type) {
		case *MirrorSessionState:
			sgs := sgobj.HandlerCtx.(*MirrorSessionState)
			return sgs, nil
		default:
			return nil, ErrIncorrectObjectType
		}

	default:
		return nil, ErrIncorrectObjectType
	}
}

// MirrorSessionState keep state of mirror session
type MirrorSessionState struct {
	MirrorSession *ctkit.MirrorSession `json:"-"`
	stateMgr      *SmMirrorSessionInterface
}

// FindMirrorSession finds mirror session state
func (smm *SmMirrorSessionInterface) FindMirrorSession(tenant, name string) (*MirrorSessionState, error) {
	// find the object
	obj, err := smm.sm.FindObject("MirrorSession", tenant, "default", name)
	if err != nil {
		return nil, err
	}

	return MirrorSessionStateFromObj(obj)
}

type mirrorCollector struct {
	refCount int
	obj      *netproto.Collector
	pushObj  memdb.PushObjectHandle
}

//SmMirrorSessionInterface is statemanagers struct for mirror session handling
type SmMirrorSessionInterface struct {
	sync.Mutex
	sm             *Statemgr
	collectors     map[string]*mirrorCollector
	mirrorSessions map[string]*MirrorSessionState
}

var smgrMirrorInterface *SmMirrorSessionInterface

// CompleteRegistration is the callback function statemgr calls after init is done
func (smm *SmMirrorSessionInterface) CompleteRegistration() {
	// if featureflags.IsOVerlayRoutingEnabled() == false {
	// 	return
	// }
	initSmMirrorInterface()
	log.Infof("Got CompleteRegistration for SmMirrorSessionInterface %v", smm)
	smm.sm.SetMirrorSessionReactor(smgrMirrorInterface)
	//Send collectors selectively
	smm.sm.EnableSelectivePushForKind("Collector")
}

func initSmMirrorInterface() {
	mgr := MustGetStatemgr()
	smgrMirrorInterface = &SmMirrorSessionInterface{
		sm:             mgr,
		collectors:     make(map[string]*mirrorCollector),
		mirrorSessions: make(map[string]*MirrorSessionState),
	}
	mgr.Register("statemgrmirrorSession", smgrMirrorInterface)
}

func init() {
	initSmMirrorInterface()
}

func collectorKey(tenant, vrf, dest string) string {
	return tenant + "-" + vrf + "-" + dest
}

func (smm *SmMirrorSessionInterface) findCollector(tenant, vrf, dest string) (*mirrorCollector, error) {

	smm.Lock()
	defer smm.Unlock()

	key := collectorKey(tenant, vrf, dest)
	col, ok := smm.collectors[key]
	if ok {
		return col, nil
	}

	return nil, errors.New("Collector not found")
}

func (smm *SmMirrorSessionInterface) addCollector(tenant, vrf, dest string) (*mirrorCollector, error) {

	smm.Lock()
	defer smm.Unlock()

	key := collectorKey(tenant, vrf, dest)
	col, ok := smm.collectors[key]
	if ok {
		col.refCount++
		return col, nil
	}

	collector := &netproto.Collector{
		TypeMeta: api.TypeMeta{Kind: "Collector"},
		ObjectMeta: api.ObjectMeta{
			Tenant:       tenant,
			Namespace:    vrf,
			Name:         key,
			GenerationID: "1",
		},
		Spec: netproto.CollectorSpec{
			Destination: dest,
			VrfName:     vrf,
		},
	}

	col = &mirrorCollector{
		obj:      collector,
		refCount: 1,
	}
	smm.collectors[key] = col

	refs := make(map[string]apiintf.ReferenceObj)
	mcolPush, err := smm.sm.mbus.AddPushObject(col.obj.GetKey(), col.obj, refs, nil)
	if err != nil {
		log.Errorf("Error adding collector to push DB %v", err)
	}
	col.pushObj = mcolPush

	return col, nil
}

func (smm *SmMirrorSessionInterface) deleteCollector(tenant, vrf, dest string) (*mirrorCollector, error) {

	smm.Lock()
	defer smm.Unlock()

	key := collectorKey(tenant, vrf, dest)
	col, ok := smm.collectors[key]
	if ok {
		if col.refCount > 0 {
			col.refCount--
		}
		if col.refCount == 0 {
			delete(smm.collectors, key)
		}
		return col, nil
	}

	return nil, fmt.Errorf("Error finding the collector %v", key)
}

// NewMirroSessionState creates new app state object
func NewMirroSessionState(mirror *ctkit.MirrorSession, stateMgr *SmMirrorSessionInterface) (*MirrorSessionState, error) {
	ms := &MirrorSessionState{
		MirrorSession: mirror,
		stateMgr:      stateMgr,
	}
	mirror.HandlerCtx = ms

	return ms, nil
}

type mirrorSelectorCollectors struct {
	selector      *labels.Selector
	txCollectors  []*mirrorCollector
	rxCollectors  []*mirrorCollector
	mirrorSession string
}

//OnMirrorSessionCreate mirror session create handle
func (smm *SmMirrorSessionInterface) OnMirrorSessionCreate(obj *ctkit.MirrorSession) error {
	log.Infof("Got mirror session create")
	// see if we already have the session
	ms, err := smm.FindMirrorSession(obj.Tenant, obj.Name)
	if err == nil {
		ms.MirrorSession = obj
		return nil
	}

	log.Infof("Creating mirror: %+v", obj)

	// create new app object
	ms, err = NewMirroSessionState(obj, smm)
	if err != nil {
		log.Errorf("Error creating mirror %+v. Err: %v", obj, err)
		return err
	}

	//Process only if no match rules
	if len(ms.MirrorSession.Spec.GetMatchRules()) != 0 {
		log.Infof("Skipping processing of mirror session %v  as flow match rules present", ms.MirrorSession.Name)
		return nil
	}

	//Process only if no match rules
	if ms.MirrorSession.Spec.InterfaceSelector == nil {
		log.Infof("Skipping processing of mirror session %v  as interface selector not assigned yet", ms.MirrorSession.Name)
		return nil
	}

	collectors := ms.MirrorSession.Spec.GetCollectors()
	if len(collectors) == 0 {
		return nil
	}

	mCollectors := []*mirrorCollector{}
	for _, collector := range collectors {
		mcol, err := smgrMirrorInterface.findCollector(ms.MirrorSession.Tenant, ms.MirrorSession.Namespace, collector.ExportCfg.Destination)
		if err != nil {
			mcol, err = smgrMirrorInterface.addCollector(ms.MirrorSession.Tenant, ms.MirrorSession.Namespace,
				collector.ExportCfg.Destination)
			if err != nil {
				log.Errorf("Error Adding collector %+v. Err: %v", collector.ExportCfg.Destination, err)
				return err
			}
		} else {
			mcol.refCount++
		}
		mCollectors = append(mCollectors, mcol)
	}

	smgrMirrorInterface.addMirrorSession(ms)

	if ms.MirrorSession.Spec.InterfaceSelector != nil {
		//Now evaluate the interfaces
		selCollector := &mirrorSelectorCollectors{
			rxCollectors:  mCollectors,
			txCollectors:  mCollectors,
			selector:      ms.MirrorSession.Spec.InterfaceSelector,
			mirrorSession: ms.MirrorSession.Name,
		}

		err = smgrNetworkInterface.UpdateCollectorsMatchingSelector(nil, selCollector)
		if err != nil {
			log.Infof("Error updating collector state %v", err)
		}
	}

	return nil
}

//AddMirrorSession add mirror session
func (smm *SmMirrorSessionInterface) addMirrorSession(ms *MirrorSessionState) error {
	smm.Lock()
	defer smm.Unlock()
	smm.mirrorSessions[ms.MirrorSession.GetKey()] = ms
	return nil
}

//deleteMirrorSession delete mirror session
func (smm *SmMirrorSessionInterface) deleteMirrorSession(ms *MirrorSessionState) error {
	smm.Lock()
	defer smm.Unlock()

	delete(smm.mirrorSessions, ms.MirrorSession.GetKey())
	return nil
}

//getMirrorSession get mirror session
func (smm *SmMirrorSessionInterface) getMirrorSession(name string) (*MirrorSessionState, error) {
	smm.Lock()
	defer smm.Unlock()

	ms, ok := smm.mirrorSessions[name]

	if ok {
		return nil, fmt.Errorf("Did not find mirror session %v", name)
	}

	return ms, nil
}

func (smm *SmMirrorSessionInterface) getAllMirrorSessionCollectors() []*mirrorSelectorCollectors {

	smm.Lock()
	defer smm.Unlock()

	mcols := []*mirrorSelectorCollectors{}
	for _, ms := range smm.mirrorSessions {
		currentCollectors := ms.MirrorSession.Spec.GetCollectors()
		curCollectors := []*mirrorCollector{}
		for _, curCol := range currentCollectors {

			key := collectorKey(ms.MirrorSession.Tenant,
				ms.MirrorSession.Namespace, curCol.ExportCfg.Destination)
			collector, ok := smm.collectors[key]
			if ok {
				curCollectors = append(curCollectors, collector)
			}
		}
		mcol := &mirrorSelectorCollectors{
			mirrorSession: ms.MirrorSession.Name,
			selector:      ms.MirrorSession.Spec.InterfaceSelector,
			txCollectors:  curCollectors,
			rxCollectors:  curCollectors,
		}
		mcols = append(mcols, mcol)

	}

	return mcols
}

//OnMirrorSessionUpdate mirror session update handle
func (smm *SmMirrorSessionInterface) OnMirrorSessionUpdate(mirror *ctkit.MirrorSession, nmirror *monitoring.MirrorSession) error {
	log.Infof("Got mirror update for %#v", nmirror.ObjectMeta)

	// see if anything changed
	_, ok := ref.ObjDiff(mirror.Spec, nmirror.Spec)
	if (mirror.GenerationID == nmirror.GenerationID) && !ok {
		//mirror.ObjectMeta = nmirror.ObjectMeta
	}

	ms, err := MirrorSessionStateFromObj(mirror)
	if err != nil {
		log.Errorf("Error finding mirror object for delete %v", err)
		return err
	}

	if len(nmirror.Spec.GetMatchRules()) != 0 && ms.MirrorSession.Spec.InterfaceSelector == nil {
		log.Infof("Skipping update processing of mirror session %v  as flow match rules present", ms.MirrorSession.Name)
		return nil
	}

	currentCollectors := ms.MirrorSession.Spec.GetCollectors()
	newCollectors := nmirror.Spec.GetCollectors()

	type colRef struct {
		col monitoring.MirrorCollector
		cnt int
	}
	newColMap := make(map[string]colRef)

	addCollectors := []*mirrorCollector{}
	delCollectors := []*mirrorCollector{}
	curCollectors := []*mirrorCollector{}
	for _, curCol := range currentCollectors {
		key := collectorKey(mirror.Tenant, mirror.Namespace, curCol.ExportCfg.Destination)
		//1 indiates object is present in update tpp
		collector, err := smgrMirrorInterface.findCollector(mirror.Tenant, mirror.Namespace,
			curCol.ExportCfg.Destination)
		if err == nil {
			curCollectors = append(curCollectors, collector)
			newColMap[key] = colRef{cnt: 1, col: curCol}
		}
	}

	for _, newCol := range newCollectors {
		key := collectorKey(nmirror.Tenant, nmirror.Namespace, newCol.ExportCfg.Destination)
		if cref, ok := newColMap[key]; !ok {
			//2 indiates object is new in update
			newColMap[key] = colRef{cnt: 2, col: newCol}
		} else {
			//0 indiates same is present in update too
			cref.cnt = 0
			newColMap[key] = cref
		}
	}

	for _, cref := range newColMap {
		if cref.cnt == 1 {
			log.Infof("Deleting  collector %v", cref.col.ExportCfg.Destination)
			mcol, err := smgrMirrorInterface.deleteCollector(mirror.Tenant, mirror.Namespace, cref.col.ExportCfg.Destination)
			if err != nil {
				//Collector may not be added before as part of create.
				log.Infof("Error deleting collector %v", err)
				return fmt.Errorf("Error adding collector %v", err)
			}
			delCollectors = append(delCollectors, mcol)
		} else if cref.cnt == 2 {
			mcol, err := smgrMirrorInterface.addCollector(nmirror.Tenant, nmirror.Namespace, cref.col.ExportCfg.Destination)
			if err != nil {
				log.Infof("Error adding collector %v", err)
				return fmt.Errorf("Error adding collector %v", err)
			}
			addCollectors = append(addCollectors, mcol)
		}
	}

	//Now evaluate the interfaces
	var oldSelCollector *mirrorSelectorCollectors
	var newSelCollector *mirrorSelectorCollectors

	selectorChanged := true
	if (mirror.Spec.InterfaceSelector == nil && nmirror.Spec.InterfaceSelector == nil) ||
		(mirror.Spec.InterfaceSelector != nil && nmirror.Spec.InterfaceSelector != nil &&
			mirror.Spec.InterfaceSelector.Print() == nmirror.Spec.InterfaceSelector.Print()) {
		selectorChanged = false
	}

	//Now evaluate the interfaces
	if len(nmirror.Spec.GetMatchRules()) != 0 ||
		(len(addCollectors) == 0 && len(delCollectors) == 0 && selectorChanged) {
		//case 1:
		//Interface selector
		//Collectors are same but label selectors have changed
		// Remove current collectors and change label
		if mirror.Spec.InterfaceSelector != nil {
			oldSelCollector = &mirrorSelectorCollectors{
				rxCollectors:  curCollectors,
				txCollectors:  curCollectors,
				selector:      mirror.Spec.InterfaceSelector,
				mirrorSession: mirror.Name,
			}
		}

		if nmirror.Spec.InterfaceSelector != nil {
			//Now evaluate the interfaces
			newSelCollector = &mirrorSelectorCollectors{
				rxCollectors:  curCollectors,
				txCollectors:  curCollectors,
				selector:      nmirror.Spec.InterfaceSelector,
				mirrorSession: mirror.Name,
			}
		}

	} else if len(addCollectors) != 0 || len(delCollectors) != 0 && !selectorChanged {
		//case 2: Collectors are different but label is the same
		// Delete old collectors and new collectors to existing label
		if mirror.Spec.InterfaceSelector != nil {
			oldSelCollector = &mirrorSelectorCollectors{
				rxCollectors:  delCollectors,
				txCollectors:  delCollectors,
				selector:      mirror.Spec.InterfaceSelector,
				mirrorSession: mirror.Name,
			}
		}

		if nmirror.Spec.InterfaceSelector != nil {
			//Now evaluate the interfaces
			newSelCollector = &mirrorSelectorCollectors{
				rxCollectors:  addCollectors,
				txCollectors:  addCollectors,
				selector:      nmirror.Spec.InterfaceSelector,
				mirrorSession: mirror.Name,
			}
		}
	} else {
		//Now evaluate the interfaces
		//case 3: Both Collectors and label has changed.
		// For new label, send both new and old col collecotrs
		// for old lablel, send the old collectors only.
		if mirror.Spec.InterfaceSelector != nil {
			oldSelCollector = &mirrorSelectorCollectors{
				rxCollectors:  curCollectors,
				txCollectors:  curCollectors,
				selector:      mirror.Spec.InterfaceSelector,
				mirrorSession: mirror.Name,
			}
		}

		if nmirror.Spec.InterfaceSelector != nil {
			addCollectors = append(addCollectors, curCollectors...)
			newSelCollector = &mirrorSelectorCollectors{
				rxCollectors:  addCollectors,
				txCollectors:  addCollectors,
				selector:      nmirror.Spec.InterfaceSelector,
				mirrorSession: mirror.Name,
			}
		}
	}

	err = smgrNetworkInterface.UpdateCollectorsMatchingSelector(oldSelCollector, newSelCollector)
	if err != nil {
		log.Infof("Error updating collector state %v", err)
	}

	//Collectors hav no references, delete it.
	for _, col := range delCollectors {
		if col.refCount == 0 {
			col.pushObj.RemoveAllObjReceivers()
			refs := make(map[string]apiintf.ReferenceObj)
			col.pushObj.DeleteObjectWithReferences(col.obj.GetKey(), col.obj, refs)
		}
	}

	//Collectors hav no references, delete it.
	for _, col := range curCollectors {
		if col.refCount == 0 {
			col.pushObj.RemoveAllObjReceivers()
			refs := make(map[string]apiintf.ReferenceObj)
			col.pushObj.DeleteObjectWithReferences(col.obj.GetKey(), col.obj, refs)
		}
	}

	ms.MirrorSession.Spec.Collectors = nmirror.Spec.Collectors
	ms.MirrorSession.Spec.InterfaceSelector = nmirror.Spec.InterfaceSelector
	log.Infof("Updated mirror session %v", ms.MirrorSession.Spec.InterfaceSelector.String())
	return nil

}

//OnMirrorSessionDelete mirror session delete handle
func (smm *SmMirrorSessionInterface) OnMirrorSessionDelete(obj *ctkit.MirrorSession) error {

	log.Infof("Got mirror delete for %#v", obj.ObjectMeta)
	ms, err := MirrorSessionStateFromObj(obj)
	if err != nil {
		log.Errorf("Error finding mirror object for delete %v", err)
		return err
	}

	collectors := ms.MirrorSession.Spec.GetCollectors()
	delCollectors := []*mirrorCollector{}
	for _, collector := range collectors {
		mcol, err := smgrMirrorInterface.findCollector(ms.MirrorSession.Tenant, ms.MirrorSession.Namespace, collector.ExportCfg.Destination)
		if err == nil {
			smgrMirrorInterface.deleteCollector(ms.MirrorSession.Tenant, ms.MirrorSession.Namespace,
				collector.ExportCfg.Destination)
			delCollectors = append(delCollectors, mcol)
		}
	}

	smgrMirrorInterface.deleteMirrorSession(ms)

	if ms.MirrorSession.Spec.InterfaceSelector != nil {
		//Now evaluate the interfaces
		oldSelCollector := &mirrorSelectorCollectors{
			rxCollectors:  delCollectors,
			txCollectors:  delCollectors,
			selector:      ms.MirrorSession.Spec.InterfaceSelector,
			mirrorSession: ms.MirrorSession.Name,
		}

		err = smgrNetworkInterface.UpdateCollectorsMatchingSelector(oldSelCollector, nil)
		if err != nil {
			log.Infof("Error updating collector state %v", err)
		}
	}

	//Collectors hav no references, delete it.
	for _, col := range delCollectors {
		if col.refCount == 0 {
			log.Infof("Deleting  collector %v", col.obj.Spec.Destination)
			col.pushObj.RemoveAllObjReceivers()
			refs := make(map[string]apiintf.ReferenceObj)
			col.pushObj.DeleteObjectWithReferences(col.obj.GetKey(), col.obj, refs)
		}
	}

	return nil

}

//GetMirrorSessionWatchOptions Get watch options
func (smm *SmMirrorSessionInterface) GetMirrorSessionWatchOptions() *api.ListWatchOptions {
	opts := &api.ListWatchOptions{}
	opts.FieldChangeSelector = []string{"ObjectMeta", "Spec"}
	return opts
}
