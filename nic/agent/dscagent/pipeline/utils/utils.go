package utils

import (
	"encoding/binary"
	"fmt"
	"math"
	"net"
	"os"
	"strconv"
	"strings"
	"time"

	"github.com/pkg/errors"
	"github.com/vishvananda/netlink"
	"google.golang.org/grpc"
	"google.golang.org/grpc/connectivity"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
)

const (
	ifTypeEth         = 1
	ifTypeEthPC       = 2
	ifTypeTunnel      = 3
	ifTypeUplink      = 5
	ifTypeL3          = 7
	ifTypeLif         = 8
	ifTypeLoopback    = 9
	ifTypeShift       = 28
	ifSlotShift       = 24
	ifParentPortShift = 16
	ifTypeMask        = 0xF
	ifSlotMask        = 0xF
	ifParentPortMask  = 0xFF
	ifChildPortMask   = 0xFF
	ifNameDelimiter   = "-"
	invalidIfIndex    = 0xFFFFFFFF
)

// CreateNewGRPCClient creates a new RPC Client to the pipeline
func CreateNewGRPCClient(portEnvVar string, defaultPort string) (rpcClient *grpc.ClientConn, err error) {
	return waitForHAL(portEnvVar, defaultPort)
}

func isHalConnected(portEnvVar string, defaultPort string) (*grpc.ClientConn, error) {
	halPort := os.Getenv(portEnvVar)
	if halPort == "" {
		halPort = defaultPort
	}
	halURL := fmt.Sprintf("%s:%s", types.HalGRPCDefaultBaseURL, halPort)
	log.Infof("HAL URL: %s", halURL)
	return grpc.Dial(halURL, grpc.WithMaxMsgSize(math.MaxInt32-1), grpc.WithInsecure(), grpc.WithBlock())
}

func waitForHAL(portEnvVar string, defaultPort string) (rpcClient *grpc.ClientConn, err error) {
	halUP := make(chan bool, 1)
	ticker := time.NewTicker(types.HalGRPCTickerDuration)
	timeout := time.After(types.HalGRPCWaitTimeout)
	rpcClient, err = isHalConnected(portEnvVar, defaultPort)
	if err == nil {
		log.Infof("1st TickStaus: %s", types.InfoConnectedToHAL)
		return
	}

	for {
		select {
		case <-ticker.C:
			rpcClient, err = isHalConnected(portEnvVar, defaultPort)
			if err != nil {
				halUP <- true
			}
		case <-halUP:
			log.Infof("Agent HAL Status: %v", types.InfoConnectedToHAL)
			return
		case <-timeout:
			log.Errorf("Agent could not connect to HAL. | Err: %v", err)
			return nil, errors.Wrapf(types.ErrPipelineNotAvailabe, "Agent could not connect to HAL. Err: %v | Err: %v", types.ErrPipelineTimeout, err)
		}
	}
}

// check if HAL is up given its URL
func IsHalUp() bool {
	var rpcClient *rpckit.RPCClient

	rpcClient, err := rpckit.NewRPCClient(types.Netagent, types.HalGRPCDefaultURL, rpckit.WithTLSProvider(nil))
	if err != nil || rpcClient == nil {
		log.Errorf("Failed to connect to HAL URL %s | Err %s", types.HalGRPCDefaultURL, err)
		return false
	}
	defer rpcClient.ClientConn.Close()
	if rpcClient.ClientConn.GetState() == connectivity.Ready {
		return true
	}
	log.Errorf("HAL is not up | Status %v", rpcClient.ClientConn.GetState())
	return false
}

// ValidateMeta validates object keys based on kind.
func ValidateMeta(oper types.Operation, kind string, meta api.ObjectMeta) error {
	if oper == types.List {
		if len(kind) == 0 {
			return errors.Wrapf(types.ErrBadRequest, "Empty Kind %v", types.ErrEmptyFields)
		}
		return nil
	}

	switch strings.ToLower(kind) {
	case "tenant", "interface":
		if len(meta.Name) == 0 {
			return errors.Wrapf(types.ErrBadRequest, "Kind: %v | Meta: %v | Err: %v", kind, meta, types.ErrEmptyFields)
		}
	case "namespace":
		if len(meta.Tenant) == 0 || len(meta.Name) == 0 {
			return errors.Wrapf(types.ErrBadRequest, "Kind: %v | Meta: %v | Err: %v", kind, meta, types.ErrEmptyFields)
		}
	case "profile":
		if len(meta.Name) == 0 {
			return errors.Wrapf(types.ErrBadRequest, "Kind: %v | Meta: %v | Err: %v", kind, meta, types.ErrEmptyFields)
		}
	default:
		if len(meta.Tenant) == 0 || len(meta.Namespace) == 0 || len(meta.Name) == 0 {
			return errors.Wrapf(types.ErrBadRequest, "Kind: %v | Meta: %v | Err: %v", kind, meta, types.ErrEmptyFields)
		}
	}
	return nil
}

