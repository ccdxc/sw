//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	"strings"

	uuid "github.com/satori/go.uuid"
	"github.com/spf13/cobra"

	"github.com/pensando/sw/nic/apollo/agent/cli/utils"
	"github.com/pensando/sw/nic/apollo/agent/gen/pds"
)

var (
	vpcID       uint32
	vpcStr      string
	mappingIP   string
	mappingType string
	subID       uint32
	subStr      string
	mappingMAC  string
	mappingID   string
)

var mappingShowCmd = &cobra.Command{
	Use:   "mapping",
	Short: "show Mapping information",
	Long:  "show Mapping object information",
}

var mappingInternalShowCmd = &cobra.Command{
	Use:    "internal",
	Short:  "show internal mapping information",
	Long:   "show internal mapping information",
	Hidden: true,
}

var localMappingInternalShowCmd = &cobra.Command{
	Use:   "local",
	Short: "show local mapping information",
	Long:  "show local mapping object information",
	Run:   localMappingInternalHandler,
}

var remoteMappingInternalShowCmd = &cobra.Command{
	Use:   "remote",
	Short: "show remote mapping information",
	Long:  "show remote mapping object information",
	Run:   remoteMappingInternalHandler,
}

var localMappingShowCmd = &cobra.Command{
	Use:   "local",
	Short: "show local mapping information",
	Long:  "show local mapping object information",
	Run:   localMappingHandler,
}

var remoteMappingShowCmd = &cobra.Command{
	Use:   "remote",
	Short: "show remote mapping information",
	Long:  "show remote mapping object information",
	Run:   remoteMappingHandler,
}

func init() {
	showCmd.AddCommand(mappingShowCmd)
	mappingShowCmd.AddCommand(mappingInternalShowCmd)
	mappingInternalShowCmd.AddCommand(localMappingInternalShowCmd)
	mappingInternalShowCmd.AddCommand(remoteMappingInternalShowCmd)
	localMappingInternalShowCmd.Flags().Uint32Var(&vpcID, "vpc", 0, "Specify VPC ID")
	localMappingInternalShowCmd.Flags().StringVar(&mappingIP, "ip", "0", "Specify mapping IP address")
	remoteMappingInternalShowCmd.Flags().StringVar(&mappingType, "type", "l3", "Specify remote mapping type - l2, l3")
	remoteMappingInternalShowCmd.Flags().Uint32Var(&subID, "subnet", 0, "Specify Subnet ID")
	remoteMappingInternalShowCmd.Flags().StringVar(&mappingMAC, "mac", "", "Specify MAC")
	remoteMappingInternalShowCmd.Flags().Uint32Var(&vpcID, "vpc", 0, "Specify VPC ID")
	remoteMappingInternalShowCmd.Flags().StringVar(&mappingIP, "ip", "0", "Specify mapping IP address")
	remoteMappingInternalShowCmd.MarkFlagRequired("type")

	mappingShowCmd.AddCommand(localMappingShowCmd)
	mappingShowCmd.AddCommand(remoteMappingShowCmd)
	localMappingShowCmd.Flags().StringVar(&vpcStr, "vpc", "", "Specify VPC uuid")
	localMappingShowCmd.Flags().StringVar(&mappingIP, "ip", "0", "Specify mapping IP address")
	localMappingShowCmd.Flags().StringVar(&subStr, "subnet", "", "Specify Subnet uuid")
	localMappingShowCmd.Flags().StringVar(&mappingMAC, "mac", "", "Specify MAC")
	localMappingShowCmd.Flags().StringVar(&mappingID, "uuid", "0", "Specify mapping uuid")
	localMappingShowCmd.Flags().StringVar(&mappingType, "type", "l3", "Specify local mapping type - l2, l3")
	localMappingShowCmd.MarkFlagRequired("type")
	remoteMappingShowCmd.Flags().StringVar(&mappingType, "type", "l3", "Specify remote mapping type - l2, l3")
	remoteMappingShowCmd.Flags().StringVar(&subStr, "subnet", "", "Specify Subnet uuid")
	remoteMappingShowCmd.Flags().StringVar(&mappingMAC, "mac", "", "Specify MAC")
	remoteMappingShowCmd.Flags().StringVar(&vpcStr, "vpc", "", "Specify VPC uuid")
	remoteMappingShowCmd.Flags().StringVar(&mappingIP, "ip", "0", "Specify mapping IP address")
	remoteMappingShowCmd.Flags().StringVar(&mappingID, "uuid", "0", "Specify mapping uuid")
	remoteMappingShowCmd.MarkFlagRequired("type")
}

