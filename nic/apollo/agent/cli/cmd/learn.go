//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	uuid "github.com/satori/go.uuid"
	"github.com/spf13/cobra"
	yaml "gopkg.in/yaml.v2"
	"reflect"
	"strings"

	"github.com/pensando/sw/nic/apollo/agent/cli/utils"
	"github.com/pensando/sw/nic/apollo/agent/gen/pds"
)

var (
	vpcId    string
	subnetId string
	epIP     string
	epMAC    string
)

var learnShowCmd = &cobra.Command{
	Use:   "learn",
	Short: "show learnt endpoint information",
	Long:  "show learnt endpoint information",
}

var learnClearCmd = &cobra.Command{
	Use:   "learn",
	Short: "clear learnt endpoint information",
	Long:  "clear learnt endpoint information",
}

var learnMACShowCmd = &cobra.Command{
	Use:   "mac",
	Short: "show learnt endpoint MAC information",
	Long:  "show learnt endpoint MAC information",
	Run:   learnMACShowCmdHandler,
}

var learnIPShowCmd = &cobra.Command{
	Use:   "ip",
	Short: "show learnt endpoint IP information",
	Long:  "show learnt endpoint IP information",
	Run:   learnIPShowCmdHandler,
}

var learnStatsShowCmd = &cobra.Command{
	Use:   "statistics",
	Short: "show endpoint learning statistics",
	Long:  "show endpoint learning statistics",
	Run:   learnStatsShowCmdHandler,
}

var learnMACClearCmd = &cobra.Command{
	Use:   "mac",
	Short: "clear learnt endpoint MAC information",
	Long:  "clear learnt endpoint MAC information",
	Run:   learnMACClearCmdHandler,
}

var learnIPClearCmd = &cobra.Command{
	Use:   "ip",
	Short: "clear learnt endpoint IP information",
	Long:  "clear learnt endpoint IP information",
	Run:   learnIPClearCmdHandler,
}

var learnStatsClearCmd = &cobra.Command{
	Use:   "statistics",
	Short: "clear endpoint learning statistics",
	Long:  "clear endpoint learning statistics",
	Run:   learnStatsClearCmdHandler,
}

func init() {
	showCmd.AddCommand(learnShowCmd)
	learnShowCmd.AddCommand(learnMACShowCmd)
	learnMACShowCmd.Flags().StringVar(&subnetId, "subnet", "0", "Specify Subnet ID")
	learnMACShowCmd.Flags().StringVar(&epMAC, "mac", "0", "Specify MAC address")
	learnMACShowCmd.Flags().Bool("yaml", false, "Output in yaml")
	learnShowCmd.AddCommand(learnIPShowCmd)
	learnIPShowCmd.Flags().StringVar(&vpcId, "vpc", "0", "Specify VPC ID")
	learnIPShowCmd.Flags().StringVar(&epIP, "ip", "0", "Specify IP address")
	learnIPShowCmd.Flags().StringVar(&subnetId, "subnet", "0", "Specify Subnet ID")
	learnIPShowCmd.Flags().Bool("yaml", false, "Output in yaml")
	learnShowCmd.AddCommand(learnStatsShowCmd)
	learnStatsShowCmd.Flags().Bool("yaml", false, "Output in yaml")
	// Clear commands
	clearCmd.AddCommand(learnClearCmd)
	learnClearCmd.AddCommand(learnMACClearCmd)
	learnMACClearCmd.Flags().StringVar(&subnetId, "subnet", "0", "Specify Subnet ID")
	learnMACClearCmd.Flags().StringVar(&epMAC, "mac", "0", "Specify MAC address")
	learnClearCmd.AddCommand(learnIPClearCmd)
	learnIPClearCmd.Flags().StringVar(&vpcId, "vpc", "0", "Specify VPC ID")
	learnIPClearCmd.Flags().StringVar(&epIP, "ip", "0", "Specify IP address")
	learnClearCmd.AddCommand(learnStatsClearCmd)
}

