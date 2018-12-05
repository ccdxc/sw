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
)

var (
	multicastID   uint64
	multicastBr   bool
	pdMulticastID uint64
	pdMulticastBr bool
)

var multicastShowCmd = &cobra.Command{
	Use:   "multicast",
	Short: "show multicast related objects",
	Long:  "show multicast related object information",
	Run:   multicastShowSpecCmdHandler,
}

var multicastSpecShowCmd = &cobra.Command{
	Use:   "spec",
	Short: "show multicast's spec information",
	Long:  "show multicast spec information",
	Run:   multicastShowSpecCmdHandler,
}

func init() {
	showCmd.AddCommand(multicastShowCmd)
	multicastShowCmd.AddCommand(multicastSpecShowCmd)

	multicastShowCmd.Flags().Bool("yaml", false, "Output in yaml")
	multicastShowCmd.Flags().Uint64Var(&multicastID, "id", 1, "Specify multicast id")
	multicastShowCmd.Flags().BoolVar(&multicastBr, "brief", false, "Display briefly")
	multicastSpecShowCmd.Flags().Uint64Var(&multicastID, "id", 1, "Specify multicast id")
	multicastSpecShowCmd.Flags().BoolVar(&multicastBr, "brief", false, "Display briefly")
}

func multicastShowSpecCmdHandler(cmd *cobra.Command, args []string) {
	if cmd.Flags().Changed("yaml") {
		multicastDetailShowCmdHandler(cmd, args)
		return
	}

	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	client := halproto.NewMulticastClient(c.ClientConn)

	defer c.Close()

	if len(args) > 0 {
		if strings.Compare(args[0], "spec") != 0 {
			fmt.Printf("Invalid argument\n")
			return
		}
	}

	var req *halproto.MulticastEntryGetRequest
	if cmd.Flags().Changed("id") {
		req = &halproto.MulticastEntryGetRequest{
			KeyOrHandle: &halproto.MulticastEntryKeyHandle{
				KeyOrHandle: &halproto.MulticastEntryKeyHandle_MulticastHandle{
					MulticastHandle: multicastID,
				},
			},
		}
	} else {
		// Get all multicasts
		req = &halproto.MulticastEntryGetRequest{}
	}
	MulticastEntryGetReqMsg := &halproto.MulticastEntryGetRequestMsg{
		Request: []*halproto.MulticastEntryGetRequest{req},
	}

	// HAL call
	respMsg, err := client.MulticastEntryGet(context.Background(), MulticastEntryGetReqMsg)
	if err != nil {
		fmt.Printf("Getting multicast failed. %v\n", err)
		return
	}

	// Print Header
	multicastShowHeader(cmd, args)

	// Print VRFs
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("HAL Returned non OK status. %v\n", resp.ApiStatus)
			continue
		}
		multicastShowOneResp(resp)
	}
}

func handleMulticastDetailShowCmd(cmd *cobra.Command, ofile *os.File) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	client := halproto.NewMulticastClient(c.ClientConn)

	defer c.Close()

	var req *halproto.MulticastEntryGetRequest
	if cmd != nil && cmd.Flags().Changed("id") {
		req = &halproto.MulticastEntryGetRequest{
			KeyOrHandle: &halproto.MulticastEntryKeyHandle{
				KeyOrHandle: &halproto.MulticastEntryKeyHandle_MulticastHandle{
					MulticastHandle: multicastID,
				},
			},
		}
	} else {
		// Get all multicasts
		req = &halproto.MulticastEntryGetRequest{}
	}
	MulticastEntryGetReqMsg := &halproto.MulticastEntryGetRequestMsg{
		Request: []*halproto.MulticastEntryGetRequest{req},
	}

	// HAL call
	respMsg, err := client.MulticastEntryGet(context.Background(), MulticastEntryGetReqMsg)
	if err != nil {
		fmt.Printf("Getting multicast failed. %v\n", err)
		return
	}

	// Print multicasts
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

func multicastDetailShowCmdHandler(cmd *cobra.Command, args []string) {
	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}
	handleMulticastDetailShowCmd(cmd, nil)
}

func multicastShowHeader(cmd *cobra.Command, args []string) {
	fmt.Printf("\n")
	fmt.Printf("Source:        Sender of the multicast traffic    Group:         Destination Group\n")
	fmt.Printf("Handle:        multicast Handle                   L2SegId:       L2 Segment Id\n")
	fmt.Printf("NumOIFs:       Number of Outgoing Interfaces      OIFs:          Outgoing Interfaces\n")
	hdrLine := strings.Repeat("-", 135)
	fmt.Println(hdrLine)
	fmt.Printf("%-18s%-18s%-10s%-10s%-10s%-20s\n",
		"Source", "Group", "Handle", "L2SegId", "NumOIFs", "OIFs")
	fmt.Println(hdrLine)
}

func multicastShowOneResp(resp *halproto.MulticastEntryGetResponse) {

	srcStr := ""
	grpStr := ""

	if resp.GetSpec().GetKeyOrHandle().GetKey().GetIp() != nil {
		src := resp.GetSpec().GetKeyOrHandle().GetKey().GetIp().GetSource()
		grp := resp.GetSpec().GetKeyOrHandle().GetKey().GetIp().GetGroup()
		srcStr = utils.IPAddrToStr(src)
		grpStr = utils.IPAddrToStr(grp)
	} else {
		grp := resp.GetSpec().GetKeyOrHandle().GetKey().GetMac().GetGroup()
		srcStr = "*"
		grpStr = utils.MactoStr(grp)
	}

	handle := resp.GetStatus().GetHandle()
	l2SegStr := resp.GetSpec().GetKeyOrHandle().GetKey().GetL2SegmentKeyHandle()

	oifList := resp.GetSpec().GetOifKeyHandles()
	oifStr := ""

	if len(oifList) > 0 {
		for i := 0; i < len(oifList); i++ {
			oifStr += fmt.Sprintf("%d ", oifList[i].GetIfHandle())
		}
	} else {
		oifStr += "None"
	}

	fmt.Printf("%-18s%-18s%-10d%-10s%-10d%-20s\n",
		srcStr,
		grpStr,
		handle,
		l2SegStr,
		len(oifList),
		oifStr)
}
