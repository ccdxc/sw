package datapath

import (
	"context"
	"errors"
	"fmt"
	"net"
	"regexp"
	"strconv"

	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/nic/agent/netagent/state/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// --------------------------- Local Endpoint CRUDs --------------------------- //
var macStripRegexp = regexp.MustCompile(`[^a-fA-F0-9]`)

// CreateLocalEndpoint creates a local endpoint in the datapath
func (hd *Datapath) CreateLocalEndpoint(ep *netproto.Endpoint, nw *netproto.Network, lifID, enicID uint64, vrf *netproto.Vrf) (*types.IntfInfo, error) {
	// This will ensure that only one datapath config will be active at a time. This is a temporary restriction
	// to ensure that HAL will use a single config thread , this will be removed prior to FCS to allow parallel configs to go through.
	// TODO Remove Global Locking
	hd.Lock()
	defer hd.Unlock()
	var halIPAddresses []*halproto.IPAddress
	var lifHandle *halproto.LifKeyHandle

	// convert mac address
	hex := macStripRegexp.ReplaceAllLiteralString(ep.Spec.MacAddress, "")
	macaddr, _ := strconv.ParseUint(hex, 16, 64)

	for _, address := range ep.Spec.IPv4Addresses {
		if len(address) == 0 {
			continue
		}

		ipaddr, _, err := net.ParseCIDR(address)
		if err != nil {
			return nil, fmt.Errorf("ipv4 address for endpoint creates should be in CIDR format")
		}
		// convert v4 address
		v4Addr := &halproto.IPAddress{
			IpAf: halproto.IPAddressFamily_IP_AF_INET,
			V4OrV6: &halproto.IPAddress_V4Addr{
				V4Addr: ipv4Touint32(ipaddr),
			},
		}

		halIPAddresses = append(halIPAddresses, v4Addr)
	}
	vrfKey := halproto.VrfKeyHandle{
		KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
			VrfId: vrf.Status.VrfID,
		},
	}

	// get sg ids
	var sgHandles []*halproto.SecurityGroupKeyHandle

	l2Key := halproto.L2SegmentKeyHandle{
		KeyOrHandle: &halproto.L2SegmentKeyHandle_SegmentId{
			SegmentId: nw.Status.NetworkID,
		},
	}

	ifKey := halproto.InterfaceKeyHandle{
		KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
			InterfaceId: enicID,
		},
	}

	epAttrs := halproto.EndpointAttributes{
		InterfaceKeyHandle: &ifKey,
		UsegVlan:           ep.Spec.UsegVlan,
		IpAddress:          halIPAddresses,
		SgKeyHandle:        sgHandles,
	}

	epHandle := halproto.EndpointKeyHandle{
		KeyOrHandle: &halproto.EndpointKeyHandle_EndpointKey{
			EndpointKey: &halproto.EndpointKey{
				EndpointL2L3Key: &halproto.EndpointKey_L2Key{
					L2Key: &halproto.EndpointL2Key{
						L2SegmentKeyHandle: &l2Key,
						MacAddress:         macaddr,
					},
				},
			},
		},
	}

	if lifID != 0 {
		lifHandle = &halproto.LifKeyHandle{
			KeyOrHandle: &halproto.LifKeyHandle_LifId{
				LifId: lifID,
			},
		}
	}

	// build enic message
	enicSpec := &halproto.InterfaceSpec{
		KeyOrHandle: &halproto.InterfaceKeyHandle{
			KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
				InterfaceId: enicID,
			},
		},
		Type: halproto.IfType_IF_TYPE_ENIC,
		// associate the lif id
		IfInfo: &halproto.InterfaceSpec_IfEnicInfo{
			IfEnicInfo: &halproto.IfEnicInfo{
				EnicType:       halproto.IfEnicType_IF_ENIC_TYPE_USEG,
				LifKeyOrHandle: lifHandle,
				EnicTypeInfo: &halproto.IfEnicInfo_EnicInfo{
					EnicInfo: &halproto.EnicInfo{
						L2SegmentKeyHandle: &l2Key,
						MacAddress:         macaddr,
						EncapVlanId:        ep.Spec.UsegVlan,
					},
				},
			},
		},
	}

	enicReqMsg := &halproto.InterfaceRequestMsg{
		Request: []*halproto.InterfaceSpec{
			enicSpec,
		},
	}

	// create enic
	if hd.Kind == "hal" {
		resp, err := hd.Hal.Ifclient.InterfaceCreate(context.Background(), enicReqMsg)
		if err != nil {
			log.Errorf("Error creating interface. Err: %v", err)
			return nil, err
		}
		if !(resp.Response[0].ApiStatus == halproto.ApiStatus_API_STATUS_OK || resp.Response[0].ApiStatus == halproto.ApiStatus_API_STATUS_EXISTS_ALREADY) {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
			return nil, fmt.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
		}
	} else {
		_, err := hd.Hal.Ifclient.InterfaceCreate(context.Background(), enicReqMsg)
		if err != nil {
			log.Errorf("Error creating interface. Err: %v", err)
			return nil, err
		}
	}

	// build endpoint message
	epinfo := halproto.EndpointSpec{
		KeyOrHandle:   &epHandle,
		EndpointAttrs: &epAttrs,
		VrfKeyHandle:  &vrfKey,
	}
	epReq := halproto.EndpointRequestMsg{
		Request: []*halproto.EndpointSpec{&epinfo},
	}

	// create endpoint
	if hd.Kind == "hal" {
		resp, err := hd.Hal.Epclient.EndpointCreate(context.Background(), &epReq)
		if err != nil {
			log.Errorf("Error creating endpoint. Err: %v", err)
			return nil, err
		}
		if !(resp.Response[0].ApiStatus == halproto.ApiStatus_API_STATUS_OK || resp.Response[0].ApiStatus == halproto.ApiStatus_API_STATUS_EXISTS_ALREADY) {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
			return nil, fmt.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
		}
	} else {
		_, err := hd.Hal.Epclient.EndpointCreate(context.Background(), &epReq)
		if err != nil {
			log.Errorf("Error creating endpoint. Err: %v", err)
			return nil, err
		}
	}

	// save the endpoint message
	//hd.Lock()
	hd.DB.EndpointDB[objectKey(&ep.ObjectMeta)] = &epReq
	//hd.Unlock()

	return nil, nil
}

