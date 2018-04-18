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
	tableID uint32
)

var tableMetaShowCmd = &cobra.Command{
	Use:   "table",
	Short: "table",
	Long:  "shows table metadata",
	Run:   tableMetaShowCmdHandler,
}

func init() {
	showCmd.AddCommand(tableMetaShowCmd)

	tableMetaShowCmd.Flags().Uint32Var(&tableID, "id", 1, "Specify table-id")
}

func tableMetaShowCmdHandler(cmd *cobra.Command, args []string) {

	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		log.Fatalf("Could not connect to the HAL. Is HAL Running?")
	}
	client := halproto.NewTableClient(c.ClientConn)

	var empty *halproto.Empty

	// HAL call
	respMsg, err := client.TableMetadataGet(context.Background(), empty)
	if err != nil {
		log.Errorf("Getting Table Metadata failed. %v", err)
		return
	}

	if respMsg.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
		log.Errorf("HAL Returned non OK status. %v", respMsg.ApiStatus)
		return
	}

	// Print Header
	tableMetaShowHeader()

	// Print VRFs
	for _, meta := range respMsg.TableMeta {
		tableMetaShow(meta)
	}
}

func tableMetaShowHeader() {
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

// tableMetaShow shows table meteadat
func tableMetaShow(meta *halproto.TableMetadata) {
	switch meta.GetKind() {
	case halproto.TableKind_TABLE_INDEX:
		tableMetaIndexShow(meta)
	case halproto.TableKind_TABLE_TCAM:
		tableMetaTcamShow(meta)
	case halproto.TableKind_TABLE_HASH:
		tableMetaHashShow(meta)
	case halproto.TableKind_TABLE_FLOW:
		tableMetaFlowShow(meta)
	case halproto.TableKind_TABLE_MET:
		tableMetaMetShow(meta)
	default:
		fmt.Printf("Invalid table type: %d", meta.GetKind())
	}
}

// tableMetaIndexShow shows Index metadata
func tableMetaIndexShow(meta *halproto.TableMetadata) {
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

// tableMetaTcamShow shows Tcam metadata
func tableMetaTcamShow(meta *halproto.TableMetadata) {
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

// tableMetaHashShow shows Hash metadata
func tableMetaHashShow(meta *halproto.TableMetadata) {
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

// tableMetaFlowShow shows Flow metadata
func tableMetaFlowShow(meta *halproto.TableMetadata) {
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

// tableMetaMetShow shows Met Metadata
func tableMetaMetShow(meta *halproto.TableMetadata) {
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
