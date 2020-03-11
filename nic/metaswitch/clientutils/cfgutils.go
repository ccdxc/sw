package clientutils

import (
	"context"
	"encoding/binary"
	"fmt"
	"net"
	"reflect"

	"github.com/satori/go.uuid"

	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	types "github.com/pensando/sw/nic/apollo/agent/gen/pds"
	"github.com/pensando/sw/venice/utils/log"
)

// API Oper for MS updates
const (
	None   = 0
	Create = 1
	Update = 2
	Delete = 3
)

type BGPPeerCfg struct {
	Oper   int
	peer   types.BGPPeerSpec
	addAfs []string
	delAfs []string
}

type BGPPeerDelCfg struct {
	Req types.BGPPeerDeleteRequest
}

type BGPCfg struct {
	Uuid       []byte
	GlobalOper int
	Global     types.BGPRequest
	DelGlobal  types.BGPDeleteRequest
	AddPeers   types.BGPPeerRequest
	UpdPeers   types.BGPPeerRequest
	DelPeers   types.BGPPeerDeleteRequest
	AddPeerAF  types.BGPPeerAfRequest
	DelPeerAF  types.BGPPeerAfDeleteRequest
}

type bgpConfig struct {
	uid       []byte
	routerID  string
	asn       uint32
	keepalive uint32
	holdtime  uint32
	neighbors []*bgpNeighbor
}

type bgpNeighbor struct {
	shutdown     bool
	ipaddress    string
	remoteAS     uint32
	multihop     uint32
	addrFamilies []string
	keepalive    uint32
	holdtime     uint32
	password     string
	localAddr    string
}

func ip2uint32(ipstr string) uint32 {
	ip := net.ParseIP(ipstr).To4()
	if len(ip) == 0 {
		return 0
	}
	return (((uint32(ip[3])*256)+uint32(ip[2]))*256+uint32(ip[1]))*256 + uint32(ip[0])
}

func ip2PDSType(ipstr string) *types.IPAddress {
	if ipstr == "0.0.0.0" || ipstr == "" {
		return &types.IPAddress{
			Af: types.IPAF_IP_AF_INET,
		}
	}
	return &types.IPAddress{
		Af:     types.IPAF_IP_AF_INET,
		V4OrV6: &types.IPAddress_V4Addr{V4Addr: ip2uint32(ipstr)},
	}
}

func pdsIPtoStr(in *types.IPAddress) string {
	b := make([]byte, 4)
	if in.Af == types.IPAF_IP_AF_INET {
		binary.LittleEndian.PutUint32(b, in.GetV4Addr())
		x := net.IP(b)
		return x.String()
	}
	return ""
}

func operStr(i int) string {
	switch i {
	case None:
		return "None"
	case Create:
		return "Create"
	case Update:
		return "Update"
	case Delete:
		return "Delete"
	default:
		return "Unknown"

	}
}