// UpdateLocalEndpoint updates the endpoint
func (hd *Datapath) UpdateLocalEndpoint(ep *netproto.Endpoint, nw *netproto.Network, lifID, enicID uint64, vrf *netproto.Vrf) error {
	// This will ensure that only one datapath config will be active at a time. This is a temporary restriction
	// to ensure that HAL will use a single config thread , this will be removed prior to FCS to allow parallel configs to go through.
	// TODO Remove Global Locking
	hd.Lock()
	defer hd.Unlock()
	var halIPAddresses []*halproto.IPAddress
	var lifHandle *halproto.LifKeyHandle

	// convert mac address
	hex := macStripRegexp.ReplaceAllLiteralString(ep.Spec.MacAddress, "")
	macaddr, _ := strconv.ParseUint(hex, 16, 64)

	for _, address := range ep.Spec.IPv4Addresses {
		if len(address) == 0 {
			continue
		}

		ipaddr, _, err := net.ParseCIDR(address)
		if err != nil {
			return fmt.Errorf("ipv4 address for endpoint creates should be in CIDR format")
		}
		// convert v4 address
		v4Addr := &halproto.IPAddress{
			IpAf: halproto.IPAddressFamily_IP_AF_INET,
			V4OrV6: &halproto.IPAddress_V4Addr{
				V4Addr: ipv4Touint32(ipaddr),
			},
		}

		halIPAddresses = append(halIPAddresses, v4Addr)
	}

	vrfKey := halproto.VrfKeyHandle{
		KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
			VrfId: vrf.Status.VrfID,
		},
	}

	// get sg ids

	l2Key := halproto.L2SegmentKeyHandle{
		KeyOrHandle: &halproto.L2SegmentKeyHandle_SegmentId{
			SegmentId: nw.Status.NetworkID,
		},
	}

	ifKey := halproto.InterfaceKeyHandle{
		KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
			InterfaceId: enicID,
		},
	}

	epAttrs := halproto.EndpointAttributes{
		InterfaceKeyHandle: &ifKey,
		UsegVlan:           ep.Spec.UsegVlan,
		IpAddress:          halIPAddresses,
	}

	epHandle := halproto.EndpointKeyHandle{
		KeyOrHandle: &halproto.EndpointKeyHandle_EndpointKey{
			EndpointKey: &halproto.EndpointKey{
				EndpointL2L3Key: &halproto.EndpointKey_L2Key{
					L2Key: &halproto.EndpointL2Key{
						L2SegmentKeyHandle: &l2Key,
						MacAddress:         macaddr,
					},
				},
			},
		},
	}

	if lifID != 0 {
		lifHandle = &halproto.LifKeyHandle{
			KeyOrHandle: &halproto.LifKeyHandle_LifId{
				LifId: lifID,
			},
		}
	}

	// build enic message
	enicSpec := &halproto.InterfaceSpec{
		KeyOrHandle: &halproto.InterfaceKeyHandle{
			KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
				InterfaceId: enicID,
			},
		},
		Type: halproto.IfType_IF_TYPE_ENIC,
		// associate the lif id
		IfInfo: &halproto.InterfaceSpec_IfEnicInfo{
			IfEnicInfo: &halproto.IfEnicInfo{
				EnicType:       halproto.IfEnicType_IF_ENIC_TYPE_USEG,
				LifKeyOrHandle: lifHandle,
				EnicTypeInfo: &halproto.IfEnicInfo_EnicInfo{
					EnicInfo: &halproto.EnicInfo{
						L2SegmentKeyHandle: &l2Key,
						MacAddress:         macaddr,
						EncapVlanId:        ep.Spec.UsegVlan,
					},
				},
			},
		},
	}

	enicReqMsg := &halproto.InterfaceRequestMsg{
		Request: []*halproto.InterfaceSpec{
			enicSpec,
		},
	}

	// create enic
	if hd.Kind == "hal" {
		resp, err := hd.Hal.Ifclient.InterfaceUpdate(context.Background(), enicReqMsg)
		if err != nil {
			log.Errorf("Error updating interface. Err: %v", err)
			return err
		}
		if !(resp.Response[0].ApiStatus == halproto.ApiStatus_API_STATUS_OK || resp.Response[0].ApiStatus == halproto.ApiStatus_API_STATUS_EXISTS_ALREADY) {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
			return fmt.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
		}
	} else {
		_, err := hd.Hal.Ifclient.InterfaceUpdate(context.Background(), enicReqMsg)
		if err != nil {
			log.Errorf("Error updating interface. Err: %v", err)
			return err
		}
	}

	// build endpoint message
	epinfo := halproto.EndpointUpdateRequest{
		KeyOrHandle:   &epHandle,
		EndpointAttrs: &epAttrs,
		VrfKeyHandle:  &vrfKey,
	}
	epReq := halproto.EndpointUpdateRequestMsg{
		Request: []*halproto.EndpointUpdateRequest{&epinfo},
	}

	// create endpoint
	if hd.Kind == "hal" {
		resp, err := hd.Hal.Epclient.EndpointUpdate(context.Background(), &epReq)
		if err != nil {
			log.Errorf("Error updating endpoint. Err: %v", err)
			return err
		}
		if !(resp.Response[0].ApiStatus == halproto.ApiStatus_API_STATUS_OK || resp.Response[0].ApiStatus == halproto.ApiStatus_API_STATUS_EXISTS_ALREADY) {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
			return fmt.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
		}
	} else {
		_, err := hd.Hal.Epclient.EndpointUpdate(context.Background(), &epReq)
		if err != nil {
			log.Errorf("Error updating endpoint. Err: %v", err)
			return err
		}
	}
	return nil
}

