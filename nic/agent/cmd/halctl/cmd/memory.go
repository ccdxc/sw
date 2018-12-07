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
	slabID uint32
)

var memShowCmd = &cobra.Command{
	Use:   "memory",
	Short: "Show system memory information",
	Long:  "Show system memory information",
}

var slabShowCmd = &cobra.Command{
	Use:   "slab",
	Short: "Show slab information",
	Long:  "Show slab information",
	Run:   slabShowCmdHandler,
}

var mtrackShowCmd = &cobra.Command{
	Use:   "mtrack",
	Short: "Show mtrack information",
	Long:  "Show mtrack information",
	Run:   mtrackShowCmdHandler,
}

var summaryShowCmd = &cobra.Command{
	Use:   "summary",
	Short: "show memory summary",
	Long:  "show memory summary",
	Run:   summaryShowCmdHandler,
}

/*
var hashShowCmd = &cobra.Command{
	Use:   "hash",
	Short: "Show hash information",
	Long:  "Show hash information",
	Run:   hashShowCmdHandler,
}
*/

func init() {
	systemShowCmd.AddCommand(memShowCmd)
	memShowCmd.AddCommand(slabShowCmd)
	memShowCmd.AddCommand(mtrackShowCmd)
	memShowCmd.AddCommand(summaryShowCmd)

	//memShowCmd.AddCommand(hashShowCmd)

	slabShowCmd.Flags().Bool("yaml", false, "Output in yaml")
	slabShowCmd.Flags().Uint32Var(&slabID, "id", 0, "Specify slab id")
	slabShowCmd.Flags().MarkHidden("id")
	mtrackShowCmd.Flags().Bool("yaml", false, "Output in yaml")
	//hashShowCmd.Flags().Bool("yaml", false, "Output in yaml")
}

func slabShowCmdHandler(cmd *cobra.Command, args []string) {
	if cmd.Flags().Changed("yaml") {
		slabDetailShowCmdHandler(cmd, args)
		return
	}

	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	defer c.Close()

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

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
		fmt.Printf("Getting slab failed. %v\n", err)
		return
	}

	// Print header
	slabShowHeader()

	// Print slab
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			if all == false {
				fmt.Printf("HAL Returned non OK status. %v\n", resp.ApiStatus)
				return
			}
			continue
		}
		slabShowResp(resp)
	}
}

func slabShowHeader() {
	hdrLine := strings.Repeat("-", 175)
	fmt.Println(hdrLine)
	fmt.Printf("%-22s%-5s%-9s%-12s%-12s%-14s%-12s%-14s%-8s%-8s%-8s%-12s%-10s%-10s\n",
		"Name", "Id", "ElemSize", "#Elem/Block", "ThreadSafe",
		"GrowDemand", "DelayDel", "ZeroAlloc", "#Elems", "#Alloc",
		"#Frees", "#AllocErr", "#Blocks", "RawBlockSz")
	fmt.Println(hdrLine)
}

func slabShowResp(resp *halproto.SlabGetResponse) {
	spec := resp.GetSpec()
	stats := resp.GetStats()

	fmt.Printf("%-22s%-5d%-9d%-12d%-12t%-14t%-12t%-14t%-8d%-8d%-8d%-12d%-10d%-10d\n",
		spec.GetName(),
		// strings.Replace(strings.ToLower(spec.GetName()), "_", " ", -1),
		spec.GetId(), spec.GetElementSize(),
		spec.GetElementsPerBlock(), spec.GetThreadSafe(),
		spec.GetGrowOnDemand(), spec.GetDelayDelete(),
		spec.GetZeroOnAllocation(), stats.GetNumElementsInUse(),
		stats.GetNumAllocs(), stats.GetNumFrees(),
		stats.GetNumAllocErrors(), stats.GetNumBlocks(),
		spec.GetRawBlockSize())
}

func slabDetailShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	defer c.Close()

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

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
		fmt.Printf("Getting slab failed. %v\n", err)
		return
	}

	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			if all == false {
				fmt.Printf("HAL Returned non OK status. %v\n", resp.ApiStatus)
				return
			}
			continue
		}
		respType := reflect.ValueOf(resp)
		b, _ := yaml.Marshal(respType.Interface())
		fmt.Println(string(b))
		fmt.Println("---")
	}
}

