package cmd

import (
	"fmt"

	"github.com/spf13/cobra"

	"github.com/pensando/sw/api/generated/workload"
)

func init() {
	rootCmd.AddCommand(verifyCmd)
	verifyCmd.AddCommand(verifyEndpointsCmd)
	verifyCmd.AddCommand(verifyPingCmd)
}

var verifyCmd = &cobra.Command{
	Use:   "verify",
	Short: "verify actions",
}

var verifyEndpointsCmd = &cobra.Command{
	Use:   "endpoints",
	Short: "verify endpoint distribution",
	Run:   verifyEndpointsAction,
}

var verifyPingCmd = &cobra.Command{
	Use:   "ping",
	Short: "verify ping between endpoints",
	Run:   verifyPingAction,
}

func verifyPingAction(cmd *cobra.Command, args []string) {
	// TBD
}

type epInfo struct {
	local bool
	vlan  int
}

func verifyEndpointsAction(cmd *cobra.Command, args []string) {
	eps, err := setupTb.ListEndpoints("default")
	if err != nil {
		errorExit("could not get endpoints", err)
	}
	epMap := make(map[string]map[string]epInfo)
	veps := make(map[string]*workload.Endpoint)
	for _, ep := range eps {
		veps[ep.Status.MacAddress] = ep
		epMap[ep.Status.MacAddress] = make(map[string]epInfo)
	}
	neps, err := setupModel.Action().GetNaplesEndpoints(setupModel.Naples())
	if err != nil {
		errorExit("getting endpoints from naples", err)
	}
	fmt.Printf("%16s%20s", "Workload", "MAC Address")
	npls := []string{}
	for naples, v := range neps {
		fmt.Printf("\t%10s", naples)
		npls = append(npls, naples)
		for m, v := range v {
			einfo := epInfo{
				local: v.Local,
				vlan:  v.Vlan,
			}
			if _, ok := epMap[m]; !ok {
				continue
			}
			epMap[m][naples] = einfo
		}
	}
	fmt.Printf("\n")
	for k, m := range epMap {
		fmt.Printf("%16s%20s", fmt.Sprintf("%.16s", veps[k].Status.WorkloadName), k)
		for _, n := range npls {
			if e, ok := m[n]; ok {
				p := "R"
				if e.local {
					p = "L"
				}
				fmt.Printf("\t%10s", fmt.Sprintf("%s/%d", p, e.vlan))
			} else {
				fmt.Printf("\t%10s", "--X--")
			}
		}
		fmt.Printf("\n")
	}
}
