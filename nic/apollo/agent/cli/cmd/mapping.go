//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"

	"github.com/spf13/cobra"

	"github.com/pensando/sw/nic/apollo/agent/cli/utils"
	"github.com/pensando/sw/nic/apollo/agent/gen/pds"

	"os"
	"time"
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

	// Send FD of Stdout
	cid := time.Now().Unix()
	err = utils.FdSend("/var/run/fd_recv_sock", cid, int(os.Stdout.Fd()))
	if err != nil {
		fmt.Printf("FD send operation failed with error %v\n", err)
		return
	}

	client := pds.NewMappingSvcClient(c)

	var req *pds.MappingDumpRequest
	if cmd.Flags().Changed("vpc-id") && cmd.Flags().Changed("ip") {
		// Dump specific Mapping
		var key *pds.MappingKey
		key = &pds.MappingKey{
			VPCId:  vpcID,
			IPAddr: utils.IPAddrStrToPDSIPAddr(mappingIP),
		}
		req = &pds.MappingDumpRequest{
			CId: cid,
			Id:  []*pds.MappingKey{key},
		}
	} else {
		// Dump all Mappings
		req = &pds.MappingDumpRequest{
			CId: cid,
			Id:  []*pds.MappingKey{},
		}
	}

	// PDS call
	client.MappingDump(context.Background(), req)
}