func mtrackShowCmdHandler(cmd *cobra.Command, args []string) {
	if cmd.Flags().Changed("yaml") {
		mtrackDetailShowCmdHandler(cmd, args)
		return
	}

	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	defer c.Close()

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	client := halproto.NewDebugClient(c.ClientConn)

	var mtrackGetReqMsg *halproto.MemTrackGetRequestMsg

	// Get all allocationIDs
	var req *halproto.MemTrackGetRequest
	req = &halproto.MemTrackGetRequest{
		Spec: &halproto.MemTrackSpec{
			AllocId: 4294967295,
		},
	}
	mtrackGetReqMsg = &halproto.MemTrackGetRequestMsg{
		Request: []*halproto.MemTrackGetRequest{req},
	}

	// HAL call
	respMsg, err := client.MemTrackGet(context.Background(), mtrackGetReqMsg)
	if err != nil {
		fmt.Printf("Getting mtrack failed. %v\n", err)
		return
	}

	// Print header
	mtrackShowHeader()

	// Print mtrack
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("HAL Returned non OK status. %v\n", resp.ApiStatus)
			continue
		}
		mtrackShowResp(resp)
	}
}

func mtrackShowHeader() {
	hdrLine := strings.Repeat("-", 35)
	fmt.Println(hdrLine)
	fmt.Printf("%-10s%-10s%-10s\n",
		"AllocID", "NumAlloc", "NumFree")
	fmt.Println(hdrLine)
}

func mtrackShowResp(resp *halproto.MemTrackGetResponse) {
	spec := resp.GetSpec()
	stats := resp.GetStats()

	fmt.Printf("%-10d%-10d%-10d\n",
		spec.GetAllocId(),
		stats.GetNumAllocs(),
		stats.GetNumFrees())
}

func mtrackDetailShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	defer c.Close()

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	client := halproto.NewDebugClient(c.ClientConn)

	var mtrackGetReqMsg *halproto.MemTrackGetRequestMsg

	// Get all allocationIDs
	var req *halproto.MemTrackGetRequest
	req = &halproto.MemTrackGetRequest{
		Spec: &halproto.MemTrackSpec{
			AllocId: 4294967295,
		},
	}
	mtrackGetReqMsg = &halproto.MemTrackGetRequestMsg{
		Request: []*halproto.MemTrackGetRequest{req},
	}

	// HAL call
	respMsg, err := client.MemTrackGet(context.Background(), mtrackGetReqMsg)
	if err != nil {
		fmt.Printf("Getting mtrack failed. %v\n", err)
		return
	}

	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("HAL Returned non OK status. %v\n", resp.ApiStatus)
			continue
		}
		respType := reflect.ValueOf(resp)
		b, _ := yaml.Marshal(respType.Interface())
		fmt.Println(string(b))
		fmt.Println("---")
	}
}

/*
func hashShowCmdHandler(cmd *cobra.Command, args []string) {
	if cmd.Flags().Changed("yaml") {
		hashDetailShowCmdHandler(cmd, args)
		return
	}

	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	defer c.Close()

	client := halproto.NewDebugClient(c.ClientConn)

	var empty *halproto.Empty

	// HAL call
	respMsg, err := client.HashTableGet(context.Background(), empty)
	if err != nil {
		fmt.Printf("Getting mtrack failed. %v\n", err)
	}

	// Print header
	hashShowHeader()

	// Print hash
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("HAL Returned non OK status. %v\n", resp.ApiStatus)
			continue
		}
		hashShowResp(resp)
	}
}

func hashShowHeader() {
	hdrLine := strings.Repeat("-", 150)
	fmt.Println(hdrLine)
	fmt.Printf("%-20s%-12s%-12s%-12s%-12s%-12s%-12s%-14s%-12s%-12s%-12s\n",
		"Name", "NumBuckets", "ThreadSafe", "MaxDepth",
		"AvgDepth", "NumEntries", "NumInsert", "NumInsertErr",
		"NumDel", "NumDelErr", "NumLookup")
	fmt.Println(hdrLine)
}

func hashShowResp(resp *halproto.HashTableGetResponse) {
	spec := resp.GetSpec()
	stats := resp.GetStats()

	fmt.Printf("%-20s%-12d%-12t%-12d%-12d%-12d%-12d%-14d%-12d%-12d%-12d\n",
		spec.GetName(), spec.GetNumBuckets(), spec.GetThreadSafe(),
		spec.GetMaxBucketDepth(), spec.GetAvgBucketDepth(),
		stats.GetNumEntries(), stats.GetNumInserts(), stats.GetNumInsertErrors(),
		stats.GetNumDeletes(), stats.GetNumDeleteErrors(), stats.GetNumLookups())
}

func hashDetailShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	defer c.Close()

	client := halproto.NewDebugClient(c.ClientConn)

	var empty *halproto.Empty

	// HAL call
	respMsg, err := client.HashTableGet(context.Background(), empty)
	if err != nil {
		fmt.Printf("Getting hash table failed. %v\n", err)
	}

	// Print hash table
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("HAL Returned non OK status. %v\n", resp.ApiStatus)
			continue
		}
		respType := reflect.ValueOf(resp)
		b, _ := yaml.Marshal(respType.Interface())
		fmt.Println(string(b))
		fmt.Println("---")
	}
}
*/

