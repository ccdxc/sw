// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.
/*
 * Package cmd is a auto generated package.
 * Input file: accel_metrics.proto
 */

package cmd

import (
	"fmt"

	"github.com/spf13/cobra"
)

//cmd - hw_ring
//rootCmd = sonic
//helpStr = Metrics for hardware rings\n Key indices - RId: ring ID, SubRId: sub-ring ID\n\n\nRing ID:\n\n  cp     : compression\n  cp_hot : compression (priority)\n  dc     : decompression\n  dc_hot : decompression (priority)\n  xts_enc: XTS encryption\n  xts_dec: XTS decryption\n  gcm_enc: GCM encryption\n  gcm_dec: GCM decryption\n\n\nValue Description:\n\nPIndex     : ring producer index\nCIndex     : ring consumer index\nInputBytes : total input bytes (not available for cp_hot, dc_hot, xts_enc/dec, gcm_enc/dec)\nOutputBytes: total output bytes (not available for cp_hot, dc_hot, xts_enc/dec, gcm_enc/dec)\nSoftResets : number of soft resets executed
var hw_ringShowCmd = &cobra.Command{
	Use:   "hw_ring",
	Short: "Metrics for hardware rings\n Key indices - RId: ring ID, SubRId: sub-ring ID\n\n\nRing ID:\n\n  cp     : compression\n  cp_hot : compression (priority)\n  dc     : decompression\n  dc_hot : decompression (priority)\n  xts_enc: XTS encryption\n  xts_dec: XTS decryption\n  gcm_enc: GCM encryption\n  gcm_dec: GCM decryption\n\n\nValue Description:\n\nPIndex     : ring producer index\nCIndex     : ring consumer index\nInputBytes : total input bytes (not available for cp_hot, dc_hot, xts_enc/dec, gcm_enc/dec)\nOutputBytes: total output bytes (not available for cp_hot, dc_hot, xts_enc/dec, gcm_enc/dec)\nSoftResets : number of soft resets executed",
	Long:  "\n---------------------------------\n Metrics for hardware rings\n Key indices - RId: ring ID, SubRId: sub-ring ID\n\n\nRing ID:\n\n  cp     : compression\n  cp_hot : compression (priority)\n  dc     : decompression\n  dc_hot : decompression (priority)\n  xts_enc: XTS encryption\n  xts_dec: XTS decryption\n  gcm_enc: GCM encryption\n  gcm_dec: GCM decryption\n\n\nValue Description:\n\nPIndex     : ring producer index\nCIndex     : ring consumer index\nInputBytes : total input bytes (not available for cp_hot, dc_hot, xts_enc/dec, gcm_enc/dec)\nOutputBytes: total output bytes (not available for cp_hot, dc_hot, xts_enc/dec, gcm_enc/dec)\nSoftResets : number of soft resets executed\n---------------------------------\n",
	RunE:  hw_ringShowCmdHandler,
}

func hw_ringShowCmdHandler(cmd *cobra.Command, args []string) error {
	jsonFormat = true
	bodyBytes, err := restGet("telemetry/v1/metrics/accelhwringmetrics/")
	if err != nil {
		fmt.Println(err)
		return err
	}
	if bodyBytes == nil {
		fmt.Println("No accelhwringmetrics object(s) found")
	}
	return nil
}

//cmd - sonic
//rootCmd =
//helpStr = Metrics for Storage Offload NIC
var sonicShowCmd = &cobra.Command{
	Use:   "sonic",
	Short: "Metrics for Storage Offload NIC",
	Long:  "\n---------------------------------\n Metrics for Storage Offload NIC\n---------------------------------\n",
}

//cmd - sequencer_info
//rootCmd = sonic
//helpStr = Sequencer queues information:\n Key indices - LifId: logical interface ID, QId: queue ID\n\n\nValue Description:\n\nQStateAddr: queue state memory address\nQGroup    : queue group\n            0 - compress/decompress\n            1 - compress/decompress status\n            2 - crypto\n            3 - crypto status\nCoreId    : CPU core ID (not available currently)
var sequencer_infoShowCmd = &cobra.Command{
	Use:   "sequencer_info",
	Short: "Sequencer queues information:\n Key indices - LifId: logical interface ID, QId: queue ID\n\n\nValue Description:\n\nQStateAddr: queue state memory address\nQGroup    : queue group\n            0 - compress/decompress\n            1 - compress/decompress status\n            2 - crypto\n            3 - crypto status\nCoreId    : CPU core ID (not available currently)",
	Long:  "\n---------------------------------\n Sequencer queues information:\n Key indices - LifId: logical interface ID, QId: queue ID\n\n\nValue Description:\n\nQStateAddr: queue state memory address\nQGroup    : queue group\n            0 - compress/decompress\n            1 - compress/decompress status\n            2 - crypto\n            3 - crypto status\nCoreId    : CPU core ID (not available currently)\n---------------------------------\n",
	RunE:  sequencer_infoShowCmdHandler,
}

