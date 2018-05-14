//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	"reflect"

	"github.com/spf13/cobra"
	yaml "gopkg.in/yaml.v2"

	"github.com/pensando/sw/nic/agent/cmd/halctl/utils"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/venice/utils/log"
)

var (
	secProfID uint32
)

var secProfShowCmd = &cobra.Command{
	Use:   "sec_prof",
	Short: "show Security Profile information",
	Long:  "show Security Profile object information",
	Run:   secProfShowCmdHandler,
}

func init() {
	showCmd.AddCommand(secProfShowCmd)

	secProfShowCmd.Flags().Uint32Var(&secProfID, "id", 1, "Specify sec-prof-id")
}

func secProfShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		log.Fatalf("Could not connect to the HAL. Is HAL Running?")
	}
	client := halproto.NewNwSecurityClient(c.ClientConn)

	var req *halproto.SecurityProfileGetRequest
	if cmd.Flags().Changed("id") {
		req = &halproto.SecurityProfileGetRequest{
			KeyOrHandle: &halproto.SecurityProfileKeyHandle{
				KeyOrHandle: &halproto.SecurityProfileKeyHandle_ProfileId{
					ProfileId: secProfID,
				},
			},
		}
	} else {
		// Get all Security Profiles
		req = &halproto.SecurityProfileGetRequest{}
	}
	secProfGetReqMsg := &halproto.SecurityProfileGetRequestMsg{
		Request: []*halproto.SecurityProfileGetRequest{req},
	}

	// HAL call
	respMsg, err := client.SecurityProfileGet(context.Background(), secProfGetReqMsg)
	if err != nil {
		log.Errorf("Getting Security Profile failed. %v", err)
	}

	// Print Security Profiles
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL Returned non OK status. %v", resp.ApiStatus)
			continue
		}
		respType := reflect.ValueOf(resp)
		b, _ := yaml.Marshal(respType.Interface())
		fmt.Println(string(b))
		fmt.Println("---")
	}
}
