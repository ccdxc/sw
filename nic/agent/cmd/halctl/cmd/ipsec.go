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

//IpsecCbID and ClearOnRead for stats
var (
	IpsecCbID   uint64
	ClearOnRead uint32
)

//IpsecEncryptShowCmd show command for encrypt
var IpsecEncryptShowCmd = &cobra.Command{
	Use:   "ipsec-encrypt",
	Short: "show ipsec-encrypt information",
	Long:  "show ipsec-encrypt information",
	Run:   ipsecEncryptShowCmdHandler,
}

//IpsecDecryptShowCmd Decrypt show command
var IpsecDecryptShowCmd = &cobra.Command{
	Use:   "ipsec-decrypt",
	Short: "show ipsec-decrypt information",
	Long:  "show ipsec-decrypt information",
	Run:   ipsecDecryptShowCmdHandler,
}

//IpsecGlobalStatisticsShowCmd Global Stats show command
var IpsecGlobalStatisticsShowCmd = &cobra.Command{
	Use:   "ipsec global-statistics",
	Short: "show ipsec global-statistics",
	Long:  "show ipsec global-statistics",
	Run:   ipsecGlobalStatisticsShowCmdHandler,
}

func init() {
	showCmd.AddCommand(IpsecEncryptShowCmd)
	showCmd.AddCommand(IpsecDecryptShowCmd)
	showCmd.AddCommand(IpsecGlobalStatisticsShowCmd)
	//IpsecEncryptShowCmd.AddCommand(IpsecDecryptShowCmd)
	//IpsecEncryptShowCmd.AddCommand(IpsecGlobalStatisticsShowCmd)
	IpsecEncryptShowCmd.Flags().Uint64Var(&IpsecCbID, "qid", 1, "Specify qid")
	IpsecDecryptShowCmd.Flags().Uint64Var(&IpsecCbID, "qid", 1, "Specify qid")
	IpsecGlobalStatisticsShowCmd.Flags().Uint32Var(&ClearOnRead, "clear-on-read", 1, "Specify clear on read")
}

func ipsecEncryptShowCmdHandler(cmd *cobra.Command, args []string) {
	if !cmd.Flags().Changed("qid") {
		fmt.Printf("Please specify a queue id(qid)...\n")
		return
	}

	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}

	client := halproto.NewIpsecClient(c.ClientConn)

	defer c.Close()

	req := &halproto.IpsecSAEncryptGetRequest{
		KeyOrHandle: &halproto.IpsecSAEncryptKeyHandle{
			KeyOrHandle: &halproto.IpsecSAEncryptKeyHandle_CbId{
				CbId: IpsecCbID,
			},
		},
	}

	IpsecEncryptGetReqMsg := &halproto.IpsecSAEncryptGetRequestMsg{
		Request: []*halproto.IpsecSAEncryptGetRequest{req},
	}
	respMsg, err := client.IpsecSAEncryptGet(context.Background(), IpsecEncryptGetReqMsg)
	if err != nil {
		fmt.Printf("IpsecSAEncryptGet failed. %v\n", err)
		return
	}
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("HAL Returned error status. %v\n", resp.ApiStatus)
			continue
		}

		fmt.Printf("%s\n", strings.Repeat("-", 80))
		fmt.Printf("\nIpsec Encrypt SA \n")
		fmt.Printf("%s\n", strings.Repeat("-", 80))
		showIpsecEncryptSA(resp)
		fmt.Printf("%s\n\n", strings.Repeat("-", 80))
	}

}

func ipsecDecryptShowCmdHandler(cmd *cobra.Command, args []string) {
	if !cmd.Flags().Changed("qid") {
		fmt.Printf("Please specify a queue id(qid)...\n")
		return
	}

	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}

	client := halproto.NewIpsecClient(c.ClientConn)

	defer c.Close()

	req := &halproto.IpsecSADecryptGetRequest{
		KeyOrHandle: &halproto.IpsecSADecryptKeyHandle{
			KeyOrHandle: &halproto.IpsecSADecryptKeyHandle_CbId{
				CbId: IpsecCbID,
			},
		},
	}

	IpsecDecryptGetReqMsg := &halproto.IpsecSADecryptGetRequestMsg{
		Request: []*halproto.IpsecSADecryptGetRequest{req},
	}
	respMsg, err := client.IpsecSADecryptGet(context.Background(), IpsecDecryptGetReqMsg)
	if err != nil {
		fmt.Printf("IpsecSADecryptGet failed. %v\n", err)
		return
	}
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("HAL Returned error status. %v\n", resp.ApiStatus)
			continue
		}

		fmt.Printf("%s\n", strings.Repeat("-", 80))
		fmt.Printf("\nIpsec Decrypt SA \n")
		fmt.Printf("%s\n", strings.Repeat("-", 80))
		showIpsecDecryptSA(resp)
		fmt.Printf("%s\n\n", strings.Repeat("-", 80))
	}

}

