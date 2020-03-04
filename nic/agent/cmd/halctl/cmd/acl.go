//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	"os"
	"reflect"
	"strconv"
	"strings"

	"github.com/spf13/cobra"
	yaml "gopkg.in/yaml.v2"

	"github.com/pensando/sw/nic/agent/cmd/halctl/utils"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
)

var (
	aclID           uint32
	aclHandle       uint64
	aclStatusID     uint32
	aclStatusHandle uint64
	aclDetailID     uint32
	aclDetailHandle uint64
)

var aclShowCmd = &cobra.Command{
	Use:   "acl",
	Short: "show acl information",
	Long:  "show acl object information",
	Run:   aclShowCmdHandler,
}

var aclShowSpecCmd = &cobra.Command{
	Use:   "spec",
	Short: "show acl spec information",
	Long:  "show acl object spec information",
	Run:   aclShowCmdHandler,
}

var aclShowStatusCmd = &cobra.Command{
	Use:   "status",
	Short: "show acl status information",
	Long:  "show acl object status information",
	Run:   aclShowStatusCmdHandler,
}

func init() {
	showCmd.AddCommand(aclShowCmd)
	aclShowCmd.AddCommand(aclShowSpecCmd)
	aclShowCmd.AddCommand(aclShowStatusCmd)
	aclShowCmd.Flags().Bool("yaml", true, "Output in yaml")

	aclShowCmd.Flags().Uint32Var(&aclID, "id", 0, "Specify acl ID")
	aclShowCmd.Flags().Uint64Var(&aclHandle, "handle", 0, "Specify acl handle")
	aclShowSpecCmd.Flags().Uint32Var(&aclID, "id", 0, "Specify acl ID")
	aclShowSpecCmd.Flags().Uint64Var(&aclHandle, "handle", 0, "Specify acl handle")
	aclShowStatusCmd.Flags().Uint32Var(&aclStatusID, "id", 0, "Specify acl ID")
	aclShowStatusCmd.Flags().Uint64Var(&aclStatusHandle, "handle", 0, "Specify acl handle")
}

func handleACLShowCmd(cmd *cobra.Command, ofile *os.File) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	defer c.Close()

	client := halproto.NewAclClient(c)

	var req *halproto.AclGetRequest
	if cmd != nil && cmd.Flags().Changed("handle") {
		// Get specific acl
		req = &halproto.AclGetRequest{
			KeyOrHandle: &halproto.AclKeyHandle{
				KeyOrHandle: &halproto.AclKeyHandle_AclHandle{
					AclHandle: aclHandle,
				},
			},
		}
	} else if cmd != nil && cmd.Flags().Changed("id") {
		// Get specific acl
		req = &halproto.AclGetRequest{
			KeyOrHandle: &halproto.AclKeyHandle{
				KeyOrHandle: &halproto.AclKeyHandle_AclId{
					AclId: aclID,
				},
			},
		}
	} else {
		// Get all acls
		req = &halproto.AclGetRequest{}
	}

	aclGetReqMsg := &halproto.AclGetRequestMsg{
		Request: []*halproto.AclGetRequest{req},
	}

	// HAL call
	respMsg, err := client.AclGet(context.Background(), aclGetReqMsg)
	if err != nil {
		fmt.Printf("Getting acl failed. %v\n", err)
		return
	}

	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
			continue
		}
		respType := reflect.ValueOf(resp)
		b, _ := yaml.Marshal(respType.Interface())
		if ofile != nil {
			if _, err := ofile.WriteString(string(b) + "\n"); err != nil {
				fmt.Printf("Failed to write to file %s, err : %v\n",
					ofile.Name(), err)
			}
		} else {
			fmt.Println(string(b) + "\n")
			fmt.Println("---")
		}
	}
}