// DeleteLocalEndpoint deletes an endpoint
func (hd *Datapath) DeleteLocalEndpoint(ep *netproto.Endpoint, nw *netproto.Network, enicID uint64) error {
	// This will ensure that only one datapath config will be active at a time. This is a temporary restriction
	// to ensure that HAL will use a single config thread , this will be removed prior to FCS to allow parallel configs to go through.
	// TODO Remove Global Locking
	hd.Lock()
	defer hd.Unlock()
	var macStripRegexp = regexp.MustCompile(`[^a-fA-F0-9]`)
	hex := macStripRegexp.ReplaceAllLiteralString(ep.Spec.MacAddress, "")
	macaddr, _ := strconv.ParseUint(hex, 16, 64)

	l2Key := &halproto.L2SegmentKeyHandle{
		KeyOrHandle: &halproto.L2SegmentKeyHandle_SegmentId{
			SegmentId: nw.Status.NetworkID,
		},
	}
	epKey := &halproto.EndpointKeyHandle{
		KeyOrHandle: &halproto.EndpointKeyHandle_EndpointKey{
			EndpointKey: &halproto.EndpointKey{
				EndpointL2L3Key: &halproto.EndpointKey_L2Key{
					L2Key: &halproto.EndpointL2Key{
						L2SegmentKeyHandle: l2Key,
						MacAddress:         macaddr,
					},
				},
			},
		},
	}
	epDel := []*halproto.EndpointDeleteRequest{
		{
			DeleteBy: &halproto.EndpointDeleteRequest_KeyOrHandle{
				KeyOrHandle: epKey,
			},
		},
	}

	epDelReq := &halproto.EndpointDeleteRequestMsg{
		Request: epDel,
	}

	// call hal to delete the endpoint
	if hd.Kind == "hal" {
		resp, err := hd.Hal.Epclient.EndpointDelete(context.Background(), epDelReq)
		if err != nil {
			log.Errorf("Error deleting local endpoint. Err: %v", err)
			return err
		}
		if !(resp.Response[0].ApiStatus == halproto.ApiStatus_API_STATUS_OK || resp.Response[0].ApiStatus == halproto.ApiStatus_API_STATUS_NOT_FOUND) {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
			return fmt.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
		}
	} else {
		_, err := hd.Hal.Epclient.EndpointDelete(context.Background(), epDelReq)
		if err != nil {
			log.Errorf("Error deleting local endpoint. Err: %v", err)
			return err
		}
	}

	// delete the associated enic
	enicKey := &halproto.InterfaceKeyHandle{
		KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
			InterfaceId: enicID,
		},
	}

	enicDelReq := &halproto.InterfaceDeleteRequestMsg{
		Request: []*halproto.InterfaceDeleteRequest{
			{
				KeyOrHandle: enicKey,
			},
		},
	}

	// call hal to delete the enic
	if hd.Kind == "hal" {
		resp, err := hd.Hal.Ifclient.InterfaceDelete(context.Background(), enicDelReq)
		if err != nil {
			log.Errorf("Error deleting enic. Err: %v", err)
			return err
		}
		if !(resp.Response[0].ApiStatus == halproto.ApiStatus_API_STATUS_OK || resp.Response[0].ApiStatus == halproto.ApiStatus_API_STATUS_NOT_FOUND) {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
			return fmt.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
		}
	} else {
		_, err := hd.Hal.Ifclient.InterfaceDelete(context.Background(), enicDelReq)
		if err != nil {
			log.Errorf("Error deleting enic. Err: %v", err)
			return err
		}
	}

	// save the endpoint delete message
	//hd.Lock()
	hd.DB.EndpointDelDB[objectKey(&ep.ObjectMeta)] = epDelReq
	delete(hd.DB.EndpointDB, objectKey(&ep.ObjectMeta))
	//hd.Unlock()

	return nil
}