func localMappingInternalHandler(cmd *cobra.Command, args []string) {
	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	if cmd != nil && (cmd.Flags().Changed("vpc") != cmd.Flags().Changed("ip")) {
		fmt.Printf("Cannot specify only one of VPC ID and mapping IP address\n")
		return
	}

	var cmdCtxt *pds.CommandCtxt
	mType := pds.MappingDumpType_MAPPING_DUMP_LOCAL

	if cmd != nil && cmd.Flags().Changed("vpc") && cmd.Flags().Changed("ip") {
		// dump specific Mapping
		key := &pds.HWMappingKey{
			Keyinfo: &pds.HWMappingKey_IPKey{
				IPKey: &pds.L3HWMappingKey{
					VPCId:  vpcID,
					IPAddr: utils.IPAddrStrToPDSIPAddr(mappingIP),
				},
			},
		}
		cmdCtxt = &pds.CommandCtxt{
			Version: 1,
			Cmd:     pds.Command_CMD_MAPPING_DUMP,
			Commandfilter: &pds.CommandCtxt_MappingDumpFilter{
				MappingDumpFilter: &pds.MappingDumpFilter{
					Key:  key,
					Type: mType,
				},
			},
		}
	} else {
		// dump all local mappings
		cmdCtxt = &pds.CommandCtxt{
			Version: 1,
			Cmd:     pds.Command_CMD_MAPPING_DUMP,
			Commandfilter: &pds.CommandCtxt_MappingDumpFilter{
				MappingDumpFilter: &pds.MappingDumpFilter{
					Type: mType,
				},
			},
		}
	}

	// handle command
	cmdResp, err := HandleCommand(cmdCtxt)
	if err != nil {
		fmt.Printf("Command failed with %v error\n", err)
		return
	}
	if cmdResp.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Command failed with %v error\n", cmdResp.ApiStatus)
		return
	}
}

func localMappingHandler(cmd *cobra.Command, args []string) {
	// Connect to PDS
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the PDS. Is PDS Running?\n")
		return
	}
	defer c.Close()

	var req *pds.MappingGetRequest

	if cmd != nil && cmd.Flags().Changed("uuid") {
		req = &pds.MappingGetRequest{
			Id: [][]byte{uuid.FromStringOrNil(mappingID).Bytes()},
		}
	} else {
		req = &pds.MappingGetRequest{
			Id: [][]byte{},
		}
	}

	if cmd != nil {
		if mappingType == "l2" {
			if cmd.Flags().Changed("vpc") || cmd.Flags().Changed("ip") {
				fmt.Printf("--type and filters specified do not match\n")
				return
			}
			if cmd.Flags().Changed("subnet") != cmd.Flags().Changed("mac") {
				fmt.Printf("Cannot specify only one of subnet ID and mapping MAC address\n")
				return
			}
		}
		if mappingType == "l3" {
			if cmd.Flags().Changed("subnet") || cmd.Flags().Changed("mac") {
				fmt.Printf("--type and filters specified do not match\n")
				return
			}
			if cmd.Flags().Changed("vpc") != cmd.Flags().Changed("ip") {
				fmt.Printf("Cannot specify only one of VPC ID and mapping IP address\n")
				return
			}
		}
	}

	client := pds.NewMappingSvcClient(c)

	// PDS Call
	respMsg, err := client.MappingGet(context.Background(), req)
	if err != nil {
		fmt.Printf("Getting mapping failed. %v\n", err)
		return
	}

	if respMsg.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", respMsg.ApiStatus)
		return
	}

	// Print local mapping
	printLocalMappingHeader()
	for _, resp := range respMsg.Response {
		if localMappingFilterCheck(resp, cmd) {
			printLocalMapping(resp)
		}
	}
}

