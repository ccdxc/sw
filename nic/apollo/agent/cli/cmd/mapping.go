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
	vpcID     uint32
	mappingIP string
)

var mappingShowCmd = &cobra.Command{
	Use:   "mapping",
	Short: "show Mapping information",
	Long:  "show Mapping object information",
	Run:   mappingShowCmdHandler,
}

func init() {
	showCmd.AddCommand(mappingShowCmd)
	mappingShowCmd.Flags().Uint32Var(&vpcID, "vpc-id", 0, "Specify VPC ID")
	mappingShowCmd.Flags().StringVar(&mappingIP, "ip", "0", "Specify mapping IP address")
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

	if cmd.Flags().Changed("vpc-id") != cmd.Flags().Changed("ip") {
		fmt.Printf("Cannot specify only one of VPC ID and mapping IP address\n")
		return
	}

	var cmdCtxt *pds.CommandCtxt

	if cmd.Flags().Changed("vpc-id") && cmd.Flags().Changed("ip") {
		// dump specific Mapping
		var key *pds.MappingKey
		key = &pds.MappingKey{
			VPCId:  vpcID,
			IPAddr: utils.IPAddrStrToPDSIPAddr(mappingIP),
		}
		cmdCtxt = &pds.CommandCtxt{
			Version: 1,
			Cmd:     pds.Command_CMD_MAPPING_DUMP,
			CommandFilter: &pds.CommandCtxt_MappingDumpFilter{
				MappingDumpFilter: &pds.MappingDumpFilter{
					Key: key,
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