func learnMACShowCmdHandler(cmd *cobra.Command, args []string) {
	var detail bool
	// Connect to PDS
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the PDS. Is PDS Running?\n")
		return
	}
	defer c.Close()

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	if cmd != nil && (cmd.Flags().Changed("subnet") != cmd.Flags().Changed("mac")) {
		fmt.Printf("Cannot specify only one of Subnet ID and endpoint MAC address\n")
		return
	}

	var req *pds.LearnMACRequest
	if cmd != nil && cmd.Flags().Changed("mac") && cmd.Flags().Changed("subnet") {
		// Get specific entry
		req = &pds.LearnMACRequest{
			Key: []*pds.LearnMACKey{
				&pds.LearnMACKey{
					SubnetId: uuid.FromStringOrNil(subnetId).Bytes(),
					MACAddr:  utils.MACAddrStrToUint64(epMAC),
				},
			},
		}
		detail = true
	} else {
		// Get all entries
		req = &pds.LearnMACRequest{
			Key: []*pds.LearnMACKey{},
		}
		detail = false
	}

	client := pds.NewLearnSvcClient(c)

	// PDS call
	resp, err := client.LearnMACGet(context.Background(), req)
	if err != nil {
		fmt.Printf("Getting MAC information failed. %v\n", err)
		return
	}

	if resp.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
		return
	}

	if cmd != nil && cmd.Flags().Changed("yaml") {
		for _, respData := range resp.Response {
			respType := reflect.ValueOf(respData)
			b, _ := yaml.Marshal(respType.Interface())
			fmt.Println(string(b))
			fmt.Println("---")
		}
	} else {
		printLearnMAC(resp, detail)
	}
}

func learnIPShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to PDS
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the PDS. Is PDS Running?\n")
		return
	}
	defer c.Close()

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	if cmd != nil && ((cmd.Flags().Changed("vpc") || cmd.Flags().Changed("ip")) && cmd.Flags().Changed("subnet")) {
		fmt.Printf("Cannot specify both [VPC ID/IP address] and Subnet ID as filters\n")
		return
	}
	if cmd != nil && (cmd.Flags().Changed("vpc") != cmd.Flags().Changed("ip")) {
		fmt.Printf("Cannot specify only one of VPC ID and endpoint IP address\n")
		return
	}

	var req *pds.LearnIPGetRequest
	if cmd != nil && cmd.Flags().Changed("ip") && cmd.Flags().Changed("vpc") {
		// Get entry by VPC/IP key
		req = &pds.LearnIPGetRequest{
			Filter: &pds.LearnIPGetRequest_Key{
				Key: &pds.LearnIPKey{
					VPCId:  uuid.FromStringOrNil(vpcId).Bytes(),
					IPAddr: utils.IPAddrStrToPDSIPAddr(epIP),
				},
			},
		}
	} else if cmd != nil && cmd.Flags().Changed("subnet") {
		// Get entries belonging to subnet
		req = &pds.LearnIPGetRequest{
			Filter: &pds.LearnIPGetRequest_SubnetId{
				SubnetId: uuid.FromStringOrNil(subnetId).Bytes(),
			},
		}
	} else {
		// Get all entries
		req = &pds.LearnIPGetRequest{}
	}

	client := pds.NewLearnSvcClient(c)

	// PDS call
	resp, err := client.LearnIPGet(context.Background(), req)
	if err != nil {
		fmt.Printf("Getting IP information failed. %v\n", err)
		return
	}

	if resp.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
		return
	}

	if cmd != nil && cmd.Flags().Changed("yaml") {
		for _, respData := range resp.Response {
			respType := reflect.ValueOf(respData)
			b, _ := yaml.Marshal(respType.Interface())
			fmt.Println(string(b))
			fmt.Println("---")
		}
	} else {
		printLearnIP(resp)
	}
}

func learnStatsShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to PDS
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the PDS. Is PDS Running?\n")
		return
	}
	defer c.Close()

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	var req *pds.Empty
	client := pds.NewLearnSvcClient(c)

	// PDS call
	resp, err := client.LearnStatsGet(context.Background(), req)
	if err != nil {
		fmt.Printf("Getting statistics failed. %v\n", err)
		return
	}

	if resp.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
		return
	}

	if cmd != nil && cmd.Flags().Changed("yaml") {
		respType := reflect.ValueOf(resp.Stats)
		b, _ := yaml.Marshal(respType.Interface())
		fmt.Println(string(b))
		fmt.Println("---")
	} else {
		printLearnStats(resp)
	}
}

func learnMACClearCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to PDS
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the PDS. Is PDS Running?\n")
		return
	}
	defer c.Close()

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	if cmd != nil && (cmd.Flags().Changed("subnet") != cmd.Flags().Changed("mac")) {
		fmt.Printf("Cannot specify only one of Subnet ID and endpoint MAC address\n")
		return
	}

	var req *pds.LearnMACRequest
	if cmd != nil && cmd.Flags().Changed("mac") && cmd.Flags().Changed("subnet") {
		// Get specific entry
		req = &pds.LearnMACRequest{
			Key: []*pds.LearnMACKey{
				&pds.LearnMACKey{
					SubnetId: uuid.FromStringOrNil(subnetId).Bytes(),
					MACAddr:  utils.MACAddrStrToUint64(epMAC),
				},
			},
		}
	} else {
		// Clear all entries
		req = &pds.LearnMACRequest{
			Key: []*pds.LearnMACKey{},
		}
	}

	client := pds.NewLearnSvcClient(c)

	// PDS call
	resp, err := client.LearnMACClear(context.Background(), req)
	if err != nil {
		fmt.Printf("Clear learnt MAC(s) failed. %v\n", err)
		return
	}

	if resp.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
		return
	}
	fmt.Printf("Clearing MAC(s) succeeded\n")
}

func learnIPClearCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to PDS
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the PDS. Is PDS Running?\n")
		return
	}
	defer c.Close()

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	if cmd != nil && (cmd.Flags().Changed("vpc") != cmd.Flags().Changed("ip")) {
		fmt.Printf("Cannot specify only one of VPC ID and endpoint IP address\n")
		return
	}

	var req *pds.LearnIPRequest
	if cmd != nil && cmd.Flags().Changed("ip") && cmd.Flags().Changed("vpc") {
		// Get specific entry
		req = &pds.LearnIPRequest{
			Key: []*pds.LearnIPKey{
				&pds.LearnIPKey{
					VPCId:  uuid.FromStringOrNil(vpcId).Bytes(),
					IPAddr: utils.IPAddrStrToPDSIPAddr(epIP),
				},
			},
		}
	} else {
		// Clear all entries
		req = &pds.LearnIPRequest{
			Key: []*pds.LearnIPKey{},
		}
	}

	client := pds.NewLearnSvcClient(c)

	// PDS call
	resp, err := client.LearnIPClear(context.Background(), req)
	if err != nil {
		fmt.Printf("Clearing learnt IP(s) failed. %v\n", err)
		return
	}

	if resp.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
		return
	}
	fmt.Printf("Clearing IP(s) succeeded\n")
}

func learnStatsClearCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to PDS
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the PDS. Is PDS Running?\n")
		return
	}
	defer c.Close()

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	var empty *pds.Empty
	client := pds.NewLearnSvcClient(c)

	// PDS call
	_, err = client.LearnStatsClear(context.Background(), empty)
	if err != nil {
		fmt.Printf("Clearing statistics failed. %v\n", err)
		return
	}
	fmt.Printf("Clearing statistics succeeded\n")
}

func LearnStateToStr(state pds.EpState) string {
	var stateStr string

	switch state {
	case pds.EpState_EP_STATE_LEARNING:
		stateStr = "Learning"
	case pds.EpState_EP_STATE_CREATED:
		stateStr = "Created"
	case pds.EpState_EP_STATE_PROBING:
		stateStr = "Probing"
	case pds.EpState_EP_STATE_UPDATING:
		stateStr = "Updating"
	case pds.EpState_EP_STATE_DELETING:
		stateStr = "Deleting"
	case pds.EpState_EP_STATE_DELETED:
		stateStr = "Deleted"
	default:
	}
	return stateStr
}