func showIpsecEncryptSA(resp *halproto.IpsecSAEncryptGetResponse) {
	spec := resp.GetSpec()

	fmt.Printf("%-30s : %-6d\n", "authentication_algorithm", spec.AuthenticationAlgorithm)
	fmt.Printf("%-30s : %-6s\n", "authentication_key", spec.GetAuthenticationKey().GetKey())
	fmt.Printf("%-30s : %-6d\n", "encryption_algorithm", spec.EncryptionAlgorithm)
	fmt.Printf("%-30s : %-6s\n", "encryption_key", spec.GetEncryptionKey().GetKey())
	fmt.Printf("%-30s : %-6s\n", "local_gateway_ip", utils.IPAddrToStr(spec.LocalGatewayIp))
	fmt.Printf("%-30s : %-6s\n", "remote_gateway_ip", utils.IPAddrToStr(spec.RemoteGatewayIp))
	fmt.Printf("%-30s : %-6d\n", "spi", spec.Spi)
	fmt.Printf("%-30s : %-6d\n", "tep_vrf", spec.GetTepVrf().GetVrfId())
	fmt.Printf("%-30s : %-6d\n", "salt", spec.Salt)
	fmt.Printf("%-30s : %-6d\n", "iv", spec.Iv)
	fmt.Printf("%-30s : %-6d\n", "key_index", spec.KeyIndex)
	fmt.Printf("%-30s : %-6d\n", "seq_no", spec.SeqNo)
	fmt.Printf("%-30s : %-6d\n", "total_rx_pkts", spec.TotalRxPkts)
	fmt.Printf("%-30s : %-6d\n", "total_rx_drops", spec.TotalRxDrops)
	fmt.Printf("%-30s : %-6d\n", "total_pkts", spec.TotalPkts)
	fmt.Printf("%-30s : %-6d\n", "total_drops", spec.TotalDrops)
}

func showIpsecDecryptSA(resp *halproto.IpsecSADecryptGetResponse) {
	spec := resp.GetSpec()

	fmt.Printf("%-30s : %-6d\n", "authentication_algorithm", spec.AuthenticationAlgorithm)
	fmt.Printf("%-30s : %-6s\n", "authentication_key", spec.GetAuthenticationKey().GetKey())
	fmt.Printf("%-30s : %-6d\n", "decryption_algorithm", spec.DecryptionAlgorithm)
	fmt.Printf("%-30s : %-6s\n", "decryption_key", spec.GetDecryptionKey().GetKey())
	fmt.Printf("%-30s : %-6d\n", "rekey_dec_algorithm", spec.RekeyDecAlgorithm)
	fmt.Printf("%-30s : %-6s\n", "rekey_decryption_key", spec.GetRekeyDecryptionKey().GetKey())
	fmt.Printf("%-30s : %-6s\n", "rekey_authentication_key", spec.GetRekeyAuthenticationKey().GetKey())
	fmt.Printf("%-30s : %-6d\n", "spi", spec.Spi)
	fmt.Printf("%-30s : %-6d\n", "key_index", spec.KeyIndex)
	if spec.RekeyActive == 1 {
		fmt.Printf("%-30s : %-6s\n", "rekey_active", "True")
		fmt.Printf("%-30s : %-6d\n", "rekey_spi", spec.RekeySpi)
		fmt.Printf("%-30s : %-6d\n", "new_key_index", spec.NewKeyIndex)
	} else {
		fmt.Printf("%-30s : %-6s\n", "rekey_active", "False")
	}
	fmt.Printf("%-30s : %-6d\n", "tep_vrf", spec.GetTepVrf().GetVrfId())
	fmt.Printf("%-30s : %-6d\n", "salt", spec.Salt)
	fmt.Printf("%-30s : %-6d\n", "expected_seq_no", spec.SeqNo)
	fmt.Printf("%-30s : %-6d\n", "total_rx_pkts", spec.TotalRxPkts)
	fmt.Printf("%-30s : %-6d\n", "total_rx_drops", spec.TotalRxDrops)
	fmt.Printf("%-30s : %-6d\n", "total_pkts", spec.TotalPkts)
	fmt.Printf("%-30s : %-6d\n", "total_drops", spec.TotalDrops)
}

