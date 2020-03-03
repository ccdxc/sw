package cmd

import (
	"context"
	"fmt"
	"os"
	"reflect"
	"strings"

	"github.com/spf13/cobra"
	yaml "gopkg.in/yaml.v2"

	"github.com/pensando/sw/nic/apollo/agent/cli/utils"
	"github.com/pensando/sw/nic/apollo/agent/gen/pds"
)

var (
	slabID uint32
)

var memShowCmd = &cobra.Command{
	Use:   "memory",
	Short: "Show system memory information",
	Long:  "Show system memory information",
	Run:   memShowCmdHandler,
}

func init() {
	systemShowCmd.AddCommand(memShowCmd)
	memShowCmd.Flags().Bool("slab", false, "Show slab information")
	memShowCmd.Flags().Bool("mtrack", false, "Show mtrack information")
	memShowCmd.Flags().Bool("heap", false, "Show heap information")
	memShowCmd.Flags().Bool("yaml", false, "Output in yaml")
	memShowCmd.Flags().Uint32Var(&slabID, "slab-id", 0, "Specify slab id")
	memShowCmd.Flags().MarkHidden("id")
}

func memShowCmdHandler(cmd *cobra.Command, args []string) {
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the PDS. Is PDS Running?\n")
		os.Exit(1)
	}
	defer c.Close()

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	todo := true
	slab := false
	mtrack := false
	heap := false
	yamlOutput := false

	client := pds.NewDebugSvcClient(c)

	if cmd != nil {
		if cmd.Flags().Changed("slab") {
			slab = true
		}
		if cmd.Flags().Changed("mtrack") {
			mtrack = true
		}
		if cmd.Flags().Changed("heap") {
			heap = true
		}
		if cmd.Flags().Changed("yaml") {
			yamlOutput = true
		}
	}

	if (slab || heap || mtrack) == false {
		slab = true
		mtrack = true
		heap = true
	}

	if slab == true {
		fmt.Printf("Slab Memory Stats:\n")
		// PDS call
		var empty *pds.Empty
		resp, err := client.SlabGet(context.Background(), empty)
		if err != nil {
			fmt.Printf("Getting slab failed. %v\n", err)
		} else {
			if resp.ApiStatus != pds.ApiStatus_API_STATUS_OK {
				fmt.Printf("Get slab operation failed with %v error\n", resp.ApiStatus)
			} else {
				if yamlOutput {
					respType := reflect.ValueOf(resp)
					b, _ := yaml.Marshal(respType.Interface())
					fmt.Println(string(b))
					fmt.Println("---")
				} else {
					// Print header
					slabShowHeader()
					// Print slab
					slabShowResp(resp)
				}
			}
		}
		fmt.Printf("\n")
	}

	if mtrack == true {
		var req *pds.MemTrackGetRequest

		// Get all allocationIDs
		req = &pds.MemTrackGetRequest{
			Spec: &pds.MemTrackSpec{
				AllocId: 4294967295,
			},
		}

		if todo == false {
			fmt.Printf("Mtrack Memory Stats:\n")
			// PDS call
			resp, err := client.MemTrackGet(context.Background(), req)
			if err != nil {
				fmt.Printf("Getting mtrack failed. %v\n", err)
			} else {
				if resp.ApiStatus != pds.ApiStatus_API_STATUS_OK {
					fmt.Printf("Get mtrack operation failed with %v error\n", resp.ApiStatus)
				} else {
					if yamlOutput {
						respType := reflect.ValueOf(resp)
						b, _ := yaml.Marshal(respType.Interface())
						fmt.Println(string(b))
						fmt.Println("---")
					} else {
						// Print header
						mtrackShowHeader()
						// Print mtrack
						mtrackShowResp(resp)
					}
				}
			}
			fmt.Printf("\n")
		}
	}

	if heap == true {
		var empty *pds.Empty

		fmt.Printf("Heap Memory Stats:\n")
		resp, err := client.HeapGet(context.Background(), empty)
		if err != nil {
			fmt.Printf("Getting heap failed. %v\n", err)
		} else {
			// Print mtrack
			if resp.ApiStatus != pds.ApiStatus_API_STATUS_OK {
				fmt.Printf("Get heap operation failed with %v error\n", resp.ApiStatus)
			} else {
				if yamlOutput {
					respType := reflect.ValueOf(resp)
					b, _ := yaml.Marshal(respType.Interface())
					fmt.Println(string(b))
					fmt.Println("---")
				} else {
					heapShowResp(resp)
				}
			}
		}
		fmt.Printf("\n")
	}
}

