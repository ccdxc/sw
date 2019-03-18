//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	"reflect"

	"github.com/spf13/cobra"
	yaml "gopkg.in/yaml.v2"

	"github.com/pensando/sw/nic/apollo/agent/cli/utils"
	"github.com/pensando/sw/nic/apollo/agent/gen/pds"
)

var (
	// ID holds PCN ID
	ID uint32
)

var pcnShowCmd = &cobra.Command{
	Use:   "pcn",
	Short: "show PCN information",
	Long:  "show PCN object information",
	Run:   pcnShowCmdHandler,
}

func init() {
	showCmd.AddCommand(pcnShowCmd)
	pcnShowCmd.Flags().Bool("yaml", false, "Output in yaml")
	pcnShowCmd.Flags().Uint32VarP(&ID, "id", "i", 0, "Specify PCN ID")
}

func pcnShowCmdHandler(cmd *cobra.Command, args []string) {
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

	if cmd.Flags().Changed("yaml") == false {
		fmt.Printf("Only yaml output supported right now. Use --yaml option\n")
		return
	}

	client := pds.NewPCNSvcClient(c)

	var req *pds.PCNGetRequest
	if cmd.Flags().Changed("id") {
		// Get specific PCN
		req = &pds.PCNGetRequest{
			Id: []uint32{ID},
		}
	} else {
		// Get all PCNs
		req = &pds.PCNGetRequest{
			Id: []uint32{ID},
		}
	}

	// PDS call
	respMsg, err := client.PCNGet(context.Background(), req)
	if err != nil {
		fmt.Printf("Getting PCN failed. %v\n", err)
		return
	}

	if respMsg.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", respMsg.ApiStatus)
		return
	}

	// Print PCNs
	for _, resp := range respMsg.Response {
		respType := reflect.ValueOf(resp)
		b, _ := yaml.Marshal(respType.Interface())
		fmt.Println(string(b))
		fmt.Println("---")
	}
}