func localMappingFilterCheck(resp *pds.Mapping, cmd *cobra.Command) bool {
	spec := resp.GetSpec()
	switch spec.GetDstinfo().(type) {
	case *pds.MappingSpec_TunnelId:
		return false
	case *pds.MappingSpec_NexthopGroupId:
		return false
	default:
		break
	}
	switch spec.GetMacOrIp().(type) {
	case *pds.MappingSpec_MACKey:
		if mappingType != "l2" {
			return false
		}
		if cmd.Flags().Changed("subnet") {
			if uuid.FromBytesOrNil(spec.GetMACKey().GetSubnetId()).String() != subStr {
				return false
			}
			if spec.GetMACKey().GetMACAddr() !=
				utils.MACAddrStrToUint64(mappingMAC) {
				return false
			}
		}
	case *pds.MappingSpec_IPKey:
		if mappingType != "l3" {
			return false
		}
		if cmd.Flags().Changed("vpc") {
			if uuid.FromBytesOrNil(spec.GetIPKey().GetVPCId()).String() != vpcStr {
				return false
			}
			if utils.IPAddrToStr(spec.GetIPKey().GetIPAddr()) != mappingIP {
				return false
			}
		}
	default:
		return false
	}
	return true
}

func printLocalMappingHeader() {
	if mappingType == "l2" {
		hdrLine := strings.Repeat("-", 266)
		fmt.Println(hdrLine)
		fmt.Printf("%-40s%-40s%-18s%-40s%-18s%-16s%-40s%-14s\n",
			"ID", "SubnetID", "MAC", "VnicID", "VnicMAC", "Encap",
			"PublicIP", "Tags")
		fmt.Println(hdrLine)
	} else {
		hdrLine := strings.Repeat("-", 328)
		fmt.Println(hdrLine)
		fmt.Printf("%-40s%-40s%-40s%-40s%-40s%-18s%-16s%-40s%-14s\n",
			"ID", "VpcID", "IPAddress", "SubnetID", "VnicID", "VnicMAC", "Encap",
			"PublicIP", "Tags")
		fmt.Println(hdrLine)
	}
}

func printLocalMapping(resp *pds.Mapping) {
	spec := resp.GetSpec()
	tags := fmt.Sprintf("%-14v", spec.GetTags())
	tags = strings.Replace(tags, "[", "", -1)
	tags = strings.Replace(tags, "]", "", -1)
	if len(tags) == 0 {
		tags = "-"
	}
	switch spec.GetMacOrIp().(type) {
	case *pds.MappingSpec_MACKey:
		fmt.Printf("%-40s%-40s%-18s%-40s%-18s%-16s%-40s%-14s\n",
			uuid.FromBytesOrNil(spec.GetId()).String(),
			uuid.FromBytesOrNil(spec.GetMACKey().GetSubnetId()).String(),
			utils.MactoStr(spec.GetMACKey().GetMACAddr()),
			uuid.FromBytesOrNil(spec.GetVnicId()).String(),
			utils.MactoStr(spec.GetMACAddr()),
			utils.EncapToString(spec.GetEncap()),
			utils.IPAddrToStr(spec.GetPublicIP()), tags)
	case *pds.MappingSpec_IPKey:
		fmt.Printf("%-40s%-40s%-40s%-40s%-40s%-18s%-16s%-40s%-14s\n",
			uuid.FromBytesOrNil(spec.GetId()).String(),
			uuid.FromBytesOrNil(spec.GetIPKey().GetVPCId()).String(),
			utils.IPAddrToStr(spec.GetIPKey().GetIPAddr()),
			uuid.FromBytesOrNil(spec.GetSubnetId()).String(),
			uuid.FromBytesOrNil(spec.GetVnicId()).String(),
			utils.MactoStr(spec.GetMACAddr()),
			utils.EncapToString(spec.GetEncap()),
			utils.IPAddrToStr(spec.GetPublicIP()), tags)
	default:
		return
	}
	return
}

func remoteMappingHandler(cmd *cobra.Command, args []string) {
	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	// Connect to PDS
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the PDS. Is PDS Running?\n")
		return
	}
	defer c.Close()

	var req *pds.MappingGetRequest

	if cmd != nil && cmd.Flags().Changed("uuid") {
		req = &pds.MappingGetRequest{
			Id: [][]byte{uuid.FromStringOrNil(mappingID).Bytes()},
		}
	} else {
		req = &pds.MappingGetRequest{
			Id: [][]byte{},
		}
	}

	if cmd != nil {
		if mappingType == "l2" {
			if cmd.Flags().Changed("vpc") || cmd.Flags().Changed("ip") {
				fmt.Printf("--type and filters specified do not match\n")
				return
			}
			if cmd.Flags().Changed("subnet") != cmd.Flags().Changed("mac") {
				fmt.Printf("Cannot specify only one of subnet ID and mapping MAC address\n")
				return
			}
		}
		if mappingType == "l3" {
			if cmd.Flags().Changed("subnet") || cmd.Flags().Changed("mac") {
				fmt.Printf("--type and filters specified do not match\n")
				return
			}
			if cmd.Flags().Changed("vpc") != cmd.Flags().Changed("ip") {
				fmt.Printf("Cannot specify only one of VPC ID and mapping IP address\n")
				return
			}
		}
	}

	client := pds.NewMappingSvcClient(c)

	// PDS Call
	respMsg, err := client.MappingGet(context.Background(), req)
	if err != nil {
		fmt.Printf("Getting mapping failed. %v\n", err)
		return
	}

	if respMsg.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", respMsg.ApiStatus)
		return
	}

	// Print local mapping
	printRemoteMappingHeader()
	for _, resp := range respMsg.Response {
		if remoteMappingFilterCheck(resp, cmd) {
			printRemoteMapping(resp)
		}
	}
}

