//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"fmt"

	uuid "github.com/satori/go.uuid"
	"github.com/spf13/cobra"

	"github.com/pensando/sw/nic/apollo/agent/cli/utils"
	"github.com/pensando/sw/nic/apollo/agent/gen/pds"
)

var (
	vpcID       string
	mappingIP   string
	mappingType string
)

var mappingShowCmd = &cobra.Command{
	Use:   "mapping",
	Short: "show Mapping information",
	Long:  "show Mapping object information",
	Run:   mappingShowCmdHandler,
}

func init() {
	showCmd.AddCommand(mappingShowCmd)
	mappingShowCmd.Flags().StringVar(&vpcID, "VPC id", "", "Specify VPC ID")
	mappingShowCmd.Flags().StringVar(&mappingIP, "ip", "0", "Specify mapping IP address")
	mappingShowCmd.Flags().StringVar(&mappingType, "type", "all", "Specify mapping type - all, local or remote")
}

func mappingShowCmdHandler(cmd *cobra.Command, args []string) {
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

	if cmd != nil && (cmd.Flags().Changed("vpc-id") != cmd.Flags().Changed("ip")) {
		fmt.Printf("Cannot specify only one of VPC ID and mapping IP address\n")
		return
	}

	mType := pds.MappingDumpType_MAPPING_DUMP_ALL
	if cmd != nil && cmd.Flags().Changed("type") {
		switch mappingType {
		case "all":
			mType = pds.MappingDumpType_MAPPING_DUMP_ALL
		case "local":
			mType = pds.MappingDumpType_MAPPING_DUMP_LOCAL
		case "remote":
			mType = pds.MappingDumpType_MAPPING_DUMP_REMOTE
		default:
			fmt.Printf("Invalid mapping type specified\n")
			return
		}
	}
	var cmdCtxt *pds.CommandCtxt

	if cmd != nil && cmd.Flags().Changed("vpc-id") && cmd.Flags().Changed("ip") {
		// dump specific Mapping
		var key *pds.MappingKey
		key = &pds.MappingKey{
			Keyinfo: &pds.MappingKey_IPKey{
				IPKey: &pds.L3MappingKey{
					VPCId:  uuid.FromStringOrNil(vpcID).Bytes(),
					IPAddr: utils.IPAddrStrToPDSIPAddr(mappingIP),
				},
			},
		}
		cmdCtxt = &pds.CommandCtxt{
			Version: 1,
			Cmd:     pds.Command_CMD_MAPPING_DUMP,
			CommandFilter: &pds.CommandCtxt_MappingDumpFilter{
				MappingDumpFilter: &pds.MappingDumpFilter{
					Key:  key,
					Type: mType,
				},
			},
		}
	} else if cmd != nil && cmd.Flags().Changed("type") {
		cmdCtxt = &pds.CommandCtxt{
			Version: 1,
			Cmd:     pds.Command_CMD_MAPPING_DUMP,
			CommandFilter: &pds.CommandCtxt_MappingDumpFilter{
				MappingDumpFilter: &pds.MappingDumpFilter{
					Type: mType,
				},
			},
		}
	} else {
		// dump all Mappings
		cmdCtxt = &pds.CommandCtxt{
			Version: 1,
			Cmd:     pds.Command_CMD_MAPPING_DUMP,
		}
	}

	// handle command
	cmdResp, err := HandleCommand(cmdCtxt)
	if cmdResp.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Command failed with %v error\n", cmdResp.ApiStatus)
		return
	}
}
