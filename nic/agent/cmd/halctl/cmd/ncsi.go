//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	"os"
	// "reflect"
	// "sort"
	"strings"

	"github.com/spf13/cobra"
	// yaml "gopkg.in/yaml.v2"

	"github.com/pensando/sw/nic/agent/cmd/halctl/utils"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
)

var (
	channelID   uint32
	mFchannelID uint32
	bFchannelID uint32
	vlanID      uint32
	macAddr     uint64
	rxCh        string
	txCh        string
)

var ncsiDebugCmd = &cobra.Command{
	Use:   "ncsi",
	Short: "debug ncsi information",
	Long:  "debug ncsi object information",
}

var ncsiVlanDebugCmd = &cobra.Command{
	Use:   "vlan",
	Short: "Add/Delete ncsi vlan information",
	Long:  "Add/Delete ncsi vlan object information",
	Run:   ncsiVlanDebugCmdHandler,
}

var ncsiMacDebugCmd = &cobra.Command{
	Use:   "mac",
	Short: "Add/Delete ncsi mac information",
	Long:  "Add/Delete ncsi mac object information",
	Run:   ncsiMacDebugCmdHandler,
}

var ncsiChannelDebugCmd = &cobra.Command{
	Use:   "channel",
	Short: "Add/Delete ncsi channel information",
	Long:  "Add/Delete ncsi channel object information",
	Run:   ncsiChannelDebugCmdHandler,
}

var ncsiShowCmd = &cobra.Command{
	Use:   "ncsi",
	Short: "show ncsi information",
	Long:  "show ncsi object information",
}

var ncsiVlanShowCmd = &cobra.Command{
	Use:   "vlan",
	Short: "show ncsi vlan information",
	Long:  "show ncsi vlan object information",
	Run:   ncsiVlanShowCmdHandler,
}

var ncsiMacShowCmd = &cobra.Command{
	Use:   "mac",
	Short: "show ncsi mac information",
	Long:  "show ncsi mac object information",
	Run:   ncsiMacShowCmdHandler,
}

var ncsiBcastShowCmd = &cobra.Command{
	Use:   "bcast",
	Short: "show ncsi bcast information",
	Long:  "show ncsi bcast object information",
	Run:   ncsiBcastShowCmdHandler,
}

var ncsiMcastShowCmd = &cobra.Command{
	Use:   "mcast",
	Short: "show ncsi mcast information",
	Long:  "show ncsi mcast object information",
	Run:   ncsiMcastShowCmdHandler,
}

var ncsiChannelShowCmd = &cobra.Command{
	Use:   "channel",
	Short: "show ncsi channel information",
	Long:  "show ncsi channel object information",
	Run:   ncsiChannelShowCmdHandler,
}

func init() {
	showCmd.AddCommand(ncsiShowCmd)
	ncsiShowCmd.AddCommand(ncsiVlanShowCmd)
	ncsiShowCmd.AddCommand(ncsiMacShowCmd)
	ncsiShowCmd.AddCommand(ncsiBcastShowCmd)
	ncsiShowCmd.AddCommand(ncsiMcastShowCmd)
	ncsiShowCmd.AddCommand(ncsiChannelShowCmd)

	// filter commands
	ncsiMcastShowCmd.Flags().Uint32Var(&mFchannelID, "id", 0xFF, "Specify channelid")
	ncsiBcastShowCmd.Flags().Uint32Var(&bFchannelID, "id", 0xFF, "Specify channelid")
	// Channel command
	ncsiChannelShowCmd.Flags().Uint32Var(&channelID, "id", 0xFF, "Specify channelid")

	// Debug commands
	debugCmd.AddCommand(ncsiDebugCmd)
	// Vlan
	ncsiDebugCmd.AddCommand(ncsiVlanDebugCmd)
	ncsiVlanDebugCmd.Flags().Uint32Var(&vlanID, "add", 0, "Specify vlan id")
	ncsiVlanDebugCmd.Flags().Uint32Var(&vlanID, "del", 0, "Specify vlan id")
	ncsiVlanDebugCmd.Flags().Uint32Var(&channelID, "ch", 0, "Specify channel id")
	// Mac
	ncsiDebugCmd.AddCommand(ncsiMacDebugCmd)
	ncsiMacDebugCmd.Flags().Uint64Var(&macAddr, "add", 0, "Specify mac address")
	ncsiMacDebugCmd.Flags().Uint64Var(&macAddr, "del", 0, "Specify mac address")
	ncsiMacDebugCmd.Flags().Uint32Var(&channelID, "ch", 0, "Specify channel id")
	// Channel
	ncsiDebugCmd.AddCommand(ncsiChannelDebugCmd)
	ncsiChannelDebugCmd.Flags().StringVar(&rxCh, "rx", "enable", "Enable channel RX enable | disable")
	ncsiChannelDebugCmd.Flags().StringVar(&txCh, "tx", "enable", "Enable channel TX enable | disable")
	ncsiChannelDebugCmd.Flags().Uint32Var(&channelID, "ch", 0, "Specify channel id")
}

func ncsiVlanDebugCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	defer c.Close()

	client := halproto.NewNcsiClient(c)

	if cmd.Flags().Changed("add") == false && cmd.Flags().Changed("del") == false {
		fmt.Printf("Either you have to add or del the vlan. Reger help\n")
		return
	}

	if cmd.Flags().Changed("add") == true && cmd.Flags().Changed("del") == true {
		fmt.Printf("You can't add and del in the same command. Reger help\n")
		return
	}

	isAdd := false
	if cmd.Flags().Changed("add") == true {
		isAdd = true
	}

	var vlanFilter *halproto.VlanFilterRequest
	vlanFilter = &halproto.VlanFilterRequest{
		VlanId:  vlanID,
		Channel: channelID,
	}

	VlanFilterRequestMsg := &halproto.VlanFilterRequestMsg{
		Request: []*halproto.VlanFilterRequest{vlanFilter},
	}

	if isAdd {
		rspMsg, err := client.VlanFilterCreate(context.Background(), VlanFilterRequestMsg)
		if err != nil {
			fmt.Printf("Vlan filter create fialed. %v\n", err)
			return
		}

		for _, updateResp := range rspMsg.Response {
			if updateResp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
				fmt.Printf("Operation failed with %v error\n", updateResp.ApiStatus)
				return
			}
		}
	} else {
		rspMsg, err := client.VlanFilterDelete(context.Background(), VlanFilterRequestMsg)
		if err != nil {
			fmt.Printf("Vlan filter create fialed. %v\n", err)
			return
		}
		for _, updateResp := range rspMsg.Response {
			if updateResp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
				fmt.Printf("Operation failed with %v error\n", updateResp.ApiStatus)
				return
			}
		}
	}

	fmt.Printf("Vlan update success\n")
}

func ncsiMacDebugCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	defer c.Close()

	client := halproto.NewNcsiClient(c)

	if cmd.Flags().Changed("add") == false && cmd.Flags().Changed("del") == false {
		fmt.Printf("Either you have to add or del the mac. Reger help\n")
		return
	}

	if cmd.Flags().Changed("add") == true && cmd.Flags().Changed("del") == true {
		fmt.Printf("You can't add and del in the same command. Reger help\n")
		return
	}

	isAdd := false
	if cmd.Flags().Changed("add") == true {
		isAdd = true
	}

	var macFilter *halproto.MacFilterRequest
	macFilter = &halproto.MacFilterRequest{
		MacAddr: macAddr,
		Channel: channelID,
	}

	MacFilterRequestMsg := &halproto.MacFilterRequestMsg{
		Request: []*halproto.MacFilterRequest{macFilter},
	}

	if isAdd {
		_, err := client.MacFilterCreate(context.Background(), MacFilterRequestMsg)
		if err != nil {
			fmt.Printf("Mac filter create fialed. %v\n", err)
			return
		}
	} else {
		_, err := client.MacFilterDelete(context.Background(), MacFilterRequestMsg)
		if err != nil {
			fmt.Printf("Mac filter create fialed. %v\n", err)
			return
		}
	}

	fmt.Printf("Mac update success\n")
}

func ncsiChannelDebugCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	defer c.Close()

	client := halproto.NewNcsiClient(c)

	rxChEnable := true
	if cmd.Flags().Changed("rx") == true {
		if strings.Compare(rxCh, "disable") == 0 {
			rxChEnable = false
		} else if strings.Compare(rxCh, "enable") == 0 {
			rxChEnable = true
		} else {
			fmt.Printf("Wrong args. Refer help.\n")
			return
		}
	}

	txChEnable := true
	if cmd.Flags().Changed("tx") == true {
		if strings.Compare(txCh, "disable") == 0 {
			txChEnable = false
		} else if strings.Compare(txCh, "enable") == 0 {
			txChEnable = true
		} else {
			fmt.Printf("Wrong args. Refer help.\n")
			return
		}
	}

	// Getting channel info
	var req *halproto.ChannelGetRequest
	req = &halproto.ChannelGetRequest{
		Channel: channelID,
	}
	channelGetRequestMsg := &halproto.ChannelGetRequestMsg{
		Request: []*halproto.ChannelGetRequest{req},
	}

	// HAL call
	respMsg, err := client.ChannelGet(context.Background(), channelGetRequestMsg)
	if err != nil {
		fmt.Printf("Getting channels failed. %v\n", err)
		return
	}

	resp := respMsg.Response[0]

	if cmd.Flags().Changed("rx") == false {
		rxChEnable = resp.GetRequest().GetRxEnable()
	}
	if cmd.Flags().Changed("tx") == false {
		txChEnable = resp.GetRequest().GetTxEnable()
	}

	var chReq *halproto.ChannelRequest
	chReq = &halproto.ChannelRequest{
		Channel:  channelID,
		TxEnable: txChEnable,
		RxEnable: rxChEnable,
	}

	channelRequestMsg := &halproto.ChannelRequestMsg{
		Request: []*halproto.ChannelRequest{chReq},
	}

	updateRespMsg, err := client.ChannelUpdate(context.Background(), channelRequestMsg)
	if err != nil {
		fmt.Printf("Channel update failed. %v\n", err)
		return
	}
	for _, updateResp := range updateRespMsg.Response {
		if updateResp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("Update Operation failed with %v error\n", updateResp.ApiStatus)
			return
		}
	}

	fmt.Printf("Channel update success\n")
}

func ncsiVlanShowCmdHandler(cmd *cobra.Command, args []string) {

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	client := halproto.NewNcsiClient(c)

	var req *halproto.VlanFilterGetRequest
	// Get all Vrfs
	req = &halproto.VlanFilterGetRequest{}

	vlanFilterGetRequestMsg := &halproto.VlanFilterGetRequestMsg{
		Request: []*halproto.VlanFilterGetRequest{req},
	}

	// HAL call
	respMsg, err := client.VlanFilterGet(context.Background(), vlanFilterGetRequestMsg)
	if err != nil {
		fmt.Printf("Getting vlan filters failed. %v\n", err)
		return
	}

	// Print Header
	ncsiVlanShowHeader()

	// Print vrfs
	// m := make(map[uint64]*halproto.VlanFilterGetResponse)
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
			continue
		}
		ncsiVlanShowResp(resp)
		// m[resp.GetSpec().GetKeyOrHandle().GetVrfId()] = resp
	}
	// var keys []uint64
	// for k := range m {
	// 	keys = append(keys, k)
	// }
	// sort.Slice(keys, func(i, j int) bool { return keys[i] < keys[j] })
	// for _, k := range keys {
	// 	vrfShowOneResp(m[k])
	// }
}

func ncsiVlanShowHeader() {
	fmt.Printf("\n")
	fmt.Printf("Ch: Channel				Vlan: vlan id\n")
	hdrLine := strings.Repeat("-", 20)
	fmt.Println(hdrLine)
	fmt.Printf("%-5s%-10s\n", "Ch", "Vlan")
	fmt.Println(hdrLine)
}

func ncsiVlanShowResp(resp *halproto.VlanFilterGetResponse) {
	fmt.Printf("%-5d%-10d\n", resp.GetRequest().GetChannel(),
		resp.GetRequest().GetVlanId())
}

func ncsiMacShowCmdHandler(cmd *cobra.Command, args []string) {

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	client := halproto.NewNcsiClient(c)

	var req *halproto.MacFilterGetRequest
	// Get all Vrfs
	req = &halproto.MacFilterGetRequest{}

	macFilterGetRequestMsg := &halproto.MacFilterGetRequestMsg{
		Request: []*halproto.MacFilterGetRequest{req},
	}

	// HAL call
	respMsg, err := client.MacFilterGet(context.Background(), macFilterGetRequestMsg)
	if err != nil {
		fmt.Printf("Getting mac filters failed. %v\n", err)
		return
	}

	// Print Header
	ncsiMacShowHeader()

	// Print vrfs
	// m := make(map[uint64]*halproto.VlanFilterGetResponse)
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
			continue
		}
		ncsiMacShowResp(resp)
		// m[resp.GetSpec().GetKeyOrHandle().GetVrfId()] = resp
	}
	// var keys []uint64
	// for k := range m {
	// 	keys = append(keys, k)
	// }
	// sort.Slice(keys, func(i, j int) bool { return keys[i] < keys[j] })
	// for _, k := range keys {
	// 	vrfShowOneResp(m[k])
	// }
}