func LearnPktDropReasonToStr(reason pds.LearnPktDropReason) string {
	var reasonStr string

	switch reason {
	case pds.LearnPktDropReason_LEARN_PKTDROP_REASON_PARSE_ERR:
		reasonStr = "Parse errors"
	case pds.LearnPktDropReason_LEARN_PKTDROP_REASON_RES_ALLOC_FAIL:
		reasonStr = "Resource allocation failures"
	case pds.LearnPktDropReason_LEARN_PKTDROP_REASON_MBUF_ERR:
		reasonStr = "Internal packet buffer errors"
	case pds.LearnPktDropReason_LEARN_PKTDROP_REASON_TX_FAIL:
		reasonStr = "Tx failures"
	case pds.LearnPktDropReason_LEARN_PKTDROP_REASON_ARP_REPLY:
		reasonStr = "ARP replies"
	default:
		reasonStr = "Unknown"
	}
	return reasonStr
}

func printLearnMACHeader() {
	hdrLine := strings.Repeat("-", 130)
	fmt.Println(hdrLine)
	fmt.Printf("%-20s%-40s%-40s%-10s%-10s%-8s\n",
		"MACAddr", "SubnetId", "VnicId", "State", "IP count",
		"TTL(s)")
	fmt.Println(hdrLine)
}

func printLearnMAC(resp *pds.LearnMACGetResponse, detail bool) {
	if len(resp.GetResponse()) > 0 {
		printLearnMACHeader()
	}
	for _, MACEntry := range resp.GetResponse() {
		fmt.Printf("%-20s%-40s%-40s%-10s%-10d%-8d\n",
			utils.MactoStr(MACEntry.GetKey().GetMACAddr()),
			uuid.FromBytesOrNil(MACEntry.GetKey().GetSubnetId()).String(),
			uuid.FromBytesOrNil(MACEntry.GetVnicId()).String(),
			LearnStateToStr(MACEntry.GetState()),
			len(MACEntry.GetIPInfo()),
			MACEntry.GetTTL())
		if detail == true {
			if len(MACEntry.GetIPInfo()) > 0 {
				fmt.Printf("	IPs learnt:\n")
			}
			for _, IPKey := range MACEntry.GetIPInfo() {
				fmt.Printf("		%s\n",
					utils.IPAddrToStr(IPKey.GetIPAddr()))
			}
		}
	}
}

func printLearnIPHeader() {
	hdrLine := strings.Repeat("-", 140)
	fmt.Println(hdrLine)
	fmt.Printf("%-20s%-40s%-20s%-40s%-10s%-8s\n",
		"IPAddr", "VpcID", "MACAddr", "SubnetId", "State", "TTL(s)")
	fmt.Println(hdrLine)
}

func printLearnIP(resp *pds.LearnIPGetResponse) {
	if len(resp.GetResponse()) > 0 {
		printLearnIPHeader()
	}
	for _, IPEntry := range resp.GetResponse() {
		fmt.Printf("%-20s%-40s%-20s%-40s%-10s%-8d\n",
			utils.IPAddrToStr(IPEntry.GetKey().GetIPAddr()),
			uuid.FromBytesOrNil(IPEntry.GetKey().GetVPCId()).String(),
			utils.MactoStr(IPEntry.GetMACInfo().GetMACAddr()),
			uuid.FromBytesOrNil(IPEntry.GetMACInfo().GetSubnetId()).String(),
			LearnStateToStr(IPEntry.GetState()),
			IPEntry.GetTTL())
	}
}

func printLearnStats(resp *pds.LearnStatsGetResponse) {
	stats := resp.GetStats()

	fmt.Printf("Endpoint learning statistics:\n")
	fmt.Printf("	Total MACs/VNICs learnt      : %-20d\n", stats.GetNumVnics())
	fmt.Printf("	Total IPs/L3 mappings learnt : %-20d\n", stats.GetNumL3Mappings())
	fmt.Printf("	# API calls                  : %-20d\n", stats.GetNumApiCalls())
	fmt.Printf("	# API failures               : %-20d\n", stats.GetNumApiFailure())
	fmt.Printf("	# Packets received           : %-20d\n", stats.GetNumPktsRcvd())
	fmt.Printf("	# Packets sent               : %-20d\n", stats.GetNumPktsSent())
	if len(stats.GetDropStats()) > 0 {
		fmt.Printf("	# Drop counters:\n")
	}
	for _, dropStats := range stats.GetDropStats() {
		fmt.Printf("		%-31s: %-20d\n",
			LearnPktDropReasonToStr(dropStats.GetReason()),
			dropStats.GetNumDrops())
	}
}