// ValidateIPAddresses ensures that IP Address string is a valid v4 address. TODO v6 support
func ValidateIPAddresses(ipAddresses ...string) (err error) {
	for _, a := range ipAddresses {
		aTrimmed := strings.TrimSpace(a)
		if len(aTrimmed) > 0 {
			ip := net.ParseIP(a)
			if len(ip) == 0 {
				err = errors.Wrapf(types.ErrInvalidIP, "IP Address: %s | Err: %v", a, types.ErrBadRequest)
				return
			}
		}
	}
	return
}

// ValidateIPAddressesPrefix ensures that IP Address string is a valid v4 address prefix in CIDR format. TODO v6 support
func ValidateIPAddressesPrefix(ipAddressPrefixes ...string) error {
	for _, p := range ipAddressPrefixes {
		_, _, err := net.ParseCIDR(strings.TrimSpace(p))
		if err != nil {
			return errors.Wrapf(types.ErrInvalidIPPrefix, "CIDR Block: %s | Err: %v", p, types.ErrBadRequest)
		}
	}
	return nil
}

// ValidateIPAddressRange ensures that IP Address range is a valid v4 address separated by a hyphen. TODO v6 support
func ValidateIPAddressRange(ipAddressRanges ...string) error {
	for _, r := range ipAddressRanges {
		components := strings.Split(r, "-")
		if len(components) != 2 {
			return errors.Wrapf(types.ErrInvalidIPRange, "Range: %s | Err: %v", r, types.ErrBadRequest)

		}
		err := ValidateIPAddresses(components[0])
		if err != nil {
			return err
		}

		err = ValidateIPAddresses(components[1])
		if err != nil {
			return err
		}
	}
	return nil
}

// ValidateMacAddresses ensures that MAC Address is in a valid OUI format
func ValidateMacAddresses(macAddresses ...string) error {
	for _, m := range macAddresses {
		_, err := net.ParseMAC(strings.TrimSpace(m))
		if err != nil {
			return errors.Wrapf(types.ErrInvalidMACAddress, "MAC Address: %s | Err: %v", m, types.ErrBadRequest)
		}
	}
	return nil
}

//func isHalConnected() (*grpc.ClientConn, error) {
//	halPort := os.Getenv("HAL_GRPC_PORT")
//	if halPort == "" {
//		halPort = types.HalGRPCDefaultPort
//	}
//	halURL := fmt.Sprintf("%s:%s", types.HalGRPCDefaultBaseURL, halPort)
//	log.Infof("HAL URL: %s", halURL)
//	return grpc.Dial(halURL, grpc.WithMaxMsgSize(math.MaxInt32-1), grpc.WithInsecure(), grpc.WithBlock())
//}

// ResolveIPAddress resolves IPAddresses and returns its ARP Cache.
//func ResolveIPAddress(mgmtIPAddress, mgmtIntf string, ipAddresses ...string) (map[string]string, error) {
//	arpCache := make(map[string]string)
//
//	mgmtLink, err := net.InterfaceByName(mgmtIntf)
//	if err != nil {
//		log.Error(errors.Wrapf(types.ErrARPManagementInterfaceNotFound, "Err: %v", err))
//		return arpCache, errors.Wrapf(types.ErrARPManagementInterfaceNotFound, "Err: %v", err)
//	}
//	arpClient, err := arp.Dial(mgmtLink)
//	if err != nil {
//		log.Error(errors.Wrapf(types.ErrARPClientDialFailure, "Err: %v", err))
//		return arpCache, errors.Wrapf(types.ErrARPClientDialFailure, "Err: %v", err)
//	}
//	defer arpClient.Close()
//
//	for _, a := range ipAddresses {
//		addr := net.ParseIP(a)
//		macAddress, err := resolveARPWithTimeout(mgmtIPAddress, addr, arpClient)
//		if err != nil || macAddress == nil {
//			log.Error(errors.Wrapf(types.ErrARPResolution, "%s | Err: %v", a, err))
//			return nil, errors.Wrapf(types.ErrARPResolution, "%s | Err: %v", a, err)
//		}
//		arpCache[a] = macAddress.String()
//	}
//
//	return arpCache, nil
//}