func allMemoryShowHandler(ofile *os.File) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	defer c.Close()

	client := halproto.NewDebugClient(c.ClientConn)

	// Get all Slabs
	var slabGetReqMsg *halproto.SlabGetRequestMsg
	var sreq *halproto.SlabGetRequest

	sreq = &halproto.SlabGetRequest{
		Id: 4294967295,
	}
	slabGetReqMsg = &halproto.SlabGetRequestMsg{
		Request: []*halproto.SlabGetRequest{sreq},
	}

	// HAL call
	sRespMsg, err := client.SlabGet(context.Background(), slabGetReqMsg)
	if err != nil {
		fmt.Printf("Getting slab failed. %v\n", err)
		return
	}

	for _, sResp := range sRespMsg.Response {
		if sResp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			continue
		}
		respType := reflect.ValueOf(sResp)
		b, _ := yaml.Marshal(respType.Interface())
		if _, err := ofile.WriteString(string(b) + "\n"); err != nil {
			fmt.Printf("Failed to write to file %s, err : %v\n",
				ofile.Name(), err)
		}
	}

	// Get all allocationIDs
	var mtrackGetReqMsg *halproto.MemTrackGetRequestMsg
	var mreq *halproto.MemTrackGetRequest
	mreq = &halproto.MemTrackGetRequest{
		Spec: &halproto.MemTrackSpec{
			AllocId: 4294967295,
		},
	}
	mtrackGetReqMsg = &halproto.MemTrackGetRequestMsg{
		Request: []*halproto.MemTrackGetRequest{mreq},
	}

	// HAL call
	mRespMsg, err := client.MemTrackGet(context.Background(), mtrackGetReqMsg)
	if err != nil {
		fmt.Printf("Getting mtrack failed. %v\n", err)
		return
	}

	for _, mResp := range mRespMsg.Response {
		if mResp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("HAL Returned non OK status. %v\n", mResp.ApiStatus)
			continue
		}
		respType := reflect.ValueOf(mResp)
		b, _ := yaml.Marshal(respType.Interface())
		if _, err := ofile.WriteString(string(b) + "\n"); err != nil {
			fmt.Printf("Failed to write to file %s, err : %v\n",
				ofile.Name(), err)
		}
	}

	/*
		// Get hash table
		var empty *halproto.Empty

		// HAL call
		hRespMsg, err := client.HashTableGet(context.Background(), empty)
		if err != nil {
			fmt.Printf("Getting hash table failed. %v\n", err)
		}

		// Print hash table
		for _, hResp := range hRespMsg.Response {
			if hResp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
				fmt.Printf("HAL Returned non OK status. %v\n", hResp.ApiStatus)
				continue
			}
			respType := reflect.ValueOf(hResp)
			b, _ := yaml.Marshal(respType.Interface())
			if _, err := ofile.WriteString(string(b) + "\n"); err != nil {
				fmt.Printf("Failed to write to file %s, err : %v\n",
					ofile.Name(), err)
			}
		}
	*/
}

func summaryShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	defer c.Close()

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	client := halproto.NewDebugClient(c.ClientConn)

	// Get all Slabs
	var slabGetReqMsg *halproto.SlabGetRequestMsg
	var sreq *halproto.SlabGetRequest

	sreq = &halproto.SlabGetRequest{
		Id: 4294967295,
	}
	slabGetReqMsg = &halproto.SlabGetRequestMsg{
		Request: []*halproto.SlabGetRequest{sreq},
	}

	// HAL call
	sRespMsg, err := client.SlabGet(context.Background(), slabGetReqMsg)
	if err != nil {
		fmt.Printf("Getting slab failed. %v\n", err)
		return
	}

	var memSlabHeld uint32
	var memSlabUsed uint32
	var spec *halproto.SlabSpec
	var stats *halproto.SlabStats

	memSlabHeld = 0
	memSlabUsed = 0

	for _, sResp := range sRespMsg.Response {
		if sResp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			continue
		}
		spec = sResp.GetSpec()
		stats = sResp.GetStats()
		memSlabHeld += (spec.GetRawBlockSize() * stats.GetNumBlocks())
		memSlabUsed += (stats.GetNumElementsInUse() * spec.GetElementSize())
	}

	fmt.Printf("Slab Memory Held       : %d bytes\n", memSlabHeld)
	fmt.Printf("Slab Memory in Use     : %d bytes\n", memSlabUsed)
}
