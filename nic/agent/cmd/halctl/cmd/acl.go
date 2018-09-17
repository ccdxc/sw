//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	"os"
	"reflect"

	"github.com/spf13/cobra"
	yaml "gopkg.in/yaml.v2"

	"github.com/pensando/sw/nic/agent/cmd/halctl/utils"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/venice/utils/log"
)

var (
	aclID     uint32
	aclHandle uint64
)

var aclShowCmd = &cobra.Command{
	Use:   "acl",
	Short: "show acl information",
	Long:  "show acl object information",
	Run:   aclShowCmdHandler,
}

func init() {
	showCmd.AddCommand(aclShowCmd)

	aclShowCmd.Flags().Uint32Var(&aclID, "id", 0, "Specify acl ID")
	aclShowCmd.Flags().Uint64Var(&aclHandle, "handle", 0, "Specify acl handle")
}

func handleACLShowCmd(cmd *cobra.Command, ofile *os.File) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		log.Fatalf("Could not connect to the HAL. Is HAL Running?")
	}
	defer c.Close()

	client := halproto.NewAclClient(c.ClientConn)

	var req *halproto.AclGetRequest
	if cmd != nil && cmd.Flags().Changed("handle") {
		// Get specific acl
		req = &halproto.AclGetRequest{
			KeyOrHandle: &halproto.AclKeyHandle{
				KeyOrHandle: &halproto.AclKeyHandle_AclHandle{
					AclHandle: aclHandle,
				},
			},
		}
	} else if cmd != nil && cmd.Flags().Changed("id") {
		// Get specific acl
		req = &halproto.AclGetRequest{
			KeyOrHandle: &halproto.AclKeyHandle{
				KeyOrHandle: &halproto.AclKeyHandle_AclId{
					AclId: aclID,
				},
			},
		}
	} else {
		// Get all acls
		req = &halproto.AclGetRequest{}
	}

	aclGetReqMsg := &halproto.AclGetRequestMsg{
		Request: []*halproto.AclGetRequest{req},
	}

	// HAL call
	respMsg, err := client.AclGet(context.Background(), aclGetReqMsg)
	if err != nil {
		log.Errorf("Getting acl failed. %v", err)
	}

	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL Returned non OK status. %v", resp.ApiStatus)
			continue
		}
		respType := reflect.ValueOf(resp)
		b, _ := yaml.Marshal(respType.Interface())
		if ofile != nil {
			if _, err := ofile.WriteString(string(b) + "\n"); err != nil {
				log.Errorf("Failed to write to file %s, err : %v",
					ofile.Name(), err)
			}
		} else {
			fmt.Println(string(b) + "\n")
			fmt.Println("---")
		}
	}
}

func aclShowCmdHandler(cmd *cobra.Command, args []string) {
	handleACLShowCmd(cmd, nil)
}