// mgmtIPAddress will be in CIDR format
//func resolveARPWithTimeout(mgmtIP string, addr net.IP, arpClient *arp.Client) (net.HardwareAddr, error) {
//
//	arpChan := make(chan net.HardwareAddr, 1)
//
//	go func() {
//		var macAddr net.HardwareAddr
//		var err error
//		// Do subnet checks here
//		_, mgmtNet, _ := net.ParseCIDR(mgmtIP)
//		if !mgmtNet.Contains(addr) {
//			macAddr, err = resolveARPForDefaultGateway(addr, arpClient)
//
//		} else {
//			log.Infof("Pipeline Utils Handler: %s", types.InfoARPingForSameSubnetIP)
//			macAddr, err = arpClient.Resolve(addr)
//			if err != nil {
//				log.Error(errors.Wrapf(types.ErrARPEntryMissingForSameSubnetIP, "Same Subnet IP: %s | Err: %v", addr.String(), err))
//			}
//		}
//		arpChan <- macAddr
//	}()
//
//	select {
//	case macAddr := <-arpChan:
//		return macAddr, nil
//	case <-time.After(types.ARPResolutionTimeout):
//		return nil, types.ErrARPResolutionTimeoutExceeded
//	}
//}

//func waitForHAL() (rpcClient *grpc.ClientConn, err error) {
//	halUP := make(chan bool, 1)
//	ticker := time.NewTicker(types.HalGRPCTickerDuration)
//	timeout := time.After(types.HalGRPCWaitTimeout)
//	rpcClient, err = isHalConnected()
//	if err == nil {
//		log.Infof("1st TickStaus: %s", types.InfoConnectedToHAL)
//		return
//	}
//
//	for {
//		select {
//		case <-ticker.C:
//			rpcClient, err = isHalConnected()
//			if err != nil {
//				halUP <- true
//			}
//		case <-halUP:
//			log.Infof("Agent HAL Status: %v", types.InfoConnectedToHAL)
//			return
//		case <-timeout:
//			log.Errorf("Agent could not connect to HAL. | Err: %v", err)
//			return nil, errors.Wrapf(types.ErrPipelineNotAvailabe, "Agent could not connect to HAL. Err: %v | Err: %v", types.ErrPipelineTimeout, err)
//		}
//	}
//}

// Ipv4Touint32 converts net.IP to 32 bit integer
func Ipv4Touint32(ip net.IP) uint32 {
	if ip == nil {
		return 0
	}
	if len(ip) == 16 {
		return binary.BigEndian.Uint32(ip[12:16])
	}
	return binary.BigEndian.Uint32(ip)
}

// HostOrderIpv4Touint32 converts net.IP considered as host order bytes to 32 bit integer
//  TODO: Assumes system byteorder to be little endian. FIX check true order of the system
func HostOrderIpv4Touint32(ip net.IP) uint32 {
	if ip == nil {
		return 0
	}
	if len(ip) == 16 {
		return binary.LittleEndian.Uint32(ip[12:16])
	}
	return binary.LittleEndian.Uint32(ip)
}

func ifIndexToSlot(ifIndex uint32) uint32 {
	return (ifIndex >> ifSlotShift) & ifSlotMask
}

func ifIndexToParentPort(ifIndex uint32) uint32 {
	return (ifIndex >> ifParentPortShift) & ifParentPortMask
}

func ifIndexToChildPort(ifIndex uint32) uint32 {
	return ifIndex & ifChildPortMask
}

func ifIndexToID(ifIndex uint32) uint32 {
	return ifIndex &^ (ifTypeMask << ifTypeShift)
}

// EthIfIndexToUplinkIfIndex converts Eth ifIndex to Uplink IfIndex
func EthIfIndexToUplinkIfIndex(ifIndex uint64) uint64 {
	return ((ifTypeUplink << ifTypeShift) | (ifIndex &^ (ifTypeMask << ifTypeShift)))
}

