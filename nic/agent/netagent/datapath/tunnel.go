package datapath

import (
	"context"
	"fmt"
	"net"

	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/venice/ctrler/npm/rpcserver/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// CreateTunnel creates a tunnel
func (hd *Datapath) CreateTunnel(tun *netproto.Tunnel, ns *netproto.Namespace) error {

	vrfKey := &halproto.VrfKeyHandle{
		KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
			VrfId: ns.Status.NamespaceID,
		},
	}

	ifInfo, err := convertIfInfo(&tun.Spec, vrfKey)
	if err != nil {
		log.Errorf("invalid tunnel interface spec. Tunnel: {%v} Err: %v", tun.Spec, err)
		return err
	}

	status, err := hd.convertIfAdminStatus(tun.Spec.AdminStatus)
	if err != nil {
		return err
	}
	tunReqMsg := &halproto.InterfaceRequestMsg{
		Request: []*halproto.InterfaceSpec{
			{
				KeyOrHandle: &halproto.InterfaceKeyHandle{
					KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
						InterfaceId: tun.Status.TunnelID,
					},
				},
				Type:        halproto.IfType_IF_TYPE_TUNNEL,
				AdminStatus: status,
				IfInfo:      ifInfo,
			},
		},
	}

	// create route object
	if hd.Kind == "hal" {
		resp, err := hd.Hal.Ifclient.InterfaceCreate(context.Background(), tunReqMsg)
		if err != nil {
			log.Errorf("Error creating tunnel interface. Err: %v", err)
			return err
		}
		if resp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus)

			return ErrHALNotOK
		}
	} else {
		_, err := hd.Hal.Ifclient.InterfaceCreate(context.Background(), tunReqMsg)
		if err != nil {
			log.Errorf("Error creating tunnel interface. Err: %v", err)
			return err
		}
	}
	return nil

}

// UpdateTunnel updates a tunnel
func (hd *Datapath) UpdateTunnel(tun *netproto.Tunnel, ns *netproto.Namespace) error {
	return nil
}

// DeleteTunnel deletes a tunnel
func (hd *Datapath) DeleteTunnel(tun *netproto.Tunnel, ns *netproto.Namespace) error {
	tunDelReqMsg := &halproto.InterfaceDeleteRequestMsg{
		Request: []*halproto.InterfaceDeleteRequest{
			{
				KeyOrHandle: &halproto.InterfaceKeyHandle{
					KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
						InterfaceId: tun.Status.TunnelID,
					},
				},
			},
		},
	}

	// create route object
	if hd.Kind == "hal" {
		resp, err := hd.Hal.Ifclient.InterfaceDelete(context.Background(), tunDelReqMsg)
		if err != nil {
			log.Errorf("Error deleting tunnel interface. Err: %v", err)
			return err
		}
		if resp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus)

			return ErrHALNotOK
		}
	} else {
		_, err := hd.Hal.Ifclient.InterfaceDelete(context.Background(), tunDelReqMsg)
		if err != nil {
			log.Errorf("Error deleting tunnel interface. Err: %v", err)
			return err
		}
	}
	return nil
}

// convertIFInfo converts tunnel spec to halproto vxlan info or gre info
func convertIfInfo(tunSpec *netproto.TunnelSpec, vrfKey *halproto.VrfKeyHandle) (*halproto.InterfaceSpec_IfTunnelInfo, error) {
	localIP := net.ParseIP(tunSpec.Src)
	if len(localIP) == 0 {
		return nil, fmt.Errorf("could not parse source IP from %v", tunSpec.Src)
	}

	srcIP := &halproto.IPAddress{
		IpAf: halproto.IPAddressFamily_IP_AF_INET,
		V4OrV6: &halproto.IPAddress_V4Addr{
			V4Addr: ipv4Touint32(localIP),
		},
	}

	remoteIP := net.ParseIP(tunSpec.Dst)
	if len(remoteIP) == 0 {
		return nil, fmt.Errorf("could not parse destination IP from %v", tunSpec.Dst)
	}

	dstIP := &halproto.IPAddress{
		IpAf: halproto.IPAddressFamily_IP_AF_INET,
		V4OrV6: &halproto.IPAddress_V4Addr{
			V4Addr: ipv4Touint32(remoteIP),
		},
	}

	switch tunSpec.Type {
	case "VXLAN":
		ifInfo := halproto.InterfaceSpec_IfTunnelInfo{
			IfTunnelInfo: &halproto.IfTunnelInfo{
				EncapType: halproto.IfTunnelEncapType_IF_TUNNEL_ENCAP_TYPE_VXLAN,
				EncapInfo: &halproto.IfTunnelInfo_VxlanInfo{
					VxlanInfo: &halproto.IfTunnelVxlanInfo{
						LocalTep:  srcIP,
						RemoteTep: dstIP,
					},
				},
				VrfKeyHandle: vrfKey,
			},
		}
		return &ifInfo, nil
	case "GRE":
		ifInfo := halproto.InterfaceSpec_IfTunnelInfo{
			IfTunnelInfo: &halproto.IfTunnelInfo{
				EncapType: halproto.IfTunnelEncapType_IF_TUNNEL_ENCAP_TYPE_GRE,
				EncapInfo: &halproto.IfTunnelInfo_GreInfo{
					GreInfo: &halproto.IfTunnelGREInfo{
						Source:      srcIP,
						Destination: dstIP,
					},
				},
				VrfKeyHandle: vrfKey,
			},
		}
		return &ifInfo, nil
	default:
		log.Errorf("invalid tunnel interface type. %v", tunSpec.Type)
		return nil, fmt.Errorf("invalid tunnel interface type. %v. Must be either VXLAN or GRE", tunSpec.Type)
	}
}
