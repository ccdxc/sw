package datapath

import (
	"context"
	"fmt"

	"github.com/pensando/sw/nic/agent/dscagent/types/irisproto"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// CreatePort will create a Port Object in HAL
func (hd *Datapath) CreatePort(ports ...*netproto.Port) error {
	var req []*halproto.PortSpec
	for _, p := range ports {
		portSpeed, autoNegEnable := hd.convertPortSpeed(p.Spec.Speed)
		portType, fecType := hd.convertPortTypeFec(p.Spec.Type)
		halPortSpec := halproto.PortSpec{
			KeyOrHandle: &halproto.PortKeyHandle{
				KeyOrHandle: &halproto.PortKeyHandle_PortId{
					PortId: uint32(p.Status.PortID),
				},
			},
			AdminState:    halproto.PortAdminState(halproto.PortAdminState_value[p.Spec.AdminStatus]),
			PortSpeed:     portSpeed,
			PortType:      portType,
			NumLanes:      p.Spec.Lanes,
			AutoNegEnable: autoNegEnable,
			FecType:       fecType,
			Pause:         halproto.PortPauseType_PORT_PAUSE_TYPE_LINK,
			TxPauseEnable: true,
			RxPauseEnable: true,
		}

		req = append(req, &halPortSpec)

	}
	portReqMsg := &halproto.PortRequestMsg{
		Request: req,
	}

	// create port object
	if hd.Kind == "hal" {
		resp, err := hd.Hal.PortClient.PortCreate(context.Background(), portReqMsg)
		if err != nil {
			log.Errorf("Error creating port. Err: %v", err)
			return err
		}
		if !(resp.Response[0].ApiStatus == halproto.ApiStatus_API_STATUS_OK || resp.Response[0].ApiStatus == halproto.ApiStatus_API_STATUS_EXISTS_ALREADY) {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
			return fmt.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
		}
		log.Infof("Received RESP from HAL: %v", resp.Response[0])
		//port.Status.OperStatus = convertPortOperState(resp.Response[0].Status.OperStatus)
	} else {
		_, err := hd.Hal.PortClient.PortCreate(context.Background(), portReqMsg)
		if err != nil {
			log.Errorf("Error creating port. Err: %v", err)
			return err
		}
	}
	return nil
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
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
			return port, fmt.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
		}
		//port.Status.OperStatus = convertPortOperState(resp.Response[0].Status.OperStatus)
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

func convertPortOperState(portOperStatus halproto.PortOperState) (status string) {
	switch portOperStatus {
	case halproto.PortOperState_PORT_OPER_STATUS_UP:
		status = "UP"
	case halproto.PortOperState_PORT_OPER_STATUS_DOWN:
		status = "DOWN"
	}
	return
}
