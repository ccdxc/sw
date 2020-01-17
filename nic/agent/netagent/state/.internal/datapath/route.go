// +build ignore

package datapath

import (
	"context"
	"fmt"
	"net"

	"github.com/pensando/sw/nic/agent/dscagent/types/irisproto"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// CreateRoute creates a Route in the datapath
func (hd *Datapath) CreateRoute(rt *netproto.Route, vrf *netproto.Vrf) error {
	// This will ensure that only one datapath config will be active at a time. This is a temporary restriction
	// to ensure that HAL will use a single config thread , this will be removed prior to FCS to allow parallel configs to go through.
	// TODO Remove Global Locking
	hd.Lock()
	defer hd.Unlock()
	vrfKey := &halproto.VrfKeyHandle{
		KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
			VrfId: vrf.Status.VrfID,
		},
	}

	// Build next hop object
	nextHopKey := &halproto.NexthopKeyHandle{
		KeyOrHandle: &halproto.NexthopKeyHandle_NexthopId{
			NexthopId: rt.Status.RouteID,
		},
	}

	epIP := net.ParseIP(rt.Spec.GatewayIP)
	if len(epIP) == 0 {
		return fmt.Errorf("could not parse IP from %v", rt.Spec.GatewayIP)
	}

	gwIPAddr := &halproto.IPAddress{
		IpAf: halproto.IPAddressFamily_IP_AF_INET,
		V4OrV6: &halproto.IPAddress_V4Addr{
			V4Addr: ipv4Touint32(epIP),
		},
	}

	epKey := &halproto.EndpointKeyHandle{
		KeyOrHandle: &halproto.EndpointKeyHandle_EndpointKey{
			EndpointKey: &halproto.EndpointKey{
				EndpointL2L3Key: &halproto.EndpointKey_L3Key{
					L3Key: &halproto.EndpointL3Key{
						VrfKeyHandle: vrfKey,
						IpAddress:    gwIPAddr,
					},
				},
			},
		},
	}

	nextHopReqMsg := &halproto.NexthopRequestMsg{
		Request: []*halproto.NexthopSpec{
			{
				KeyOrHandle: nextHopKey,
				IfOrEp: &halproto.NexthopSpec_EpKeyOrHandle{
					EpKeyOrHandle: epKey,
				},
			},
		},
	}

	// create nexthop object
	if hd.Kind == "hal" {
		resp, err := hd.Hal.Netclient.NexthopCreate(context.Background(), nextHopReqMsg)
		if err != nil {
			log.Errorf("Error creating next hop. Err: %v", err)
			return err
		}
		if !(resp.Response[0].ApiStatus == halproto.ApiStatus_API_STATUS_OK || resp.Response[0].ApiStatus == halproto.ApiStatus_API_STATUS_EXISTS_ALREADY) {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
			return fmt.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
		}
	} else {
		_, err := hd.Hal.Netclient.NexthopCreate(context.Background(), nextHopReqMsg)
		if err != nil {
			log.Errorf("Error creating next Hop Err: %v", err)
			return err
		}
	}

	// build route object
	ip, net, err := net.ParseCIDR(rt.Spec.IPPrefix)
	if err != nil {
		return fmt.Errorf("error parsing the IP Prefix mask from %v. Err: %v", rt.Spec.IPPrefix, err)

	}
	prefixLen, _ := net.Mask.Size()
	ipPrefix := &halproto.IPPrefix{
		Address: &halproto.IPAddress{
			IpAf: halproto.IPAddressFamily_IP_AF_INET,
			V4OrV6: &halproto.IPAddress_V4Addr{
				V4Addr: ipv4Touint32(ip),
			},
		},
		PrefixLen: uint32(prefixLen),
	}

	routeReqMsg := &halproto.RouteRequestMsg{
		Request: []*halproto.RouteSpec{
			{
				KeyOrHandle: &halproto.RouteKeyHandle{
					KeyOrHandle: &halproto.RouteKeyHandle_RouteKey{
						RouteKey: &halproto.RouteKey{
							VrfKeyHandle: vrfKey,
							IpPrefix:     ipPrefix,
						},
					},
				},
				NhKeyOrHandle: nextHopKey,
			},
		},
	}

	// create route object
	if hd.Kind == "hal" {
		resp, err := hd.Hal.Netclient.RouteCreate(context.Background(), routeReqMsg)
		if err != nil {
			log.Errorf("Error creating route. Err: %v", err)
			return err
		}
		if !(resp.Response[0].ApiStatus == halproto.ApiStatus_API_STATUS_OK || resp.Response[0].ApiStatus == halproto.ApiStatus_API_STATUS_EXISTS_ALREADY) {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
			return fmt.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
		}
	} else {
		_, err := hd.Hal.Netclient.RouteCreate(context.Background(), routeReqMsg)
		if err != nil {
			log.Errorf("Error creating route. Err: %v", err)
			return err
		}
	}
	return nil
}

