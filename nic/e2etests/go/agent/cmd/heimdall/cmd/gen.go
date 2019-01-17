package cmd

import (
	log "github.com/sirupsen/logrus"
	"github.com/spf13/cobra"

	"github.com/pensando/sw/nic/e2etests/go/agent/pkg/cfggen"
)

var (
	manifestFile, templateFile, outDir string
	nodeUUIDs                          []string
)

var genCmd = &cobra.Command{
	Use:   "gen",
	Short: "gen generates netagent configs",
	Long:  `gen accepts the object manifest file, generates agent objects`,
	RunE: func(cmd *cobra.Command, args []string) error {
		if len(manifestFile) == 0 || len(outDir) == 0 || len(nodeUUIDs) == 0 {
			log.Error("Must specify --manifest-file, --output-dir and --node-uuid")
			cmd.Usage()
		}
		c, err := cfggen.NewGenerator(manifestFile, templateFile, nodeUUIDs)
		if err != nil {
			log.Errorf("Failed to initialize heimdall config generator. Err: %v", err)
			return err
		}

		// Generate objects
		if err := c.GenerateNamespaces(); err != nil {
			log.Errorf("Failed to generate namespaces. Err: %v", err)
			return err
		}

		if err := c.GenerateNetworks(); err != nil {
			log.Errorf("Failed to generate networks. Err: %v", err)
			return err
		}

		if err := c.GenerateEndpoints(); err != nil {
			log.Errorf("Failed to generate endpoints. Err: %v", err)
			return err
		}

		if err := c.GenerateALGs(); err != nil {
			log.Errorf("Failed to generate ALGs ")
		}

		if err := c.GenerateSecurityProfiles(); err != nil {
			log.Errorf("Failed to generate  security profiles. Err: %v", err)
			return err
		}

		if err := c.GenerateFirewallPolicies(); err != nil {
			log.Errorf("Failed to generate firewall policies. Err: %v", err)
			return err
		}

		if err := c.GenerateFlowMonitorRules(); err != nil {
			log.Errorf("Failed to generate flow monitor rules. Err: %v", err)
			return err
		}

		// Persist the generated JSONs.
		if err := c.WriteJSON(outDir); err != nil {
			log.Errorf("Failed to write generated JSONs. Err: %v", err)
			return err
		}
		log.Infof("Heimdall successfully generated objects in %s", outDir)
		return nil
	},
}

func init() {
	genCmd.Flags().StringVarP(&manifestFile, "manifest-file", "f", "", "Object config manifest file")
	genCmd.Flags().StringVarP(&templateFile, "template-file", "t", "", "Optional template file to be used in object specs")
	genCmd.Flags().StringVarP(&outDir, "output-dir", "o", "", "Output directory for the generated files")
	genCmd.Flags().StringSliceVarP(&nodeUUIDs, "node-uuid", "u", nil, "Naples Node UUID for the configs")
}