// GetBGPConfiguration updates BGP Configuration
func GetBGPConfiguration(old interface{}, new interface{}, oldLb string, newLb string) (*BGPCfg, error) {
	if new == nil && old == nil {
		return nil, fmt.Errorf("both old and new are nil")
	}
	// Assume both in and out are of the same type
	summary := ""
	var oldCfg, newCfg *bgpConfig
	ret := &BGPCfg{}
	if new != nil {
		switch new.(type) {
		case *netproto.RoutingConfig:
			c := new.(*netproto.RoutingConfig)
			uid, err := uuid.FromString(c.UUID)
			if err != nil {
				return nil, fmt.Errorf("could not parse UUID [%v]", c.UUID)
			}
			if c.Spec.BGPConfig == nil {
				break
			}
			b := c.Spec.BGPConfig
			newCfg = &bgpConfig{
				uid:       uid.Bytes(),
				routerID:  b.RouterId,
				asn:       b.ASNumber,
				keepalive: b.KeepaliveInterval,
				holdtime:  b.Holdtime,
			}
			for _, n := range b.Neighbors {
				laddr := newLb
				if n.RemoteAS != newCfg.asn {
					// Force to use local IP for the eBGP sessions
					laddr = ""
				}
				log.Infof("new [%v] peer:%s local: %s", newLb, n.IPAddress, laddr)
				newCfg.neighbors = append(newCfg.neighbors, &bgpNeighbor{
					shutdown:     n.Shutdown,
					ipaddress:    n.IPAddress,
					remoteAS:     n.RemoteAS,
					multihop:     n.MultiHop,
					addrFamilies: n.EnableAddressFamilies,
					keepalive:    newCfg.keepalive,
					holdtime:     newCfg.holdtime,
					password:     n.Password,
					localAddr:    laddr,
				})
			}
		case *network.RoutingConfig:
			c := new.(*network.RoutingConfig)
			if c.Spec.BGPConfig == nil {
				break
			}
			b := c.Spec.BGPConfig
			uid, err := uuid.FromString(c.UUID)
			if err != nil {
				return nil, fmt.Errorf("could not parse UUID [%v]", c.UUID)
			}
			newCfg = &bgpConfig{
				uid:       uid.Bytes(),
				routerID:  b.RouterId,
				asn:       b.ASNumber,
				keepalive: b.KeepaliveInterval,
				holdtime:  b.Holdtime,
			}
			for _, n := range b.Neighbors {
				laddr := newLb
				if n.RemoteAS != newCfg.asn {
					// Force to use local IP for the eBGP sessions
					laddr = ""
				}
				log.Infof("new [%v] peer:%s local: %s", newLb, n.IPAddress, laddr)
				newCfg.neighbors = append(newCfg.neighbors, &bgpNeighbor{
					shutdown:     n.Shutdown,
					ipaddress:    n.IPAddress,
					remoteAS:     n.RemoteAS,
					multihop:     n.MultiHop,
					addrFamilies: n.EnableAddressFamilies,
					password:     n.Password,
					keepalive:    newCfg.keepalive,
					holdtime:     newCfg.holdtime,
					localAddr:    laddr,
				})
			}
		default:
			return nil, fmt.Errorf("unknown config type")
		}
	}

	if old != nil {
		switch old.(type) {
		case *netproto.RoutingConfig:
			c := old.(*netproto.RoutingConfig)
			if c == nil {
				break
			}
			uid, err := uuid.FromString(c.UUID)
			if err != nil {
				return nil, fmt.Errorf("could not parse UUID [%v]", c.UUID)
			}
			if c.Spec.BGPConfig == nil {
				break
			}
			b := c.Spec.BGPConfig
			oldCfg = &bgpConfig{
				uid:      uid.Bytes(),
				routerID: b.RouterId,
				asn:      b.ASNumber,
			}
			for _, n := range b.Neighbors {
				laddr := oldLb
				if n.RemoteAS != oldCfg.asn {
					// Force to use local IP for the eBGP sessions
					laddr = ""
				}
				log.Infof("old [%v] peer:%s local: %s", oldLb, n.IPAddress, laddr)
				oldCfg.neighbors = append(oldCfg.neighbors, &bgpNeighbor{
					shutdown:     n.Shutdown,
					ipaddress:    n.IPAddress,
					remoteAS:     n.RemoteAS,
					multihop:     n.MultiHop,
					addrFamilies: n.EnableAddressFamilies,
					password:     n.Password,
					keepalive:    oldCfg.keepalive,
					holdtime:     oldCfg.holdtime,
					localAddr:    laddr,
				})
			}
		case *network.RoutingConfig:
			c := old.(*network.RoutingConfig)
			if c == nil {
				break
			}
			uid, err := uuid.FromString(c.UUID)
			if err != nil {
				return nil, fmt.Errorf("could not parse UUID [%v]", c.UUID)
			}
			if c.Spec.BGPConfig == nil {
				break
			}
			b := c.Spec.BGPConfig
			oldCfg = &bgpConfig{
				uid:      uid.Bytes(),
				routerID: b.RouterId,
				asn:      b.ASNumber,
			}
			for _, n := range b.Neighbors {
				laddr := oldLb
				if n.RemoteAS != oldCfg.asn {
					// Force to use local IP for the eBGP sessions
					laddr = ""
				}
				log.Infof("old [%v] peer:%s local: %s", oldLb, n.IPAddress, laddr)
				oldCfg.neighbors = append(oldCfg.neighbors, &bgpNeighbor{
					shutdown:     n.Shutdown,
					ipaddress:    n.IPAddress,
					remoteAS:     n.RemoteAS,
					multihop:     n.MultiHop,
					addrFamilies: n.EnableAddressFamilies,
					password:     n.Password,
					keepalive:    oldCfg.keepalive,
					holdtime:     oldCfg.holdtime,
					localAddr:    laddr,
				})
			}
		default:
			return nil, fmt.Errorf("unknown config type")
		}

	}

	switch {
	case oldCfg == nil:
		ret.GlobalOper = Create
		var rid uint32
		if newCfg.routerID == "0.0.0.0" || newCfg.routerID == "" {
			rid = ip2uint32(newLb)
		} else {
			rid = ip2uint32(newCfg.routerID)
		}
		ret.Global.Request = &types.BGPSpec{
			Id:       newCfg.uid,
			LocalASN: newCfg.asn,
			RouterId: rid,
		}
	case newCfg == nil:
		ret.GlobalOper = Delete
		ret.DelGlobal.Request = &types.BGPKeyHandle{Id: oldCfg.uid}
	default:
		if oldCfg.routerID == newCfg.routerID && oldCfg.asn == newCfg.asn {
			ret.GlobalOper = None
		} else {
			if oldCfg.asn != newCfg.asn {
				return nil, fmt.Errorf("change in ASN not supported")
			}
			ret.GlobalOper = Update
			var rid uint32
			if newCfg.routerID == "0.0.0.0" || newCfg.routerID == "" {
				rid = ip2uint32(newLb)
			} else {
				rid = ip2uint32(newCfg.routerID)
			}
			ret.Global = types.BGPRequest{
				Request: &types.BGPSpec{
					LocalASN: newCfg.asn,
					RouterId: rid,
				},
			}
		}
	}

	summary = fmt.Sprintf("Global: [oper %v", operStr(ret.GlobalOper))
	if ret.Global.Request != nil {
		summary = fmt.Sprintf("%s ASN:%d RID:%x", summary, ret.Global.Request.LocalASN, ret.Global.Request.RouterId)
	}
	summary = fmt.Sprintf("%s] Peers: [", summary)
	if newCfg != nil {
		forceUpdate := false
	outer:
		for _, n := range newCfg.neighbors {
			found := false
			state := types.AdminState_ADMIN_STATE_ENABLE
			if n.shutdown {
				state = types.AdminState_ADMIN_STATE_DISABLE
			}
			if oldCfg != nil {
				if oldCfg.holdtime != newCfg.holdtime || oldCfg.keepalive != newCfg.keepalive {
					forceUpdate = true
				}
				for _, o := range oldCfg.neighbors {
					if n.ipaddress == o.ipaddress {
						found = true
						oper := Update
						if forceUpdate || !reflect.DeepEqual(n, o) {
							if n.localAddr != o.localAddr {
								// need to delete add rather than update
								ret.DelPeers.Request = append(ret.DelPeers.Request, &types.BGPPeerKeyHandle{
									IdOrKey: &types.BGPPeerKeyHandle_Key{Key: &types.BGPPeerKey{PeerAddr: ip2PDSType(o.ipaddress), LocalAddr: ip2PDSType(o.localAddr)}},
								})
								oper = Create
							}

							peer := &types.BGPPeerSpec{
								Id:           newCfg.uid,
								State:        state,
								PeerAddr:     ip2PDSType(n.ipaddress),
								LocalAddr:    ip2PDSType(n.localAddr),
								RemoteASN:    n.remoteAS,
								SendComm:     true,
								SendExtComm:  true,
								KeepAlive:    n.keepalive,
								HoldTime:     n.holdtime,
								TTL:          n.multihop,
								ConnectRetry: 5,
								Password:     []byte(n.password),
							}
							summary = fmt.Sprintf("%s %s-%s-%s", summary, operStr(oper), pdsIPtoStr(peer.PeerAddr), n.localAddr)
							// Add Afs
							for _, afn := range n.addrFamilies {
								found := false
								for _, afo := range o.addrFamilies {
									if afn == afo {
										found = true
										break
									}
								}
								if !found {
									afp := &types.BGPPeerAfSpec{
										Id:          newCfg.uid,
										LocalAddr:   ip2PDSType(n.localAddr),
										PeerAddr:    ip2PDSType(n.ipaddress),
										NexthopSelf: false,
										DefaultOrig: false,
									}
									switch afn {
									case network.BGPAddressFamily_EVPN.String():
										afp.Afi = types.BGPAfi_BGP_AFI_L2VPN
										afp.Safi = types.BGPSafi_BGP_SAFI_EVPN
									case network.BGPAddressFamily_IPv4Unicast.String():
										afp.Afi = types.BGPAfi_BGP_AFI_IPV4
										afp.Safi = types.BGPSafi_BGP_SAFI_UNICAST
									}
									ret.AddPeerAF.Request = append(ret.AddPeerAF.Request, afp)
								}
							}

							// Del Afs
							if oper != Create {
								for _, afo := range o.addrFamilies {
									found := false
									for _, afn := range o.addrFamilies {
										if afo == afn {
											found = true
											break
										}
									}
									if !found {
										afpk := &types.BGPPeerAfKey{LocalAddr: ip2PDSType(o.localAddr), PeerAddr: ip2PDSType(o.ipaddress)}

										switch afo {
										case network.BGPAddressFamily_EVPN.String():
											afpk.Afi = types.BGPAfi_BGP_AFI_L2VPN
											afpk.Safi = types.BGPSafi_BGP_SAFI_EVPN
										case network.BGPAddressFamily_IPv4Unicast.String():
											afpk.Afi = types.BGPAfi_BGP_AFI_IPV4
											afpk.Safi = types.BGPSafi_BGP_SAFI_UNICAST
										}
										ret.DelPeerAF.Request = append(ret.DelPeerAF.Request, &types.BGPPeerAfKeyHandle{
											IdOrKey: &types.BGPPeerAfKeyHandle_Key{Key: afpk},
										})
									}
								}
							}

							if oper == Create {
								ret.AddPeers.Request = append(ret.AddPeers.Request, peer)
							} else {
								ret.UpdPeers.Request = append(ret.UpdPeers.Request, peer)
							}
						}
						continue outer
					}
				}
			}

			if !found {
				// Not found in old. New Peer
				ret.AddPeers.Request = append(ret.AddPeers.Request, &types.BGPPeerSpec{
					Id:           newCfg.uid,
					State:        state,
					PeerAddr:     ip2PDSType(n.ipaddress),
					LocalAddr:    ip2PDSType(n.localAddr),
					RemoteASN:    n.remoteAS,
					SendComm:     true,
					SendExtComm:  true,
					KeepAlive:    n.keepalive,
					HoldTime:     n.holdtime,
					TTL:          n.multihop,
					ConnectRetry: 5,
					// TODO add Holdtime and Keepalive
					Password: []byte(n.password),
				})
				summary = fmt.Sprintf("%s %s-%s-%s", summary, operStr(Create), n.ipaddress, n.localAddr)
				// Add Afs
				for _, afn := range n.addrFamilies {
					afp := &types.BGPPeerAfSpec{
						Id:          newCfg.uid,
						LocalAddr:   ip2PDSType(n.localAddr),
						PeerAddr:    ip2PDSType(n.ipaddress),
						NexthopSelf: false,
						DefaultOrig: false,
					}
					switch afn {
					case network.BGPAddressFamily_EVPN.String():
						afp.Afi = types.BGPAfi_BGP_AFI_L2VPN
						afp.Safi = types.BGPSafi_BGP_SAFI_EVPN
					case network.BGPAddressFamily_IPv4Unicast.String():
						afp.Afi = types.BGPAfi_BGP_AFI_IPV4
						afp.Safi = types.BGPSafi_BGP_SAFI_UNICAST
					}
					ret.AddPeerAF.Request = append(ret.AddPeerAF.Request, afp)
				}
			}
		}
	}

	// Look for stale neighbors
	if oldCfg != nil {
	outer1:
		for _, o := range oldCfg.neighbors {
			found := false
			if newCfg != nil {
				for _, n := range newCfg.neighbors {
					if n.ipaddress == o.ipaddress {
						found = true
						continue outer1
					}
				}
			}

			if !found {
				ret.DelPeers.Request = append(ret.DelPeers.Request, &types.BGPPeerKeyHandle{
					IdOrKey: &types.BGPPeerKeyHandle_Key{Key: &types.BGPPeerKey{PeerAddr: ip2PDSType(o.ipaddress), LocalAddr: ip2PDSType(o.localAddr)}},
				})
				summary = fmt.Sprintf("%s %s-%s-%s", summary, operStr(Delete), o.ipaddress, o.localAddr)
			}
		}
	}
	log.Infof("%s] AddPeers: %d UpdPeers: %d DelPeers: %d AddAfs: %d DelAfs: %d", summary, len(ret.AddPeers.Request), len(ret.UpdPeers.Request), len(ret.DelPeers.Request), len(ret.AddPeerAF.Request), len(ret.DelPeerAF.Request))
	return ret, nil
}