func remoteMappingFilterCheck(resp *pds.Mapping, cmd *cobra.Command) bool {
	spec := resp.GetSpec()
	switch spec.GetDstinfo().(type) {
	case *pds.MappingSpec_VnicId:
		return false
	default:
		break
	}
	switch spec.GetMacOrIp().(type) {
	case *pds.MappingSpec_MACKey:
		if mappingType != "l2" {
			return false
		}
		if cmd.Flags().Changed("subnet") {
			if uuid.FromBytesOrNil(spec.GetMACKey().GetSubnetId()).String() != subStr {
				return false
			}
			if spec.GetMACKey().GetMACAddr() !=
				utils.MACAddrStrToUint64(mappingMAC) {
				return false
			}
		}
	case *pds.MappingSpec_IPKey:
		if mappingType != "l3" {
			return false
		}
		if cmd.Flags().Changed("vpc") {
			if uuid.FromBytesOrNil(spec.GetIPKey().GetVPCId()).String() != vpcStr {
				return false
			}
			if utils.IPAddrToStr(spec.GetIPKey().GetIPAddr()) != mappingIP {
				return false
			}
		}
	default:
		return false
	}
	return true
}

func printRemoteMappingHeader() {
	if mappingType == "l2" {
		hdrLine := strings.Repeat("-", 274)
		fmt.Println(hdrLine)
		fmt.Printf("%-40s%-40s%-18s%-8s%-18s%-16s%-40s%-14s\n",
			"ID", "SubnetID", "MAC", "NhType", "VnicMAC", "Encap",
			"PublicIP", "Tags")
		fmt.Println(hdrLine)
	} else {
		hdrLine := strings.Repeat("-", 336)
		fmt.Println(hdrLine)
		fmt.Printf("%-40s%-40s%-40s%-40s%-8s%-18s%-16s%-40s%-14s\n",
			"ID", "VpcID", "IPAddress", "SubnetID", "NhType", "VnicMAC",
			"Encap", "PublicIP", "Tags")
		fmt.Println(hdrLine)
	}
}

func printRemoteMapping(resp *pds.Mapping) {
	spec := resp.GetSpec()
	tags := fmt.Sprintf("%-14v", spec.GetTags())
	tags = strings.Replace(tags, "[", "", -1)
	tags = strings.Replace(tags, "]", "", -1)
	if len(tags) == 0 {
		tags = "-"
	}
	var nhType string
	switch spec.GetDstinfo().(type) {
	case *pds.MappingSpec_TunnelId:
		nhType = "overlay"
	case *pds.MappingSpec_NexthopGroupId:
		nhType = "ecmp"
	}
	switch spec.GetMacOrIp().(type) {
	case *pds.MappingSpec_MACKey:
		fmt.Printf("%-40s%-40s%-18s%-8s%-18s%-16s%-40s%-14s\n",
			uuid.FromBytesOrNil(spec.GetId()).String(),
			uuid.FromBytesOrNil(spec.GetMACKey().GetSubnetId()).String(),
			utils.MactoStr(spec.GetMACKey().GetMACAddr()),
			nhType, utils.MactoStr(spec.GetMACAddr()),
			utils.EncapToString(spec.GetEncap()),
			utils.IPAddrToStr(spec.GetPublicIP()), tags)
	case *pds.MappingSpec_IPKey:
		fmt.Printf("%-40s%-40s%-40s%-40s%-8s%-18s%-16s%-40s%-14s\n",
			uuid.FromBytesOrNil(spec.GetId()).String(),
			uuid.FromBytesOrNil(spec.GetIPKey().GetVPCId()).String(),
			utils.IPAddrToStr(spec.GetIPKey().GetIPAddr()),
			uuid.FromBytesOrNil(spec.GetSubnetId()).String(),
			nhType, utils.MactoStr(spec.GetMACAddr()),
			utils.EncapToString(spec.GetEncap()),
			utils.IPAddrToStr(spec.GetPublicIP()), tags)
	default:
		return
	}
	return
}

