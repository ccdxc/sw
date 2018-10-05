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
)

var (
	nwsecProfID   uint32
	nwsecVrfID    uint64
	nwsecPolicyID uint64
)

var nwsecShowCmd = &cobra.Command{
	Use:   "nwsec",
	Short: "show Security information",
	Long:  "show Security object information",
}

var nwsecProfShowCmd = &cobra.Command{
	Use:   "profile",
	Short: "show Security profile information",
	Long:  "show Security profile object information",
	Run:   nwsecProfShowCmdHandler,
}

var nwsecPolicyShowCmd = &cobra.Command{
	Use:   "policy",
	Short: "show Security policy information",
	Long:  "show Security policy object information",
	Run:   nwsecPolicyShowCmdHandler,
}

func init() {
	showCmd.AddCommand(nwsecShowCmd)
	nwsecShowCmd.AddCommand(nwsecProfShowCmd)
	nwsecShowCmd.AddCommand(nwsecPolicyShowCmd)

	nwsecProfShowCmd.Flags().Uint32Var(&nwsecProfID, "id", 1, "Specify security profile ID")
	nwsecPolicyShowCmd.Flags().Uint64Var(&nwsecVrfID, "vrf-id", 1, "Specify VRF ID")
	nwsecPolicyShowCmd.Flags().Uint64Var(&nwsecPolicyID, "policy-id", 1, "Specify security policy ID")
}

func handleNwsecProfShowCmd(cmd *cobra.Command, ofile *os.File) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	client := halproto.NewNwSecurityClient(c.ClientConn)

	var req *halproto.SecurityProfileGetRequest
	if cmd != nil && cmd.Flags().Changed("id") {
		req = &halproto.SecurityProfileGetRequest{
			KeyOrHandle: &halproto.SecurityProfileKeyHandle{
				KeyOrHandle: &halproto.SecurityProfileKeyHandle_ProfileId{
					ProfileId: nwsecProfID,
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
		fmt.Printf("Getting Security Profile failed. %v\n", err)
		return
	}

	// Print Security Profiles
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("HAL Returned non OK status. %v\n", resp.ApiStatus)
			continue
		}
		respType := reflect.ValueOf(resp)
		b, _ := yaml.Marshal(respType.Interface())
		if ofile != nil {
			if _, err := ofile.WriteString(string(b) + "\n"); err != nil {
				fmt.Printf("Failed to write to file %s, err : %v\n",
					ofile.Name(), err)
			}
		} else {
			fmt.Println(string(b) + "\n")
			fmt.Println("---")
		}
	}
}

func nwsecProfShowCmdHandler(cmd *cobra.Command, args []string) {
	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}
	handleNwsecProfShowCmd(cmd, nil)
}

func handleNwsecPolicyShowCmd(cmd *cobra.Command, ofile *os.File) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	client := halproto.NewNwSecurityClient(c.ClientConn)

	var req *halproto.SecurityPolicyGetRequest
	if cmd != nil && cmd.Flags().Changed("vrf-id") && cmd.Flags().Changed("policy-id") {
		req = &halproto.SecurityPolicyGetRequest{
			KeyOrHandle: &halproto.SecurityPolicyKeyHandle{
				PolicyKeyOrHandle: &halproto.SecurityPolicyKeyHandle_SecurityPolicyKey{
					SecurityPolicyKey: &halproto.SecurityPolicyKey{
						SecurityPolicyId: nwsecPolicyID,
						VrfIdOrHandle: &halproto.VrfKeyHandle{
							KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
								VrfId: nwsecVrfID,
							},
						},
					},
				},
			},
		}
	} else {
		// Get all Security Policies
		req = &halproto.SecurityPolicyGetRequest{}
	}

	secPolicyGetReqMsg := &halproto.SecurityPolicyGetRequestMsg{
		Request: []*halproto.SecurityPolicyGetRequest{req},
	}

	// HAL call
	respMsg, err := client.SecurityPolicyGet(context.Background(), secPolicyGetReqMsg)
	if err != nil {
		fmt.Printf("Getting Security Policy failed. %v\n", err)
		return
	}

	// Print Security Profiles
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("HAL Returned non OK status. %v\n", resp.ApiStatus)
			fmt.Println("Policy Show not ok")
			continue
		}
		respType := reflect.ValueOf(resp)
		b, _ := yaml.Marshal(respType.Interface())
		if ofile != nil {
			if _, err := ofile.WriteString(string(b) + "\n"); err != nil {
				fmt.Printf("Failed to write to file %s, err : %v\n",
					ofile.Name(), err)
			}
		} else {
			fmt.Println(string(b) + "\n")
			fmt.Println("---")
		}
	}
}

func nwsecPolicyShowCmdHandler(cmd *cobra.Command, args []string) {
	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}
	handleNwsecPolicyShowCmd(cmd, nil)
}
