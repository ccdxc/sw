package vchub

import (
	"fmt"
	"sort"
	"time"

	"github.com/vmware/govmomi/vim25/soap"
	"github.com/vmware/govmomi/vim25/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/orchestration"
	"github.com/pensando/sw/api/generated/workload"
	"github.com/pensando/sw/events/generated/eventtypes"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/vcprobe/session"
	"github.com/pensando/sw/venice/utils/events/recorder"
)

func (v *VCHub) startEventsListener() {
	v.Log.Infof("Running store")
	v.Log.Infof("Starting probe channel watch for %s", v.OrchConfig.Name)
	defer v.Wg.Done()
	apiServerCh, err := v.StateMgr.GetProbeChannel(v.OrchConfig.GetName())
	if err != nil {
		v.Log.Errorf("Could not get probe channel for %s. Err : %v", v.OrchConfig.GetKey(), err)
		return
	}

	for {
		select {
		case <-v.Ctx.Done():
			return

		case m, active := <-v.vcEventCh:
			if !active {
				return
			}
			switch m.MsgType {
			case defs.VCNotification:
				// These are notifications from vcenter's EventManager
				v.handleVCNotification(m.Val.(defs.VCNotificationMsg))
			default:
				v.Log.Errorf("Unknown event %s", m.MsgType)
			}
		case m, active := <-v.vcReadCh:
			if !active {
				return
			}
			switch m.MsgType {
			case defs.VCEvent:
				// These are watch events
				v.handleVCEvent(m.Val.(defs.VCEventMsg))
			case defs.VCConnectionStatus:
				// we've reconnected, trigger sync
				connStatus := m.Val.(session.ConnectionState)
				o, err := v.StateMgr.Controller().Orchestrator().Find(&v.OrchConfig.ObjectMeta)
				// Connection transisitions
				// only healthy can go to degraded.
				if err != nil {
					// orchestrator object does not exists anymore
					// this should never happen
					v.Log.Errorf("Orchestrator Object %v does not exist",
						v.OrchConfig.GetKey())
					return
				}

				previousState := o.Orchestrator.Status.Status
				previousMsg := o.Orchestrator.Status.Message

				v.Log.Infof("Updating orchestrator connection status to %v", connStatus.State)

				var msg string
				if connStatus.State == orchestration.OrchestratorStatus_Success.String() {
					// Degraded -> Success should not resync
					if previousState == orchestration.OrchestratorStatus_Degraded.String() ||
						previousState == orchestration.OrchestratorStatus_Success.String() {
						break
					}
					// sync and start watchers, network event watcher
					// will not start until after sync finishes (blocked on processVeniceEvents flag)
					v.discoveredDCs = []string{}
					v.sync()

					v.probe.StartWatchers()

					v.watchStarted = true

				} else if connStatus.State == orchestration.OrchestratorStatus_Failure.String() {
					evt := eventtypes.ORCH_CONNECTION_ERROR
					msg = connStatus.Err.Error()
					evtMsg := fmt.Sprintf("%s: %s", v.OrchConfig.Name, msg)
					// Generate event depending on error type
					// Check if it is a credential issue
					if soap.IsSoapFault(connStatus.Err) {
						soapErr := soap.ToSoapFault(connStatus.Err)
						msg = soapErr.String
						evtMsg = fmt.Sprintf("%s: %s", v.OrchConfig.Name, msg)
						if _, ok := soapErr.Detail.Fault.(types.InvalidLogin); ok {
							// Login error
							evt = eventtypes.ORCH_LOGIN_FAILURE
						}
					}
					recorder.Event(evt, evtMsg, v.State.OrchConfig)
					// Stop acting on network events from venice until reconnect sync
					// Lock must be taken in case periodic sync runs
					v.processVeniceEventsLock.Lock()
					v.processVeniceEvents = false
					v.processVeniceEventsLock.Unlock()
				} else if connStatus.State == orchestration.OrchestratorStatus_Degraded.String() {
					evt := eventtypes.ORCH_CONNECTION_ERROR
					msg = connStatus.Err.Error()
					evtMsg := fmt.Sprintf("%s: %s", v.OrchConfig.Name, msg)
					if v.probe.IsREST401(connStatus.Err) {
						evt = eventtypes.ORCH_LOGIN_FAILURE
					}
					recorder.Event(evt, evtMsg, v.State.OrchConfig)
				}

				//
				if connStatus.State == previousState && msg == previousMsg {
					// Duplicate event, nothing to do
					v.Log.Debugf("Duplicate connection event, nothing to do.")
					return
				}

				v.orchUpdateLock.Lock()
				o.Orchestrator.Status.Status = connStatus.State
				o.Orchestrator.Status.LastTransitionTime = &api.Timestamp{}
				o.Orchestrator.Status.LastTransitionTime.SetTime(time.Now())
				o.Orchestrator.Status.Message = msg
				o.Status.OrchID = v.OrchConfig.Status.OrchID
				if connStatus.State == orchestration.OrchestratorStatus_Failure.String() {
					v.discoveredDCs = []string{}
					o.Status.DiscoveredNamespaces = []string{}
				}
				o.Write()
				v.orchUpdateLock.Unlock()

			default:
				v.Log.Errorf("Unknown event %s", m.MsgType)
			}
		case evt, ok := <-apiServerCh:
			// processVeniceEvents will be false when we are disconnected,
			// and will be set to true once connection is restored AND sync has run
			v.processVeniceEventsLock.Lock()
			processEvent := v.processVeniceEvents
			v.processVeniceEventsLock.Unlock()
			v.Log.Infof("Received API event")

			if ok && processEvent {
				switch obj := evt.Object.(type) {
				case *network.Network:
					v.Log.Infof("Processing network event %s", obj.Name)
					v.handleNetworkEvent(evt.Type, obj)
				case *workload.Workload:
					v.Log.Infof("Processing workload event %s", obj.Name)
					v.handleWorkloadEvent(evt.Type, obj)
				}
			} else {
				v.Log.Infof("Skipped API Event %v", ok && processEvent)
			}
		}
	}
}

