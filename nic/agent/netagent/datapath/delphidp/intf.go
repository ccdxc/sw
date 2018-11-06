// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package delphidp

import (
	"errors"

	"github.com/pensando/sw/nic/agent/netagent/datapath/delphidp/goproto"
	"github.com/pensando/sw/venice/ctrler/npm/rpcserver/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// ------------------------- Datapath APIs called from netagent ----------------------

// CreateInterface creates an interface
func (dp *DelphiDatapath) CreateInterface(intf *netproto.Interface, lif *netproto.Interface, port *netproto.Port, ns *netproto.Namespace) error {
	switch intf.Spec.Type {
	case "UPLINK_ETH", "UPLINK_MGMT":
		var portID uint32

		// TODO remove hack once hal/dol is fixed with correct mapping
		if port.Status.PortID == 5 {
			portID = 2
		} else {
			portID = uint32(port.Status.PortID)
		}

		// build interface spec
		spec := &goproto.InterfaceSpec_{
			KeyOrHandle: &goproto.InterfaceKeyHandle_{
				InterfaceId: intf.Status.InterfaceID,
			},
			IfType: goproto.IntfType_IF_TYPE_UPLINK,
			IfUplinkInfo: &goproto.IfUplinkInfo_{
				PortNum: uint32(portID),
			},
		}

		// write it to delphi
		ifSpec := goproto.NewInterfaceSpecFromMessage(dp.delphiClient, spec)
		if ifSpec == nil {
			log.Errorf("Error creating interface spec in delphi")
			return errors.New("Error writing object to delphi")
		}

	case "ENIC":
		// build the spec
		spec := &goproto.InterfaceSpec_{
			KeyOrHandle: &goproto.InterfaceKeyHandle_{
				InterfaceId: intf.Status.InterfaceID,
			},
			IfType: goproto.IntfType_IF_TYPE_ENIC,
			// associate the lif id
			IfEnicInfo: &goproto.IfEnicInfo_{
				EnicType: goproto.IntfEnicType_IF_ENIC_TYPE_USEG,
				LifKeyOrHandle: &goproto.LifKeyHandle_{
					LifId: lif.Status.InterfaceID,
				},
			},
		}

		// write it to delphi
		ifSpec := goproto.NewInterfaceSpecFromMessage(dp.delphiClient, spec)
		if ifSpec == nil {
			log.Errorf("Error creating interface spec in delphi")
			return errors.New("Error writing object to delphi")
		}

	default:
		return errors.New("invalid interface type")
	}

	return nil
}

// UpdateInterface updates an interface
func (dp *DelphiDatapath) UpdateInterface(intf *netproto.Interface, ns *netproto.Namespace) error {
	return nil
}

// DeleteInterface deletes an interface
func (dp *DelphiDatapath) DeleteInterface(intf *netproto.Interface, ns *netproto.Namespace) error {
	switch intf.Spec.Type {
	case "UPLINK_ETH", "UPLINK_MGMT":
		ifKey := &goproto.InterfaceKeyHandle{}
		ifKey.SetInterfaceId(intf.Status.InterfaceID)
		ifSpec := goproto.NewInterfaceSpecWithKey(dp.delphiClient, ifKey)
		dp.delphiClient.DeleteObject(ifSpec)

	case "ENIC":
		ifKey := &goproto.InterfaceKeyHandle{}
		ifKey.SetInterfaceId(intf.Status.InterfaceID)
		ifSpec := goproto.NewInterfaceSpecWithKey(dp.delphiClient, ifKey)
		dp.delphiClient.DeleteObject(ifSpec)
	default:
		return errors.New("invalid interface type")
	}
	return nil
}

// ListInterfaces Lists all the lifs and uplinks from the datapath state
func (dp *DelphiDatapath) ListInterfaces() ([]*netproto.Interface, []*netproto.Port, error) {
	return nil, nil, nil
}

//--------------------------- Delphi reactors ---------------------------

// OnInterfaceStatusCreate event handler
func (dp *DelphiDatapath) OnInterfaceStatusCreate(obj *goproto.InterfaceStatus) {
	dp.incrEventStats("OnInterfaceStatusCreate")
	return
}

// OnInterfaceStatusUpdate event handler
func (dp *DelphiDatapath) OnInterfaceStatusUpdate(obj *goproto.InterfaceStatus) {
	dp.incrEventStats("OnInterfaceStatusUpdate")
	return
}

// OnInterfaceStatusDelete event handler
func (dp *DelphiDatapath) OnInterfaceStatusDelete(obj *goproto.InterfaceStatus) {
	dp.incrEventStats("OnInterfaceStatusDelete")
	return
}
