//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"fmt"

	"github.com/spf13/cobra"

	"github.com/pensando/sw/nic/apollo/agent/cli/utils"
	"github.com/pensando/sw/nic/apollo/agent/gen/pds"
)

var (
	vpcID       uint32
	mappingIP   string
	mappingType string
	subID       uint32
	mappingMAC  string
)

var mappingShowCmd = &cobra.Command{
	Use:   "mapping",
	Short: "show Mapping information",
	Long:  "show Mapping object information",
}

var localMappingShowCmd = &cobra.Command{
	Use:   "local",
	Short: "show local mapping information",
	Long:  "show local mapping object information",
	Run:   localMappingShowCmdHandler,
}

var remoteMappingShowCmd = &cobra.Command{
	Use:   "remote",
	Short: "show remote mapping information",
	Long:  "show remote mapping object information",
	Run:   remoteMappingShowCmdHandler,
}

func init() {
	showCmd.AddCommand(mappingShowCmd)
	mappingShowCmd.AddCommand(localMappingShowCmd)
	mappingShowCmd.AddCommand(remoteMappingShowCmd)
	localMappingShowCmd.Flags().Uint32Var(&vpcID, "vpc", 0, "Specify VPC ID")
	localMappingShowCmd.Flags().StringVar(&mappingIP, "ip", "0", "Specify mapping IP address")
	remoteMappingShowCmd.Flags().StringVar(&mappingType, "type", "l3", "Specify remote mapping type - l2, l3")
	remoteMappingShowCmd.Flags().Uint32Var(&subID, "subnet", 0, "Specify Subnet ID")
	remoteMappingShowCmd.Flags().StringVar(&mappingMAC, "mac", "", "Specify MAC")
	remoteMappingShowCmd.Flags().Uint32Var(&vpcID, "vpc", 0, "Specify VPC ID")
	remoteMappingShowCmd.Flags().StringVar(&mappingIP, "ip", "0", "Specify mapping IP address")
	remoteMappingShowCmd.MarkFlagRequired("type")
}

func localMappingShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to PDS
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the PDS. Is PDS Running?\n")
		return
	}
	defer c.Close()

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
	if cmdResp.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Command failed with %v error\n", cmdResp.ApiStatus)
		return
	}
}

func remoteL2MappingShowCmdHandler(cmd *cobra.Command) {
	// Connect to PDS
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the PDS. Is PDS Running?\n")
		return
	}
	defer c.Close()

	if cmd != nil && cmd.Flags().Changed("subnet") != cmd.Flags().Changed("mac") {
		fmt.Printf("Cannot specify only one of Subnet ID and mapping MAC\n")
		return
	}

	mType := pds.MappingDumpType_MAPPING_DUMP_REMOTE_L2

	var cmdCtxt *pds.CommandCtxt

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
	if cmdResp.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Command failed with %v error\n", cmdResp.ApiStatus)
		return
	}
}

func remoteL3MappingShowCmdHandler(cmd *cobra.Command) {
	// Connect to PDS
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the PDS. Is PDS Running?\n")
		return
	}
	defer c.Close()

	if cmd != nil && cmd.Flags().Changed("vpc") != cmd.Flags().Changed("ip") {
		fmt.Printf("Cannot specify only one of VPC ID and mapping IP address\n")
		return
	}

	mType := pds.MappingDumpType_MAPPING_DUMP_REMOTE_L3

	var cmdCtxt *pds.CommandCtxt

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
	if cmdResp.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Command failed with %v error\n", cmdResp.ApiStatus)
		return
	}
}

func remoteMappingShowCmdHandler(cmd *cobra.Command, args []string) {
	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	switch mappingType {
	case "l2":
		remoteL2MappingShowCmdHandler(cmd)
	case "l3":
		remoteL3MappingShowCmdHandler(cmd)
	default:
		fmt.Printf("Invalid mapping type specified\n")
	}
	return
}