func getIfTypeStr(ifIndex uint32, subType string) (intfType string, err error) {
	ifType := (ifIndex >> ifTypeShift) & ifTypeMask
	switch ifType {
	case ifTypeEth:
		if subType == "PORT_TYPE_ETH_MGMT" || subType == "PORT_TYPE_MGMT" {
			return "mgmt", nil
		}
		return "uplink", nil
	case ifTypeEthPC:
		return "uplink-pc", nil
	case ifTypeTunnel:
		return "tunnel", nil
	case ifTypeL3:
		return "l3", nil
	case ifTypeLif:
		return "pf", nil
	case ifTypeLoopback:
		return "lo", nil
	}
	return "", errors.Wrapf(types.ErrInvalidInterfaceType,
		"Unsupported interface type in ifindex %x | Err: %v", ifIndex, types.ErrInvalidInterfaceType)
}

// GetIfName given encoded interface index and its type, form interface name that consists of system MAC
func GetIfName(systemMac string, ifIndex uint32, subType string) (ifName string, err error) {
	ifTypeStr, err := getIfTypeStr(ifIndex, subType)
	if err != nil {
		return "", err
	}
	ifType := (ifIndex >> ifTypeShift) & ifTypeMask
	switch ifType {
	case ifTypeEth:
		slotStr := strconv.FormatUint(uint64(ifIndexToSlot(ifIndex)), 10)
		parentPortStr := strconv.FormatUint(uint64(ifIndexToParentPort(ifIndex)), 10)
		return systemMac + ifNameDelimiter + ifTypeStr + ifNameDelimiter + slotStr + ifNameDelimiter + parentPortStr, nil
	case ifTypeEthPC, ifTypeTunnel, ifTypeL3, ifTypeLif, ifTypeLoopback:
		return systemMac + ifNameDelimiter + ifTypeStr + ifNameDelimiter + strconv.FormatUint(uint64(ifIndexToID(ifIndex)), 10), nil
	}
	return "", errors.Wrapf(types.ErrInvalidInterfaceType,
		"Unsupported interface type in ifindex %x | Err: %v", ifIndex, types.ErrInvalidInterfaceType)
}

// GetIfIndex returns a IfIndex given a type and port paramenters
func GetIfIndex(subType string, slot, parent, port uint32) uint32 {
	switch subType {
	case netproto.InterfaceSpec_HOST_PF.String():
		return ((ifTypeLif & ifTypeMask) << ifTypeShift) | (slot&ifSlotMask)<<ifSlotShift | (parent&ifParentPortMask)<<ifParentPortShift | port&ifChildPortMask
	case netproto.InterfaceSpec_UPLINK_ETH.String():
		return ((ifTypeEth & ifTypeMask) << ifTypeShift) | (slot&ifSlotMask)<<ifSlotShift | (parent&ifParentPortMask)<<ifParentPortShift | port&ifChildPortMask
	case netproto.InterfaceSpec_UPLINK_MGMT.String():
		return ((ifTypeEth & ifTypeMask) << ifTypeShift) | (slot&ifSlotMask)<<ifSlotShift | (parent&ifParentPortMask)<<ifParentPortShift | port&ifChildPortMask
	case netproto.InterfaceSpec_L3.String():
		return ((ifTypeL3 & ifTypeMask) << ifTypeShift) | (slot&ifSlotMask)<<ifSlotShift | (parent&ifParentPortMask)<<ifParentPortShift | port&ifChildPortMask
	case netproto.InterfaceSpec_LOOPBACK.String():
		return ((ifTypeLoopback & ifTypeMask) << ifTypeShift) | (slot&ifSlotMask)<<ifSlotShift | (parent&ifParentPortMask)<<ifParentPortShift | port&ifChildPortMask
	}
	return 0
}

// GetLifIndex converts hwLifID to IfIndex
func GetLifIndex(hwLifID uint64) uint32 {
	return uint32((ifTypeLif << ifTypeShift) | hwLifID)
}

// BuildDestKey builds the destination key with vrf name
func BuildDestKey(vrfName, dstIP string) string {
	return vrfName + "-" + dstIP
}

// GetMgmtLink return the management link for an management IP
func GetMgmtLink(mgmtIP string) (mgmtLink netlink.Link) {
	links, err := netlink.LinkList()
	if err != nil {
		log.Errorf("Failed to list the available links. Err: %v", err)
		return
	}

	for _, l := range links {
		addrs, _ := netlink.AddrList(l, netlink.FAMILY_V4)
		for _, a := range addrs {
			if a.IP.String() == mgmtIP {
				mgmtLink = l
				return
			}
		}
	}
	return
}

// BuildCollectorKey builds the collector key with vrfname, IP and port
func BuildCollectorKey(vrfName string, c netproto.ExportConfig) string {
	return vrfName + "-" + c.Destination + "-" + c.Transport.Port
}
