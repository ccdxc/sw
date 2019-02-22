package cmd

import (
	log "github.com/sirupsen/logrus"
	"github.com/spf13/cobra"

	"github.com/pensando/sw/nic/e2etests/go/agent/pkg/cfgpush"
)

var (
	genDir                string
	netagentRESTEndpoints []string
)

var pushCmd = &cobra.Command{
	Use:   "push",
	Short: "push pushes the generated netagent configs",
	Long:  `push pushes the generated netagent configs`,
	RunE: func(cmd *cobra.Command, args []string) error {
		if len(genDir) == 0 || len(netagentRESTEndpoints) == 0 {
			log.Error("Must specify --gen-dir, --urls")
			cmd.Usage()
		}
		p, err := cfgpush.NewPusher(genDir, netagentRESTEndpoints)
		if err != nil {
			log.Errorf("Failed to initialize heimdall config generator. Err: %v", err)
			return err
		}

		if err := p.PushConfigs(); err != nil {
			log.Errorf("Failed to push configs. Err: %v", err)
			return err
		}

		log.Infof("Heimdall successfully pushed objects in %s", genDir)
		return nil
	},
}

func init() {
	pushCmd.Flags().StringVarP(&genDir, "gen-dir", "g", "", "Output directory for the generated files")
	pushCmd.Flags().StringSliceVarP(&netagentRESTEndpoints, "urls", "u", nil, "Naples NetAgent URLs")
}
