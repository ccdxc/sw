//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	"os"
	"reflect"
	"strings"

	"github.com/spf13/cobra"
	yaml "gopkg.in/yaml.v2"

	"github.com/pensando/sw/nic/agent/cmd/halctl/utils"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/venice/utils/log"
)

var nwShowCmd = &cobra.Command{
	Use:   "network",
	Short: "show network objects",
	Long:  "show network object information",
	Run:   nwShowSpecCmdHandler,
}

var nwSpecShowCmd = &cobra.Command{
	Use:   "spec",
	Short: "show network spec information",
	Long:  "show network spec information",
	Run:   nwShowSpecCmdHandler,
}

var nwStatusShowCmd = &cobra.Command{
	Use:   "status",
	Short: "show network status information",
	Long:  "show network status information",
	Run:   nwShowStatusCmdHandler,
}

var nwDetailShowCmd = &cobra.Command{
	Use:   "detail",
	Short: "show detailed network information",
	Long:  "show detailed information about network objects",
	Run:   nwDetailShowCmdHandler,
}

func init() {
	showCmd.AddCommand(nwShowCmd)
	nwShowCmd.AddCommand(nwSpecShowCmd)
	nwShowCmd.AddCommand(nwStatusShowCmd)
	nwShowCmd.AddCommand(nwDetailShowCmd)
}

func nwShowCmdHandler(cmd *cobra.Command, spec bool, status bool) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		log.Fatalf("Could not connect to the HAL. Is HAL Running?")
	}
	client := halproto.NewNetworkClient(c.ClientConn)

	// Get all Nw
	req := &halproto.NetworkGetRequest{}
	networkGetReqMsg := &halproto.NetworkGetRequestMsg{
		Request: []*halproto.NetworkGetRequest{req},
	}

	// HAL call
	respMsg, err := client.NetworkGet(context.Background(), networkGetReqMsg)
	if err != nil {
		log.Errorf("Getting Network failed. %v", err)
	}

	// Print Header
	if spec == true {
		nwShowSpecHeader()
	}

	if status == true {
		nwShowStatusHeader()
	}

	// Print NHs
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL Returned non OK status. %v", resp.ApiStatus)
			continue
		}
		if spec == true {
			nwSpecShowOneResp(resp)
		}
		if status == true {
			nwStatusShowOneResp(resp)
		}
	}
}

func nwShowSpecCmdHandler(cmd *cobra.Command, args []string) {
	if len(args) > 0 {
		if strings.Compare(args[0], "spec") != 0 {
			fmt.Printf("Invalid argument\n")
			return
		}
	}

	nwShowCmdHandler(cmd, true, false)
}

func nwShowStatusCmdHandler(cmd *cobra.Command, args []string) {
	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	nwShowCmdHandler(cmd, false, true)
}

func handleNwDetailShowCmd(cmd *cobra.Command, ofile *os.File) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		log.Fatalf("Could not connect to the HAL. Is HAL Running?")
	}
	client := halproto.NewNetworkClient(c.ClientConn)

	// Get all Nw
	req := &halproto.NetworkGetRequest{}
	networkGetReqMsg := &halproto.NetworkGetRequestMsg{
		Request: []*halproto.NetworkGetRequest{req},
	}

	// HAL call
	respMsg, err := client.NetworkGet(context.Background(), networkGetReqMsg)
	if err != nil {
		log.Errorf("Getting Network failed. %v", err)
	}

	// Print NHs
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL Returned non OK status. %v", resp.ApiStatus)
			continue
		}
		respType := reflect.ValueOf(resp)
		b, _ := yaml.Marshal(respType.Interface())
		if ofile != nil {
			if _, err := ofile.WriteString(string(b)); err != nil {
				log.Errorf("Failed to write to file %s, err : %v",
					ofile.Name(), err)
			}
		} else {
			fmt.Println(string(b))
			fmt.Println("---")
		}
	}
}

func nwDetailShowCmdHandler(cmd *cobra.Command, args []string) {
	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	handleNwDetailShowCmd(cmd, nil)
}

func nwShowSpecHeader() {
	fmt.Printf("\n")
	fmt.Printf("VrfId:       Network's vrf Id    GatewayIP:   Gateway IP\n")
	fmt.Printf("Subnet:      Network's subnet    Router MAC:  Router MAC\n")
	fmt.Printf("SGHandle:    Security Group Handles\n")
	hdrLine := strings.Repeat("-", 86)
	fmt.Println(hdrLine)
	fmt.Printf("%-6s%-18s%-32s%-18s%-24s\n",
		"VrfId", "GatewayIP", "Subnet", "Router MAC", "SGHandle")
	fmt.Println(hdrLine)
}

func nwShowStatusHeader() {
	hdrLine := strings.Repeat("-", 15)
	fmt.Println(hdrLine)
	fmt.Printf("%-15s\n", "Network Handle")
	fmt.Println(hdrLine)
}

func nwSpecShowOneResp(resp *halproto.NetworkGetResponse) {
	spec := resp.GetSpec()

	sgKhList := spec.GetSgKeyHandle()
	sgKhStr := ""
	first := true
	for _, sgKh := range sgKhList {
		if first == true {
			first = false
			sgKhStr += fmt.Sprintf("%d", sgKh.GetSecurityGroupHandle())
		} else {
			sgKhStr += fmt.Sprintf(", %d", sgKh.GetSecurityGroupHandle())
		}
	}
	if len(sgKhStr) == 0 {
		sgKhStr += "-"
	}

	fmt.Printf("%-6d%-18s%-32s%-18s%-24s\n",
		spec.GetKeyOrHandle().GetNwKey().GetVrfKeyHandle().GetVrfId(),
		utils.IPAddrToStr(spec.GetGatewayIp()),
		utils.IPPrefixToStr(spec.GetKeyOrHandle().GetNwKey().GetIpPrefix()),
		utils.MactoStr(spec.GetRmac()), sgKhStr)
}

func nwStatusShowOneResp(resp *halproto.NetworkGetResponse) {
	status := resp.GetStatus()
	fmt.Printf("%-15d\n", status.GetNwHandle())
}