// UpdateRoute updates a Route in the datapath
func (hd *Datapath) UpdateRoute(rt *netproto.Route, vrf *netproto.Vrf) error {
	// This will ensure that only one datapath config will be active at a time. This is a temporary restriction
	// to ensure that HAL will use a single config thread , this will be removed prior to FCS to allow parallel configs to go through.
	// TODO Remove Global Locking
	hd.Lock()
	defer hd.Unlock()
	vrfKey := &halproto.VrfKeyHandle{
		KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
			VrfId: vrf.Status.VrfID,
		},
	}

	// Build next hop object
	nextHopKey := &halproto.NexthopKeyHandle{
		KeyOrHandle: &halproto.NexthopKeyHandle_NexthopId{
			NexthopId: rt.Status.RouteID,
		},
	}

	epIP := net.ParseIP(rt.Spec.GatewayIP)
	if len(epIP) == 0 {
		return fmt.Errorf("could not parse IP from %v", rt.Spec.GatewayIP)
	}

	gwIPAddr := &halproto.IPAddress{
		IpAf: halproto.IPAddressFamily_IP_AF_INET,
		V4OrV6: &halproto.IPAddress_V4Addr{
			V4Addr: ipv4Touint32(epIP),
		},
	}

	epKey := &halproto.EndpointKeyHandle{
		KeyOrHandle: &halproto.EndpointKeyHandle_EndpointKey{
			EndpointKey: &halproto.EndpointKey{
				EndpointL2L3Key: &halproto.EndpointKey_L3Key{
					L3Key: &halproto.EndpointL3Key{
						VrfKeyHandle: vrfKey,
						IpAddress:    gwIPAddr,
					},
				},
			},
		},
	}

	nextHopUpdateReqMsg := &halproto.NexthopRequestMsg{
		Request: []*halproto.NexthopSpec{
			{
				KeyOrHandle: nextHopKey,
				IfOrEp: &halproto.NexthopSpec_EpKeyOrHandle{
					EpKeyOrHandle: epKey,
				},
			},
		},
	}

	// create nexthop object
	if hd.Kind == "hal" {
		resp, err := hd.Hal.Netclient.NexthopUpdate(context.Background(), nextHopUpdateReqMsg)
		if err != nil {
			log.Errorf("Error updating next hop. Err: %v", err)
			return err
		}
		if resp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
			return fmt.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
		}
	} else {
		_, err := hd.Hal.Netclient.NexthopUpdate(context.Background(), nextHopUpdateReqMsg)
		if err != nil {
			log.Errorf("Error updating next Hop Err: %v", err)
			return err
		}
	}

	// build route object
	ip, net, err := net.ParseCIDR(rt.Spec.IPPrefix)
	if err != nil {
		return fmt.Errorf("error parsing the IP Prefix mask from %v. Err: %v", rt.Spec.IPPrefix, err)

	}
	prefixLen, _ := net.Mask.Size()
	ipPrefix := &halproto.IPPrefix{
		Address: &halproto.IPAddress{
			IpAf: halproto.IPAddressFamily_IP_AF_INET,
			V4OrV6: &halproto.IPAddress_V4Addr{
				V4Addr: ipv4Touint32(ip),
			},
		},
		PrefixLen: uint32(prefixLen),
	}

	routeUpdateReqMsg := &halproto.RouteRequestMsg{
		Request: []*halproto.RouteSpec{
			{
				KeyOrHandle: &halproto.RouteKeyHandle{
					KeyOrHandle: &halproto.RouteKeyHandle_RouteKey{
						RouteKey: &halproto.RouteKey{
							VrfKeyHandle: vrfKey,
							IpPrefix:     ipPrefix,
						},
					},
				},
				NhKeyOrHandle: nextHopKey,
			},
		},
	}

	// create route object
	if hd.Kind == "hal" {
		resp, err := hd.Hal.Netclient.RouteUpdate(context.Background(), routeUpdateReqMsg)
		if err != nil {
			log.Errorf("Error updating route. Err: %v", err)
			return err
		}
		if resp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
			return fmt.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
		}
	} else {
		_, err := hd.Hal.Netclient.RouteUpdate(context.Background(), routeUpdateReqMsg)
		if err != nil {
			log.Errorf("Error updating route. Err: %v", err)
			return err
		}
	}
	return nil
}

