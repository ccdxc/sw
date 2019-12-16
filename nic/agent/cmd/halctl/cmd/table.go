//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	"os"
	"strings"

	"github.com/spf13/cobra"

	"github.com/pensando/sw/nic/agent/cmd/halctl/utils"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
)

var (
	tableID               uint32
	tableEntryIDSpecified bool
	tableEntryID          uint32
	tableHandle           uint64
)

var tableShowCmd = &cobra.Command{
	Use:   "table",
	Short: "show table info/dump",
	Long:  "show table info/dump",
}

var tableInfoShowCmd = &cobra.Command{
	Use:   "info",
	Short: "show table info",
	Long:  "show table info",
	Run:   tableInfoShowCmdHandler,
}

var tableDumpShowCmd = &cobra.Command{
	Use:   "dump",
	Short: "show table entries",
	Long:  "show table entries",
	Run:   tableDumpShowCmdHandler,
	// PreRunE: func(cmd *cobra.Command, args []string) error {
	// 	        return CheckRequiredFlags(cmd.Flags())
	// 		},
}

// func CheckRequiredFlags(flags *pflag.FlagSet) error {
// 	requiredError := false
// 	flagName := ""
//
// 	fmt.Println("Entering required flags")
//
// 	flags.VisitAll(func(flag *pflag.Flag) {
// 		requiredAnnotation := flag.Annotations[cobra.BashCompOneRequiredFlag]
// 		fmt.Printf("Annotation: %d", len(requiredAnnotation))
// 		if len(requiredAnnotation) == 0 {
// 			return
// 		}
//
// 		flagRequired := requiredAnnotation[0] == "true"
//
// 		if flagRequired && !flag.Changed {
// 			requiredError = true
// 			flagName = flag.Name
// 		}
// 	})
//
// 	if requiredError {
// 		fmt.Printf("Required flag %s has not been set", flagName)
// 		return errors.New("Required flag `" + flagName + "` has not been set")
// 	}
//
// 	return nil
// }

func init() {
	showCmd.AddCommand(tableShowCmd)
	tableShowCmd.AddCommand(tableInfoShowCmd)
	tableShowCmd.AddCommand(tableDumpShowCmd)

	tableDumpShowCmd.Flags().Uint32Var(&tableEntryID, "entry-id", 0, "Specify entry-id(default all)")
	tableDumpShowCmd.Flags().Uint32Var(&tableID, "table-id", 1, "Specify table-id")
}

func tableInfoShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	client := halproto.NewTableClient(c)

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	var empty *halproto.Empty

	// HAL call
	respMsg, err := client.TableMetadataGet(context.Background(), empty)
	if err != nil {
		fmt.Printf("Getting Table Metadata failed. %v\n", err)
		return
	}

	if respMsg.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", respMsg.ApiStatus)
		return
	}

	// Print Header
	tableInfoShowHeader()

	// Print VRFs
	for _, meta := range respMsg.TableMeta {
		tableInfoShow(meta)
	}
}

func tableDumpShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	client := halproto.NewTableClient(c)

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	tableEntryIDSpecified = false

	var spec *halproto.TableSpec
	if cmd != nil && cmd.Flags().Changed("table-id") {
		// Get specific tables
		spec = &halproto.TableSpec{
			Key: &halproto.TableIdName{
				IdOrName: &halproto.TableIdName_TableId{
					TableId: tableID,
				},
			},
		}
		// Check if specific entry-ID
		if cmd.Flags().Changed("entry-id") {
			tableEntryIDSpecified = true
		}
	} else {
		// Get all tables
		spec = &halproto.TableSpec{}
	}

	tableReqMsg := &halproto.TableRequestMsg{
		Spec: []*halproto.TableSpec{spec},
	}
	// HAL call
	respMsg, err := client.TableGet(context.Background(), tableReqMsg)
	if err != nil {
		fmt.Printf("Getting Table failed. %v\n", err)
		return
	}

	// Print Header
	// tableShowHeader()

	// Print Tables: For now its only one at a time
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
			continue
		}
		tableShowOneResp(resp)
	}
}

