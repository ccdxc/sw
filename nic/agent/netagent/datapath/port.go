package datapath

import (
	"context"

	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/nic/agent/netagent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// CreatePort will create a Port Object in HAL
func (hd *Datapath) CreatePort(port *netproto.Port) (*netproto.Port, error) {
	portSpeed, autoNegEnable := hd.convertPortSpeed(port.Spec.Speed)
	portType, fecType := hd.convertPortTypeFec(port.Spec.Type)
	portReqMsg := &halproto.PortRequestMsg{
		Request: []*halproto.PortSpec{
			{
				KeyOrHandle: &halproto.PortKeyHandle{
					KeyOrHandle: &halproto.PortKeyHandle_PortId{
						PortId: uint32(port.Status.PortID),
					},
				},
				AdminState:    halproto.PortAdminState_PORT_ADMIN_STATE_UP,
				PortSpeed:     portSpeed,
				PortType:      portType,
				NumLanes:      port.Spec.Lanes,
				AutoNegEnable: autoNegEnable,
				FecType:       fecType,
			},
		},
	}

	// create port object
	if hd.Kind == "hal" {
		resp, err := hd.Hal.PortClient.PortCreate(context.Background(), portReqMsg)
		if err != nil {
			log.Errorf("Error creating port. Err: %v", err)
			return port, err
		}
		if resp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus)
			return port, ErrHALNotOK
		}
		log.Infof("Received RESP from HAL: %v", resp.Response[0])
		//port.Status.OperStatus = convertPortOperStatus(resp.Response[0].Status.OperStatus)
	} else {
		_, err := hd.Hal.PortClient.PortCreate(context.Background(), portReqMsg)
		if err != nil {
			log.Errorf("Error creating port. Err: %v", err)
			return port, err
		}
	}
	return port, nil
}

// UpdatePort updates a port in the datapath
func (hd *Datapath) UpdatePort(port *netproto.Port) (*netproto.Port, error) {
	portSpeed, autoNegEnable := hd.convertPortSpeed(port.Spec.Speed)
	_, fecType := hd.convertPortTypeFec(port.Spec.Type)

	portUpdateReqMsg := &halproto.PortRequestMsg{
		Request: []*halproto.PortSpec{
			{
				KeyOrHandle: &halproto.PortKeyHandle{
					KeyOrHandle: &halproto.PortKeyHandle_PortId{
						PortId: uint32(port.Status.PortID),
					},
				},
				AdminState:    halproto.PortAdminState_PORT_ADMIN_STATE_UP,
				PortSpeed:     portSpeed,
				AutoNegEnable: autoNegEnable,
				FecType:       fecType,
			},
		},
	}
	// update port object
	if hd.Kind == "hal" {
		resp, err := hd.Hal.PortClient.PortUpdate(context.Background(), portUpdateReqMsg)
		if err != nil {
			log.Errorf("Error updating port. Err: %v", err)
			return port, err
		}
		if resp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus)
			return port, ErrHALNotOK
		}
		//port.Status.OperStatus = convertPortOperStatus(resp.Response[0].Status.OperStatus)
	} else {
		_, err := hd.Hal.PortClient.PortUpdate(context.Background(), portUpdateReqMsg)
		if err != nil {
			log.Errorf("Error updating port. Err: %v", err)
			return port, err
		}
	}
	return port, nil
}

// DeletePort deletes a port in the datapath
func (hd *Datapath) DeletePort(port *netproto.Port) error {
	return nil
}

func convertPortOperStatus(portOperStatus halproto.PortOperStatus) (status string) {
	switch portOperStatus {
	case halproto.PortOperStatus_PORT_OPER_STATUS_UP:
		status = "UP"
	case halproto.PortOperStatus_PORT_OPER_STATUS_DOWN:
		status = "DOWN"
	}
	return
}
