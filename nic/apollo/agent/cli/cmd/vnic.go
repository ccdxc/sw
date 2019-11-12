//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	"reflect"
	"strings"

	"github.com/spf13/cobra"
	yaml "gopkg.in/yaml.v2"

	"github.com/pensando/sw/nic/apollo/agent/cli/utils"
	"github.com/pensando/sw/nic/apollo/agent/gen/pds"
)

var (
	vnicID uint32
)

var vnicShowCmd = &cobra.Command{
	Use:   "vnic",
	Short: "show Vnic information",
	Long:  "show Vnic object information",
	Run:   vnicShowCmdHandler,
}

func init() {
	showCmd.AddCommand(vnicShowCmd)
	vnicShowCmd.Flags().Bool("yaml", false, "Output in yaml")
	vnicShowCmd.Flags().Uint32VarP(&vnicID, "id", "i", 0, "Specify Vnic ID")
}

func vnicShowCmdHandler(cmd *cobra.Command, args []string) {
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

	client := pds.NewVnicSvcClient(c)

	var req *pds.VnicGetRequest
	if cmd.Flags().Changed("id") {
		// Get specific Vnic
		req = &pds.VnicGetRequest{
			VnicId: []uint32{vnicID},
		}
	} else {
		// Get all Vnics
		req = &pds.VnicGetRequest{
			VnicId: []uint32{},
		}
	}

	// PDS call
	respMsg, err := client.VnicGet(context.Background(), req)
	if err != nil {
		fmt.Printf("Getting Vnic failed. %v\n", err)
		return
	}

	if respMsg.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", respMsg.ApiStatus)
		return
	}

	// Print Vnics
	if cmd.Flags().Changed("yaml") {
		for _, resp := range respMsg.Response {
			respType := reflect.ValueOf(resp)
			b, _ := yaml.Marshal(respType.Interface())
			fmt.Println(string(b))
			fmt.Println("---")
		}
	} else {
		printVnicHeader()
		for _, resp := range respMsg.Response {
			printVnic(resp)
		}
	}
}

func printVnicHeader() {
	hdrLine := strings.Repeat("-", 165)
	fmt.Println(hdrLine)
	fmt.Printf("%-7s%-6s%-9s%-14s%-20s%-10s%-14s%-18s%-18s%-11s%-10s%-10s%-10s\n",
		"VnicID", "VpcID", "SubnetID", "VnicEncap", "MAC",
		"SrcGuard", "FabricEncap", "RxMirrorSessionID",
		"TxMirrorSessionID", "SwitchVnic", "V4MeterId", "V6MeterID",
		"HostIf")
	fmt.Println(hdrLine)
}

func printVnic(vnic *pds.Vnic) {
	spec := vnic.GetSpec()
	fabricEncapStr := utils.EncapToString(spec.GetFabricEncap())
	vnicEncapStr := utils.EncapToString(spec.GetVnicEncap())
	txMirrorSessionStr := "-"
	if len(spec.GetTxMirrorSessionId()) != 0 {
		txMirrorSessionStr = strings.Replace(strings.Trim(
			fmt.Sprint(spec.GetTxMirrorSessionId()), "[]"), " ", ",", -1)
	}
	rxMirrorSessionStr := "-"
	if len(spec.GetRxMirrorSessionId()) != 0 {
		rxMirrorSessionStr = strings.Replace(strings.Trim(
			fmt.Sprint(spec.GetRxMirrorSessionId()), "[]"), " ", ",", -1)
	}
	lifName := "-"
	if spec.GetHostIfIndex() != 0 {
		lifName = lifGetNameFromIfIndex(spec.GetHostIfIndex())
	}

	fmt.Printf("%-7d%-6d%-9d%-14s%-20s%-10t%-14s%-18s%-18s%-11t%-10d%-10d%-10s\n",
		spec.GetVnicId(), spec.GetVPCId(), spec.GetSubnetId(),
		vnicEncapStr, utils.MactoStr(spec.GetMACAddress()),
		spec.GetSourceGuardEnable(), fabricEncapStr,
		rxMirrorSessionStr, txMirrorSessionStr, spec.GetSwitchVnic(),
		spec.GetV4MeterId(), spec.GetV6MeterId(), lifName)
}
