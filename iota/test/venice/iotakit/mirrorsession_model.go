// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package iotakit

import (
	//"fmt"
	//"strconv"
	"strings"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/utils/log"
)

// MirrorSession represents mirrorsession
type MirrorSession struct {
	veniceMirrorSess *monitoring.MirrorSession
	sm               *SysModel // pointer back to the model
}

// MirrorSessionCollection is list of sessions
type MirrorSessionCollection struct {
	err      error
	sessions []*MirrorSession
}

// NewMirrorSession creates a new mirrorsession
func (sm *SysModel) NewMirrorSession(name string) *MirrorSessionCollection {
	return &MirrorSessionCollection{
		sessions: []*MirrorSession{
			{
				veniceMirrorSess: &monitoring.MirrorSession{
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
				sm: sm,
			},
		},
	}
}

// AddCollector adds a collector to the mirrorsession
func (msc *MirrorSessionCollection) AddCollector(wc *WorkloadCollection, transport string, wlnum int) *MirrorSessionCollection {
	if msc.err != nil {
		return msc
	}
	if wc.err != nil {
		return &MirrorSessionCollection{err: wc.err}
	}
	collector := monitoring.MirrorCollector{
		Type: monitoring.PacketCollectorType_ERSPAN.String(),
		ExportCfg: &monitoring.MirrorExportConfig{
			Destination: strings.Split(wc.workloads[wlnum].iotaWorkload.IpPrefix, "/")[0],
		},
	}

	for _, sess := range msc.sessions {
		sess.veniceMirrorSess.Spec.Collectors = append(sess.veniceMirrorSess.Spec.Collectors, collector)
	}

	return msc
}

// ClearCollectors adds a collector to the mirrorsession
func (msc *MirrorSessionCollection) ClearCollectors() *MirrorSessionCollection {
	if msc.err != nil {
		return msc
	}

	for _, sess := range msc.sessions {
		sess.veniceMirrorSess.Spec.Collectors = nil
	}

	return msc
}

// Commit writes the mirrorsession to venice
func (msc *MirrorSessionCollection) Commit() error {
	if msc.err != nil {
		return msc.err
	}
	for _, sess := range msc.sessions {
		err := sess.sm.tb.CreateMirrorSession(sess.veniceMirrorSess)
		if err != nil {
			// try updating it
			err = sess.sm.tb.UpdateMirrorSession(sess.veniceMirrorSess)
			if err != nil {
				msc.err = err
				return err
			}
		}

		log.Debugf("Created session: %#v", sess.veniceMirrorSess)

		sess.sm.msessions[sess.veniceMirrorSess.Name] = sess
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

	for _, sess := range msc.sessions {
		sess.veniceMirrorSess.Spec.MatchRules = append(sess.veniceMirrorSess.Spec.MatchRules, rule)
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
	for _, wpair := range wpc.pairs {
		fromIP := strings.Split(wpair.second.iotaWorkload.IpPrefix, "/")[0]
		toIP := strings.Split(wpair.first.iotaWorkload.IpPrefix, "/")[0]
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
	for _, sess := range msc.sessions {
		err := sess.sm.tb.DeleteMirrorSession(sess.veniceMirrorSess)
		if err != nil {
			return err
		}
		delete(sess.sm.msessions, sess.veniceMirrorSess.Name)
	}

	return nil
}

// MirrorSessions returns all Mirror Sessions in the model
func (sm *SysModel) MirrorSessions() *MirrorSessionCollection {
	msc := MirrorSessionCollection{}
	for _, sess := range msc.sessions {
		msc.sessions = append(msc.sessions, sess)
	}

	return &msc
}
