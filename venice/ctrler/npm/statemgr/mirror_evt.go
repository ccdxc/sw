// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"errors"
	"fmt"
	"strings"
	"time"

	"github.com/gogo/protobuf/types"

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

type mirrorTimerType int

const (
	mirrorSchTimer mirrorTimerType = iota
	mirrorExpTimer
)

const (
	watcherQueueLen = 16
	// MaxMirrorSessions is the maximum number of mirror sessions allowed
	MaxMirrorSessions = 8
)

// MirrorTimerEvent - schedule or expiry
type MirrorTimerEvent struct {
	Type  mirrorTimerType
	genID string
	*MirrorSessionState
}

type mirrorHandler struct {
	genID string
	*MirrorSessionState
}

func (mh *mirrorHandler) handleSchTimer() {
	log.Infof("Sch Timer Done... %v", mh.MirrorSession.Name)
	mh.MirrorSessionState.stateMgr.mirrorTimerWatcher <- MirrorTimerEvent{
		Type:               mirrorSchTimer,
		genID:              mh.genID,
		MirrorSessionState: mh.MirrorSessionState}
	log.Infof("Sch Timer Firing Done... %v", mh.MirrorSession.Name)
}

/* XXX Uncomment after NAPLES adds support for mirror expiry
func (mss *MirrorSessionState) getExpDuration() time.Duration {
	// format conversion and max duration (2h) is checked by common Venice parameter checker hook
	expDuration, _ := time.ParseDuration(mss.MirrorSession.Spec.StopConditions.ExpiryDuration)
	return expDuration
} */

func (mss *MirrorSessionState) runMsExpTimer() bool {
	return true
	/* XXX Uncomment after NAPLES adds support for mirror expiry
	expDuration := mss.getExpDuration()
	expTime := mss.schTime.Add(expDuration)
	if expTime.After(time.Now()) {
		log.Infof("Expiry Time set to %v for %v\n", expTime, mss.Name)
		mss.expTimer = time.AfterFunc(time.Until(expTime), mss.handleExpTimer)
		return true
	}
	log.Infof("Expiry Time expired in the past %v for %v\n", expTime, mss.Name)
	return false */
}

func (mss *MirrorSessionState) handleExpTimer() {
	log.Infof("Exp Timer Done...%v", mss.MirrorSession.Name)
	mss.stateMgr.mirrorTimerWatcher <- MirrorTimerEvent{
		Type:               mirrorExpTimer,
		MirrorSessionState: mss}
}

func buildDSCMirrorSession(mss *MirrorSessionState) *netproto.MirrorSession {
	ms := mss.MirrorSession
	tms := netproto.MirrorSession{
		TypeMeta:   ms.TypeMeta,
		ObjectMeta: ms.ObjectMeta,
	}
	tSpec := &tms.Spec
	//tSpec.CaptureAt = netproto.MirrorSrcDst_SRC_DST
	tSpec.MirrorDirection = netproto.MirrorDir_BOTH
	//tSpec.Enable = (mss.State == monitoring.MirrorSessionState_ACTIVE)
	tSpec.PacketSize = ms.Spec.PacketSize
	//tSpec.PacketFilters = ms.Spec.PacketFilters

	for _, c := range ms.Spec.Collectors {
		var export monitoring.MirrorExportConfig
		if c.ExportCfg != nil {
			export = *c.ExportCfg
		}
		tc := netproto.MirrorCollector{
			//Type:      c.Type,
			ExportCfg: netproto.MirrorExportConfig{Destination: export.Destination,
				Gateway: export.Gateway,
			},
		}
		tSpec.Collectors = append(tSpec.Collectors, tc)
	}
	for _, mr := range ms.Spec.MatchRules {
		tmr := netproto.MatchRule{}
		if mr.Src == nil && mr.Dst == nil {
			log.Debugf("Ignore MatchRule with Src = * and Dst = *")
			continue
		}
		if mr.Src != nil {
			tmr.Src = &netproto.MatchSelector{}
			tmr.Src.Addresses = mr.Src.IPAddresses
			//tmr.Src.MACAddresses = mr.Src.MACAddresses
		}
		if mr.Dst != nil {
			tmr.Dst = &netproto.MatchSelector{}
			tmr.Dst.Addresses = mr.Dst.IPAddresses
			if mr.AppProtoSel != nil {
				for _, pp := range mr.AppProtoSel.ProtoPorts {
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
					tmr.Dst.ProtoPorts = append(tmr.Dst.ProtoPorts, &protoPort)
				}
			}
			//tmr.Dst.MACAddresses = mr.Dst.MACAddresses
		}
		//if mr.AppProtoSel != nil {
		//	tmr.AppProtoSel = &netproto.AppProtoSelector{}
		//	for _, port := range mr.AppProtoSel.ProtoPorts {
		//		tmr.AppProtoSel.Ports = append(tmr.AppProtoSel.Ports, port)
		//	}
		//	for _, app := range mr.AppProtoSel.Apps {
		//		tmr.AppProtoSel.Apps = append(tmr.AppProtoSel.Apps, app)
		//	}
		//}
		tSpec.MatchRules = append(tSpec.MatchRules, tmr)
	}
	return &tms
}

