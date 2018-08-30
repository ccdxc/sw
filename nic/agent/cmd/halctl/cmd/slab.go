//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	"strings"

	"github.com/spf13/cobra"

	"github.com/pensando/sw/nic/agent/cmd/halctl/utils"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/venice/utils/log"
)

var (
	slabID uint32
)

var slabShowCmd = &cobra.Command{
	Use:   "slab",
	Short: "Show slab information",
	Long:  "Show slab information",
	Run:   slabShowCmdHandler,
}

func init() {
	showCmd.AddCommand(slabShowCmd)

	slabShowCmd.Flags().Uint32Var(&slabID, "id", 0, "Specify slab id")
	slabShowCmd.Flags().MarkHidden("id")
}

func slabShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		log.Fatalf("Could not connect to the HAL. Is HAL Running?")
	}
	defer c.Close()

	all := false
	client := halproto.NewDebugClient(c.ClientConn)

	var slabGetReqMsg *halproto.SlabGetRequestMsg

	if cmd.Flags().Changed("id") {
		var req *halproto.SlabGetRequest
		req = &halproto.SlabGetRequest{
			Id: slabID,
		}
		slabGetReqMsg = &halproto.SlabGetRequestMsg{
			Request: []*halproto.SlabGetRequest{req},
		}
	} else {
		// Get all Slabs
		all = true
		var req *halproto.SlabGetRequest
		req = &halproto.SlabGetRequest{
			Id: 4294967295,
		}
		slabGetReqMsg = &halproto.SlabGetRequestMsg{
			Request: []*halproto.SlabGetRequest{req},
		}
	}

	// HAL call
	respMsg, err := client.SlabGet(context.Background(), slabGetReqMsg)
	if err != nil {
		log.Errorf("Getting slab failed. %v", err)
	}

	// Print header
	slabShowHeader()

	// Print slab
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			if all == false {
				log.Errorf("HAL Returned non OK status. %v", resp.ApiStatus)
			}
			continue
		}
		slabShowResp(resp)
	}
}

func slabShowHeader() {
	hdrLine := strings.Repeat("-", 170)
	fmt.Println(hdrLine)
	fmt.Printf("%-20s%-8s%-10s%-16s%-14s%-14s%-12s%-14s%-16s%-10s%-10s%-12s%-10s\n",
		"Name", "ID", "ElemSize", "NumElem/Block", "ThreadSafe",
		"GrowDemand", "DelayDel", "ZeroAlloc", "NumElems",
		"NumAlloc", "NumFree", "NumAllocErr", "NumBlocks")
	fmt.Println(hdrLine)
}

func slabShowResp(resp *halproto.SlabGetResponse) {
	spec := resp.GetSpec()
	stats := resp.GetStats()

	fmt.Printf("%-20s%-8d%-10d%-16d%-14t%-14t%-12t%-14t%-16d%-10d%-10d%-12d%-10d\n",
		spec.GetName(), spec.GetId(), spec.GetElementSize(),
		spec.GetElementsPerBlock(), spec.GetThreadSafe(),
		spec.GetGrowOnDemand(), spec.GetDelayDelete(),
		spec.GetZeroOnAllocation(), stats.GetNumElementsInUse(),
		stats.GetNumAllocs(), stats.GetNumFrees(),
		stats.GetNumAllocErrors(), stats.GetNumBlocks())
}
