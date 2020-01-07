package cmd

import (
	"fmt"

	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/spf13/cobra"
)

func init() {
	rootCmd.AddCommand(showCmd)
	showCmd.AddCommand(showTestbedCmd)
}

var showCmd = &cobra.Command{
	Use:   "show",
	Short: "Show information",
}

var showTestbedCmd = &cobra.Command{
	Use:   "testbed",
	Short: "Show testbed information",
	Run:   showTestBedAction,
}

func showTestBedAction(cmd *cobra.Command, args []string) {

	stressCmd.AddCommand()
	for _, node := range setupTb.Nodes {
		fmt.Printf("\nNode Name        : %v", node.NodeName)
		fmt.Printf("\n\tType        : %v", node.Type.String())
		fmt.Printf("\n\tPersonality : %v", node.Personality.String())
		fmt.Printf("\n\tIP: %v", node.NodeMgmtIP)
		fmt.Printf("\n\tUUID: %v", node.NodeUUID)
		switch node.Personality {
		case iota.PersonalityType_PERSONALITY_VENICE:
			{
				fmt.Printf("\n\tVenice Control IP: %v", node.VeniceConfig.ControlIp)
				fmt.Printf("\n\tVenice Peers: %v", node.VeniceConfig.VenicePeers)
			}
		case iota.PersonalityType_PERSONALITY_NAPLES:
			{
				for _, naplesConfig := range node.NaplesConfigs.Configs {
					fmt.Printf("\n\tVenice IPs%v", naplesConfig.VeniceIps)
				}

			}
		}
	}
	fmt.Printf("\n")
}
