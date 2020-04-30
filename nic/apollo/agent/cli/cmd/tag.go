//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	"reflect"

	uuid "github.com/satori/go.uuid"
	"github.com/spf13/cobra"
	yaml "gopkg.in/yaml.v2"

	"github.com/pensando/sw/nic/apollo/agent/cli/utils"
	"github.com/pensando/sw/nic/apollo/agent/gen/pds"
)

var (
	tagID string
)

var tagShowCmd = &cobra.Command{
	Use:   "tag",
	Short: "show Tag information",
	Long:  "show Tag object information",
	Run:   tagShowCmdHandler,
}

func init() {
	showCmd.AddCommand(tagShowCmd)
	tagShowCmd.Flags().Bool("yaml", false, "Output in yaml")
	tagShowCmd.Flags().Bool("summary", false, "Display number of objects")
	tagShowCmd.Flags().StringVarP(&tagID, "id", "i", "", "Specify tag policy ID")
}

func tagShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to PDS
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the PDS, is PDS running?\n")
		return
	}
	defer c.Close()

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	if cmd != nil && cmd.Flags().Changed("yaml") == false {
		fmt.Printf("Only yaml output is supported, use --yaml flag\n")
		return
	}

	client := pds.NewTagSvcClient(c)

	var req *pds.TagGetRequest
	if cmd != nil && cmd.Flags().Changed("id") {
		// Get specific Tag
		req = &pds.TagGetRequest{
			Id: [][]byte{uuid.FromStringOrNil(tagID).Bytes()},
		}
	} else {
		// Get all Tags
		req = &pds.TagGetRequest{
			Id: [][]byte{},
		}
	}

	// PDS call
	respMsg, err := client.TagGet(context.Background(), req)
	if err != nil {
		fmt.Printf("Getting Tag failed, err %v\n", err)
		return
	}

	if respMsg.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", respMsg.ApiStatus)
		return
	}

	// Print Tags
	if cmd == nil || cmd.Flags().Changed("yaml") {
		for _, resp := range respMsg.Response {
			respType := reflect.ValueOf(resp)
			b, _ := yaml.Marshal(respType.Interface())
			fmt.Println(string(b))
			fmt.Println("---")
		}
	} else if cmd.Flags().Changed("summary") {
		printTagSummary(len(respMsg.Response))
	}
}

func printTagSummary(count int) {
	fmt.Printf("\nNo. of tags : %d\n\n", count)
}
