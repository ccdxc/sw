// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.

// +build apulu

package utils

import (
	"encoding/binary"
	"fmt"
	"net"

	"strconv"

	"github.com/pkg/errors"

	"github.com/pensando/sw/nic/agent/dscagent/pipeline/utils"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	halapi "github.com/pensando/sw/nic/agent/dscagent/types/apuluproto"
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
			log.Error(types.ErrDatapathHandling, "%s | Status: %s", format, apiStatus.String())
			return errors.Wrapf(types.ErrDatapathHandling, "%s | Status: %s", format, apiStatus.String())
		}
	case types.Delete:
		if !(apiStatus == halapi.ApiStatus_API_STATUS_OK) {
			log.Error(types.ErrDatapathHandling, "%s | Status: %s", format, apiStatus.String())
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
				V4Addr: utils.Ipv4Touint32(addr),
			},
		}
		ipAddresses = append(ipAddresses, v4Addr)
	}
	return
}

// MacStrtoUint64 converts a MAC string to uint64
func MacStrtoUint64(macStr string) uint64 {
	var bytes [6]uint64
	var mac uint64

	fmt.Sscanf(macStr, "%x:%x:%x:%x:%x:%x", &bytes[0], &bytes[1], &bytes[2], &bytes[3], &bytes[4], &bytes[5])

	mac = (bytes[0] << 40)
	mac |= (bytes[1] << 32)
	mac |= (bytes[2] << 24)
	mac |= (bytes[3] << 16)
	mac |= (bytes[4] << 8)
	mac |= bytes[5]

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