func sequencer_infoShowCmdHandler(cmd *cobra.Command, args []string) error {
	jsonFormat = true
	bodyBytes, err := restGet("telemetry/v1/metrics/accelseqqueueinfometrics/")
	if err != nil {
		fmt.Println(err)
		return err
	}
	if bodyBytes == nil {
		fmt.Println("No accelseqqueueinfometrics object(s) found")
	}
	return nil
}

//cmd - sequencer_metrics
//rootCmd = sonic
//helpStr = Metrics for sequencer queues:\n Key indices - LifId: logical interface ID, QId: queue ID\n\n\nValue Description:\n\nInterruptsRaised : CPU interrupts raised\nNextDBsRung      : chaining doorbells rung\nSeqDescsProcessed: sequencer descriptors processed\nSeqDescsAborted  : sequencer descriptors aborted (due to reset)\nStatusPdmaXfers  : status descriptors copied\nHwDescXfers      : descriptors transferred to hardware\nHwBatchErrors    : hardware batch (length) errors\nHwOpErrors       : hardware operation errors\nAolUpdateReqs    : AOL list updates requested\nSglUpdateReqs    : scatter/gather list updates requested\nSglPdmaXfers     : payload DMA transfers executed\nSglPdmaErrors    : payload DMA errors encountered\nSglPadOnlyXfers  : pad-data-only DMA transfers executed\nSglPadOnlyErrors : pad-data-only DMA errors encountered\nAltDescsTaken    : alternate (bypass-onfail) descriptors executed\nAltBufsTaken     : alternate buffers taken\nLenUpdateReqs    : length updates requested\nCpHeaderUpdates  : compression header updates requested\nSeqHwBytes       : bytes processed
var sequencer_metricsShowCmd = &cobra.Command{
	Use:   "sequencer_metrics",
	Short: "Metrics for sequencer queues:\n Key indices - LifId: logical interface ID, QId: queue ID\n\n\nValue Description:\n\nInterruptsRaised : CPU interrupts raised\nNextDBsRung      : chaining doorbells rung\nSeqDescsProcessed: sequencer descriptors processed\nSeqDescsAborted  : sequencer descriptors aborted (due to reset)\nStatusPdmaXfers  : status descriptors copied\nHwDescXfers      : descriptors transferred to hardware\nHwBatchErrors    : hardware batch (length) errors\nHwOpErrors       : hardware operation errors\nAolUpdateReqs    : AOL list updates requested\nSglUpdateReqs    : scatter/gather list updates requested\nSglPdmaXfers     : payload DMA transfers executed\nSglPdmaErrors    : payload DMA errors encountered\nSglPadOnlyXfers  : pad-data-only DMA transfers executed\nSglPadOnlyErrors : pad-data-only DMA errors encountered\nAltDescsTaken    : alternate (bypass-onfail) descriptors executed\nAltBufsTaken     : alternate buffers taken\nLenUpdateReqs    : length updates requested\nCpHeaderUpdates  : compression header updates requested\nSeqHwBytes       : bytes processed",
	Long:  "\n---------------------------------\n Metrics for sequencer queues:\n Key indices - LifId: logical interface ID, QId: queue ID\n\n\nValue Description:\n\nInterruptsRaised : CPU interrupts raised\nNextDBsRung      : chaining doorbells rung\nSeqDescsProcessed: sequencer descriptors processed\nSeqDescsAborted  : sequencer descriptors aborted (due to reset)\nStatusPdmaXfers  : status descriptors copied\nHwDescXfers      : descriptors transferred to hardware\nHwBatchErrors    : hardware batch (length) errors\nHwOpErrors       : hardware operation errors\nAolUpdateReqs    : AOL list updates requested\nSglUpdateReqs    : scatter/gather list updates requested\nSglPdmaXfers     : payload DMA transfers executed\nSglPdmaErrors    : payload DMA errors encountered\nSglPadOnlyXfers  : pad-data-only DMA transfers executed\nSglPadOnlyErrors : pad-data-only DMA errors encountered\nAltDescsTaken    : alternate (bypass-onfail) descriptors executed\nAltBufsTaken     : alternate buffers taken\nLenUpdateReqs    : length updates requested\nCpHeaderUpdates  : compression header updates requested\nSeqHwBytes       : bytes processed\n---------------------------------\n",
	RunE:  sequencer_metricsShowCmdHandler,
}

func sequencer_metricsShowCmdHandler(cmd *cobra.Command, args []string) error {
	jsonFormat = true
	bodyBytes, err := restGet("telemetry/v1/metrics/accelseqqueuemetrics/")
	if err != nil {
		fmt.Println(err)
		return err
	}
	if bodyBytes == nil {
		fmt.Println("No accelseqqueuemetrics object(s) found")
	}
	return nil
}

func init() {

	sonicShowCmd.AddCommand(hw_ringShowCmd)

	//cmd - sonic
	//rootCmd =
	//helpStr = Metrics for Storage Offload NIC

	metricsShowCmd.AddCommand(sonicShowCmd)

	sonicShowCmd.AddCommand(sequencer_infoShowCmd)

	sonicShowCmd.AddCommand(sequencer_metricsShowCmd)

}