func remoteL2MappingInternalHandler(cmd *cobra.Command) {
	if cmd != nil && (cmd.Flags().Changed("vpc") != cmd.Flags().Changed("ip")) {
		fmt.Printf("Cannot specify only one of VPC ID and mapping IP address\n")
		return
	}

	var cmdCtxt *pds.CommandCtxt
	mType := pds.MappingDumpType_MAPPING_DUMP_REMOTE_L2

	if cmd != nil && cmd.Flags().Changed("subnet") && cmd.Flags().Changed("mac") {
		// dump specific Mapping
		key := &pds.HWMappingKey{
			Keyinfo: &pds.HWMappingKey_MACKey{
				MACKey: &pds.L2HWMappingKey{
					SubnetId: subID,
					MACAddr:  utils.MACAddrStrToUint64(mappingMAC),
				},
			},
		}
		cmdCtxt = &pds.CommandCtxt{
			Version: 1,
			Cmd:     pds.Command_CMD_MAPPING_DUMP,
			Commandfilter: &pds.CommandCtxt_MappingDumpFilter{
				MappingDumpFilter: &pds.MappingDumpFilter{
					Key:  key,
					Type: mType,
				},
			},
		}
	} else {
		// dump all l2 remote mappings
		cmdCtxt = &pds.CommandCtxt{
			Version: 1,
			Cmd:     pds.Command_CMD_MAPPING_DUMP,
			Commandfilter: &pds.CommandCtxt_MappingDumpFilter{
				MappingDumpFilter: &pds.MappingDumpFilter{
					Type: mType,
				},
			},
		}
	}

	// handle command
	cmdResp, err := HandleCommand(cmdCtxt)
	if err != nil {
		fmt.Printf("Command failed with %v error\n", err)
		return
	}
	if cmdResp.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Command failed with %v error\n", cmdResp.ApiStatus)
		return
	}
}

func remoteL3MappingInternalHandler(cmd *cobra.Command) {
	if cmd != nil && (cmd.Flags().Changed("vpc") != cmd.Flags().Changed("ip")) {
		fmt.Printf("Cannot specify only one of VPC ID and mapping IP address\n")
		return
	}

	var cmdCtxt *pds.CommandCtxt
	mType := pds.MappingDumpType_MAPPING_DUMP_REMOTE_L3

	if cmd != nil && cmd.Flags().Changed("vpc") && cmd.Flags().Changed("ip") {
		// dump specific Mapping
		key := &pds.HWMappingKey{
			Keyinfo: &pds.HWMappingKey_IPKey{
				IPKey: &pds.L3HWMappingKey{
					VPCId:  vpcID,
					IPAddr: utils.IPAddrStrToPDSIPAddr(mappingIP),
				},
			},
		}
		cmdCtxt = &pds.CommandCtxt{
			Version: 1,
			Cmd:     pds.Command_CMD_MAPPING_DUMP,
			Commandfilter: &pds.CommandCtxt_MappingDumpFilter{
				MappingDumpFilter: &pds.MappingDumpFilter{
					Key:  key,
					Type: mType,
				},
			},
		}
	} else {
		// dump all l3 remote mappings
		cmdCtxt = &pds.CommandCtxt{
			Version: 1,
			Cmd:     pds.Command_CMD_MAPPING_DUMP,
			Commandfilter: &pds.CommandCtxt_MappingDumpFilter{
				MappingDumpFilter: &pds.MappingDumpFilter{
					Type: mType,
				},
			},
		}
	}

	// handle command
	cmdResp, err := HandleCommand(cmdCtxt)
	if err != nil {
		fmt.Printf("Command failed with %v error\n", err)
		return
	}
	if cmdResp.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Command failed with %v error\n", cmdResp.ApiStatus)
		return
	}
}

func remoteMappingInternalHandler(cmd *cobra.Command, args []string) {
	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	switch mappingType {
	case "l2":
		remoteL2MappingInternalHandler(cmd)
	case "l3":
		remoteL3MappingInternalHandler(cmd)
	default:
		fmt.Printf("Invalid mapping type specified\n")
	}
	return
}
