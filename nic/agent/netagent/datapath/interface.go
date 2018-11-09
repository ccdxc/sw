package datapath

import (
	"context"
	"errors"
	"fmt"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/venice/ctrler/npm/rpcserver/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// CreateInterface creates an interface
func (hd *Datapath) CreateInterface(intf *netproto.Interface, lif *netproto.Interface, port *netproto.Port, ns *netproto.Namespace) error {
	var ifSpec *halproto.InterfaceSpec
	switch intf.Spec.Type {
	case "LIF":
		lifReqMsg := &halproto.LifRequestMsg{
			Request: []*halproto.LifSpec{
				{
					KeyOrHandle: &halproto.LifKeyHandle{
						KeyOrHandle: &halproto.LifKeyHandle_LifId{
							LifId: intf.Status.InterfaceID,
						},
					},
				},
			},
		}
		_, err := hd.Hal.Ifclient.LifCreate(context.Background(), lifReqMsg)
		if err != nil {
			log.Errorf("Error creating lif. Err: %v", err)
			return err
		}
		//hd.Lock()
		hd.DB.LifDB[objectKey(&ns.ObjectMeta)] = lifReqMsg
		//hd.Unlock()
		return nil

	case "UPLINK_ETH", "UPLINK_MGMT":
		var portID uint32

		// TODO remove hack once hal/dol is fixed with correct mapping
		if port.Status.PortID == 5 {
			portID = 2
		} else {
			portID = uint32(port.Status.PortID)
		}

		ifSpec = &halproto.InterfaceSpec{
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

	case "ENIC":
		ifSpec = &halproto.InterfaceSpec{
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
	default:
		return errors.New("invalid interface type")
	}

	ifReqMsg := &halproto.InterfaceRequestMsg{
		Request: []*halproto.InterfaceSpec{
			ifSpec,
		},
	}

	if hd.Kind == "hal" {
		resp, err := hd.Hal.Ifclient.InterfaceCreate(context.Background(), ifReqMsg)
		if err != nil {
			log.Errorf("Error creating interface. Err: %v", err)
			return err
		}
		if resp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus)

			return ErrHALNotOK
		}
	} else {
		_, err := hd.Hal.Ifclient.InterfaceCreate(context.Background(), ifReqMsg)
		if err != nil {
			log.Errorf("Error creating inteface. Err: %v", err)
			return err
		}
	}

	//hd.Lock()
	hd.DB.InterfaceDB[objectKey(&ns.ObjectMeta)] = ifReqMsg
	//hd.Unlock()
	return nil
}

// DeleteInterface deletes an interface
func (hd *Datapath) DeleteInterface(intf *netproto.Interface, ns *netproto.Namespace) error {
	var ifDelReq *halproto.InterfaceDeleteRequest
	switch intf.Spec.Type {
	case "LIF":
		lifDelReqMsg := &halproto.LifDeleteRequestMsg{
			Request: []*halproto.LifDeleteRequest{
				{
					KeyOrHandle: &halproto.LifKeyHandle{
						KeyOrHandle: &halproto.LifKeyHandle_LifId{
							LifId: intf.Status.InterfaceID,
						},
					},
				},
			},
		}
		_, err := hd.Hal.Ifclient.LifDelete(context.Background(), lifDelReqMsg)
		if err != nil {
			log.Errorf("Error creating lif. Err: %v", err)
			return err
		}
		// save the lif delete message
		//hd.Lock()
		hd.DB.LifDelDB[objectKey(&intf.ObjectMeta)] = lifDelReqMsg
		delete(hd.DB.LifDB, objectKey(&intf.ObjectMeta))
		//hd.Unlock()

		return nil
	case "ENIC":
		fallthrough

	case "UPLINK":
		ifDelReq = &halproto.InterfaceDeleteRequest{
			KeyOrHandle: &halproto.InterfaceKeyHandle{
				KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
					InterfaceId: intf.Status.InterfaceID,
				},
			},
		}
	default:
		return errors.New("invalid interface type")
	}

	ifDelReqMsg := &halproto.InterfaceDeleteRequestMsg{
		Request: []*halproto.InterfaceDeleteRequest{
			ifDelReq,
		},
	}
	_, err := hd.Hal.Ifclient.InterfaceDelete(context.Background(), ifDelReqMsg)
	if err != nil {
		log.Errorf("Error creating lif. Err: %v", err)
		return err
	}
	// save the lif delete message
	//hd.Lock()
	hd.DB.InterfaceDelDB[objectKey(&intf.ObjectMeta)] = ifDelReqMsg
	delete(hd.DB.LifDB, objectKey(&intf.ObjectMeta))
	//hd.Unlock()
	return nil

}