// MirrorSessionCountAllocate : Increment the active session counter if max is not reached
func (smm *SmMirrorSessionInterface) MirrorSessionCountAllocate() bool {
	smm.Mutex.Lock()
	defer smm.Mutex.Unlock()

	if smm.numMirrorSessions < MaxMirrorSessions {
		smm.numMirrorSessions++
		log.Infof("Allocated mirror session: count %v", smm.numMirrorSessions)
		return true
	}
	log.Infof("Max mirror session count reached")
	return false
}

// MirrorSessionCountFree : decrement the active session counter
func (smm *SmMirrorSessionInterface) MirrorSessionCountFree() {
	smm.Mutex.Lock()
	defer smm.Mutex.Unlock()
	if smm.numMirrorSessions > 0 {
		smm.numMirrorSessions--
		log.Infof("Active mirror session: count %v", smm.numMirrorSessions)
	} else {
		panic("Bug - mirror session free below 0")
	}
}

func isFlowBasedMirroring(ms *monitoring.MirrorSession) bool {
	return len(ms.Spec.GetMatchRules()) != 0
}

func (mss *MirrorSessionState) isFlowBasedMirroring() bool {

	return isFlowBasedMirroring(&mss.MirrorSession.MirrorSession)
}

func (mss *MirrorSessionState) setMirrorSessionRunning(ms *monitoring.MirrorSession) {
	if isFlowBasedMirroring(ms) && !mss.stateMgr.MirrorSessionCountAllocate() {
		mss.State = monitoring.MirrorSessionState_ERR_NO_MIRROR_SESSION
		mss.MirrorSession.Status.ScheduleState = monitoring.MirrorSessionState_ERR_NO_MIRROR_SESSION.String()
		log.Infof("Mirror session  %v not scheduled", mss.MirrorSession.Name)
	} else {
		mss.State = monitoring.MirrorSessionState_ACTIVE
		mss.MirrorSession.Status.ScheduleState = monitoring.MirrorSessionState_ACTIVE.String()
		ts, _ := types.TimestampProto(time.Now())
		mss.MirrorSession.Status.StartedAt.Timestamp = *ts
		// create PCAP file URL for sessions with venice collector
		_t, _ := mss.MirrorSession.Status.StartedAt.Time()
		log.Infof("Mirror session StartedAt %v", _t)
	}
}

