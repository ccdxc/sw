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
	nwsecProfID   uint32
	nwsecVrfID    uint64
	nwsecPolicyID uint64
)

var nwsecShowCmd = &cobra.Command{
	Use:   "nwsec",
	Short: "show Security information",
	Long:  "show Security object information",
}

var nwsecProfShowCmd = &cobra.Command{
	Use:   "profile",
	Short: "show Security profile information",
	Long:  "show Security profile object information",
	Run:   nwsecProfShowCmdHandler,
}

var nwsecPolicyShowCmd = &cobra.Command{
	Use:   "policy",
	Short: "show Security policy information",
	Long:  "show Security policy object information",
	Run:   nwsecPolicyShowCmdHandler,
}

var nwsecFlowGateShowCmd = &cobra.Command{
	Use:   "flow-gate",
	Short: "show Security flow-gate information",
	Long:  "show Security flow-gate object information",
	Run:   nwsecFlowGateShowCmdHandler,
}

func init() {
	showCmd.AddCommand(nwsecShowCmd)
	nwsecShowCmd.AddCommand(nwsecProfShowCmd)
	nwsecShowCmd.AddCommand(nwsecPolicyShowCmd)
	nwsecShowCmd.AddCommand(nwsecFlowGateShowCmd)

	nwsecProfShowCmd.Flags().Uint32Var(&nwsecProfID, "id", 1, "Specify security profile ID")
	nwsecPolicyShowCmd.Flags().Uint64Var(&nwsecVrfID, "vrf-id", 1, "Specify VRF ID")
	nwsecPolicyShowCmd.Flags().Uint64Var(&nwsecPolicyID, "policy-id", 1, "Specify security policy ID")
}

func handleNwsecProfShowCmd(cmd *cobra.Command, ofile *os.File) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	client := halproto.NewNwSecurityClient(c.ClientConn)

	var req *halproto.SecurityProfileGetRequest
	if cmd != nil && cmd.Flags().Changed("id") {
		req = &halproto.SecurityProfileGetRequest{
			KeyOrHandle: &halproto.SecurityProfileKeyHandle{
				KeyOrHandle: &halproto.SecurityProfileKeyHandle_ProfileId{
					ProfileId: nwsecProfID,
				},
			},
		}
	} else {
		// Get all Security Profiles
		req = &halproto.SecurityProfileGetRequest{}
	}
	secProfGetReqMsg := &halproto.SecurityProfileGetRequestMsg{
		Request: []*halproto.SecurityProfileGetRequest{req},
	}

	// HAL call
	respMsg, err := client.SecurityProfileGet(context.Background(), secProfGetReqMsg)
	if err != nil {
		fmt.Printf("Getting Security Profile failed. %v\n", err)
		return
	}

	// Print Security Profiles
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("HAL Returned non OK status. %v\n", resp.ApiStatus)
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

func nwsecProfShowCmdHandler(cmd *cobra.Command, args []string) {
	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}
	handleNwsecProfShowCmd(cmd, nil)
}

func handleNwsecPolicyShowCmd(cmd *cobra.Command, ofile *os.File) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	client := halproto.NewNwSecurityClient(c.ClientConn)

	var req *halproto.SecurityPolicyGetRequest
	if cmd != nil && cmd.Flags().Changed("vrf-id") && cmd.Flags().Changed("policy-id") {
		req = &halproto.SecurityPolicyGetRequest{
			KeyOrHandle: &halproto.SecurityPolicyKeyHandle{
				PolicyKeyOrHandle: &halproto.SecurityPolicyKeyHandle_SecurityPolicyKey{
					SecurityPolicyKey: &halproto.SecurityPolicyKey{
						SecurityPolicyId: nwsecPolicyID,
						VrfIdOrHandle: &halproto.VrfKeyHandle{
							KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
								VrfId: nwsecVrfID,
							},
						},
					},
				},
			},
		}
	} else {
		// Get all Security Policies
		req = &halproto.SecurityPolicyGetRequest{}
	}

	secPolicyGetReqMsg := &halproto.SecurityPolicyGetRequestMsg{
		Request: []*halproto.SecurityPolicyGetRequest{req},
	}

	// HAL call
	respMsg, err := client.SecurityPolicyGet(context.Background(), secPolicyGetReqMsg)
	if err != nil {
		fmt.Printf("Getting Security Policy failed. %v\n", err)
		return
	}

	// Print Security Policy
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("HAL Returned non OK status. %v\n", resp.ApiStatus)
			fmt.Println("Policy Show not ok")
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

