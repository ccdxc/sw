package vchub

import (
	"github.com/vmware/govmomi/vim25/types"

	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
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

		case m, active := <-v.vcReadCh:
			if !active {
				return
			}

			switch m.MsgType {
			case defs.VCNotification:
				v.handleVCNotification(m.Val.(defs.VCNotificationMsg))
			case defs.VCEvent:
				v.handleVCEvent(m.Val.(defs.VCEventMsg))
			default:
				v.Log.Errorf("Unknown event %s", m.MsgType)
			}
		case evt, ok := <-apiServerCh:
			if ok {
				nw := evt.Object.(*network.Network)
				v.handleNetworkEvent(evt.Type, nw)
			}
		}
	}
}

func (v *VCHub) handleVCEvent(m defs.VCEventMsg) {
	v.Log.Infof("Msg from %v, key: %s prop: %s", m.Originator, m.Key, m.VcObject)
	switch m.VcObject {
	case defs.VirtualMachine:
		v.handleWorkload(m)
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
	for _, prop := range m.Changes {
		name := prop.Val.(string)
		v.Log.Infof("Handle DC %s", name)
		v.DcMapLock.Lock()
		if penDc, ok := v.DcMap[name]; ok {
			penDc.Lock()
			if _, ok := penDc.DvsMap[createDVSName(name)]; ok {
				v.DcMapLock.Unlock()
				penDc.Unlock()
				return
			}
			penDc.Unlock()
		}
		v.DcMapLock.Unlock()
		// We create DVS and check networks
		_, ok := v.ForceDCNames[name]
		if len(v.ForceDCNames) > 0 && !ok {
			v.Log.Infof("Skipping DC event for DC %s", name)
			continue
		}
		v.Log.Infof("new DC %s", name)
		penDC, err := v.NewPenDC(name, m.Key)
		if err == nil {
			v.probe.StartEventReceiver([]types.ManagedObjectReference{penDC.dcRef})
		}
	}
}
