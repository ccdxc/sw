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
	mcastHndl      uint64
	multicastOif   bool
	pdMulticastID  uint64
	pdMulticastBr  bool
	multicastBrief bool
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
	multicastShowCmd.Flags().Uint64Var(&mcastHndl, "handle", 1, "Specify multicast handle")
	multicastShowCmd.Flags().BoolVar(&multicastOif, "oif-list", false, "Display oif list")
	multicastShowCmd.Flags().BoolVar(&multicastBrief, "brief", false, "Show multicast object in brief")
	multicastSpecShowCmd.Flags().Uint64Var(&mcastHndl, "handle", 1, "Specify multicast handle")
	multicastSpecShowCmd.Flags().BoolVar(&multicastOif, "oif-list", false, "Display oif list")
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
	client := halproto.NewMulticastClient(c)

	defer c.Close()

	if len(args) > 0 {
		if strings.Compare(args[0], "spec") != 0 {
			fmt.Printf("Invalid argument\n")
			return
		}
	}

	var req *halproto.MulticastEntryGetRequest
	if cmd.Flags().Changed("handle") {
		req = &halproto.MulticastEntryGetRequest{
			KeyOrHandle: &halproto.MulticastEntryKeyHandle{
				KeyOrHandle: &halproto.MulticastEntryKeyHandle_MulticastHandle{
					MulticastHandle: mcastHndl,
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

	if cmd.Flags().Changed("oif-list") {
		// Print Header
		multicastShowOifListHeader(cmd, args)

		// Get map of all ifs to if names
		ifIDToStr := ifGetAllStr()

		// Print Entries
		for _, resp := range respMsg.Response {
			if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
				fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
				continue
			}
			multicastShowOifList(resp.GetStatus().GetOifList(), ifIDToStr)
		}
	} else if cmd.Flags().Changed("brief") {
		// Print Header
		multicastShowBriefHeader(cmd, args)

		// Get map of all ifs to if names
		ifIDToStr := ifGetAllStr()

		// Print Entries
		for _, resp := range respMsg.Response {
			if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
				fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
				continue
			}
			multicastShowBrief(resp, ifIDToStr)
		}
	} else {
		// Print Header
		multicastShowHeader(cmd, args)

		// Print Entries
		for _, resp := range respMsg.Response {
			if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
				fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
				continue
			}
			multicastShowOneResp(resp)
		}
	}
}

func handleMulticastDetailShowCmd(cmd *cobra.Command, ofile *os.File) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	client := halproto.NewMulticastClient(c)

	defer c.Close()

	var req *halproto.MulticastEntryGetRequest
	if cmd != nil && cmd.Flags().Changed("handle") {
		req = &halproto.MulticastEntryGetRequest{
			KeyOrHandle: &halproto.MulticastEntryKeyHandle{
				KeyOrHandle: &halproto.MulticastEntryKeyHandle_MulticastHandle{
					MulticastHandle: mcastHndl,
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
			fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
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

func multicastShowBriefHeader(cmd *cobra.Command, args []string) {
	fmt.Printf("L2SegId:      L2 segment id    Group:        Destination group\n")
	fmt.Printf("OifList:      Oif list\n")
	hdrLine := strings.Repeat("-", 60)
	fmt.Println(hdrLine)
	fmt.Printf("%-10s%-20s%-30s\n",
		"L2SegId", "Group", "OifList")
	fmt.Println(hdrLine)
}

func multicastShowBrief(resp *halproto.MulticastEntryGetResponse, ifIDToStr map[uint64]string) {
	grpStr := ""

	if resp.GetSpec().GetKeyOrHandle().GetKey().GetIp() != nil {
		grp := resp.GetSpec().GetKeyOrHandle().GetKey().GetIp().GetGroup()
		grpStr = utils.IPAddrToStr(grp)
	} else {
		grp := resp.GetSpec().GetKeyOrHandle().GetKey().GetMac().GetGroup()
		grpStr = utils.MactoStr(grp)
	}

	l2SegID := resp.GetSpec().GetKeyOrHandle().GetKey().
		GetL2SegmentKeyHandle().GetSegmentId()

	oifList := resp.GetSpec().GetOifKeyHandles()
	oifStr := ""

	if len(oifList) > 0 {
		count := 0
		for i := 0; i < len(oifList); i++ {
			if i == len(oifList)-1 {
				oifStr += fmt.Sprintf("%s", ifIDToStr[oifList[i].GetInterfaceId()])
			} else {
				oifStr += fmt.Sprintf("%s,", ifIDToStr[oifList[i].GetInterfaceId()])
				count++
				if count%2 == 0 {
					oifStr += fmt.Sprintf("\n%-30s", "")
				}
			}
		}
	} else {
		oifStr += "None"
	}

	fmt.Printf("%-10d%-20s%-30s\n",
		l2SegID, grpStr, oifStr)
}

func multicastShowHeader(cmd *cobra.Command, args []string) {
	fmt.Printf("Source:        Sender of the multicast traffic    Group:        Destination group\n")
	fmt.Printf("Handle:        HAL Handle                         L2SegId:      L2 segment id\n")
	fmt.Printf("NumOIFs:       Number of outgoing interfaces      OifListId:    Oif list id\n")
	hdrLine := strings.Repeat("-", 85)
	fmt.Println(hdrLine)
	fmt.Printf("%-18s%-18s%-10s%-10s%-10s%-10s\n",
		"Source", "Group", "Handle", "L2SegId", "NumOIFs", "OifListId")
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
	l2SegID := resp.GetSpec().GetKeyOrHandle().GetKey().
		GetL2SegmentKeyHandle().GetSegmentId()

	oifList := resp.GetSpec().GetOifKeyHandles()

	fmt.Printf("%-18s%-18s%-10d%-10d%-10d%-10d\n",
		srcStr,
		grpStr,
		handle,
		l2SegID,
		len(oifList),
		resp.GetStatus().GetOifList().GetId())
}

func multicastShowOifListHeader(cmd *cobra.Command, args []string) {
	fmt.Printf("Id:            Oif list id\n")
	fmt.Printf("HonrIng:       Whether a copy will be generated as per the ingress lookup decision\n")
	fmt.Printf("NextList:      The id of the next list, if any, that this list is spliced to\n")
	fmt.Printf("L2SegId:       Oif L2 segment id\n")
	fmt.Printf("OifIntf:       Oif Interface id:queue id:queue purpose\n")
	hdrLine := strings.Repeat("-", 68)
	fmt.Println(hdrLine)
	fmt.Printf("%-10s%-10s%-10s%-8s%-30s\n", "Id", "HonrIng", "NextList", "L2SegId", "OifIntf")
	fmt.Println(hdrLine)
}

func multicastShowOifList(oifList *halproto.OifList, ifIDToStr map[uint64]string) {
	honorStr := ""
	first := true

	if oifList.GetIsHonorIngress() {
		honorStr = "Yes"
	} else {
		honorStr = "No"
	}

	fmt.Printf("%-10d%-10s%-10d", oifList.GetId(), honorStr, oifList.GetAttachedListId())
	for i := 0; i < len(oifList.GetOifs()); i++ {
		if first != true {
			fmt.Printf("%-30s", "")
		}
		fmt.Printf("%-8d%-30s\n",
			oifList.GetOifs()[i].GetL2Segment().GetSegmentId(),
			fmt.Sprintf("%s:%d:%d", ifIDToStr[oifList.GetOifs()[i].GetInterface().GetInterfaceId()],
				oifList.GetOifs()[i].GetQId(),
				oifList.GetOifs()[i].GetQPurpose()))
		first = false
	}
}