// --------------------------- Remote Endpoint CRUDs --------------------------- //

// CreateRemoteEndpoint creates remote endpoint
func (hd *Datapath) CreateRemoteEndpoint(ep *netproto.Endpoint, nw *netproto.Network, uplinkID uint64, vrf *netproto.Vrf) error {
	// This will ensure that only one datapath config will be active at a time. This is a temporary restriction
	// to ensure that HAL will use a single config thread , this will be removed prior to FCS to allow parallel configs to go through.
	// TODO Remove Global Locking
	hd.Lock()
	defer hd.Unlock()
	var halIPAddresses []*halproto.IPAddress
	var ifKey *halproto.InterfaceKeyHandle

	// convert mac address
	var macStripRegexp = regexp.MustCompile(`[^a-fA-F0-9]`)
	hex := macStripRegexp.ReplaceAllLiteralString(ep.Spec.MacAddress, "")
	macaddr, _ := strconv.ParseUint(hex, 16, 64)

	for _, address := range ep.Spec.IPv4Addresses {
		if len(address) == 0 {
			continue
		}

		ipaddr, _, err := net.ParseCIDR(address)
		if err != nil {
			return fmt.Errorf("ipv4 address for endpoint creates should be in CIDR format")
		}
		// convert v4 address
		v4Addr := &halproto.IPAddress{
			IpAf: halproto.IPAddressFamily_IP_AF_INET,
			V4OrV6: &halproto.IPAddress_V4Addr{
				V4Addr: ipv4Touint32(ipaddr),
			},
		}

		halIPAddresses = append(halIPAddresses, v4Addr)
	}

	l2Key := halproto.L2SegmentKeyHandle{
		KeyOrHandle: &halproto.L2SegmentKeyHandle_SegmentId{
			SegmentId: nw.Status.NetworkID,
		},
	}

	if uplinkID != 0 {
		ifKey = &halproto.InterfaceKeyHandle{
			KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
				InterfaceId: uplinkID,
			},
		}
	}

	epAttrs := halproto.EndpointAttributes{
		InterfaceKeyHandle: ifKey,
		UsegVlan:           ep.Spec.UsegVlan,
		IpAddress:          halIPAddresses,
	}

	epHandle := halproto.EndpointKeyHandle{
		KeyOrHandle: &halproto.EndpointKeyHandle_EndpointKey{
			EndpointKey: &halproto.EndpointKey{
				EndpointL2L3Key: &halproto.EndpointKey_L2Key{
					L2Key: &halproto.EndpointL2Key{
						L2SegmentKeyHandle: &l2Key,
						MacAddress:         macaddr,
					},
				},
			},
		},
	}

	// build endpoint message
	epinfo := halproto.EndpointSpec{
		KeyOrHandle:   &epHandle,
		EndpointAttrs: &epAttrs,
		VrfKeyHandle: &halproto.VrfKeyHandle{
			KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
				VrfId: vrf.Status.VrfID,
			},
		},
	}
	epReq := halproto.EndpointRequestMsg{
		Request: []*halproto.EndpointSpec{&epinfo},
	}

	// call hal to create the endpoint
	if hd.Kind == "hal" {
		resp, err := hd.Hal.Epclient.EndpointCreate(context.Background(), &epReq)
		if err != nil {
			log.Errorf("Error creating endpoint. Err: %v", err)
			return err
		}
		if !(resp.Response[0].ApiStatus == halproto.ApiStatus_API_STATUS_OK || resp.Response[0].ApiStatus == halproto.ApiStatus_API_STATUS_EXISTS_ALREADY) {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
			return fmt.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
		}
	} else {
		_, err := hd.Hal.Epclient.EndpointCreate(context.Background(), &epReq)
		if err != nil {
			log.Errorf("Error creating endpoint. Err: %v", err)
			return err
		}
	}

	// save the endpoint message
	//hd.Lock()
	hd.DB.EndpointDB[objectKey(&ep.ObjectMeta)] = &epReq
	//hd.Unlock()

	return nil
}