func (mss *MirrorSessionState) programMirrorSession(ms *monitoring.MirrorSession) {
	// common function called for both create and update operations
	// New or previously SCHEDULED session
	if ms.Spec.StartConditions.ScheduleTime != nil {
		schTime, _ := ms.Spec.StartConditions.ScheduleTime.Time()
		if schTime.After(mss.schTime) {
			mss.schTime = schTime
			// start the timer routine only if time is in future
			log.Infof("Schedule time is %v", mss.schTime)
			mh := mirrorHandler{MirrorSessionState: mss, genID: mss.MirrorSession.ObjectMeta.GenerationID}
			mss.schTimer = time.AfterFunc(time.Until(mss.schTime), mh.handleSchTimer)
			mss.State = monitoring.MirrorSessionState_SCHEDULED
			mss.MirrorSession.Status.ScheduleState = monitoring.MirrorSessionState_SCHEDULED.String()
		} else {
			// schedule time in the past, run it right-away
			log.Warnf("Schedule time %v already passed, starting the mirror-session now - %v\n", schTime, mss.MirrorSession.Name)
			mss.setMirrorSessionRunning(ms)
		}
	} else {
		mss.schTime = time.Now()
		mss.setMirrorSessionRunning(ms)
		log.Infof("Mirror Session  %v, State %v", mss.MirrorSession.Name, mss.MirrorSession.Status.ScheduleState)
	}
	if !mss.runMsExpTimer() {
		if mss.State == monitoring.MirrorSessionState_ACTIVE {
			mss.stateMgr.MirrorSessionCountFree()
		}
		mss.State = monitoring.MirrorSessionState_STOPPED
		mss.MirrorSession.Status.ScheduleState = monitoring.MirrorSessionState_STOPPED.String()
		mss.expTimer = nil
	}
}

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
	schTime       time.Time
	schTimer      *time.Timer
	expTimer      *time.Timer
	// Local information
	State   monitoring.MirrorSessionState
	deleted bool
}

// runMirrorSessionWatcher watches on a channel for changes from api server and internal events
func (smm *SmMirrorSessionInterface) runMirrorSessionWatcher() {
	log.Infof("Mirror Session Watcher running")

	// loop till channel is closed
	for {
		select {
		case evt, ok := <-smm.mirrorTimerWatcher:
			if !ok {
				// Since the channel is within the same controller process... no need to restart it
				log.Infof("Mirror Session Watcher exited")
				return
			}
			log.Infof("Watcher: Got Mirror session Timer event(%v) on %v, ver %v", evt.Type, evt.MirrorSessionState.MirrorSession.Name,
				evt.MirrorSessionState.MirrorSession.ResourceVersion)
			smm.handleMirrorSessionTimerEvent(evt.Type, evt.genID, evt.MirrorSessionState)
		}
	}
}

func (smm *SmMirrorSessionInterface) handleMirrorSessionTimerEvent(et mirrorTimerType, genID string, mss *MirrorSessionState) {
	mss.MirrorSession.Lock()
	defer mss.MirrorSession.Unlock()
	if mss.deleted || genID != mss.MirrorSession.ObjectMeta.GenerationID {
		return
	}
	switch et {
	case mirrorSchTimer:
		mss.programMirrorSession(&mss.MirrorSession.MirrorSession)

		if mss.State == monitoring.MirrorSessionState_ERR_NO_MIRROR_SESSION ||
			mss.State == monitoring.MirrorSessionState_SCHEDULED {
			mss.MirrorSession.Write()
		}
		smm.addMirror(mss)
	case mirrorExpTimer:
		//smm.stopMirrorSession(mss)
	default:
	}
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
	featureMgrBase
	sm                 *Statemgr
	collectors         map[string]*mirrorCollector
	mirrorSessions     map[string]*MirrorSessionState
	mirrorTimerWatcher chan MirrorTimerEvent // mirror session Timer watcher
	numMirrorSessions  int                   // total mirror sessions created
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
	go smgrMirrorInterface.runMirrorSessionWatcher()
}

