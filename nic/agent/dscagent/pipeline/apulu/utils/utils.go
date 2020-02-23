// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.

// +build apulu

package utils

import (
	"encoding/binary"
	"fmt"
	"net"
	"strconv"
	"strings"

	"github.com/pkg/errors"

	"github.com/pensando/sw/nic/agent/dscagent/pipeline/utils"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	halapi "github.com/pensando/sw/nic/apollo/agent/gen/pds"

	"github.com/pensando/sw/venice/utils/log"
)

// HandleErr wraps handles datapath errors
func HandleErr(oper int, apiStatus halapi.ApiStatus, err error, format string) error {
	if err != nil {
		log.Error(format)
		return errors.Wrapf(types.ErrDatapathTransport, format, err)
	}

	switch oper {
	case types.Create:
		if !(apiStatus == halapi.ApiStatus_API_STATUS_OK || apiStatus == halapi.ApiStatus_API_STATUS_EXISTS_ALREADY) {
			log.Error(errors.Wrapf(types.ErrDatapathHandling, "%s | Status: %s", format, apiStatus.String()))
			return errors.Wrapf(types.ErrDatapathHandling, "%s | Status: %s", format, apiStatus.String())
		}
	case types.Update:
		if !(apiStatus == halapi.ApiStatus_API_STATUS_OK) {
			log.Error(errors.Wrapf(types.ErrDatapathHandling, "%s | Status: %s", format, apiStatus.String()))
			return errors.Wrapf(types.ErrDatapathHandling, "%s | Status: %s", format, apiStatus.String())
		}
	case types.Delete:
		if !(apiStatus == halapi.ApiStatus_API_STATUS_OK) {
			log.Error(errors.Wrapf(types.ErrDatapathHandling, "%s | Status: %s", format, apiStatus.String()))
			return errors.Wrapf(types.ErrDatapathHandling, "%s | Status: %s", format, apiStatus.String())
		}
	}
	return nil
}

func convertID64(agentID uint64) []byte {
	pipelineID := make([]byte, 8)
	binary.LittleEndian.PutUint64(pipelineID, agentID)
	return pipelineID
}

func convertID32(agentID uint32) []byte {
	pipelineID := make([]byte, 4)
	binary.LittleEndian.PutUint32(pipelineID, agentID)
	return pipelineID
}

// Ipv4Touint32 converts net.IP to 32 bit integer
func Ipv4Touint32(ip net.IP) uint32 {
	if ip == nil {
		return 0
	}
	if len(ip) == 16 {
		return binary.LittleEndian.Uint32(ip[12:16])
	}
	return binary.LittleEndian.Uint32(ip)
}

// ConvertID32 converts agent object 32 bit ID to HAL Object ID
func ConvertID32(agentIDs ...uint32) [][]byte {
	var halIDs [][]byte
	for _, a := range agentIDs {
		halIDs = append(halIDs, convertID32(a))
	}
	return halIDs
}

// ConvertID64 converts agent object 32 bit ID to HAL Object ID
func ConvertID64(agentIDs ...uint64) [][]byte {
	var halIDs [][]byte
	for _, a := range agentIDs {
		halIDs = append(halIDs, convertID64(a))
	}
	return halIDs
}

// ConvertMacAddress converts string MAC address into uint64 value
func ConvertMacAddress(mac string) (macAddress uint64) {
	hex := types.MacStringRegex.ReplaceAllLiteralString(mac, "")
	macAddress, _ = strconv.ParseUint(hex, 16, 64)
	return
}

// ConvertIPAddresses converts IP Address string to hal ip address. TODO v6
func ConvertIPAddresses(addresses ...string) (ipAddresses []*halapi.IPAddress) {
	for _, a := range addresses {
		addr := net.ParseIP(a)
		v4Addr := &halapi.IPAddress{
			Af: halapi.IPAF_IP_AF_INET,
			V4OrV6: &halapi.IPAddress_V4Addr{
				V4Addr: Ipv4Touint32(addr),
			},
		}
		ipAddresses = append(ipAddresses, v4Addr)
	}
	return
}

// MacStrtoUint64 converts a MAC string to uint64
func MacStrtoUint64(macStr string) uint64 {
	var mac uint64

	addr, err := net.ParseMAC(macStr)
	if err != nil {
		return 0
	}

	for _, x := range addr {
		mac = mac*256 + uint64(x)
	}

	return mac
}

// ConvertIPAddress converts IP Address string to hal ip address. TODO v6
func ConvertIPAddress(address string) (ipAddress *halapi.IPAddress) {
	addr := net.ParseIP(address)
	v4Addr := &halapi.IPAddress{
		Af: halapi.IPAF_IP_AF_INET,
		V4OrV6: &halapi.IPAddress_V4Addr{
			V4Addr: utils.Ipv4Touint32(addr),
		},
	}
	ipAddress = v4Addr
	return
}

// ConvertIPPrefix converts IP Address string to hal ip prefix. TODO v6
func ConvertIPPrefix(address string) (ipAddress *halapi.IPPrefix, err error) {
	addr, net, err := net.ParseCIDR(address)
	if err != nil {
		return nil, err
	}
	if addr.To4() == nil {
		// Does not support V6.
		return nil, fmt.Errorf("only ipv4 supported")
	}
	len, _ := net.Mask.Size()
	v4Addr := &halapi.IPPrefix{
		Addr: &halapi.IPAddress{
			Af: halapi.IPAF_IP_AF_INET,
			V4OrV6: &halapi.IPAddress_V4Addr{
				V4Addr: utils.HostOrderIpv4Touint32(addr),
			},
		},
		Len: uint32(len),
	}
	ipAddress = v4Addr
	return
}

// RTToBytes converts the RouteDistinguisher to bytes
func RTToBytes(r *netproto.RouteDistinguisher) []byte {
	var ret = make([]byte, 8)
	switch strings.ToLower(r.Type) {
	case strings.ToLower(netproto.RouteDistinguisher_Type0.String()):
		binary.BigEndian.PutUint16(ret[0:2], uint16(0x0002))
		binary.BigEndian.PutUint16(ret[2:4], uint16(r.AdminValue))
		binary.BigEndian.PutUint32(ret[4:8], uint32(r.AssignedValue))
	case strings.ToLower(netproto.RouteDistinguisher_Type1.String()):
		binary.BigEndian.PutUint16(ret[0:2], uint16(0x0102))
		binary.BigEndian.PutUint32(ret[2:6], uint32(r.AdminValue))
		binary.BigEndian.PutUint16(ret[6:8], uint16(r.AssignedValue))
	case strings.ToLower(netproto.RouteDistinguisher_Type2.String()):
		binary.BigEndian.PutUint16(ret[0:2], uint16(0x0202))
		binary.BigEndian.PutUint32(ret[2:6], uint32(r.AdminValue))
		binary.BigEndian.PutUint16(ret[6:8], uint16(r.AssignedValue))
	}
	return ret
}