func tableInfoShowHeader() {
	fmt.Println("Name:     Table Name                          Id:       Table Id")
	fmt.Println("Type:     Table Type                          Cap:      Capacity (Hash: Only dleft, Flow: Only Flow Hash")
	fmt.Println("AugCap:   Aug Table Cap.(Otcam, Flow Coll)    Usage:    Usage in table")
	fmt.Println("AugUsage: Usage in Augmented table            Ins[Err]: Insert inc. errors")
	fmt.Println("Upd[Err]: Update inc. errors                  Del[Err]: Delete inc. errors")
	fmt.Println("")

	hdrLine := strings.Repeat("-", 120)
	fmt.Println(hdrLine)
	fmt.Printf("%-30s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s\n",
		"Name", "Id", "Type", "Cap.", "AugCap.", "Usage", "AugUsage", "Ins[Err]", "Upd[Err]", "Del[Err]")
	fmt.Println(hdrLine)
}

// tableInfoShow shows table meteadat
func tableInfoShow(meta *halproto.TableMetadata) {
	switch meta.GetKind() {
	case halproto.TableKind_TABLE_INDEX:
		tableInfoIndexShow(meta)
	case halproto.TableKind_TABLE_TCAM:
		tableInfoTcamShow(meta)
	case halproto.TableKind_TABLE_HASH:
		tableInfoHashShow(meta)
	case halproto.TableKind_TABLE_FLOW:
		tableInfoFlowShow(meta)
	case halproto.TableKind_TABLE_MET:
		tableInfoMetShow(meta)
	default:
		fmt.Printf("Invalid table type: %d", meta.GetKind())
	}
}

// tableInfoIndexShow shows Index metadata
func tableInfoIndexShow(meta *halproto.TableMetadata) {
	iMeta := meta.GetIndexMeta()
	insStr := fmt.Sprintf("%d[%d]", iMeta.GetNumInserts(), iMeta.GetNumInsertFailures())
	updStr := fmt.Sprintf("%d[%d]", iMeta.GetNumUpdates(), iMeta.GetNumUpdateFailures())
	delStr := fmt.Sprintf("%d[%d]", iMeta.GetNumDeletes(), iMeta.GetNumDeleteFailures())
	fmt.Printf("%-30s%-10d%-10s%-10d%-10s%-10d%-10s%-10s%-10s%-10s\n",
		meta.GetTableName(),
		meta.GetTableId(),
		utils.TableKindToStr(meta.GetKind()),
		iMeta.GetCapacity(),
		"-",
		iMeta.GetUsage(),
		"-",
		insStr,
		updStr,
		delStr)
}

// tableInfoTcamShow shows Tcam metadata
func tableInfoTcamShow(meta *halproto.TableMetadata) {
	tMeta := meta.GetTcamMeta()
	insStr := fmt.Sprintf("%d[%d]", tMeta.GetNumInserts(), tMeta.GetNumInsertFailures())
	updStr := fmt.Sprintf("%d[%d]", tMeta.GetNumUpdates(), tMeta.GetNumUpdateFailures())
	delStr := fmt.Sprintf("%d[%d]", tMeta.GetNumDeletes(), tMeta.GetNumDeleteFailures())
	fmt.Printf("%-30s%-10d%-10s%-10d%-10s%-10d%-10s%-10s%-10s%-10s\n",
		meta.GetTableName(),
		meta.GetTableId(),
		utils.TableKindToStr(meta.GetKind()),
		tMeta.GetCapacity(),
		"-",
		tMeta.GetUsage(),
		"-",
		insStr,
		updStr,
		delStr)
}

