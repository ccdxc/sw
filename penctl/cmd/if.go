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

var ifCreateCmd = &cobra.Command{
	Use:   "interface",
	Short: "Create interface",
	Long:  "Create interface",
	Run:   ifCreateCmdHandler,
}

func init() {
	createCmd.AddCommand(ifCreateCmd)

	ifCreateCmd.Flags().StringVar(&ifEncap, "encap", "", "Encap type (Ex: MPLSoUDP)")
	ifCreateCmd.Flags().StringVar(&ifName, "name", "", "Interface name")
	ifCreateCmd.Flags().StringVar(&ifSubIP, "substrate-ip", "", "Substrate IPv4 address")
	ifCreateCmd.Flags().StringVar(&ifOverlayIP, "overlay-ip", "", "Specify overlay IPv4 address in comma separated list (Max of 2 supported). Ex: 1.2.3.4,2.3.4.5")
	ifCreateCmd.Flags().StringVar(&ifMplsIn, "mpls-in", "", "Specify incoming MPLS label as comma separated list (Max 2 supported")
	ifCreateCmd.Flags().Uint32Var(&ifMplsOut, "mpls-out", 0, "Specify outgoing MPLS label")
	ifCreateCmd.Flags().StringVar(&ifTunnelDestIP, "tunnel-dest-ip", "", "Tunnel destination IPv4 address")
	ifCreateCmd.Flags().StringVar(&ifSourceGw, "source-gw", "", "Specify source gateway. Must be IPv4 prefix as a.b.c.d/nn")
	ifCreateCmd.Flags().StringVar(&ifGwMac, "gw-mac", "", "Specify gateway MAC address as aabb.ccdd.eeff")
	ifCreateCmd.Flags().Uint32Var(&ifIngressBw, "ingress-bw", 0, "Specify ingress bandwidth in Kbytes/sec")
	ifCreateCmd.Flags().Uint32Var(&ifEgressBw, "egress-bw", 0, "Specify egress bandwidth in Kbytes/sec")

	ifCreateCmd.MarkFlagRequired("encap")
	ifCreateCmd.MarkFlagRequired("name")
	ifCreateCmd.MarkFlagRequired("substrate-ip")
	ifCreateCmd.MarkFlagRequired("overlay-ip")
	ifCreateCmd.MarkFlagRequired("mpls-in")
	ifCreateCmd.MarkFlagRequired("mpls-out")
	ifCreateCmd.MarkFlagRequired("tunnel-dest-ip")
	ifCreateCmd.MarkFlagRequired("source-gw")
	ifCreateCmd.MarkFlagRequired("gw-mac")
	ifCreateCmd.MarkFlagRequired("ingress-bw")
	ifCreateCmd.MarkFlagRequired("egress-bw")
}

func ifCreateCmdHandler(cmd *cobra.Command, args []string) {
	halctlStr := "/nic/bin/halctl debug create interface --encap " + ifEncap + " --substrate-ip " + ifSubIP + " --overlay-ip " + ifOverlayIP + " --mpls-in " + ifMplsIn + " --mpls-out " + fmt.Sprint(ifMplsOut) + " --tunnel-dest-ip " + ifTunnelDestIP + " --source-gw " + ifSourceGw + " --gw-mac " + ifGwMac + " --ingress-bw " + fmt.Sprint(ifIngressBw) + " --egress-bw " + fmt.Sprint(ifEgressBw) + " --name " + ifName

	execCmd := strings.Fields(halctlStr)
	v := &nmd.NaplesCmdExecute{
		Executable: execCmd[0],
		Opts:       strings.Join(execCmd[1:], " "),
	}

	resp, err := restGetWithBody(v, revProxyPort, "cmd/v1/naples/")
	if err != nil {
		fmt.Println(err)
		return
	}

	if len(resp) > 3 {
		s := strings.Replace(string(resp[1:len(resp)-2]), `\n`, "\n", -1)
		fmt.Printf("%s", s)
	}
}
