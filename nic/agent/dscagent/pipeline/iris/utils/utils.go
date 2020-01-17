// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.

// +build iris

package utils

import (
	"net"
	"strconv"

	"github.com/pkg/errors"

	"github.com/pensando/sw/nic/agent/dscagent/pipeline/utils"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	halapi "github.com/pensando/sw/nic/agent/dscagent/types/irisproto"
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
			IpAf: halapi.IPAddressFamily_IP_AF_INET,
			V4OrV6: &halapi.IPAddress_V4Addr{
				V4Addr: utils.Ipv4Touint32(addr),
			},
		}
		ipAddresses = append(ipAddresses, v4Addr)
	}
	return
}