// tableInfoHashShow shows Hash metadata
func tableInfoHashShow(meta *halproto.TableMetadata) {
	hMeta := meta.GetHashMeta()
	insStr := fmt.Sprintf("%d[%d]", hMeta.GetNumInserts(), hMeta.GetNumInsertFailures())
	updStr := fmt.Sprintf("%d[%d]", hMeta.GetNumUpdates(), hMeta.GetNumUpdateFailures())
	delStr := fmt.Sprintf("%d[%d]", hMeta.GetNumDeletes(), hMeta.GetNumDeleteFailures())
	fmt.Printf("%-30s%-10d%-10s%-10d%-10d%-10d%-10d%-10s%-10s%-10s\n",
		meta.GetTableName(),
		meta.GetTableId(),
		utils.TableKindToStr(meta.GetKind()),
		hMeta.GetCapacity(),
		hMeta.GetOtcamCapacity(),
		hMeta.GetHashUsage(),
		hMeta.GetOtcamUsage(),
		insStr,
		updStr,
		delStr)
}

// tableInfoFlowShow shows Flow metadata
func tableInfoFlowShow(meta *halproto.TableMetadata) {
	fMeta := meta.GetFlowMeta()
	insStr := fmt.Sprintf("%d[%d]", fMeta.GetNumInserts(), fMeta.GetNumInsertFailures())
	updStr := fmt.Sprintf("%d[%d]", fMeta.GetNumUpdates(), fMeta.GetNumUpdateFailures())
	delStr := fmt.Sprintf("%d[%d]", fMeta.GetNumDeletes(), fMeta.GetNumDeleteFailures())
	fmt.Printf("%-30s%-10d%-10s%-10d%-10d%-10d%-10d%-10s%-10s%-10s\n",
		meta.GetTableName(),
		meta.GetTableId(),
		utils.TableKindToStr(meta.GetKind()),
		fMeta.GetCapacity(),
		fMeta.GetCollCapacity(),
		fMeta.GetHashUsage(),
		fMeta.GetCollUsage(),
		insStr,
		updStr,
		delStr)
}

// tableInfoMetShow shows Met Metadata
func tableInfoMetShow(meta *halproto.TableMetadata) {
	mMeta := meta.GetMetMeta()
	insStr := fmt.Sprintf("%d[%d]", mMeta.GetNumInserts(), mMeta.GetNumInsertFailures())
	updStr := fmt.Sprintf("%d[%d]", mMeta.GetNumUpdates(), mMeta.GetNumUpdateFailures())
	delStr := fmt.Sprintf("%d[%d]", mMeta.GetNumDeletes(), mMeta.GetNumDeleteFailures())
	fmt.Printf("%-30s%-10d%-10s%-10d%-10s%-10d%-10s%-10s%-10s%-10s\n",
		meta.GetTableName(),
		meta.GetTableId(),
		utils.TableKindToStr(meta.GetKind()),
		mMeta.GetCapacity(),
		"-",
		mMeta.GetUsage(),
		"-",
		insStr,
		updStr,
		delStr)
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
		tableMetShow(resp)
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
		if !tableEntryIDSpecified {
			fmt.Printf("Primary Index valid=%t\n", entry.GetPrimaryIndexValid())
			fmt.Printf("Primary Index=0x%x\n", entry.GetPrimaryIndex())
			fmt.Printf("Secondary Index valid=%t\n", entry.GetSecondaryIndexValid())
			fmt.Printf("Secondary Index=0x%x\n", entry.GetSecondaryIndex())
			fmt.Printf("Key=%s\n", entry.GetKey())
			fmt.Printf("Data=%s\n", entry.GetData())
			fmt.Printf("-----------\n")
		}
	}
}

func tableMetShow(resp *halproto.TableResponse) {
	metMsg := resp.GetMetTable()

	if len(metMsg.GetMetEntry()) == 0 {
		fmt.Printf("No elements programmed\n")
		return
	}

	for _, entry := range metMsg.GetMetEntry() {
		// All or specific entry ID
		if !tableEntryIDSpecified || (tableEntryID == entry.GetIndex()) {
			fmt.Printf("Index: %d\n", entry.GetIndex())
			fmt.Printf("-----------\n")
			fmt.Printf("%s\n", entry.GetEntry())
		}
	}
}