func ncsiMacShowHeader() {
	fmt.Printf("\n")
	fmt.Printf("Ch: Channel		Mac: mac address\n")
	hdrLine := strings.Repeat("-", 25)
	fmt.Println(hdrLine)
	fmt.Printf("%-5s%-20s\n", "Ch", "Mac")
	fmt.Println(hdrLine)
}

func ncsiMacShowResp(resp *halproto.MacFilterGetResponse) {
	macStr := utils.MactoStr(resp.GetRequest().GetMacAddr())
	fmt.Printf("%-5d%-20s\n", resp.GetRequest().GetChannel(), macStr)
}

func ncsiBcastShowCmdHandler(cmd *cobra.Command, args []string) {

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	client := halproto.NewNcsiClient(c)

	var req *halproto.BcastFilterGetRequest
	req = &halproto.BcastFilterGetRequest{
		Channel: bFchannelID,
	}

	bcastFilterGetRequestMsg := &halproto.BcastFilterGetRequestMsg{
		Request: []*halproto.BcastFilterGetRequest{req},
	}

	// HAL call
	respMsg, err := client.BcastFilterGet(context.Background(), bcastFilterGetRequestMsg)
	if err != nil {
		fmt.Printf("Getting bcast filters failed. %v\n", err)
		return
	}

	// Print Header
	ncsiBcastShowHeader()

	// Print vrfs
	// m := make(map[uint64]*halproto.VlanFilterGetResponse)
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
			continue
		}
		ncsiBcastShowResp(resp)
		// m[resp.GetSpec().GetKeyOrHandle().GetVrfId()] = resp
	}
	// var keys []uint64
	// for k := range m {
	// 	keys = append(keys, k)
	// }
	// sort.Slice(keys, func(i, j int) bool { return keys[i] < keys[j] })
	// for _, k := range keys {
	// 	vrfShowOneResp(m[k])
	// }
}

func ncsiBcastShowHeader() {
	fmt.Printf("\n")
	fmt.Printf("Ch: Channel		Bcast Filters: ARP, DHCP_CLIENT, DHCP_SERVER, NETBIOS\n")
	hdrLine := strings.Repeat("-", 50)
	fmt.Println(hdrLine)
	fmt.Printf("%-5s%-45s\n", "Ch", "Bcast-Filters")
	fmt.Println(hdrLine)
}

func bcastFltrToStr(resp *halproto.BcastFilterGetResponse) string {
	var str string

	if resp.GetRequest().GetEnableArp() {
		str += "ARP,"
	}
	if resp.GetRequest().GetEnableDhcpClient() {
		str += "DHCP_CLIENT,"
	}
	if resp.GetRequest().GetEnableDhcpServer() {
		str += "DHCP_SERVER,"
	}
	if resp.GetRequest().GetEnableNetbios() {
		str += "NETBIOS"
	}
	sz := len(str)
	if sz == 0 {
		return "-"
	}
	if sz > 0 && str[sz-1] == ',' {
		str = str[:sz-1]
	}

	return str
}

func ncsiBcastShowResp(resp *halproto.BcastFilterGetResponse) {
	fmt.Printf("%-5d%-20s\n", resp.GetRequest().GetChannel(),
		bcastFltrToStr(resp))
}

func ncsiMcastShowCmdHandler(cmd *cobra.Command, args []string) {

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	client := halproto.NewNcsiClient(c)

	var req *halproto.McastFilterGetRequest
	req = &halproto.McastFilterGetRequest{
		Channel: mFchannelID,
	}

	mcastFilterGetRequestMsg := &halproto.McastFilterGetRequestMsg{
		Request: []*halproto.McastFilterGetRequest{req},
	}

	// HAL call
	respMsg, err := client.McastFilterGet(context.Background(), mcastFilterGetRequestMsg)
	if err != nil {
		fmt.Printf("Getting mcast filters failed. %v\n", err)
		return
	}

	// Print Header
	ncsiMcastShowHeader()

	// Print vrfs
	// m := make(map[uint64]*halproto.VlanFilterGetResponse)
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
			continue
		}
		ncsiMcastShowResp(resp)
		// m[resp.GetSpec().GetKeyOrHandle().GetVrfId()] = resp
	}
	// var keys []uint64
	// for k := range m {
	// 	keys = append(keys, k)
	// }
	// sort.Slice(keys, func(i, j int) bool { return keys[i] < keys[j] })
	// for _, k := range keys {
	// 	vrfShowOneResp(m[k])
	// }
}