// UpdateInterface updates an interface
func (hd *Datapath) UpdateInterface(intf *netproto.Interface, ns *netproto.Namespace) error {
	var ifSpec *halproto.InterfaceSpec
	switch intf.Spec.Type {
	case "LIF":
		lifReqMsg := &halproto.LifRequestMsg{
			Request: []*halproto.LifSpec{
				{
					KeyOrHandle: &halproto.LifKeyHandle{
						KeyOrHandle: &halproto.LifKeyHandle_LifId{
							LifId: intf.Status.InterfaceID,
						},
					},
				},
			},
		}
		_, err := hd.Hal.Ifclient.LifUpdate(context.Background(), lifReqMsg)
		if err != nil {
			log.Errorf("Error creating lif. Err: %v", err)
			return err
		}
		//hd.Lock()
		hd.DB.LifDB[objectKey(&ns.ObjectMeta)] = lifReqMsg
		//hd.Unlock()
		return nil

	case "UPLINK":
		ifSpec = &halproto.InterfaceSpec{
			KeyOrHandle: &halproto.InterfaceKeyHandle{
				KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
					InterfaceId: intf.Status.InterfaceID,
				},
			},
			Type: halproto.IfType_IF_TYPE_UPLINK,
		}

	case "ENIC":
		ifSpec = &halproto.InterfaceSpec{
			KeyOrHandle: &halproto.InterfaceKeyHandle{
				KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
					InterfaceId: intf.Status.InterfaceID,
				},
			},
			Type: halproto.IfType_IF_TYPE_ENIC,
		}
	default:
		return errors.New("invalid interface type")
	}

	ifReqMsg := &halproto.InterfaceRequestMsg{
		Request: []*halproto.InterfaceSpec{
			ifSpec,
		},
	}
	_, err := hd.Hal.Ifclient.InterfaceUpdate(context.Background(), ifReqMsg)
	if err != nil {
		log.Errorf("Error updating interface. Err: %v", err)
		return err
	}
	//hd.Lock()
	hd.DB.InterfaceDB[objectKey(&ns.ObjectMeta)] = ifReqMsg
	//hd.Unlock()
	return nil
}

// ListInterfaces returns the lisg of lifs and uplinks from the datapath
func (hd *Datapath) ListInterfaces() ([]*netproto.Interface, []*netproto.Port, error) {
	var lifResp []*netproto.Interface
	var portResp []*netproto.Port
	var lifs *halproto.LifGetResponseMsg
	var ports *halproto.PortInfoGetResponseMsg
	var err error

	if hd.Kind == "hal" {
		lifReq := &halproto.LifGetRequest{}
		lifReqMsg := &halproto.LifGetRequestMsg{
			Request: []*halproto.LifGetRequest{
				lifReq,
			},
		}

		// ToDo Add lif checks once nic mgr is integrated
		lifs, err = hd.Hal.Ifclient.LifGet(context.Background(), lifReqMsg)
		if err != nil {
			log.Errorf("Error getting lifs from the datapath. Err: %v", err)
			return nil, nil, nil
		}

		// get all ports
		portReq := &halproto.PortInfoGetRequest{}
		portReqMsg := &halproto.PortInfoGetRequestMsg{
			Request: []*halproto.PortInfoGetRequest{portReq},
		}
		ports, err = hd.Hal.PortClient.PortInfoGet(context.Background(), portReqMsg)
		if err != nil {
			return nil, nil, err
		}

		// return all the ports
		for _, p := range ports.Response {
			if p.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
				return nil, nil, ErrHALNotOK
			}
		}

	} else {

		// ToDo Remove the List Mock prior to FCS
		lifs, ports, err = hd.generateMockHwState()
	}

	for _, lif := range lifs.Response {
		id := lif.Spec.KeyOrHandle.GetLifId()
		l := &netproto.Interface{
			TypeMeta: api.TypeMeta{
				Kind: "Interface",
			},
			ObjectMeta: api.ObjectMeta{
				Tenant:    "default",
				Namespace: "default",
				Name:      fmt.Sprintf("lif%d", id),
			},
			Spec: netproto.InterfaceSpec{
				Type: "LIF",
			},
			Status: netproto.InterfaceStatus{
				InterfaceID: lif.Spec.KeyOrHandle.GetLifId(),
			},
		}

		lifResp = append(lifResp, l)
	}

	var numLanes uint32
	// Populate Agent state
	for _, port := range ports.Response {
		var portType, speed string
		id := 1 + numLanes
		numLanes += port.Spec.NumLanes
		if port.Spec.PortType == halproto.PortType_PORT_TYPE_MGMT {
			portType = "TYPE_MANAGEMENT"
			speed = "SPEED_1G"
		} else {
			portType = "TYPE_ETHERNET"
			speed = "SPEED_100G"
		}
		p := &netproto.Port{
			TypeMeta: api.TypeMeta{
				Kind: "Port",
			},
			ObjectMeta: api.ObjectMeta{
				Tenant:    "default",
				Namespace: "default",
				Name:      fmt.Sprintf("port%d", id),
			},
			Spec: netproto.PortSpec{
				Speed:        speed,
				BreakoutMode: "BREAKOUT_NONE",
				AdminStatus:  "UP",
				Type:         portType,
				Lanes:        port.Spec.NumLanes,
			},
			Status: netproto.PortStatus{
				PortID: uint64(id),
			},
		}

		portResp = append(portResp, p)
	}

	// return resp
	return lifResp, portResp, err
}