func nwsecPolicyShowCmdHandler(cmd *cobra.Command, args []string) {
	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}
	handleNwsecPolicyShowCmd(cmd, nil)
}

func nwsecFlowGateShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	client := halproto.NewNwSecurityClient(c.ClientConn)

	var req *halproto.SecurityFlowGateGetRequest

	req = &halproto.SecurityFlowGateGetRequest{}
	secProfGetReqMsg := &halproto.SecurityFlowGateGetRequestMsg{
		Request: []*halproto.SecurityFlowGateGetRequest{req},
	}

	// HAL call
	respMsg, err := client.SecurityFlowGateGet(context.Background(), secProfGetReqMsg)
	if err != nil {
		fmt.Printf("Getting Security Flow Gates failed. %v\n", err)
		return
	}

	nwsecFlowGateShowHeader()

	// Print Security Profiles
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("HAL Returned non OK status. %v\n", resp.ApiStatus)
			continue
		}
		nwsecFlowGateShowOneResp(resp)
	}
}

func nwsecFlowGateShowHeader() {
	hdrLine := strings.Repeat("-", 130)
	fmt.Printf("Legend:\n")
	fmt.Printf("Dir: Direction: U (Uplink), H (Host)\n")
	fmt.Printf("KeyType: Flow Key Type\n")
	fmt.Printf("VrfID: Source VRF ID/Destination VRF ID\n")
	fmt.Printf("SMAC|SIP[:sport]: Source MAC Address | Source IP Address and Port Number\n")
	fmt.Printf("DMAC|DIP[:dport]: Destination MAC Address | Destination IP Address and Port Number\n")
	fmt.Printf("Proto: L4 Protocol\n")
	fmt.Printf("ALG: ALG that opened flow gate\n")
	fmt.Printf("State: MD (Marked for deletion)\t Ref Count: Number of active sessions based on the gate\n")
	fmt.Printf("Time To Age: Inactivity time remaining for flow to age in seconds (IDF- No aging indefinite time)\n")
	fmt.Println(hdrLine)
	fmt.Printf("%-6s%-8s%-10s%-24s%-24s%-16s%-10s%-16s%-10s%-10s\n",
		"Dir", "KeyType", "VrfID",
		"SIP[:sport]", "DIP[:dport]", "Proto", "ALG", "State",
		"RefCount", "Time To Age")
	fmt.Println(hdrLine)
}

func nwsecFlowGateShowOneResp(resp *halproto.SecurityFlowGateGetResponse) {
	flowGatekey := resp.GetFlowGateKey()
	flowStr := strings.Replace(flowGatekey.GetDirection().String(), "FLOW_DIRECTION_FROM_", "", -1)
	flowStr = flowStr[0:1]

	keyType := "IPv4"
	src := Uint32IPAddrToStr(flowGatekey.GetSrcIp().GetV4Addr())
	dst := Uint32IPAddrToStr(flowGatekey.GetDstIp().GetV4Addr())
	src += ":["
	src += strconv.Itoa(int(flowGatekey.GetSrcPort()))
	src += "]"
	dst += ":["
	dst += strconv.Itoa(int(flowGatekey.GetDstPort()))
	dst += "]"
	proto := strings.Replace(flowGatekey.GetIpProto().String(), "IPPROTO_", "", -1)
	svrf := flowGatekey.GetSrcVrfId()
	dvrf := flowGatekey.GetDstVrfId()
	vrfStr := strconv.Itoa(int(svrf)) + "/" + strconv.Itoa(int(dvrf))
	mdStr := ""
	if resp.GetDeleteMarked() == true {
		mdStr = "MD"
	}
	algStr := strings.Replace(resp.GetAlg().String(), "APP_SVC_", "", -1)
	timeToAge := resp.GetTimeToAge()
	timeToAgeStr := ""
	if timeToAge == 0xFFFFFFFF {
		timeToAgeStr = "IDF"
	} else {
		if timeToAge > 59 {
			timeToAgeStr += strconv.Itoa(int(timeToAge/60)) + "m "
		}
		timeToAgeStr += strconv.Itoa(int(timeToAge%60)) + "s"
	}
	refCountstr := strconv.Itoa(int(resp.GetRefCount()))
	fmt.Printf("%-6s%-8s%-10s%-24s%-24s%-16s%-10s%-16s%-10s%-10s\n",
		flowStr, keyType, vrfStr, src, dst, proto, algStr,
		mdStr, refCountstr, timeToAgeStr)
}