// UpdateRemoteEndpoint updates an existing endpoint
func (hd *Datapath) UpdateRemoteEndpoint(ep *netproto.Endpoint, nw *netproto.Network) error {
	// This will ensure that only one datapath config will be active at a time. This is a temporary restriction
	// to ensure that HAL will use a single config thread , this will be removed prior to FCS to allow parallel configs to go through.
	// TODO Remove Global Locking
	hd.Lock()
	defer hd.Unlock()
	var halIPAddresses []*halproto.IPAddress
	for _, address := range ep.Spec.IPv4Addresses {
		if len(address) == 0 {
			continue
		}

		ipaddr, _, err := net.ParseCIDR(address)
		if err != nil {
			return fmt.Errorf("ipv4 address for endpoint creates should be in CIDR format")
		}
		// convert v4 address
		v4Addr := &halproto.IPAddress{
			IpAf: halproto.IPAddressFamily_IP_AF_INET,
			V4OrV6: &halproto.IPAddress_V4Addr{
				V4Addr: ipv4Touint32(ipaddr),
			},
		}

		halIPAddresses = append(halIPAddresses, v4Addr)
	}

	l2Handle := halproto.L2SegmentKeyHandle{
		KeyOrHandle: &halproto.L2SegmentKeyHandle_L2SegmentHandle{
			L2SegmentHandle: nw.Status.NetworkID,
		},
	}

	ifKeyHandle := halproto.InterfaceKeyHandle{
		KeyOrHandle: &halproto.InterfaceKeyHandle_IfHandle{
			IfHandle: 0, //FIXME
		},
	}

	epAttrs := halproto.EndpointAttributes{
		InterfaceKeyHandle: &ifKeyHandle, //FIXME
		UsegVlan:           ep.Spec.UsegVlan,
		IpAddress:          halIPAddresses,
	}

	epHandle := halproto.EndpointKeyHandle{
		KeyOrHandle: &halproto.EndpointKeyHandle_EndpointKey{
			EndpointKey: &halproto.EndpointKey{
				EndpointL2L3Key: &halproto.EndpointKey_L2Key{
					L2Key: &halproto.EndpointL2Key{
						L2SegmentKeyHandle: &l2Handle,
					},
				},
			},
		},
	}

	// build endpoint message
	epUpdateReq := halproto.EndpointUpdateRequest{
		KeyOrHandle:   &epHandle,
		EndpointAttrs: &epAttrs,
	}

	epUpdateReqMsg := halproto.EndpointUpdateRequestMsg{
		Request: []*halproto.EndpointUpdateRequest{&epUpdateReq},
	}

	// call hal to update the endpoint
	// FIXME: handle response
	_, err := hd.Hal.Epclient.EndpointUpdate(context.Background(), &epUpdateReqMsg)
	if err != nil {
		log.Errorf("Error updating endpoint. Err: %v", err)
		return err
	}

	// save the endpoint message
	//hd.Lock()
	hd.DB.EndpointUpdateDB[objectKey(&ep.ObjectMeta)] = &epUpdateReqMsg
	//hd.Unlock()

	return nil
}

