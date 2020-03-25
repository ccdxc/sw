package vchub

import (
	"github.com/vmware/govmomi/vim25/soap"
	"github.com/vmware/govmomi/vim25/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/orchestration"
	"github.com/pensando/sw/api/generated/workload"
	"github.com/pensando/sw/events/generated/eventtypes"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/ctrler/orchhub/utils"
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
				var connErr error
				if m.Val != nil {
					connErr = m.Val.(error)
				}
				o, err := v.StateMgr.Controller().Orchestrator().Find(&v.OrchConfig.ObjectMeta)
				if err != nil {
					// orchestrator object does not exists anymore
					// this should never happen
					v.Log.Errorf("Orchestrator Object %v does not exist",
						v.OrchConfig.GetKey())
					return
				}
				if connErr == nil {
					v.Log.Infof("Updating orchestrator connection status to %v",
						orchestration.OrchestratorStatus_Success.String())
					o.Orchestrator.Status.Status = orchestration.OrchestratorStatus_Success.String()

					o.Status.OrchID = v.OrchConfig.Status.OrchID
					o.Write()

					// sync and start watchers, network event watcher
					// will not start until after sync finishes (blocked on processVeniceEvents flag)
					v.sync()

					v.probe.StartWatchers()

				} else {
					v.Log.Infof("Updating orchestrator connection status to %v",
						orchestration.OrchestratorStatus_Failure.String())
					o.Orchestrator.Status.Status = orchestration.OrchestratorStatus_Failure.String()
					o.Write()

					evt := eventtypes.ORCH_CONNECTION_ERROR
					evtMsg := connErr.Error()
					// Generate event depending on error type
					// Check if it is a credential issue
					if soap.IsSoapFault(connErr) {
						soapErr := soap.ToSoapFault(connErr)
						evtMsg = soapErr.String
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
				}

			default:
				v.Log.Errorf("Unknown event %s", m.MsgType)
			}
		case evt, ok := <-apiServerCh:
			// processVeniceEvents will be false when we are disconnected,
			// and will be set to true once connection is restored AND sync has run
			v.processVeniceEventsLock.Lock()
			processEvent := v.processVeniceEvents
			v.processVeniceEventsLock.Unlock()
			v.Log.Errorf("Received API event")

			if ok && processEvent {
				switch obj := evt.Object.(type) {
				case *network.Network:
					v.Log.Infof("Processing network event %s", obj.Name)
					v.handleNetworkEvent(evt.Type, obj)
				case *workload.Workload:
					v.Log.Infof("Processing workload event %s", obj.Name)
					v.handleWorkloadEvent(evt.Type, obj)
				}
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
	// TODO: HANDLE RENAME
	// Check if we have a DC object
	v.Log.Infof("Handle DC called")
	existingDC := v.GetDCFromID(m.Key)
	if m.UpdateType == types.ObjectUpdateKindLeave {
		v.Log.Infof("DC delete for %s", m.Key)
		if existingDC == nil {
			v.Log.Infof("No state for DC %s", m.Key)
			return
		}
		v.probe.StopWatchForDC(existingDC.Name, m.Key)
		// Cleanup internal state
		v.DcMapLock.Lock()

		// Delete any Workloads or hosts associated with this DC
		opts := api.ListWatchOptions{}
		workloads, err := v.StateMgr.Controller().Workload().List(v.Ctx, &opts)
		if err != nil {
			v.Log.Errorf("Failed to get network list. Err : %v", err)
		}
		for _, workload := range workloads {
			if utils.IsObjForOrch(workload.Labels, v.VcID, existingDC.Name) {
				v.deleteWorkload(&workload.Workload)
			}
		}

		hosts, err := v.StateMgr.Controller().Host().List(v.Ctx, &opts)
		if err != nil {
			v.Log.Errorf("Failed to get network list. Err : %v", err)
		}
		for _, host := range hosts {
			if utils.IsObjForOrch(host.Labels, v.VcID, existingDC.Name) {
				v.deleteHostFromDc(&host.Host, existingDC)
			}
		}

		delete(v.DcMap, existingDC.Name)
		v.DcMapLock.Unlock()
		return
	}

	for _, prop := range m.Changes {
		name := prop.Val.(string)
		v.Log.Infof("Handle DC %s", name)
		v.DcMapLock.Lock()
		if penDc, ok := v.DcMap[name]; ok {
			penDc.Lock()
			if _, ok := penDc.DvsMap[CreateDVSName(name)]; ok {
				penDc.Unlock()
				v.DcMapLock.Unlock()
				v.probe.StartWatchForDC(name, m.Key)
				return
			}
			penDc.Unlock()
		}
		v.DcMapLock.Unlock()
		// We create DVS and check networks
		if !v.isManagedNamespace(name) {
			v.Log.Infof("Skipping DC event for DC %s", name)
			continue
		}
		v.Log.Infof("new DC %s", name)
		_, err := v.NewPenDC(name, m.Key)
		if err == nil {
			v.probe.StartWatchForDC(name, m.Key)
		}
		v.checkNetworks(name)
	}
}
