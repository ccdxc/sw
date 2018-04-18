package cmd

import (
	"context"
	"fmt"

	"github.com/spf13/cobra"

	"github.com/pensando/sw/nic/agent/cmd/halctl/utils"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/venice/utils/log"
)

var (
	tableEntryIDSpecified bool
	tableEntryID          uint32
)

var tableShowCmd = &cobra.Command{
	Use:   "dump",
	Short: "table dump",
	Long:  "shows table entries",
	Run:   tableShowCmdHandler,
}

func init() {
	tableMetaShowCmd.AddCommand(tableShowCmd)

	tableShowCmd.Flags().Uint32Var(&tableEntryID, "entry-id", 0, "Specify entry-id(default all)")
	tableShowCmd.Flags().Uint32Var(&tableID, "id", 1, "Specify table-id")
}

func tableShowCmdHandler(cmd *cobra.Command, args []string) {

	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		log.Fatalf("Could not connect to the HAL. Is HAL Running?")
	}
	client := halproto.NewTableClient(c.ClientConn)

	// Have to specify table id
	var spec *halproto.TableSpec
	if cmd.Flags().Changed("id") {
		spec = &halproto.TableSpec{
			Key: &halproto.TableIdName{
				IdOrName: &halproto.TableIdName_TableId{
					TableId: tableID,
				},
			},
		}
	} else {
		fmt.Println("Specify the table-id to dump")
		return
	}

	// Check if specific entry-ID
	if cmd.Flags().Changed("entry-id") {
		tableEntryIDSpecified = true
	} else {
		tableEntryIDSpecified = false
	}

	tableReqMsg := &halproto.TableRequestMsg{
		Spec: []*halproto.TableSpec{spec},
	}

	// HAL call
	respMsg, err := client.TableGet(context.Background(), tableReqMsg)
	if err != nil {
		log.Errorf("Getting VRF failed. %v", err)
	}

	// Print Header
	// tableShowHeader()

	// Print Tables: For now its only one at a time
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL Returned non OK status. %v", resp.ApiStatus)
			continue
		}
		tableShowOneResp(resp)
	}
}

func tableShowOneResp(resp *halproto.TableResponse) {
	if resp.GetIndexTable() != nil {
		tableIndexShow(resp)
	} else if resp.GetTcamTable() != nil {
		tableTcamShow(resp)
	} else if resp.GetHashTable() != nil {
		tableHashShow(resp)
	} else if resp.GetFlowTable() != nil {
		tableFlowShow(resp)
	} else if resp.GetMetTable() != nil {
	} else {
	}
}

func tableIndexShow(resp *halproto.TableResponse) {
	indexMsg := resp.GetIndexTable()

	if len(indexMsg.GetIndexEntry()) == 0 {
		fmt.Printf("No elements programmed\n")
		return
	}

	for _, entry := range indexMsg.GetIndexEntry() {
		// All or specific entry ID
		if !tableEntryIDSpecified || (tableEntryID == entry.GetIndex()) {
			fmt.Printf("Index: %d\n", entry.GetIndex())
			fmt.Printf("%s\n", entry.GetEntry())
		}
	}
}

func tableTcamShow(resp *halproto.TableResponse) {
	tcamMsg := resp.GetTcamTable()

	if len(tcamMsg.GetTcamEntry()) == 0 {
		fmt.Printf("No elements programmed\n")
		return
	}

	for _, entry := range tcamMsg.GetTcamEntry() {
		// All or specific entry ID
		if !tableEntryIDSpecified || (tableEntryID == entry.GetIndex()) {
			fmt.Printf("Index: %d\n", entry.GetIndex())
			fmt.Printf("%s\n", entry.GetEntry())
		}
	}
}

func tableHashShow(resp *halproto.TableResponse) {
	hashMsg := resp.GetHashTable()

	if len(hashMsg.GetHashEntry()) == 0 {
		fmt.Printf("No elements programmed\n")
		return
	}

	for _, entry := range hashMsg.GetHashEntry() {
		// All or specific entry ID
		if !tableEntryIDSpecified || (tableEntryID == entry.GetIndex()) {
			fmt.Printf("Index: %d\n", entry.GetIndex())
			fmt.Printf("%s\n", entry.GetEntry())
		}
	}
}

func tableFlowShow(resp *halproto.TableResponse) {
	flowMsg := resp.GetFlowTable()

	if len(flowMsg.GetFlowEntry()) == 0 {
		fmt.Printf("No elements programmed\n")
		return
	}

	for _, entry := range flowMsg.GetFlowEntry() {
		// All or specific entry ID
		if !tableEntryIDSpecified || (tableEntryID == entry.GetIndex()) {
			fmt.Printf("Index: %d\n", entry.GetIndex())
			fmt.Printf("-----------\n")
			fmt.Printf("%s\n", entry.GetEntry())
		}
	}
}
