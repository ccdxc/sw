package datapath

import (
	"context"

	"strings"

	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/venice/utils/log"
)

// CreateVrf creates a vrf
func (hd *Datapath) CreateVrf(vrfID uint64, vrfType string) error {
	var vrf halproto.VrfType
	if strings.ToLower(vrfType) == "infra" {
		vrf = halproto.VrfType_VRF_TYPE_INFRA
	} else {
		vrf = halproto.VrfType_VRF_TYPE_CUSTOMER
	}

	vrfSpec := halproto.VrfSpec{
		Meta: &halproto.ObjectMeta{
			VrfId: vrfID,
		},
		KeyOrHandle: &halproto.VrfKeyHandle{
			KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
				VrfId: vrfID,
			},
		},
		// All tenant creates are currently customer type as we don't intend to expose infra vrf creates to the user.
		VrfType: vrf,
	}
	vrfReqMsg := halproto.VrfRequestMsg{
		Request: []*halproto.VrfSpec{&vrfSpec},
	}

	// create the tenant. Enforce HAL Status == OK for HAL datapath
	if hd.Kind == "hal" {
		resp, err := hd.Hal.Tnclient.VrfCreate(context.Background(), &vrfReqMsg)
		if err != nil {
			log.Errorf("Error creating tenant. Err: %v", err)
			return err
		}
		if resp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus)
			return ErrHALNotOK
		}
	} else {
		_, err := hd.Hal.Tnclient.VrfCreate(context.Background(), &vrfReqMsg)
		if err != nil {
			log.Errorf("Error creating tenant. Err: %v", err)
			return err
		}
	}

	return nil
}

// DeleteVrf deletes a vrf
func (hd *Datapath) DeleteVrf(vrfID uint64) error {

	vrfDelReq := halproto.VrfDeleteRequest{
		Meta: &halproto.ObjectMeta{
			VrfId: vrfID,
		},
	}

	vrfDelReqMsg := halproto.VrfDeleteRequestMsg{
		Request: []*halproto.VrfDeleteRequest{&vrfDelReq},
	}

	// delete the tenant
	_, err := hd.Hal.Tnclient.VrfDelete(context.Background(), &vrfDelReqMsg)
	if err != nil {
		log.Errorf("Error deleting tenant. Err: %v", err)
		return err
	}

	return nil
}

// UpdateVrf deletes a tenant
func (hd *Datapath) UpdateVrf(vrfID uint64) error {
	vrfSpec := halproto.VrfSpec{
		Meta: &halproto.ObjectMeta{
			VrfId: vrfID,
		},
	}
	vrfReqMsg := halproto.VrfRequestMsg{
		Request: []*halproto.VrfSpec{&vrfSpec},
	}

	// update the tenant
	_, err := hd.Hal.Tnclient.VrfUpdate(context.Background(), &vrfReqMsg)
	if err != nil {
		log.Errorf("Error deleting tenant. Err: %v", err)
		return err
	}

	return nil
}