// DeleteRoute deletes a Route in the datapath
func (hd *Datapath) DeleteRoute(rt *netproto.Route, vrf *netproto.Vrf) error {
	// This will ensure that only one datapath config will be active at a time. This is a temporary restriction
	// to ensure that HAL will use a single config thread , this will be removed prior to FCS to allow parallel configs to go through.
	// TODO Remove Global Locking
	hd.Lock()
	defer hd.Unlock()
	vrfKey := &halproto.VrfKeyHandle{
		KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
			VrfId: vrf.Status.VrfID,
		},
	}

	// Build next hop key
	nextHopKey := &halproto.NexthopKeyHandle{
		KeyOrHandle: &halproto.NexthopKeyHandle_NexthopId{
			NexthopId: rt.Status.RouteID,
		},
	}

	// Build route key
	ip, net, err := net.ParseCIDR(rt.Spec.IPPrefix)
	if err != nil {
		return fmt.Errorf("error parsing the IP Prefix mask from %v. Err: %v", rt.Spec.IPPrefix, err)

	}
	prefixLen, _ := net.Mask.Size()
	ipPrefix := &halproto.IPPrefix{
		Address: &halproto.IPAddress{
			IpAf: halproto.IPAddressFamily_IP_AF_INET,
			V4OrV6: &halproto.IPAddress_V4Addr{
				V4Addr: ipv4Touint32(ip),
			},
		},
		PrefixLen: uint32(prefixLen),
	}

	routeKey := &halproto.RouteKeyHandle{
		KeyOrHandle: &halproto.RouteKeyHandle_RouteKey{
			RouteKey: &halproto.RouteKey{
				VrfKeyHandle: vrfKey,
				IpPrefix:     ipPrefix,
			},
		},
	}

	nhDelReq := &halproto.NexthopDeleteRequestMsg{
		Request: []*halproto.NexthopDeleteRequest{
			{
				KeyOrHandle: nextHopKey,
			},
		},
	}

	rtDelReq := &halproto.RouteDeleteRequestMsg{
		Request: []*halproto.RouteDeleteRequest{
			{
				KeyOrHandle: routeKey,
			},
		},
	}

	// delete hal objects
	if hd.Kind == "hal" {
		// delete route
		rtResp, err := hd.Hal.Netclient.RouteDelete(context.Background(), rtDelReq)
		if err != nil {
			log.Errorf("Error deleting route. Err: %v", err)
			return err
		}
		if rtResp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL returned non OK status. %v", rtResp.Response[0].ApiStatus.String())
			return fmt.Errorf("HAL returned non OK status. %v", rtResp.Response[0].ApiStatus.String())
		}

		// delete next hop
		nhResp, err := hd.Hal.Netclient.NexthopDelete(context.Background(), nhDelReq)
		if err != nil {
			log.Errorf("Error deleting next hop. Err: %v", err)
			return err
		}
		if nhResp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL returned non OK status. %v", nhResp.Response[0].ApiStatus.String())
			return fmt.Errorf("HAL returned non OK status. %v", nhResp.Response[0].ApiStatus.String())
		}
	} else {
		_, err := hd.Hal.Netclient.RouteDelete(context.Background(), rtDelReq)
		if err != nil {
			log.Errorf("Error creating route. Err: %v", err)
			return err
		}
	}

	return nil
}