func (v *VCHub) handleVCEvent(m defs.VCEventMsg) {
	v.Log.Infof("Msg from %v, key: %s prop: %s", m.Originator, m.Key, m.VcObject)
	v.syncLock.RLock()
	defer v.syncLock.RUnlock()
	switch m.VcObject {
	case defs.VirtualMachine:
		v.handleVM(m)
	case defs.HostSystem:
		v.handleHost(m)
	case defs.DistributedVirtualPortgroup:
		v.handlePG(m)
	case defs.VmwareDistributedVirtualSwitch:
		v.handleDVS(m)
	case defs.Datacenter:
		v.handleDC(m)
	default:
		v.Log.Errorf("Unknown object %s", m.VcObject)
	}
}

func (v *VCHub) handleVCNotification(m defs.VCNotificationMsg) {
	v.Log.Debugf("Received VC Notification %s %v", m.Type, m.Msg)
	// XXX Do we need m.Type or just use type switching
	switch m.Msg.(type) {
	case defs.VMotionStartMsg:
		v.handleVMotionStart(m.Msg.(defs.VMotionStartMsg))
	case defs.VMotionFailedMsg:
		v.handleVMotionFailed(m.Msg.(defs.VMotionFailedMsg))
	case defs.VMotionDoneMsg:
		v.handleVMotionDone(m.Msg.(defs.VMotionDoneMsg))
	}
}

func (v *VCHub) handleDC(m defs.VCEventMsg) {
	// Check if we have a DC object
	v.Log.Infof("Handle DC called")
	existingDC := v.GetDCFromID(m.Key)
	if m.UpdateType == types.ObjectUpdateKindLeave {
		v.Log.Infof("DC delete for %s", m.Key)
		if existingDC == nil {
			v.Log.Infof("No state for DC %s", m.Key)
			v.DcMapLock.Lock()
			if dcName, ok := v.DcID2NameMap[m.Key]; ok {
				v.removeDiscoveredDC(dcName)
				delete(v.DcID2NameMap, m.Key)
			}
			v.DcMapLock.Unlock()
			return
		}
		v.probe.StopWatchForDC(existingDC.Name, m.Key)
		// Cleanup internal state
		v.RemovePenDC(existingDC.Name)
		v.removeDiscoveredDC(existingDC.Name)
		return
	}

	for _, prop := range m.Changes {
		name := prop.Val.(string)
		v.Log.Infof("Handle DC %s %s", name, m.Key)
		// Check ID first to detect rename
		v.DcMapLock.Lock()

		oldName, ok := v.DcID2NameMap[m.Key]
		if ok && oldName != name {
			// Check if we are managing it
			if _, ok := v.DcMap[oldName]; !ok {
				// DC we aren't managing is renamed, update map entry
				v.DcID2NameMap[m.Key] = name
				v.DcMapLock.Unlock()
				v.renameDiscoveredDC(oldName, name)
				return
			}

			v.Log.Infof("DC %s renamed to %s, changing back...", name, oldName)

			evtMsg := fmt.Sprintf("User renamed a Pensando managed DC. Name has been changed back.")
			recorder.Event(eventtypes.ORCH_INVALID_ACTION, evtMsg, v.State.OrchConfig)

			err := v.probe.RenameDC(name, oldName, 3)
			if err != nil {
				v.Log.Errorf("Failed to rename DC %s back to %s, err %s", name, oldName, err)
			}
			v.DcMapLock.Unlock()
			return
		}

		if penDc, ok := v.DcMap[name]; ok {
			penDc.Lock()
			if _, ok := penDc.DvsMap[CreateDVSName(name)]; ok {
				penDc.Unlock()
				v.DcMapLock.Unlock()
				v.probe.StartWatchForDC(name, m.Key)
				v.addDiscoveredDC(name)
				return
			}
			penDc.Unlock()
		}
		v.DcMapLock.Unlock()

		// We create DVS and check networks
		if !v.isManagedNamespace(name) {
			v.Log.Infof("Skipping DC event for DC %s", name)
			v.DcMapLock.Lock()
			v.DcID2NameMap[m.Key] = name
			v.DcMapLock.Unlock()
		} else {
			v.Log.Infof("new DC %s", name)
			_, err := v.NewPenDC(name, m.Key)
			if err == nil {
				v.probe.StartWatchForDC(name, m.Key)
			}
			v.checkNetworks(name)
		}

		// update discovered list
		v.addDiscoveredDC(name)
	}
}

