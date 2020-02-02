package cmd

import (
	"context"
	"time"

	"github.com/spf13/cobra"

	"github.com/pensando/sw/iota/test/venice/iotakit/model/objects"
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
	return setupModel.FlapDataSwitchPorts(sc, 10*time.Second)
}

func switchFlapAction(cmd *cobra.Command, args []string) {
	var sc *objects.SwitchPortCollection
	var err error
	sc = setupModel.SwitchPorts()

	log.Info("Switch Ports will  be flapped")

	err = setupModel.FlapDataSwitchPorts(sc, 10)

	if err != nil {
		errorExit("Flapping ports failed", err)
	}

}

var (
	flapPortCancel context.CancelFunc
)

func startFlapPortsPeriodically(rate int, flapCount int, percent int) error {

	stopFlapPortsPeriodically()

	sc, err := setupModel.SwitchPorts().SelectByPercentage(percent)

	if err != nil {
		return err
	}
	ctx, cancel := context.WithCancel(context.Background())
	flapPortCancel = cancel
	go setupModel.FlapDataSwitchPortsPeriodically(ctx, sc,
		10*time.Second, time.Duration(rate)*time.Second, flapCount)

	return nil

}

func stopFlapPortsPeriodically() error {
	if flapPortCancel != nil {
		flapPortCancel()
		flapPortCancel = nil
	}
	return nil
}