func ipsecGlobalStatisticsShowCmdHandler(cmd *cobra.Command, args []string) {
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}

	client := halproto.NewIpsecClient(c.ClientConn)

	defer c.Close()

	req := &halproto.IpsecGlobalStatisticsGetRequest{
		ClearOnRead: ClearOnRead,
	}
	IpsecGlobalStatisticsGetRequestMsg := &halproto.IpsecGlobalStatisticsGetRequestMsg{
		Request: []*halproto.IpsecGlobalStatisticsGetRequest{req},
	}
	respMsg, err := client.IpsecGlobalStatisticsGet(context.Background(), IpsecGlobalStatisticsGetRequestMsg)
	if err != nil {
		fmt.Printf("IpsecGlobalStatisticsGetRequest failed. %v\n", err)
		return
	}
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("HAL Returned error status. %v\n", resp.ApiStatus)
			continue
		}

		fmt.Printf("%s\n", strings.Repeat("-", 80))
		fmt.Printf("\nIpsec Global Statistics \n")
		fmt.Printf("%s\n", strings.Repeat("-", 80))
		showIpsecGlobalStatistics(resp)
		fmt.Printf("%s\n\n", strings.Repeat("-", 80))
	}
}

func showIpsecGlobalStatistics(resp *halproto.IpsecGlobalStatisticsGetResponse) {
	spec := resp.GetSpec()

	fmt.Printf("%-40s : %-6d\n", "encrypt_input_desc_errors", spec.EncryptInputDescErrors)
	fmt.Printf("%-40s : %-6d\n", "encrypt_output_desc_errors", spec.EncryptOutputDescErrors)
	fmt.Printf("%-40s : %-6d\n", "encrypt_cb_ring_base_errors", spec.EncryptCbRingBaseErrors)
	fmt.Printf("%-40s : %-6d\n", "encrypt_input_page_errors", spec.EncryptInputPageErrors)
	fmt.Printf("%-40s : %-6d\n", "encrypt_barco_req_addr_errors", spec.EncryptBarcoReqAddrErrors)
	fmt.Printf("%-40s : %-6d\n", "encrypt_barco_cb_base_errors", spec.EncryptBarcoCbBaseErrors)
	fmt.Printf("%-40s : %-6d\n", "encrypt_pad_addr_errors", spec.EncryptPadAddrErrors)
	fmt.Printf("%-40s : %-6d\n", "encrypt_tail_bytes_errors", spec.EncryptTailBytesErrors)
	fmt.Printf("%-40s : %-6d\n", "encrypt_output_page_errors", spec.EncryptOutputPageErrors)
	fmt.Printf("%-40s : %-6d\n", "encrypt_stage4_inpage_errors", spec.EncryptStage4InpageErrors)
	fmt.Printf("%-40s : %-6d\n", "encrypt_table0_inpage_errors", spec.EncryptTable0InpageErrors)
	fmt.Printf("%-40s : %-6d\n", "encrypt_table2_inpage_errors", spec.EncryptTable2InpageErrors)
	fmt.Printf("%-40s : %-6d\n", "encrypt_table3_inpage_errors", spec.EncryptTable3InpageErrors)
	fmt.Printf("%-40s : %-6d\n", "encrypt_bad_barco_addr_errors", spec.EncryptBadBarcoAddrErrors)
	fmt.Printf("%-40s : %-6d\n", "encrypt_barco_full_errors", spec.EncryptBarcoFullErrors)
	fmt.Printf("%-40s : %-6d\n", "encrypt_cb_ring_dma_errors", spec.EncryptCbRingDmaErrors)

	fmt.Printf("%-40s : %-6d\n", "decrypt_input_desc_errors", spec.DecryptInputDescErrors)
	fmt.Printf("%-40s : %-6d\n", "decrypt_output_desc_errors", spec.DecryptOutputDescErrors)
	fmt.Printf("%-40s : %-6d\n", "decrypt_cb_ring_base_errors", spec.DecryptCbRingBaseErrors)
	fmt.Printf("%-40s : %-6d\n", "decrypt_input_page_errors", spec.DecryptInputPageErrors)
	fmt.Printf("%-40s : %-6d\n", "decrypt_barco_req_addr_errors", spec.DecryptBarcoReqAddrErrors)
	fmt.Printf("%-40s : %-6d\n", "decrypt_barco_cb_addr_errors", spec.DecryptBarcoCbAddrErrors)
	fmt.Printf("%-40s : %-6d\n", "decrypt_stage4_inpage_errors", spec.DecryptStage4InpageErrors)
	fmt.Printf("%-40s : %-6d\n", "decrypt_output_page_errors", spec.DecryptOutputPageErrors)
	fmt.Printf("%-40s : %-6d\n", "decrypt_txdma1_enter_counters", spec.DecryptTxdma1EnterCounters)
	fmt.Printf("%-40s : %-6d\n", "decrypt_txdma2_enter_counters", spec.DecryptTxdma2EnterCounters)
	fmt.Printf("%-40s : %-6d\n", "decrypt_txdma1_drop_counters", spec.DecryptTxdma1DropCounters)
}