func aclShowCmdHandler(cmd *cobra.Command, args []string) {
	if cmd.Flags().Changed("yaml") {
		if len(args) > 0 {
			fmt.Printf("Invalid argument\n")
			return
		}
		handleACLShowCmd(cmd, nil)
		return
	}

	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	defer c.Close()

	client := halproto.NewAclClient(c)

	if len(args) > 0 {
		if strings.Compare(args[0], "spec") != 0 {
			fmt.Printf("Invalid argument\n")
			return
		}
	}

	var req *halproto.AclGetRequest
	if cmd != nil && cmd.Flags().Changed("handle") {
		// Get specific acl
		req = &halproto.AclGetRequest{
			KeyOrHandle: &halproto.AclKeyHandle{
				KeyOrHandle: &halproto.AclKeyHandle_AclHandle{
					AclHandle: aclHandle,
				},
			},
		}
	} else if cmd != nil && cmd.Flags().Changed("id") {
		// Get specific acl
		req = &halproto.AclGetRequest{
			KeyOrHandle: &halproto.AclKeyHandle{
				KeyOrHandle: &halproto.AclKeyHandle_AclId{
					AclId: aclID,
				},
			},
		}
	} else {
		// Get all acls
		req = &halproto.AclGetRequest{}
	}

	aclGetReqMsg := &halproto.AclGetRequestMsg{
		Request: []*halproto.AclGetRequest{req},
	}

	// HAL call
	respMsg, err := client.AclGet(context.Background(), aclGetReqMsg)
	if err != nil {
		fmt.Printf("Getting acl failed. %v\n", err)
		return
	}

	// Print eth acl rules
	aclShowSpecEthHeader()
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
			continue
		}
		aclShowSpecOneResp(resp, true, false, false, false, false)
	}

	// Print ip acl rules
	aclShowSpecIPHeader()
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
			continue
		}
		aclShowSpecOneResp(resp, false, true, false, false, false)
	}

	// Print tcp acl rules
	aclShowSpecTCPHeader()
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
			continue
		}
		aclShowSpecOneResp(resp, false, false, true, false, false)
	}

	// Print udp acl rules
	aclShowSpecUDPHeader()
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
			continue
		}
		aclShowSpecOneResp(resp, false, false, false, true, false)
	}

	// Print icmp acl rules
	aclShowSpecIcmpHeader()
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
			continue
		}
		aclShowSpecOneResp(resp, false, false, false, false, true)
	}
}

func aclShowSpecEthHeader() {
	hdrLine := strings.Repeat("-", 137)
	fmt.Printf("Eth ACL Entries\n")
	fmt.Println(hdrLine)
	fmt.Printf("%-6s%-6s%-5s%-6s%-6s%-15s%-36s%-36s%-5s%-8s%-6s\n",
		"AclID", "L2Seg", "Prio", "SrcIf", "DstIf",
		"EthType/Mask", "SrcMAC/Mask", "DstMAC/Mask",
		"Copp", "RedirIf", "Action")
	fmt.Println(hdrLine)
}

func aclShowSpecIPHeader() {
	hdrLine := strings.Repeat("-", 104)
	fmt.Printf("\nIP ACL Entries\n")
	fmt.Println(hdrLine)
	fmt.Printf("%-6s%-4s%-5s%-6s%-6s%-24s%-24s%-8s%-5s%-8s%-8s\n",
		"AclID", "Vrf", "Prio", "SrcIf", "DstIf",
		"SrcIPPrefix", "DstIPPrefix", "IPProto",
		"Copp", "RedirIf", "Action")
	fmt.Println(hdrLine)
}

func aclShowSpecUDPHeader() {
	hdrLine := strings.Repeat("-", 124)
	fmt.Printf("\nUDP ACL Entries\n")
	fmt.Println(hdrLine)
	fmt.Printf("%-6s%-4s%-5s%-6s%-6s%-24s%-24s%-14s%-14s%-5s%-8s%-8s\n",
		"AclID", "Vrf", "Prio", "SrcIf", "DstIf",
		"SrcIPPrefix", "DstIPPrefix", "SrcPortRange",
		"DstPortRange", "Copp", "RedirIf", "Action")
	fmt.Println(hdrLine)
}

func aclShowSpecTCPHeader() {
	hdrLine := strings.Repeat("-", 136)
	fmt.Printf("\nTCP ACL Entries\n")
	fmt.Println(hdrLine)
	fmt.Printf("%-6s%-4s%-5s%-6s%-6s%-24s%-24s%-14s%-14s%-12s%-5s%-8s%-8s\n",
		"AclID", "Vrf", "Prio", "SrcIf", "DstIf",
		"SrcIPPrefix", "DstIPPrefix", "SrcPortRange",
		"DstPortRange", "Flags", "Copp", "RedirIf", "Action")
	fmt.Println(hdrLine)
}

