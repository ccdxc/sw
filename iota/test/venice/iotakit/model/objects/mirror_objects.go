package objects

import (
	"strings"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/iota/test/venice/iotakit/cfg/objClient"
	"github.com/pensando/sw/iota/test/venice/iotakit/testbed"
	"github.com/pensando/sw/venice/utils/log"
)

// MirrorSession represents mirrorsession
type MirrorSession struct {
	VeniceMirrorSess *monitoring.MirrorSession
}

// MirrorSessionCollection is list of sessions
type MirrorSessionCollection struct {
	CollectionCommon
	err      error
	Sessions []*MirrorSession
}

func NewMirrorSession(name string, client objClient.ObjClient, testbed *testbed.TestBed) *MirrorSessionCollection {
	return &MirrorSessionCollection{
		Sessions: []*MirrorSession{
			{
				VeniceMirrorSess: &monitoring.MirrorSession{
					TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
					ObjectMeta: api.ObjectMeta{
						Tenant:    "default",
						Namespace: "default",
						Name:      name,
					},
					Spec: monitoring.MirrorSessionSpec{
						PacketSize: 128,
					},
				},
			},
		},
		CollectionCommon: CollectionCommon{Testbed: testbed, Client: client},
	}
}

// AddCollector adds a collector to the mirrorsession
func (msc *MirrorSessionCollection) AddCollector(wc *WorkloadCollection, transport string, wlnum int) *MirrorSessionCollection {
	if msc.HasError() {
		return msc
	}
	if wc.err != nil {
		return &MirrorSessionCollection{err: wc.err}
	}
	collector := monitoring.MirrorCollector{
		Type: monitoring.PacketCollectorType_ERSPAN.String(),
		ExportCfg: &monitoring.MirrorExportConfig{
			Destination: strings.Split(wc.Workloads[wlnum].iotaWorkload.IpPrefix, "/")[0],
		},
	}

	for _, sess := range msc.Sessions {
		sess.VeniceMirrorSess.Spec.Collectors = append(sess.VeniceMirrorSess.Spec.Collectors, collector)
	}

	return msc
}

// AddVeniceCollector adds venice collector
func (msc *MirrorSessionCollection) AddVeniceCollector(vnc *VeniceNodeCollection, transport string, wlnum int) *MirrorSessionCollection {
	if msc.HasError() {
		return msc
	}
	collector := monitoring.MirrorCollector{
		Type: monitoring.PacketCollectorType_ERSPAN.String(),
		ExportCfg: &monitoring.MirrorExportConfig{
			Destination: strings.Split(vnc.Nodes[0].iotaNode.IpAddress, "/")[0],
		},
	}

	for _, sess := range msc.Sessions {
		sess.VeniceMirrorSess.Spec.Collectors = append(sess.VeniceMirrorSess.Spec.Collectors, collector)
	}

	return msc
}

// ClearCollectors adds a collector to the mirrorsession
func (msc *MirrorSessionCollection) ClearCollectors() *MirrorSessionCollection {
	if msc.HasError() {
		return msc
	}

	for _, sess := range msc.Sessions {
		sess.VeniceMirrorSess.Spec.Collectors = nil
	}

	return msc
}

// Commit writes the mirrorsession to venice
func (msc *MirrorSessionCollection) Commit() error {
	if msc.err != nil {
		return msc.err
	}
	for _, sess := range msc.Sessions {
		err := msc.Client.CreateMirrorSession(sess.VeniceMirrorSess)
		log.Infof("Create mirror session failed %v", err)
		if err != nil {
			// try updating it
			err = msc.Client.UpdateMirrorSession(sess.VeniceMirrorSess)
			if err != nil {
				msc.err = err
				log.Infof("Updating mirror session failed %v", err)
				return err
			}
		}

		log.Debugf("Created session: %#v", sess.VeniceMirrorSess)

	}

	return nil
}

// AddRule adds a rule to the mirrorsession
func (msc *MirrorSessionCollection) AddRule(fromIP, toIP, port string) *MirrorSessionCollection {
	if msc.err != nil {
		return msc
	}

	// build the rule
	rule := monitoring.MatchRule{
		Src:         &monitoring.MatchSelector{IPAddresses: []string{fromIP}},
		Dst:         &monitoring.MatchSelector{IPAddresses: []string{toIP}},
		AppProtoSel: &monitoring.AppProtoSelector{ProtoPorts: []string{port}},
	}

	for _, sess := range msc.Sessions {
		sess.VeniceMirrorSess.Spec.MatchRules = append(sess.VeniceMirrorSess.Spec.MatchRules, rule)
	}

	return msc
}

// AddRulesForWorkloadPairs adds rule for each workload pair into the sessions
func (msc *MirrorSessionCollection) AddRulesForWorkloadPairs(wpc *WorkloadPairCollection, port string) *MirrorSessionCollection {
	if msc.err != nil {
		return msc
	}
	if wpc.err != nil {
		return &MirrorSessionCollection{err: wpc.err}
	}

	// walk each workload pair
	for _, wpair := range wpc.Pairs {
		fromIP := wpair.Second.GetIP()
		toIP := wpair.First.GetIP()
		nmsc := msc.AddRule(fromIP, toIP, port)
		if nmsc.err != nil {
			return nmsc
		}
	}

	return msc
}

// Delete deletes all sessions in the collection
func (msc *MirrorSessionCollection) Delete() error {
	if msc.err != nil {
		return msc.err
	}

	// walk all sessions and delete them
	for _, sess := range msc.Sessions {
		err := msc.Client.DeleteMirrorSession(sess.VeniceMirrorSess)
		if err != nil {
			return err
		}
	}

	return nil
}