func slabShowHeader() {
	hdrLine := strings.Repeat("-", 150)
	fmt.Println(hdrLine)
	fmt.Printf("%-22s%-5s%-9s%-12s%-31s%-14s%-8s%-8s%-8s%-12s%-10s%-10s\n",
		"Name", "Id", "ElemSize", "#Elem/Block", "ThreadSafe/GrowDemand/DelayDel",
		"ZeroAlloc", "#Elems", "#Alloc",
		"#Frees", "#AllocErr", "#Blocks", "RawBlockSz")
	fmt.Println(hdrLine)
}

func slabShowResp(resp *pds.SlabGetResponse) {
	for _, slab := range resp.GetSlab() {
		spec := slab.GetSpec()
		stats := slab.GetStats()
		boolString := ""
		if spec.GetThreadSafe() {
			boolString += "T"
		} else {
			boolString += "F"
		}
		boolString += "/"
		if spec.GetGrowOnDemand() {
			boolString += "T"
		} else {
			boolString += "F"
		}
		boolString += "/"
		if spec.GetDelayDelete() {
			boolString += "T"
		} else {
			boolString += "F"
		}
		fmt.Printf("%-22s%-5d%-9d%-12d%-31s%-14t%-8d%-8d%-8d%-12d%-10d%-10d\n",
			spec.GetName(), spec.GetId(), spec.GetElementSize(),
			spec.GetElementsPerBlock(), boolString,
			spec.GetZeroOnAllocation(), stats.GetNumElementsInUse(),
			stats.GetNumAllocs(), stats.GetNumFrees(),
			stats.GetNumAllocErrors(), stats.GetNumBlocks(),
			spec.GetRawBlockSize())
	}
}

func heapShowResp(resp *pds.HeapGetResponse) {
	stats := resp.GetStats()
	fmt.Printf("Num Bytes Arena Alloc    : %d\n", stats.GetNumArenaBytesAlloc())
	fmt.Printf("Num Free Blocks          : %d\n", stats.GetNumFreeBlocks())
	fmt.Printf("Num Fast Bin Free Blocks : %d\n", stats.GetNumFastBinFreeBlocks())
	fmt.Printf("Num mmap Blocks Alloc    : %d\n", stats.GetNumMmapBlocksAlloc())
	fmt.Printf("Num mmap Bytes Alloc     : %d\n", stats.GetNumMmapBytesAlloc())
	fmt.Printf("Max Bytes Alloc          : %d\n", stats.GetMaxBlocksAlloc())
	fmt.Printf("Num Fast Bin Free Bytes  : %d\n", stats.GetNumFastBinFreeBytes())
	fmt.Printf("Num Bytes Alloc          : %d\n", stats.GetNumBytesAlloc())
	fmt.Printf("Num Free Bytes           : %d\n", stats.GetNumFreeBytes())
	fmt.Printf("Releasable Free Bytes    : %d\n", stats.GetReleasableFreeBytes())
}

func mtrackShowHeader() {
	hdrLine := strings.Repeat("-", 35)
	fmt.Println(hdrLine)
	fmt.Printf("%-10s%-10s%-10s\n",
		"AllocID", "NumAlloc", "NumFree")
	fmt.Println(hdrLine)
}

func mtrackShowResp(resp *pds.MemTrackGetResponse) {
	spec := resp.GetSpec()
	stats := resp.GetStats()

	fmt.Printf("%-10d%-10d%-10d\n",
		spec.GetAllocId(),
		stats.GetNumAllocs(),
		stats.GetNumFrees())
}
