//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	"io"
	"os"
	"reflect"
	"strings"

	"github.com/spf13/cobra"
	yaml "gopkg.in/yaml.v2"

	"github.com/pensando/sw/nic/agent/cmd/halctl/utils"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
)

var (
	oifID uint32
)

var oiflShowCmd = &cobra.Command{
	Use:   "oifl",
	Short: "show oiflists",
	Long:  "show oiflists information",
	Run:   oiflShowCmdHandler,
}

func init() {
	showCmd.AddCommand(oiflShowCmd)

	oiflShowCmd.Flags().Bool("yaml", false, "Output in yaml")
	oiflShowCmd.Flags().Uint32Var(&oifID, "oifl_id", 0, "Specify oiflist id")
}

func oiflShowCmdHandler(cmd *cobra.Command, args []string) {
	if cmd.Flags().Changed("yaml") {
		oiflDetailShowCmdHandler(cmd, args)
		return
	}

	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	client := halproto.NewDebugClient(c)

	defer c.Close()

	if len(args) > 0 {
		if strings.Compare(args[0], "spec") != 0 {
			fmt.Printf("Invalid argument\n")
			return
		}
	}

	var req *halproto.OifListGetRequest
	var OifListGetReqMsg *halproto.OifListGetRequestMsg
	if cmd != nil && cmd.Flags().Changed("oifl_id") {
		req = &halproto.OifListGetRequest{
			Id: oifID,
		}
		OifListGetReqMsg = &halproto.OifListGetRequestMsg{
			Request: []*halproto.OifListGetRequest{req},
		}
	} else {
		// Get all oifls
		// req = &halproto.OifListGetRequest{}
		OifListGetReqMsg = &halproto.OifListGetRequestMsg{}
	}

	oifListShowHeader(cmd, args)
	ifIDToStr := ifGetAllStr()

	// HAL call
	stream, err := client.OifListGet(context.Background(), OifListGetReqMsg)
	if err != nil {
		fmt.Printf("Getting multicast failed. %v\n", err)
		return
	}

	for {
		respMsg, err := stream.Recv()
		if err == io.EOF {
			break
		}
		if err != nil {
			fmt.Printf("Getting session stream failure. %v\n", err)
		}

		// Print Sessions
		for _, resp := range respMsg.Response {
			if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
				fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
				continue
			}
			oifListShowOneResp(resp.GetOifList(), ifIDToStr)
		}
	}
}

func handleoiflDetailShowCmd(cmd *cobra.Command, ofile *os.File) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	client := halproto.NewDebugClient(c)

	defer c.Close()

	var req *halproto.OifListGetRequest
	var OifListGetReqMsg *halproto.OifListGetRequestMsg
	if cmd != nil && cmd.Flags().Changed("oifl_id") {
		req = &halproto.OifListGetRequest{
			Id: oifID,
		}
		OifListGetReqMsg = &halproto.OifListGetRequestMsg{
			Request: []*halproto.OifListGetRequest{req},
		}
	} else {
		// Get all oifls
		// req = &halproto.OifListGetRequest{}
		OifListGetReqMsg = &halproto.OifListGetRequestMsg{}
	}

	// HAL call
	stream, err := client.OifListGet(context.Background(), OifListGetReqMsg)
	if err != nil {
		fmt.Printf("Getting multicast failed. %v\n", err)
		return
	}

	for {
		respMsg, err := stream.Recv()
		if err == io.EOF {
			break
		}
		if err != nil {
			fmt.Printf("Getting session stream failure. %v\n", err)
		}

		// Print oifls
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
}

func oiflDetailShowCmdHandler(cmd *cobra.Command, args []string) {
	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}
	handleoiflDetailShowCmd(cmd, nil)
}

func oifListShowHeader(cmd *cobra.Command, args []string) {
	fmt.Printf("Id:       Oif list id\n")
	fmt.Printf("NextPtr:  Next list\n")
	fmt.Printf("#Oifs:    Number of OIFs\n")
	fmt.Printf("Oifs:     Interfaces\n")
	hdrLine := strings.Repeat("-", 40)
	fmt.Println(hdrLine)
	fmt.Printf("%-6s%-5s%-6s%-30s\n", "Id", "Next", "#Oifs", "Oifs")
	fmt.Println(hdrLine)
}

func oifListShowOneResp(oifList *halproto.OifList, ifIDToStr map[uint64]string) {
	ifStr := ""

	if oifList.GetIsHonorIngress() {
		ifStr = "HI,"
	}

	for i := 0; i < len(oifList.GetOifs()); i++ {
		ifStr += ifIDToStr[oifList.GetOifs()[i].GetInterface().GetInterfaceId()]
		ifStr += ","
	}
	if len(ifStr) > 0 {
		ifStr = ifStr[:len(ifStr)-1]
	}
	fmt.Printf("%-6d%-5d%-6d%-30s\n",
		oifList.GetId(),
		oifList.GetAttachedListId(),
		len(oifList.GetOifs()),
		ifStr)

	// first := true
	// fmt.Printf("%-10d%-10s%-10d", oifList.GetId(), honorStr, oifList.GetAttachedListId())
	// for i := 0; i < len(oifList.GetOifs()); i++ {
	// 	if first != true {
	// 		fmt.Printf("%-30s", "")
	// 	}
	// 	fmt.Printf("%-8d%-30s\n",
	// 		oifList.GetOifs()[i].GetL2Segment().GetSegmentId(),
	// 		fmt.Sprintf("%s:%d:%d", ifIDToStr[oifList.GetOifs()[i].GetInterface().GetInterfaceId()],
	// 			oifList.GetOifs()[i].GetQId(),
	// 			oifList.GetOifs()[i].GetQPurpose()))
	// 	first = false
	// }
}
