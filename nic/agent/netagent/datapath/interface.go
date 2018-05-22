package datapath

import (
	"context"
	"errors"

	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/venice/ctrler/npm/rpcserver/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// CreateInterface creates an interface
func (hd *Datapath) CreateInterface(intf *netproto.Interface, lif *netproto.Interface, ns *netproto.Namespace) error {
	var ifSpec *halproto.InterfaceSpec
	switch intf.Spec.Type {
	case "LIF":
		lifReqMsg := &halproto.LifRequestMsg{
			Request: []*halproto.LifSpec{
				{
					Meta: &halproto.ObjectMeta{
						VrfId: ns.Status.NamespaceID,
					},
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
		hd.Lock()
		hd.DB.LifDB[objectKey(&ns.ObjectMeta)] = lifReqMsg
		hd.Unlock()
		return nil

	case "UPLINK":
		ifSpec = &halproto.InterfaceSpec{
			Meta: &halproto.ObjectMeta{
				VrfId: ns.Status.NamespaceID,
			},
			KeyOrHandle: &halproto.InterfaceKeyHandle{
				KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
					InterfaceId: intf.Status.InterfaceID,
				},
			},
			Type: halproto.IfType_IF_TYPE_UPLINK,
		}

	case "ENIC":
		ifSpec = &halproto.InterfaceSpec{
			Meta: &halproto.ObjectMeta{
				VrfId: ns.Status.NamespaceID,
			},
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

	hd.Lock()
	hd.DB.InterfaceDB[objectKey(&ns.ObjectMeta)] = ifReqMsg
	hd.Unlock()
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
					Meta: &halproto.ObjectMeta{
						VrfId: ns.Status.NamespaceID,
					},
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
		hd.Lock()
		hd.DB.LifDelDB[objectKey(&intf.ObjectMeta)] = lifDelReqMsg
		delete(hd.DB.LifDB, objectKey(&intf.ObjectMeta))
		hd.Unlock()

		return nil
	case "ENIC":
		fallthrough

	case "UPLINK":
		ifDelReq = &halproto.InterfaceDeleteRequest{
			Meta: &halproto.ObjectMeta{
				VrfId: ns.Status.NamespaceID,
			},
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
	hd.Lock()
	hd.DB.InterfaceDelDB[objectKey(&intf.ObjectMeta)] = ifDelReqMsg
	delete(hd.DB.LifDB, objectKey(&intf.ObjectMeta))
	hd.Unlock()
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
					Meta: &halproto.ObjectMeta{
						VrfId: ns.Status.NamespaceID,
					},
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
		hd.Lock()
		hd.DB.LifDB[objectKey(&ns.ObjectMeta)] = lifReqMsg
		hd.Unlock()
		return nil

	case "UPLINK":
		ifSpec = &halproto.InterfaceSpec{
			Meta: &halproto.ObjectMeta{
				VrfId: ns.Status.NamespaceID,
			},
			KeyOrHandle: &halproto.InterfaceKeyHandle{
				KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
					InterfaceId: intf.Status.InterfaceID,
				},
			},
			Type: halproto.IfType_IF_TYPE_UPLINK,
		}

	case "ENIC":
		ifSpec = &halproto.InterfaceSpec{
			Meta: &halproto.ObjectMeta{
				VrfId: ns.Status.NamespaceID,
			},
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
	hd.Lock()
	hd.DB.InterfaceDB[objectKey(&ns.ObjectMeta)] = ifReqMsg
	hd.Unlock()
	return nil
}

// ListInterfaces returns the lisg of lifs and uplinks from the datapath
func (hd *Datapath) ListInterfaces() (*halproto.LifGetResponseMsg, *halproto.InterfaceGetResponseMsg, error) {
	if hd.Kind == "hal" {
		lifReq := &halproto.LifGetRequest{}
		var uplinks halproto.InterfaceGetResponseMsg
		lifReqMsg := &halproto.LifGetRequestMsg{
			Request: []*halproto.LifGetRequest{
				lifReq,
			},
		}

		// ToDo Add lif checks once nic mgr is integrated
		lifs, err := hd.Hal.Ifclient.LifGet(context.Background(), lifReqMsg)
		if err != nil {
			log.Errorf("Error getting lifs from the datapath. Err: %v", err)
			return nil, nil, nil
		}

		// get all interfaces
		ifReq := &halproto.InterfaceGetRequest{}
		ifReqMsg := &halproto.InterfaceGetRequestMsg{
			Request: []*halproto.InterfaceGetRequest{ifReq},
		}
		intfs, err := hd.Hal.Ifclient.InterfaceGet(context.Background(), ifReqMsg)
		if err != nil {
			return nil, nil, err
		}

		// return only the uplinks
		for _, intf := range intfs.Response {
			if intf.Spec.Type == halproto.IfType_IF_TYPE_UPLINK {
				uplinks.Response = append(uplinks.Response, intf)
			}
		}
		return lifs, &uplinks, err
	}

	// ToDo Remove the List Mock prior to FCS
	lifs, uplinks, err := generateMockHwState()
	return lifs, uplinks, err
}