func (v *VCHub) removeDiscoveredDC(dcName string) {
	v.Log.Infof("removing DC %s to discovered DCs", dcName)
	dcList := v.discoveredDCs
	found := -1
	for i, entry := range dcList {
		if dcName == entry {
			found = i
		}
	}
	if found >= 0 {
		dcList = append(dcList[:found], dcList[found+1:]...)
		v.discoveredDCs = dcList

		v.orchUpdateLock.Lock()
		defer v.orchUpdateLock.Unlock()
		o, err := v.StateMgr.Controller().Orchestrator().Find(&v.OrchConfig.ObjectMeta)
		if err != nil {
			// orchestrator object does not exists anymore
			// this should never happen
			v.Log.Errorf("removeDiscoveredDC: Orchestrator Object %v does not exist",
				v.OrchConfig.GetKey())
			return
		}

		o.Orchestrator.Status.DiscoveredNamespaces = v.discoveredDCs
		err = o.Write()

		if err != nil {
			v.Log.Errorf("removeDiscoveredDC: Failed to update orch status %s", err)
		}
	}
}

func (v *VCHub) addDiscoveredDC(dcName string) {
	v.Log.Infof("adding DC %s to discovered DCs", dcName)
	dcList := v.discoveredDCs
	found := -1
	for i, entry := range dcList {
		if dcName == entry {
			found = i
		}
	}
	if found == -1 {
		dcList = append(dcList, dcName)
		sort.Strings(dcList)
		v.discoveredDCs = dcList

		v.orchUpdateLock.Lock()
		defer v.orchUpdateLock.Unlock()
		o, err := v.StateMgr.Controller().Orchestrator().Find(&v.OrchConfig.ObjectMeta)
		if err != nil {
			// orchestrator object does not exists anymore
			// this should never happen
			v.Log.Errorf("AddDiscoveredDC: Orchestrator Object %v does not exist",
				v.OrchConfig.GetKey())
			return
		}

		o.Orchestrator.Status.DiscoveredNamespaces = v.discoveredDCs
		err = o.Write()

		if err != nil {
			v.Log.Errorf("AddDiscoveredDC: Failed to update orch status %s", err)
		}
	}
}

func (v *VCHub) renameDiscoveredDC(oldName, newName string) {
	v.Log.Infof("renaming DC %s to %s in discovered DCs", oldName, newName)
	dcList := v.discoveredDCs
	found := -1
	for i, entry := range dcList {
		if oldName == entry {
			found = i
		}
	}

	if found >= 0 {
		dcList[found] = newName
		sort.Strings(dcList)
		v.discoveredDCs = dcList

		v.orchUpdateLock.Lock()
		defer v.orchUpdateLock.Unlock()
		o, err := v.StateMgr.Controller().Orchestrator().Find(&v.OrchConfig.ObjectMeta)
		if err != nil {
			// orchestrator object does not exists anymore
			// this should never happen
			v.Log.Errorf("RenameDiscoveredDC: Orchestrator Object %v does not exist",
				v.OrchConfig.GetKey())
			return
		}

		o.Orchestrator.Status.DiscoveredNamespaces = v.discoveredDCs
		err = o.Write()

		if err != nil {
			v.Log.Errorf("RenameDiscoveredDC: Failed to update orch status %s", err)
		}
	}
}
