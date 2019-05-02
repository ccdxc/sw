//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	"os"

	"github.com/spf13/cobra"

	"github.com/pensando/sw/nic/agent/cmd/halctl/utils"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
)

var (
	isL2Key       bool
	isL3Key       bool
	flvrfID       uint64
	srcIP         string
	dstIP         string
	srcPort       uint32
	dstPort       uint32
	ipProto       uint32
	icmpType      uint32
	icmpCode      uint32
	icmpID        uint32
	srcMac        string
	dstMac        string
	spi           uint32
	etherType     uint32
	l2Segid       uint32
	flowDirection uint32
	flowInstance  uint32
	hwVrf         uint64
)

var flowHashCmd = &cobra.Command{
	Use:    "flow-hash",
	Short:  "show flow-hash information",
	Long:   "show flow-hash table information",
	Hidden: true,
	Run:    flowHashCmdHandler,
}

func init() {
	showCmd.AddCommand(flowHashCmd)

	flowHashCmd.Flags().BoolVar(&isL2Key, "isL2Key", false, "Specify if the key is L2")
	flowHashCmd.Flags().BoolVar(&isL3Key, "isL3Key", false, "Specify if the key is L3")
	flowHashCmd.Flags().Uint64Var(&flvrfID, "vrfid", 0, "Specify vrf-id (default is 0)")
	flowHashCmd.Flags().Uint64Var(&hwVrf, "hwvrf", 0, "Specify hardware vrf (default is 0)")
	flowHashCmd.Flags().StringVar(&srcIP, "srcip", "0.0.0.0", "Specify session src ip (x.x.x.x)")
	flowHashCmd.Flags().StringVar(&dstIP, "dstip", "0.0.0.0", "Specify session dst ip (y.y.y.y)")
	flowHashCmd.Flags().Uint32Var(&srcPort, "srcport", 0, "Specify session src port")
	flowHashCmd.Flags().Uint32Var(&dstPort, "dstport", 0, "Specify session dst port")
	flowHashCmd.Flags().Uint32Var(&ipProto, "ipproto", 0, "Specify session IP proto (1: ICMP, 6: TCP, 17: UDP)")
	flowHashCmd.Flags().Uint32Var(&icmpType, "icmptype", 0, "Specify icmp type")
	flowHashCmd.Flags().Uint32Var(&icmpCode, "icmpcode", 0, "Specify icmp code")
	flowHashCmd.Flags().StringVar(&srcMac, "srcmac", "00:00:00:00:00:00", "Specify ethernet smac (xx:xx:xx:xx:xx:xx)")
	flowHashCmd.Flags().StringVar(&dstMac, "dstmac", "00:00:00:00:00:00", "Specify ethernet dmac (xx:xx:xx:xx:xx:xx)")
	flowHashCmd.Flags().Uint32Var(&spi, "spi", 0, "Specify ESP SPI")
	flowHashCmd.Flags().Uint32Var(&etherType, "ethertype", 0, "Specify Ethernet type")
	flowHashCmd.Flags().Uint32Var(&l2Segid, "l2segid", 0, "Specify L2 Segment ID")
	flowHashCmd.Flags().Uint32Var(&flowDirection, "dir", 0, "Specify flow direction 1 (from host) 2 (from uplink)")
	flowHashCmd.Flags().Uint32Var(&flowInstance, "lkpinst", 0, "Specify flow instance 0/1")
}

func flowHashCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	defer c.Close()

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	var req *halproto.FlowHashGetRequest
	var reqMsg *halproto.FlowHashGetRequestMsg

	fmt.Printf("Src IP: %s Dst IP: %s src: %d dst: %d\n", srcIP, dstIP, utils.IPAddrStrtoUint32(srcIP), utils.IPAddrStrtoUint32(dstIP))
	if cmd.Flags().Changed("isL3Key") {
		if halproto.IPProtocol(ipProto) == halproto.IPProtocol_IPPROTO_UDP ||
			halproto.IPProtocol(ipProto) == halproto.IPProtocol_IPPROTO_TCP {
			req = &halproto.FlowHashGetRequest{
				HardwareVrfId: hwVrf,
				FlowKey: &halproto.FlowKey{
					SrcVrfId: flvrfID,
					DstVrfId: flvrfID,
					FlowKey: &halproto.FlowKey_V4Key{
						V4Key: &halproto.FlowKeyV4{
							Sip:     utils.IPAddrStrtoUint32(srcIP),
							Dip:     utils.IPAddrStrtoUint32(dstIP),
							IpProto: halproto.IPProtocol(ipProto),
							L4Fields: &halproto.FlowKeyV4_TcpUdp{
								TcpUdp: &halproto.FlowKeyTcpUdpInfo{
									Sport: srcPort,
									Dport: dstPort,
								},
							},
						},
					},
				},
				FlowDirection: halproto.FlowDirection(flowDirection),
				FlowInstance:  halproto.FlowInstance(flowInstance),
			}
		} else if halproto.IPProtocol(ipProto) == halproto.IPProtocol_IPPROTO_ICMP {
			req = &halproto.FlowHashGetRequest{
				HardwareVrfId: hwVrf,
				FlowKey: &halproto.FlowKey{
					SrcVrfId: flvrfID,
					DstVrfId: flvrfID,
					FlowKey: &halproto.FlowKey_V4Key{
						V4Key: &halproto.FlowKeyV4{
							Sip:     utils.IPAddrStrtoUint32(srcIP),
							Dip:     utils.IPAddrStrtoUint32(dstIP),
							IpProto: halproto.IPProtocol(ipProto),
							L4Fields: &halproto.FlowKeyV4_Icmp{
								Icmp: &halproto.FlowKeyICMPInfo{
									Type: icmpType,
									Code: icmpCode,
									Id:   icmpID,
								},
							},
						},
					},
				},
				FlowDirection: halproto.FlowDirection(flowDirection),
				FlowInstance:  halproto.FlowInstance(flowInstance),
			}
		} else if halproto.IPProtocol(ipProto) == halproto.IPProtocol_IPPROTO_ESP {
			req = &halproto.FlowHashGetRequest{
				HardwareVrfId: hwVrf,
				FlowKey: &halproto.FlowKey{
					SrcVrfId: flvrfID,
					DstVrfId: flvrfID,
					FlowKey: &halproto.FlowKey_V4Key{
						V4Key: &halproto.FlowKeyV4{
							Sip:     utils.IPAddrStrtoUint32(srcIP),
							Dip:     utils.IPAddrStrtoUint32(dstIP),
							IpProto: halproto.IPProtocol(ipProto),
							L4Fields: &halproto.FlowKeyV4_Esp{
								Esp: &halproto.FlowKeyESPInfo{
									Spi: spi,
								},
							},
						},
					},
				},
				FlowDirection: halproto.FlowDirection(flowDirection),
				FlowInstance:  halproto.FlowInstance(flowInstance),
			}
		} else {
			fmt.Printf("IP Protocol %d not found\n", ipProto)
			return
		}
	} else if cmd.Flags().Changed("isL2Key") {
		req = &halproto.FlowHashGetRequest{
			HardwareVrfId: hwVrf,
			FlowKey: &halproto.FlowKey{
				SrcVrfId: flvrfID,
				DstVrfId: flvrfID,
				FlowKey: &halproto.FlowKey_L2Key{
					L2Key: &halproto.FlowKeyL2{
						Smac:        utils.MacStrtoUint64(srcMac),
						Dmac:        utils.MacStrtoUint64(dstMac),
						EtherType:   etherType,
						L2SegmentId: l2Segid,
					},
				},
			},
			FlowDirection: halproto.FlowDirection(flowDirection),
			FlowInstance:  halproto.FlowInstance(flowInstance),
		}
	} else {
		fmt.Printf("Argument required. Specify L2 or L3 key using '--isL2Key ... or --isL3Key ...' flag\n")
		return
	}

	reqMsg = &halproto.FlowHashGetRequestMsg{
		Request: []*halproto.FlowHashGetRequest{req},
	}

	// HAL call
	client := halproto.NewInternalClient(c)

	// HAL call
	respMsg, err := client.FlowHashGet(context.Background(), reqMsg)
	if err != nil {
		fmt.Printf("Getting System Stats failed. %v\n", err)
		return
	}

	// Print Result
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("Operation failed with %v error\n", resp.GetApiStatus())
			return
		}

		fmt.Printf("Primary Index valid=%t\n", resp.GetPrimaryIndexValid())
		fmt.Printf("Primary Index=0x%x\n", resp.GetPrimaryIndex())
		fmt.Printf("Secondary Index valid=%t\n", resp.GetSecondaryIndexValid())
		fmt.Printf("Secondary Index=0x%x\n", resp.GetSecondaryIndex())
		fmt.Printf("Key=%s\n", resp.GetKey())
		fmt.Printf("Data=%s\n", resp.GetData())
	}
}
