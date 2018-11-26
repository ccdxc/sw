// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package delphidp

import (
	"errors"

	"github.com/pensando/sw/nic/agent/netagent/datapath/delphidp/halproto"
	"github.com/pensando/sw/venice/ctrler/npm/rpcserver/netproto"
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
		ifSpec := &halproto.InterfaceSpec{
			KeyOrHandle: &halproto.InterfaceKeyHandle{
				KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
					InterfaceId: intf.Status.InterfaceID,
				},
			},
			Type: halproto.IfType_IF_TYPE_UPLINK,
			IfInfo: &halproto.InterfaceSpec_IfUplinkInfo{
				IfUplinkInfo: &halproto.IfUplinkInfo{
					PortNum: uint32(portID),
				},
			},
		}

		// write it to delphi
		dp.delphiClient.SetObject(ifSpec)

	case "ENIC":
		// build the spec
		ifSpec := &halproto.InterfaceSpec{
			KeyOrHandle: &halproto.InterfaceKeyHandle{
				KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
					InterfaceId: intf.Status.InterfaceID,
				},
			},
			Type: halproto.IfType_IF_TYPE_ENIC,
			// associate the lif id
			IfInfo: &halproto.InterfaceSpec_IfEnicInfo{
				IfEnicInfo: &halproto.IfEnicInfo{
					EnicType: halproto.IfEnicType_IF_ENIC_TYPE_USEG,
					LifKeyOrHandle: &halproto.LifKeyHandle{
						KeyOrHandle: &halproto.LifKeyHandle_LifId{
							LifId: lif.Status.InterfaceID,
						},
					},
				},
			},
		}

		// write it to delphi
		dp.delphiClient.SetObject(ifSpec)

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
		ifSpec := &halproto.InterfaceSpec{
			KeyOrHandle: &halproto.InterfaceKeyHandle{
				KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
					InterfaceId: intf.Status.InterfaceID,
				},
			},
		}
		dp.delphiClient.DeleteObject(ifSpec)

	case "ENIC":
		ifSpec := &halproto.InterfaceSpec{
			KeyOrHandle: &halproto.InterfaceKeyHandle{
				KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
					InterfaceId: intf.Status.InterfaceID,
				},
			},
		}
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
func (dp *DelphiDatapath) OnInterfaceStatusCreate(obj *halproto.InterfaceStatus) {
	dp.incrEventStats("OnInterfaceStatusCreate")
	return
}

// OnInterfaceStatusUpdate event handler
func (dp *DelphiDatapath) OnInterfaceStatusUpdate(old, obj *halproto.InterfaceStatus) {
	dp.incrEventStats("OnInterfaceStatusUpdate")
	return
}

// OnInterfaceStatusDelete event handler
func (dp *DelphiDatapath) OnInterfaceStatusDelete(obj *halproto.InterfaceStatus) {
	dp.incrEventStats("OnInterfaceStatusDelete")
	return
}