func aclShowSpecIcmpHeader() {
	hdrLine := strings.Repeat("-", 126)
	fmt.Printf("\nICMP ACL Entries\n")
	fmt.Println(hdrLine)
	fmt.Printf("%-6s%-4s%-5s%-6s%-6s%-24s%-24s%-15s%-15s%-5s%-8s%-8s\n",
		"AclID", "Vrf", "Prio", "SrcIf", "DstIf",
		"SrcIPPrefix", "DstIPPrefix", "IcmpCode/Mask",
		"IcmpType/Mask", "Copp", "RedirIf", "Action")
	fmt.Println(hdrLine)
}

func aclShowSpecOneResp(resp *halproto.AclGetResponse,
	eth bool, ip bool, tcp bool, udp bool, icmp bool) {
	spec := resp.GetSpec()
	match := spec.GetMatch()
	action := spec.GetAction()

	actionStr := strings.ToLower(strings.Replace(action.GetAction().String(), "ACL_ACTION_", "", -1))
	coppHandleStr := "-"
	if action.GetCoppKeyHandle().GetCoppHandle() != 0 {
		coppHandleStr = strconv.Itoa(int(action.GetCoppKeyHandle().GetCoppHandle()))
	}

	redirectIfIDStr := "-"
	srcIfIDStr := "*"
	dstIfIDStr := "*"

	// Convert all ifIDs to Strings
	var ifID []uint64
	ifIndex := 0
	if action.GetRedirectIfKeyHandle().GetInterfaceId() != 0 {
		ifID = append(ifID, action.GetRedirectIfKeyHandle().GetInterfaceId())
	}
	if match.GetSrcIfKeyHandle().GetInterfaceId() != 0 {
		ifID = append(ifID, match.GetSrcIfKeyHandle().GetInterfaceId())
	}
	if match.GetDstIfKeyHandle().GetInterfaceId() != 0 {
		ifID = append(ifID, match.GetDstIfKeyHandle().GetInterfaceId())
	}
	if len(ifID) > 0 {
		var ifIDStr []string
		ret, ifIDStr := ifGetStrFromID(ifID)
		if ret == 0 {
			ifIndex = 0
			if action.GetRedirectIfKeyHandle().GetInterfaceId() != 0 {
				redirectIfIDStr = ifIDStr[ifIndex]
				ifIndex++
			}
			if match.GetSrcIfKeyHandle().GetInterfaceId() != 0 {
				srcIfIDStr = ifIDStr[ifIndex]
				ifIndex++
			}
			if match.GetDstIfKeyHandle().GetInterfaceId() != 0 {
				dstIfIDStr = ifIDStr[ifIndex]
				ifIndex++
			}
		} else {
			if action.GetRedirectIfKeyHandle().GetInterfaceId() != 0 {
				redirectIfIDStr = strconv.Itoa(int(action.GetRedirectIfKeyHandle().GetInterfaceId()))
			}
			if match.GetSrcIfKeyHandle().GetInterfaceId() != 0 {
				srcIfIDStr = strconv.Itoa(int(match.GetSrcIfKeyHandle().GetInterfaceId()))
			}
			if match.GetDstIfKeyHandle().GetInterfaceId() != 0 {
				dstIfIDStr = strconv.Itoa(int(match.GetDstIfKeyHandle().GetInterfaceId()))
			}
		}
	}
	vrfIDStr := "*"
	l2SegIDStr := "*"

	aclID := spec.GetKeyOrHandle().GetAclId()
	prio := spec.GetPriority()

	switch match.GetSegSelector().(type) {
	case *halproto.AclSelector_VrfKeyHandle:
		if match.GetVrfKeyHandle().GetVrfId() != 0 {
			vrfIDStr = strconv.Itoa(int(match.GetVrfKeyHandle().GetVrfId()))
		}
	case *halproto.AclSelector_L2SegmentKeyHandle:
		if match.GetL2SegmentKeyHandle().GetSegmentId() != 0 {
			l2SegIDStr = strconv.Itoa(int(match.GetL2SegmentKeyHandle().GetSegmentId()))
		}
	}

	switch match.GetPktSelector().(type) {
	case *halproto.AclSelector_EthSelector:
		if eth != true {
			return
		}
		ethSelector := match.GetEthSelector()
		ethTypeStr := "*"
		srcMacStr := "*"
		dstMacStr := "*"
		if ethSelector.GetEthTypeMask() != 0 {
			ethTypeStr = fmt.Sprintf("%d/%#x", ethSelector.GetEthType(), ethSelector.GetEthTypeMask())
		}
		if ethSelector.GetSrcMacMask() != 0 {
			srcMacStr = fmt.Sprintf("%s/%#x", utils.MactoStr(ethSelector.GetSrcMac()), ethSelector.GetSrcMacMask())
		}
		if ethSelector.GetDstMacMask() != 0 {
			dstMacStr = fmt.Sprintf("%s/%#x", utils.MactoStr(ethSelector.GetDstMac()), ethSelector.GetDstMacMask())
		}
		fmt.Printf("%-6d%-6s%-5d%-6s%-6s%-15s%-36s%-36s%-5s%-8s%-8s\n",
			aclID, l2SegIDStr, prio, srcIfIDStr, dstIfIDStr,
			ethTypeStr, srcMacStr, dstMacStr,
			coppHandleStr, redirectIfIDStr, actionStr)
	case *halproto.AclSelector_IpSelector:
		ipSelector := match.GetIpSelector()
		srcIPPrefix := "*"
		dstIPPrefix := "*"
		if ipSelector.GetSrcPrefix().GetPrefixLen() != 0 {
			srcIPPrefix = fmt.Sprintf("%s/%d",
				utils.IPAddrToStr(ipSelector.GetSrcPrefix().GetAddress()),
				ipSelector.GetSrcPrefix().GetPrefixLen())
		}
		if ipSelector.GetDstPrefix().GetPrefixLen() != 0 {
			dstIPPrefix = fmt.Sprintf("%s/%d",
				utils.IPAddrToStr(ipSelector.GetDstPrefix().GetAddress()),
				ipSelector.GetDstPrefix().GetPrefixLen())
		}

		switch ipSelector.GetL4Selectors().(type) {
		case *halproto.IPSelector_IpProtocol:
			if ip != true {
				return
			}
			fmt.Printf("%-6d%-4s%-5d%-6s%-6s%-24s%-24s%-8s%-5s%-8s%-8s\n",
				aclID, vrfIDStr, prio, srcIfIDStr, dstIfIDStr,
				srcIPPrefix, dstIPPrefix,
				ipSelector.GetIpProtocol(),
				coppHandleStr, redirectIfIDStr, actionStr)
		case *halproto.IPSelector_IcmpSelector:
			if icmp != true {
				return
			}
			icmpSelector := ipSelector.GetIcmpSelector()
			icmpCodeStr := fmt.Sprintf("%d/%#x",
				icmpSelector.GetIcmpCode(),
				icmpSelector.GetIcmpCodeMask())
			icmpTypeStr := fmt.Sprintf("%d/%#x",
				icmpSelector.GetIcmpType(),
				icmpSelector.GetIcmpTypeMask())
			fmt.Printf("%-6d%-4s%-5d%-6s%-6s%-24s%-24s%-15s%-15s%-5s%-8s%-8s\n",
				aclID, vrfIDStr, prio, srcIfIDStr, dstIfIDStr,
				srcIPPrefix, dstIPPrefix,
				icmpCodeStr, icmpTypeStr,
				coppHandleStr, redirectIfIDStr, actionStr)
		case *halproto.IPSelector_UdpSelector:
			if udp != true {
				return
			}
			udpSelector := ipSelector.GetUdpSelector()

			srcPortRange := "*"
			dstPortRange := "*"
			if udpSelector.GetSrcPortRange().GetPortHigh() != 0 {
				srcPortRange = utils.L4PortRangeToStr(udpSelector.GetSrcPortRange())
			}
			if udpSelector.GetDstPortRange().GetPortHigh() != 0 {
				dstPortRange = utils.L4PortRangeToStr(udpSelector.GetDstPortRange())
			}

			fmt.Printf("%-6d%-4s%-5d%-6s%-6s%-24s%-24s%-14s%-14s%-5s%-8s%-8s\n",
				aclID, vrfIDStr, prio, srcIfIDStr, dstIfIDStr,
				srcIPPrefix, dstIPPrefix,
				srcPortRange, dstPortRange,
				coppHandleStr, redirectIfIDStr, actionStr)
		case *halproto.IPSelector_TcpSelector:
			if tcp != true {
				return
			}
			tcpSelector := ipSelector.GetTcpSelector()

			srcPortRange := "*"
			dstPortRange := "*"
			if tcpSelector.GetSrcPortRange().GetPortHigh() != 0 {
				srcPortRange = utils.L4PortRangeToStr(tcpSelector.GetSrcPortRange())
			}
			if tcpSelector.GetDstPortRange().GetPortHigh() != 0 {
				dstPortRange = utils.L4PortRangeToStr(tcpSelector.GetDstPortRange())
			}

			flagStr := ""
			switch tcpSelector.GetTcpSyn().(type) {
			case *halproto.TCPSelector_TcpSynSet:
				flagStr = "S"
			case *halproto.TCPSelector_TcpSynClear:
				flagStr = "!S"
			}
			switch tcpSelector.GetTcpAck().(type) {
			case *halproto.TCPSelector_TcpAckSet:
				flagStr += "A"
			case *halproto.TCPSelector_TcpAckClear:
				flagStr += "!A"
			}
			switch tcpSelector.GetTcpFin().(type) {
			case *halproto.TCPSelector_TcpFinSet:
				flagStr += "F"
			case *halproto.TCPSelector_TcpFinClear:
				flagStr += "!F"
			}
			switch tcpSelector.GetTcpRst().(type) {
			case *halproto.TCPSelector_TcpRstSet:
				flagStr += "R"
			case *halproto.TCPSelector_TcpRstClear:
				flagStr += "!R"
			}
			switch tcpSelector.GetTcpUrg().(type) {
			case *halproto.TCPSelector_TcpUrgSet:
				flagStr += "U"
			case *halproto.TCPSelector_TcpUrgClear:
				flagStr += "!U"
			}
			if len(flagStr) == 0 {
				flagStr += "*"
			}

			fmt.Printf("%-6d%-4s%-5d%-6s%-6s%-24s%-24s%-14s%-14s%-12s%-5s%-8s%-8s\n",
				aclID, vrfIDStr, prio, srcIfIDStr, dstIfIDStr,
				srcIPPrefix, dstIPPrefix,
				srcPortRange, dstPortRange, flagStr,
				coppHandleStr, redirectIfIDStr, actionStr)
		}
	}
}

func aclShowStatusCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	defer c.Close()

	client := halproto.NewAclClient(c)

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	var req *halproto.AclGetRequest
	if cmd != nil && cmd.Flags().Changed("handle") {
		// Get specific acl
		req = &halproto.AclGetRequest{
			KeyOrHandle: &halproto.AclKeyHandle{
				KeyOrHandle: &halproto.AclKeyHandle_AclHandle{
					AclHandle: aclStatusHandle,
				},
			},
		}
	} else if cmd != nil && cmd.Flags().Changed("id") {
		// Get specific acl
		req = &halproto.AclGetRequest{
			KeyOrHandle: &halproto.AclKeyHandle{
				KeyOrHandle: &halproto.AclKeyHandle_AclId{
					AclId: aclStatusID,
				},
			},
		}
	} else {
		// Get all acls
		req = &halproto.AclGetRequest{}
	}

	aclGetReqMsg := &halproto.AclGetRequestMsg{
		Request: []*halproto.AclGetRequest{req},
	}

	// HAL call
	respMsg, err := client.AclGet(context.Background(), aclGetReqMsg)
	if err != nil {
		fmt.Printf("Getting acl failed. %v\n", err)
		return
	}

	// Print Header
	aclShowStatusHeader()

	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
			continue
		}
		aclShowStatusOneResp(resp)
	}
}

func aclShowStatusHeader() {
	hdrLine := strings.Repeat("-", 40)
	fmt.Println(hdrLine)
	fmt.Printf("%-10s%-10s%-11s%-10s\n", "Handle", "TCAMIdx", "StatsIdx", "Packets")
	fmt.Println(hdrLine)
}

func aclShowStatusOneResp(resp *halproto.AclGetResponse) {
	status := resp.GetStatus()
	fmt.Printf("%-10d%-10d%-11d%-10d\n",
		status.GetAclHandle(),
		status.GetEpdStatus().GetHwTcamIdx(),
		status.GetEpdStatus().GetHwStatsIdx(),
		resp.GetStats().GetNumPackets())
}
