//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"fmt"
	"strings"

	"github.com/spf13/cobra"

	nmd "github.com/pensando/sw/nic/agent/nmd/protos"
)

var (
	ifEncap        string
	ifName         string
	ifSubIP        string
	ifOverlayIP    string
	ifMplsIn       string
	ifMplsOut      uint32
	ifTunnelDestIP string
	ifSourceGw     string
	ifGwMac        string
	ifIngressBw    uint32
	ifEgressBw     uint32
)

var ifUpdateCmd = &cobra.Command{
	Use:   "interface",
	Short: "Create interface",
	Long:  "Create interface",
	Run:   ifUpdateCmdHandler,
}

var ifDeleteCmd = &cobra.Command{
	Use:   "interface",
	Short: "Delete interface",
	Long:  "Delete interface",
	Run:   ifDeleteCmdHandler,
}

func init() {
	deleteCmd.AddCommand(ifDeleteCmd)
	ifDeleteCmd.Flags().StringVar(&ifEncap, "encap", "", "Encap type (Ex: MPLSoUDP)")
	ifDeleteCmd.Flags().StringVar(&ifName, "name", "", "Interface name")
	ifDeleteCmd.MarkFlagRequired("encap")
	ifDeleteCmd.MarkFlagRequired("name")

	updateCmd.AddCommand(ifUpdateCmd)

	ifUpdateCmd.Flags().StringVar(&ifEncap, "encap", "", "Encap type (Ex: MPLSoUDP)")
	ifUpdateCmd.Flags().StringVar(&ifName, "name", "", "Interface name")
	ifUpdateCmd.Flags().StringVar(&ifSubIP, "substrate-ip", "", "Substrate IPv4 address")
	ifUpdateCmd.Flags().StringVar(&ifOverlayIP, "overlay-ip", "", "Specify overlay IPv4 address in comma separated list (Max of 2 supported). Ex: 1.2.3.4,2.3.4.5")
	ifUpdateCmd.Flags().StringVar(&ifMplsIn, "mpls-in", "", "Specify incoming MPLS label as comma separated list (Max 2 supported")
	ifUpdateCmd.Flags().Uint32Var(&ifMplsOut, "mpls-out", 0, "Specify outgoing MPLS label")
	ifUpdateCmd.Flags().StringVar(&ifTunnelDestIP, "tunnel-dest-ip", "", "Tunnel destination IPv4 address")
	ifUpdateCmd.Flags().StringVar(&ifSourceGw, "source-gw", "", "Specify source gateway. Must be IPv4 prefix as a.b.c.d/nn")
	ifUpdateCmd.Flags().StringVar(&ifGwMac, "gw-mac", "", "Specify gateway MAC address as aabb.ccdd.eeff")
	ifUpdateCmd.Flags().Uint32Var(&ifIngressBw, "ingress-bw", 0, "Specify ingress bandwidth in Kbytes/sec")
	ifUpdateCmd.Flags().Uint32Var(&ifEgressBw, "egress-bw", 0, "Specify egress bandwidth in Kbytes/sec")

	ifUpdateCmd.MarkFlagRequired("encap")
	ifUpdateCmd.MarkFlagRequired("name")
	ifUpdateCmd.MarkFlagRequired("substrate-ip")
	ifUpdateCmd.MarkFlagRequired("overlay-ip")
	ifUpdateCmd.MarkFlagRequired("mpls-in")
	ifUpdateCmd.MarkFlagRequired("mpls-out")
	ifUpdateCmd.MarkFlagRequired("tunnel-dest-ip")
	ifUpdateCmd.MarkFlagRequired("source-gw")
	ifUpdateCmd.MarkFlagRequired("gw-mac")
	ifUpdateCmd.MarkFlagRequired("ingress-bw")
	ifUpdateCmd.MarkFlagRequired("egress-bw")
}

func ifDeleteCmdHandler(cmd *cobra.Command, args []string) {
	halctlStr := "/nic/bin/halctl debug delete interface --encap " + ifEncap + " --name " + ifName

	execCmd := strings.Fields(halctlStr)
	v := &nmd.NaplesCmdExecute{
		Executable: execCmd[0],
		Opts:       strings.Join(execCmd[1:], " "),
	}

	resp, err := restGetWithBody(v, "cmd/v1/naples/")
	if err != nil {
		fmt.Println(err)
		return
	}

	if len(resp) > 3 {
		s := strings.Replace(string(resp[1:len(resp)-2]), `\n`, "\n", -1)
		fmt.Printf("%s", s)
	}
}

func ifUpdateCmdHandler(cmd *cobra.Command, args []string) {
	halctlStr := "/nic/bin/halctl debug update interface --encap " + ifEncap + " --substrate-ip " + ifSubIP + " --overlay-ip " + ifOverlayIP + " --mpls-in " + ifMplsIn + " --mpls-out " + fmt.Sprint(ifMplsOut) + " --tunnel-dest-ip " + ifTunnelDestIP + " --source-gw " + ifSourceGw + " --gw-mac " + ifGwMac + " --ingress-bw " + fmt.Sprint(ifIngressBw) + " --egress-bw " + fmt.Sprint(ifEgressBw) + " --name " + ifName

	execCmd := strings.Fields(halctlStr)
	v := &nmd.NaplesCmdExecute{
		Executable: execCmd[0],
		Opts:       strings.Join(execCmd[1:], " "),
	}

	resp, err := restGetWithBody(v, "cmd/v1/naples/")
	if err != nil {
		fmt.Println(err)
		return
	}

	if len(resp) > 3 {
		s := strings.Replace(string(resp[1:len(resp)-2]), `\n`, "\n", -1)
		fmt.Printf("%s", s)
	}
}