func initSmMirrorInterface() {
	mgr := MustGetStatemgr()
	smgrMirrorInterface = &SmMirrorSessionInterface{
		sm:                 mgr,
		collectors:         make(map[string]*mirrorCollector),
		mirrorSessions:     make(map[string]*MirrorSessionState),
		mirrorTimerWatcher: make(chan MirrorTimerEvent, watcherQueueLen),
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

	key := collectorKey(tenant, vrf, dest)
	col, ok := smm.collectors[key]
	if ok {
		return col, nil
	}

	return nil, errors.New("Collector not found")
}

func (smm *SmMirrorSessionInterface) addCollector(tenant, vrf, dest, gateway string) (*mirrorCollector, error) {

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
			Gateway:     gateway,
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
	selectors     []*labels.Selector
	txCollectors  []*mirrorCollector
	rxCollectors  []*mirrorCollector
	mirrorSession string
}

func (smm *SmMirrorSessionInterface) pushAddMirrorSession(mss *MirrorSessionState) error {
	return smm.sm.mbus.AddObjectWithReferences(mss.MirrorSession.MakeKey("monitoring"),
		buildDSCMirrorSession(mss), references(mss.MirrorSession))
}

func (smm *SmMirrorSessionInterface) pushUpdateMirrorSession(mss *MirrorSessionState) error {
	return smm.sm.mbus.UpdateObjectWithReferences(mss.MirrorSession.MakeKey("monitoring"),
		buildDSCMirrorSession(mss), references(mss.MirrorSession))
}

func (smm *SmMirrorSessionInterface) pushDeleteMirrorSession(mss *MirrorSessionState) error {
	return smm.sm.mbus.DeleteObjectWithReferences(mss.MirrorSession.MakeKey("monitoring"),
		buildDSCMirrorSession(mss), references(mss.MirrorSession))
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

	ms, err = NewMirroSessionState(obj, smm)
	if err != nil {
		log.Errorf("Error creating mirror %+v. Err: %v", obj, err)
		return err
	}

	if ms.MirrorSession.Status.ScheduleState != "" && (ms.MirrorSession.Status.ScheduleState != monitoring.MirrorSessionState_NONE.String()) {
		ms.State = monitoring.MirrorSessionState(monitoring.MirrorSessionState_value[ms.MirrorSession.Status.ScheduleState])
		ms.schTime, _ = ms.MirrorSession.Status.StartedAt.Time()
	} else {
		ms.State = monitoring.MirrorSessionState_NONE
		ms.schTime = time.Now()
	}

	ms.programMirrorSession(&ms.MirrorSession.MirrorSession)

	if ms.State == monitoring.MirrorSessionState_ERR_NO_MIRROR_SESSION ||
		ms.State == monitoring.MirrorSessionState_SCHEDULED {
		ms.MirrorSession.Write()
		return nil
	}

	return smm.addMirror(ms)

}

//OnMirrorSessionCreate mirror session create handle
func (smm *SmMirrorSessionInterface) addMirror(ms *MirrorSessionState) error {

	defer ms.MirrorSession.Write()

	//Process only if no match rules
	if ms.isFlowBasedMirroring() {
		return smm.addFlowMirror(ms)
	}

	return smm.addInterfaceMirror(ms)
}

//AddMirrorSession add mirror session
func (smm *SmMirrorSessionInterface) addMirrorSession(ms *MirrorSessionState) error {
	smm.mirrorSessions[ms.MirrorSession.GetKey()] = ms
	return nil
}

//deleteMirrorSession delete mirror session
func (smm *SmMirrorSessionInterface) deleteMirrorSession(ms *MirrorSessionState) error {
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

func getMirrorCollectors(ms *monitoring.MirrorSession, collectors []*mirrorCollector) *mirrorSelectorCollectors {
	mcol := &mirrorSelectorCollectors{
		mirrorSession: ms.Name,
	}

	if ms.Spec.Interfaces != nil {
		mcol.selectors = ms.Spec.Interfaces.Selectors
	}
	if ms.Spec.Interfaces.Direction == monitoring.Direction_RX.String() {
		mcol.rxCollectors = collectors
	} else if ms.Spec.Interfaces.Direction == monitoring.Direction_TX.String() {
		mcol.txCollectors = collectors
	} else {
		mcol.txCollectors = collectors
		mcol.rxCollectors = collectors
	}

	return mcol
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
		mcol := getMirrorCollectors(&ms.MirrorSession.MirrorSession, curCollectors)
		mcols = append(mcols, mcol)

	}

	return mcols
}

func (smm *SmMirrorSessionInterface) addInterfaceMirror(ms *MirrorSessionState) error {

	smm.Lock()
	defer smm.Unlock()
	//Process only if no match rules
	if ms.MirrorSession.Spec.Interfaces == nil {
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
				collector.ExportCfg.Destination, collector.ExportCfg.Gateway)
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

	if ms.MirrorSession.Spec.Interfaces != nil {
		//Now evaluate the interfaces
		selCollector := getMirrorCollectors(&ms.MirrorSession.MirrorSession, mCollectors)

		err := smgrNetworkInterface.UpdateCollectorsMatchingSelector(nil, selCollector)
		if err != nil {
			log.Infof("Error updating collector state %v", err)
		}
	}

	return nil
}

func selectorsEqual(sel []*labels.Selector, otherSel []*labels.Selector) bool {
	sliceEqual := func(X, Y []string) bool {
		m := make(map[string]int)

		for _, y := range Y {
			m[y]++
		}

		for _, x := range X {
			if m[x] > 0 {
				m[x]--
				continue
			}
			//not present or execess
			return false
		}

		return len(m) == 0
	}

	firstSelectors := []string{}
	otherSelectors := []string{}
	for _, i := range sel {
		firstSelectors = append(firstSelectors, i.Print())
	}
	for _, i := range otherSel {
		otherSelectors = append(otherSelectors, i.Print())
	}

	return sliceEqual(firstSelectors, otherSelectors)
}

//OnMirrorSessionUpdate mirror session update handle
func (smm *SmMirrorSessionInterface) updateInterfaceMirror(ms *MirrorSessionState, nmirror *monitoring.MirrorSession) error {
	log.Infof("Got mirror update for %#v", nmirror.ObjectMeta)

	smm.Lock()
	defer smm.Unlock()
	// see if anything changed

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
		key := collectorKey(ms.MirrorSession.Tenant, ms.MirrorSession.Namespace, curCol.ExportCfg.Destination)
		//1 indiates object is present in update tpp
		collector, err := smgrMirrorInterface.findCollector(ms.MirrorSession.Tenant, ms.MirrorSession.Namespace,
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
			mcol, err := smgrMirrorInterface.deleteCollector(ms.MirrorSession.Tenant, ms.MirrorSession.Namespace, cref.col.ExportCfg.Destination)
			if err != nil {
				//Collector may not be added before as part of create.
				log.Infof("Error deleting collector %v", err)
				return fmt.Errorf("Error adding collector %v", err)
			}
			delCollectors = append(delCollectors, mcol)
		} else if cref.cnt == 2 {
			mcol, err := smgrMirrorInterface.addCollector(nmirror.Tenant, nmirror.Namespace,
				cref.col.ExportCfg.Destination, cref.col.ExportCfg.Gateway)
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
	if (ms.MirrorSession.Spec.Interfaces == nil && nmirror.Spec.Interfaces == nil) ||
		(ms.MirrorSession.Spec.Interfaces != nil && nmirror.Spec.Interfaces != nil &&
			selectorsEqual(ms.MirrorSession.Spec.Interfaces.Selectors, nmirror.Spec.Interfaces.Selectors)) {
		selectorChanged = false
	}

	//Now evaluate the interfaces
	if len(nmirror.Spec.GetMatchRules()) != 0 ||
		(len(addCollectors) == 0 && len(delCollectors) == 0 && selectorChanged) {
		//case 1:
		//Interface selector
		//Collectors are same but label selectors have changed
		// Remove current collectors and change label
		if ms.MirrorSession.Spec.Interfaces != nil {
			oldSelCollector = getMirrorCollectors(&ms.MirrorSession.MirrorSession, curCollectors)
		}

		if nmirror.Spec.Interfaces != nil {
			//Now evaluate the interfaces
			newSelCollector = getMirrorCollectors(nmirror, curCollectors)
		}

	} else if (len(addCollectors) != 0 || len(delCollectors) != 0) && !selectorChanged {
		//case 2: Collectors are different but label is the same
		// Delete old collectors and new collectors to existing label
		if ms.MirrorSession.Spec.Interfaces != nil {
			oldSelCollector = getMirrorCollectors(&ms.MirrorSession.MirrorSession, delCollectors)
		}

		if nmirror.Spec.Interfaces != nil {
			//Now evaluate the interfaces
			newSelCollector = getMirrorCollectors(nmirror, addCollectors)
		}
	} else {
		//Now evaluate the interfaces
		//case 3: Both Collectors and label has changed.
		// For new label, send both new and old col collecotrs
		// for old lablel, send the old collectors only.
		if ms.MirrorSession.Spec.Interfaces != nil {
			oldSelCollector = getMirrorCollectors(&ms.MirrorSession.MirrorSession, curCollectors)
		}
		newCurCollectors := []*mirrorCollector{}
		for _, col := range curCollectors {
			found := false
			for _, delCol := range delCollectors {
				if delCol.obj.Spec.Destination == col.obj.Spec.Destination &&
					delCol.obj.Tenant == col.obj.Tenant {
					found = true
				}
			}
			if !found {
				newCurCollectors = append(newCurCollectors, col)
			}
		}
		if nmirror.Spec.Interfaces != nil {
			addCollectors = append(addCollectors, newCurCollectors...)
			newSelCollector = getMirrorCollectors(nmirror, addCollectors)
		}
	}

	err := smgrNetworkInterface.UpdateCollectorsMatchingSelector(oldSelCollector, newSelCollector)
	if err != nil {
		log.Infof("Error updating collector state %v", err)
	}

	//Collectors hav no references, delete it.
	for _, col := range delCollectors {
		if col.refCount == 0 {
			col.pushObj.RemoveAllObjReceivers()
			refs := make(map[string]apiintf.ReferenceObj)
			log.Infof("Deleting collector as no referenees %v", col.obj.GetKey())
			col.pushObj.DeleteObjectWithReferences(col.obj.GetKey(), col.obj, refs)
		}
	}

	//Collectors hav no references, delete it.
	for _, col := range curCollectors {
		if col.refCount == 0 {
			col.pushObj.RemoveAllObjReceivers()
			refs := make(map[string]apiintf.ReferenceObj)
			log.Infof("Deleting collector as no referenees %v", col.obj.GetKey())
			col.pushObj.DeleteObjectWithReferences(col.obj.GetKey(), col.obj, refs)
		}
	}

	ms.MirrorSession.Spec.Collectors = nmirror.Spec.Collectors
	ms.MirrorSession.Spec.Interfaces = nmirror.Spec.Interfaces

	if ms.MirrorSession.Spec.Interfaces != nil {
		for index, selector := range ms.MirrorSession.Spec.Interfaces.Selectors {
			log.Infof("Updated mirror session  with index : %v selector %v", index, selector.String())
		}
	}
	return nil

}

//OnMirrorSessionUpdate mirror session update handle
func (smm *SmMirrorSessionInterface) OnMirrorSessionUpdate(mirror *ctkit.MirrorSession, nmirror *monitoring.MirrorSession) error {
	log.Infof("Got mirror update for %#v", nmirror.ObjectMeta)

	// see if anything changed
	_, ok := ref.ObjDiff(mirror.Spec, nmirror.Spec)
	if (mirror.GenerationID == nmirror.GenerationID) && !ok {
		//mss.MirrorSession.ObjectMeta = nmirror.ObjectMeta
		return nil
	}

	ms, err := MirrorSessionStateFromObj(mirror)
	if err != nil {
		log.Errorf("Error finding mirror object for delete %v", err)
		return err
	}

	curState := ms.State
	if ms.isFlowBasedMirroring() && ms.State == monitoring.MirrorSessionState_ACTIVE {
		smm.MirrorSessionCountFree()
		ms.State = monitoring.MirrorSessionState_NONE
	}

	// stop any running timers
	// If timer already fired GENID will prevent it from running stale routine
	ms.MirrorSession.ObjectMeta = nmirror.ObjectMeta
	if ms.schTimer != nil {
		ms.schTimer.Stop()
		ms.schTimer = nil
	}
	if ms.expTimer != nil {
		ms.expTimer.Stop()
		ms.expTimer = nil
	}

	ms.programMirrorSession(nmirror)

	defer ms.MirrorSession.Write()
	if curState == monitoring.MirrorSessionState_SCHEDULED && ms.State == monitoring.MirrorSessionState_SCHEDULED {
		//nothing has changed w.r.t to scheduling, return
		return nil
	}

	if curState == monitoring.MirrorSessionState_ACTIVE && ms.State == monitoring.MirrorSessionState_SCHEDULED {
		//have to delete the old state
		if ms.isFlowBasedMirroring() {
			err = smm.deleteFlowMirror(ms)
		} else {
			err = smm.deleteInterfaceMirror(ms)
		}
		return err
	}

	if (curState == monitoring.MirrorSessionState_ACTIVE && ms.State == monitoring.MirrorSessionState_ACTIVE) ||
		(curState == monitoring.MirrorSessionState_SCHEDULED && ms.State == monitoring.MirrorSessionState_ACTIVE) {
		//have to delete the old state

		add := curState == monitoring.MirrorSessionState_SCHEDULED

		if isFlowBasedMirroring(nmirror) && !ms.isFlowBasedMirroring() {
			//If new one is flow based mirroring,  delete interface based mirroing
			err = smm.deleteInterfaceMirror(ms)
			if err != nil {
				log.Errorf("Error deleting interface mirror %v", err)
			}
			ms.MirrorSession.Spec = nmirror.Spec
			return smm.addFlowMirror(ms)
		} else if !isFlowBasedMirroring(nmirror) && ms.isFlowBasedMirroring() {
			//If new one is interface based,  delete the flow one
			err = smm.deleteFlowMirror(ms)
			ms.MirrorSession.Spec = nmirror.Spec
			//Add Interface mirror
			return smm.addInterfaceMirror(ms)
		} else if !isFlowBasedMirroring(nmirror) && !ms.isFlowBasedMirroring() {
			//If both new and old are interface based mirroring
			if add {
				ms.MirrorSession.Spec = nmirror.Spec
				err = smgrMirrorInterface.addInterfaceMirror(ms)
			} else {
				err = smgrMirrorInterface.updateInterfaceMirror(ms, nmirror)
			}
			if err != nil {
				log.Errorf("Error updating interface mirroing %v", err)
				return err
			}
		} else {
			//If both new and old are flow  based mirroring
			ms.MirrorSession.Spec = nmirror.Spec
			if add {
				err = smgrMirrorInterface.addFlowMirror(ms)
			} else {
				err = smgrMirrorInterface.updateFlowMirror(ms)
			}
			if err != nil {
				log.Errorf("Error updating interface mirroing %v", err)
				return err
			}
		}
	}

	return nil
}

// ListMirrorSesssions lists all mirror sessions
func (sm *Statemgr) ListMirrorSesssions() ([]*MirrorSessionState, error) {
	objs := sm.ListObjects("MirrorSession")

	var mss []*MirrorSessionState
	for _, obj := range objs {
		ms, err := MirrorSessionStateFromObj(obj)
		if err != nil {
			return mss, err
		}

		mss = append(mss, ms)
	}

	return mss, nil
}

//OnMirrorSessionDelete mirror session delete handle
func (smm *SmMirrorSessionInterface) deleteFlowMirror(ms *MirrorSessionState) error {

	smgrMirrorInterface.pushDeleteMirrorSession(ms)
	smm.deleteMirrorSession(ms)
	return nil
}

func (smm *SmMirrorSessionInterface) addFlowMirror(ms *MirrorSessionState) error {

	if ms.State == monitoring.MirrorSessionState_ERR_NO_MIRROR_SESSION {
		log.Infof("Skipping add of mirror session %+v as limit has reached", ms.MirrorSession.ObjectMeta)
		return nil
	}

	smgrMirrorInterface.pushAddMirrorSession(ms)
	return smm.addMirrorSession(ms)
}

func (smm *SmMirrorSessionInterface) updateFlowMirror(ms *MirrorSessionState) error {

	if ms.State == monitoring.MirrorSessionState_ERR_NO_MIRROR_SESSION {
		log.Infof("Skipping update of mirror session %+v as limit has reached", ms.MirrorSession.ObjectMeta)
		return nil
	}
	smgrMirrorInterface.pushUpdateMirrorSession(ms)
	return smm.addMirrorSession(ms)
}

//OnMirrorSessionDelete mirror session delete handle
func (smm *SmMirrorSessionInterface) deleteInterfaceMirror(ms *MirrorSessionState) error {

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

	if ms.MirrorSession.Spec.Interfaces != nil {
		//Now evaluate the interfaces
		oldSelCollector := getMirrorCollectors(&ms.MirrorSession.MirrorSession, delCollectors)

		err := smgrNetworkInterface.UpdateCollectorsMatchingSelector(oldSelCollector, nil)
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

//OnMirrorSessionDelete mirror session delete handle
func (smm *SmMirrorSessionInterface) OnMirrorSessionDelete(obj *ctkit.MirrorSession) error {

	log.Infof("Got mirror delete for %#v", obj.ObjectMeta)
	ms, err := MirrorSessionStateFromObj(obj)
	if err != nil {
		log.Errorf("Error finding mirror object for delete %v", err)
		return err
	}

	// Stop the timers,
	// If Timers cannot be stopped, its ok. Set the state to STOPPED. Timers
	// check the state and will no-op if it is STOPPED
	//mark as deleted to that fired timer will not schedule
	ms.deleted = true
	if ms.schTimer != nil {
		log.Infof("STOP SchTimer for %v", ms.MirrorSession.Name)
		ms.schTimer.Stop()
	}
	if ms.expTimer != nil {
		log.Infof("STOP ExpTimer for %v", ms.MirrorSession.Name)
		ms.expTimer.Stop()
	}

	if ms.State == monitoring.MirrorSessionState_ACTIVE && ms.isFlowBasedMirroring() {
		smm.MirrorSessionCountFree()
	}
	ms.State = monitoring.MirrorSessionState_STOPPED

	if ms.isFlowBasedMirroring() {
		smgrMirrorInterface.pushDeleteMirrorSession(ms)
		smm.deleteMirrorSession(ms)

		// bring-up any of the mirror session in failed state
		ml, err := smm.sm.ListMirrorSesssions()
		if err == nil {
			for _, m := range ml {
				if m.MirrorSession.Status.ScheduleState == monitoring.MirrorSessionState_ERR_NO_MIRROR_SESSION.String() {
					log.Infof("retry session %v in state:%v ", m.MirrorSession.Name, m.MirrorSession.Status.ScheduleState)
					m.MirrorSession.Lock()
					m.setMirrorSessionRunning(&m.MirrorSession.MirrorSession)
					if ms.State == monitoring.MirrorSessionState_ERR_NO_MIRROR_SESSION {
						m.MirrorSession.Unlock()
						continue
					}
					m.State = monitoring.MirrorSessionState_SCHEDULED
					m.MirrorSession.Unlock()
					return smm.addMirror(m)
				}
			}
		}
	}

	return smm.deleteInterfaceMirror(ms)
}

//GetMirrorSessionWatchOptions Get watch options
func (smm *SmMirrorSessionInterface) GetMirrorSessionWatchOptions() *api.ListWatchOptions {
	opts := &api.ListWatchOptions{}
	opts.FieldChangeSelector = []string{"ObjectMeta", "Spec"}
	return opts
}