// DeleteRemoteEndpoint deletes remote endpoint
func (hd *Datapath) DeleteRemoteEndpoint(ep *netproto.Endpoint, nw *netproto.Network) error {
	// This will ensure that only one datapath config will be active at a time. This is a temporary restriction
	// to ensure that HAL will use a single config thread , this will be removed prior to FCS to allow parallel configs to go through.
	// TODO Remove Global Locking
	hd.Lock()
	defer hd.Unlock()
	var macStripRegexp = regexp.MustCompile(`[^a-fA-F0-9]`)
	hex := macStripRegexp.ReplaceAllLiteralString(ep.Spec.MacAddress, "")
	macaddr, _ := strconv.ParseUint(hex, 16, 64)

	l2Key := &halproto.L2SegmentKeyHandle{
		KeyOrHandle: &halproto.L2SegmentKeyHandle_SegmentId{
			SegmentId: nw.Status.NetworkID,
		},
	}
	epKey := &halproto.EndpointKeyHandle{
		KeyOrHandle: &halproto.EndpointKeyHandle_EndpointKey{
			EndpointKey: &halproto.EndpointKey{
				EndpointL2L3Key: &halproto.EndpointKey_L2Key{
					L2Key: &halproto.EndpointL2Key{
						L2SegmentKeyHandle: l2Key,
						MacAddress:         macaddr,
					},
				},
			},
		},
	}
	epDel := []*halproto.EndpointDeleteRequest{
		{
			DeleteBy: &halproto.EndpointDeleteRequest_KeyOrHandle{
				KeyOrHandle: epKey,
			},
		},
	}

	epDelReq := &halproto.EndpointDeleteRequestMsg{
		Request: epDel,
	}

	// call hal to delete the endpoint
	if hd.Kind == "hal" {
		resp, err := hd.Hal.Epclient.EndpointDelete(context.Background(), epDelReq)
		if err != nil {
			log.Errorf("Error deleting remote endpoint. Err: %v", err)
			return err
		}
		if !(resp.Response[0].ApiStatus == halproto.ApiStatus_API_STATUS_OK || resp.Response[0].ApiStatus == halproto.ApiStatus_API_STATUS_NOT_FOUND) {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
			return fmt.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
		}
	} else {
		_, err := hd.Hal.Epclient.EndpointDelete(context.Background(), epDelReq)
		if err != nil {
			log.Errorf("Error deleting remote endpoint. Err: %v", err)
			return err
		}
	}

	// save the endpoint delete message
	//hd.Lock()
	hd.DB.EndpointDelDB[objectKey(&ep.ObjectMeta)] = epDelReq
	delete(hd.DB.EndpointDB, objectKey(&ep.ObjectMeta))
	//hd.Unlock()

	return nil
}

// --------------------------- Endpoint Test utilities --------------------------- //

// FindEndpoint finds an endpoint in datapath
// used for testing mostly..
func (hd *Datapath) FindEndpoint(epKey string) (*halproto.EndpointRequestMsg, error) {
	hd.Lock()
	epr, ok := hd.DB.EndpointDB[epKey]
	hd.Unlock()
	if !ok {
		return nil, errors.New("endpoint not found")
	}

	return epr, nil
}

// FindEndpointDel finds an endpoint delete record
// used for testing mostly
func (hd *Datapath) FindEndpointDel(epKey string) (*halproto.EndpointDeleteRequestMsg, error) {
	hd.Lock()
	epdr, ok := hd.DB.EndpointDelDB[epKey]
	hd.Unlock()
	if !ok {
		return nil, errors.New("endpoint delete record not found")
	}

	return epdr, nil
}

// GetEndpointCount returns number of endpoints in db
func (hd *Datapath) GetEndpointCount() int {
	hd.Lock()
	defer hd.Unlock()
	return len(hd.DB.EndpointDB)
}
