package cmd

import (
	"github.com/spf13/cobra"

	"github.com/pensando/sw/iota/test/venice/iotakit"
	"github.com/pensando/sw/venice/utils/log"
)

func init() {
	rootCmd.AddCommand(switchCmd)
	switchCmd.AddCommand(switchFlapCmd)
}

var switchCmd = &cobra.Command{
	Use:   "switch",
	Short: "actions on hosts",
}

var switchFlapCmd = &cobra.Command{
	Use:   "flap",
	Short: "flap data switch ports",
	Run:   switchFlapAction,
}

func flapPorts(percent int) error {

	sc, err := setupModel.SwitchPorts().SelectByPercentage(percent)

	if err != nil {
		return err
	}
	return setupModel.Action().FlapDataSwitchPorts(sc, 10)
}

func switchFlapAction(cmd *cobra.Command, args []string) {
	var sc *iotakit.SwitchPortCollection
	var err error
	sc = setupModel.SwitchPorts()

	log.Info("Switch Ports will  be flapped")

	err = setupModel.Action().FlapDataSwitchPorts(sc, 10)

	if err != nil {
		errorExit("Flapping ports failed", err)
	}

}