func ncsiMcastShowHeader() {
	fmt.Printf("\n")
	fmt.Printf("Ch: Channel		Mcast Filters: IPv6_NA, IPv6_RA, DHCPv6_RELAY, DHCPv6_MCAST, IPv6_MLD, IPv6_NSOL\n")
	hdrLine := strings.Repeat("-", 70)
	fmt.Println(hdrLine)
	fmt.Printf("%-5s%-65s\n", "Ch", "Mcast-Filters")
	fmt.Println(hdrLine)
}

func mcastFltrToStr(resp *halproto.McastFilterGetResponse) string {
	var str string

	if resp.GetRequest().GetEnableIpv6NeighAdv() {
		str += "IPv6_NA,"
	}
	if resp.GetRequest().GetEnableIpv6RouterAdv() {
		str += "IPv6_RA,"
	}
	if resp.GetRequest().GetEnableDhcpv6Relay() {
		str += "DHCPv6_RELAY,"
	}
	if resp.GetRequest().GetEnableDhcpv6Mcast() {
		str += "DHCPv6_MCAST,"
	}
	if resp.GetRequest().GetEnableIpv6Mld() {
		str += "IPv6_MLD,"
	}
	if resp.GetRequest().GetEnableIpv6NeighSol() {
		str += "IPv6_NSOL"
	}
	sz := len(str)
	if sz == 0 {
		return "-"
	}
	if sz > 0 && str[sz-1] == ',' {
		str = str[:sz-1]
	}

	return str
}

func ncsiMcastShowResp(resp *halproto.McastFilterGetResponse) {
	fmt.Printf("%-5d%-20s\n", resp.GetRequest().GetChannel(),
		mcastFltrToStr(resp))
}

func ncsiChannelShowCmdHandler(cmd *cobra.Command, args []string) {

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	client := halproto.NewNcsiClient(c)

	var req *halproto.ChannelGetRequest
	req = &halproto.ChannelGetRequest{
		Channel: channelID,
	}
	// if cmd.Flags().Changed("id") {
	// 	req = &halproto.ChannelGetRequest{
	// 		Channel = channelID,
	// 	}
	// } else {
	// 	req = &halproto.ChannelGetRequest{
	// 		Channel = 0xFF,
	// 	}
	// }

	channelGetRequestMsg := &halproto.ChannelGetRequestMsg{
		Request: []*halproto.ChannelGetRequest{req},
	}

	// HAL call
	respMsg, err := client.ChannelGet(context.Background(), channelGetRequestMsg)
	if err != nil {
		fmt.Printf("Getting channels failed. %v\n", err)
		return
	}

	// Print Header
	ncsiChannelShowHeader()

	// Print vrfs
	// m := make(map[uint64]*halproto.VlanFilterGetResponse)
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
			continue
		}
		ncsiChannelShowResp(resp)
		// m[resp.GetSpec().GetKeyOrHandle().GetVrfId()] = resp
	}
	// var keys []uint64
	// for k := range m {
	// 	keys = append(keys, k)
	// }
	// sort.Slice(keys, func(i, j int) bool { return keys[i] < keys[j] })
	// for _, k := range keys {
	// 	vrfShowOneResp(m[k])
	// }
}

func ncsiChannelShowHeader() {
	fmt.Printf("\n")
	fmt.Printf("Ch: Channel		Channel Modes: TX, RX\n")
	hdrLine := strings.Repeat("-", 15)
	fmt.Println(hdrLine)
	fmt.Printf("%-5s%-10s\n", "Ch", "Channel-Modes")
	fmt.Println(hdrLine)
}

func channelModeToStr(resp *halproto.ChannelGetResponse) string {
	var str string

	if resp.GetRequest().GetTxEnable() {
		str += "TX,"
	}
	if resp.GetRequest().GetRxEnable() {
		str += "RX,"
	}

	sz := len(str)
	if sz == 0 {
		return "-"
	}
	if sz > 0 && str[sz-1] == ',' {
		str = str[:sz-1]
	}

	return str
}

func ncsiChannelShowResp(resp *halproto.ChannelGetResponse) {
	fmt.Printf("%-5d%-10s\n", resp.GetRequest().GetChannel(),
		channelModeToStr(resp))
}