func UpdateBGPConfiguration(ctx context.Context, client types.BGPSvcClient, cfg *BGPCfg) error {
	// Del AF Peers
	if len(cfg.DelPeerAF.Request) > 0 {
		resp, err := client.BGPPeerAfDelete(ctx, &cfg.DelPeerAF)
		if err != nil {
			log.Errorf("BGPConfig: Delete Peer AF failed | Err: %s", err)
			return err
		}
		if resp.ApiStatus != types.ApiStatus_API_STATUS_OK {
			log.Errorf("BGPConfig: Delete Peer AF failed | Status: %s", resp.ApiStatus)
		}
		log.Infof("Peer AF [%d] delete succeeded [%s]", len(cfg.DelPeerAF.Request), resp.ApiStatus)
	}

	// Del Peers
	if len(cfg.DelPeers.Request) > 0 {
		resp, err := client.BGPPeerDelete(ctx, &cfg.DelPeers)
		if err != nil {
			log.Errorf("BGPConfig: Delete Peer failed | Err: %s", err)
			return err
		}
		if resp.ApiStatus != types.ApiStatus_API_STATUS_OK {
			log.Errorf("BGPConfig: Delete Peer failed | Status: %s", resp.ApiStatus)
		}
		log.Infof("BGPConfig: Peers [%d] delete succeeded [%s]", len(cfg.DelPeers.Request), resp.ApiStatus)
	}

	// Update/Add Global
	switch cfg.GlobalOper {
	case Create:
		resp, err := client.BGPCreate(ctx, &cfg.Global)
		if err != nil {
			log.Errorf("BGPConfig: Create Global failed | Err: %s", err)
			return err
		}
		if resp.ApiStatus != types.ApiStatus_API_STATUS_OK {
			log.Errorf("BGPConfig: Create Global failed | Status: %s", resp.ApiStatus)
		}
		log.Infof("BGPConfig: Create Global succeeded [%s]", resp.ApiStatus)

	case Update:
		resp, err := client.BGPUpdate(ctx, &cfg.Global)
		if err != nil {
			log.Errorf("BGPConfig: Update Global failed | Err: %s", err)
			return err
		}
		if resp.ApiStatus != types.ApiStatus_API_STATUS_OK {
			log.Errorf("BGPConfig: Create Global failed | Status: %s", resp.ApiStatus)
		}
		log.Infof("BGPConfig: Update Global succeeded [%s]", resp.ApiStatus)
	}

	// ADD Peers
	if len(cfg.AddPeers.Request) > 0 {
		resp, err := client.BGPPeerCreate(ctx, &cfg.AddPeers)
		if err != nil {
			log.Errorf("BGPConfig: Create Peer failed | Err: %s", err)
			return err
		}
		if resp.ApiStatus != types.ApiStatus_API_STATUS_OK {
			log.Errorf("BGPConfig: Create Peer failed | Status: %s", resp.ApiStatus)
		}
		log.Infof("BGPConfig: Create Peers [%d] succeeded [%s]", len(cfg.AddPeers.Request), resp.ApiStatus)
	}

	// Update Peers
	if len(cfg.UpdPeers.Request) > 0 {
		resp, err := client.BGPPeerUpdate(ctx, &cfg.UpdPeers)
		if err != nil {
			log.Errorf("BGPConfig: Update Peer failed | Err: %s", err)
			return err
		}
		if resp.ApiStatus != types.ApiStatus_API_STATUS_OK {
			log.Errorf("BGPConfig: Update Peer failed | Status: %s", resp.ApiStatus)
		}
		log.Infof("BGPConfig: Update Peers [%d] succeeded [%s]", len(cfg.UpdPeers.Request), resp.ApiStatus)
	}

	// Add Peers AF
	if len(cfg.AddPeerAF.Request) > 0 {
		resp, err := client.BGPPeerAfCreate(ctx, &cfg.AddPeerAF)
		if err != nil {
			log.Errorf("BGPConfig: Update Peer failed | Err: %s", err)
			return err
		}
		if resp.ApiStatus != types.ApiStatus_API_STATUS_OK {
			log.Errorf("BGPConfig: Update Peer failed | Status: %s", resp.ApiStatus)
		}
		log.Infof("BGPConfig: Update Peers [%d] succeeded [%s]", len(cfg.UpdPeers.Request), resp.ApiStatus)
	}

	// Delete Global
	if cfg.GlobalOper == Delete {
		resp, err := client.BGPDelete(ctx, &cfg.DelGlobal)
		if err != nil {
			log.Errorf("BGPConfig: Delete global failed | Err: %s", err)
			return err
		}
		if resp.ApiStatus != types.ApiStatus_API_STATUS_OK {
			log.Errorf("BGPConfig:Delete global failed | Status: %s", resp.ApiStatus)
		}
		log.Infof("BGPConfig: Delete global [%d] succeeded [%s]", len(cfg.UpdPeers.Request), resp.ApiStatus)
	}
	return nil
}
