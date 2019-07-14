package cmd

import (
	"errors"
	"strings"

	"github.com/spf13/cobra"
)

var (
	nodeNames string
)

func init() {
	rootCmd.AddCommand(naplesCmd)
	naplesCmd.AddCommand(naplesAddCmd)
	naplesCmd.AddCommand(naplesDelCmd)
	naplesAddCmd.Flags().StringVarP(&nodeNames, "names", "", "", "Node names")
	naplesDelCmd.Flags().StringVarP(&nodeNames, "names", "", "", "Node names")
}

var naplesCmd = &cobra.Command{
	Use:   "naples",
	Short: "actions on naples",
}

var naplesAddCmd = &cobra.Command{
	Use:   "add",
	Short: "Add a node with naples personality",
	Run:   naplesAddAction,
}

var naplesDelCmd = &cobra.Command{
	Use:   "delete",
	Short: "Delete a node with naples personality",
	Run:   naplesDeleteAction,
}

func naplesAddAction(cmd *cobra.Command, args []string) {

	if nodeNames == "" {
		errorExit("No node name specified", errors.New("No node name specified"))
	}

	err := setupModel.AddNaplesNodes(strings.Split(nodeNames, ","))

	if err != nil {
		errorExit("Error adding  naples node", err)
	}
}

func naplesDeleteAction(cmd *cobra.Command, args []string) {

	if nodeNames == "" {
		errorExit("No node name specified", errors.New("No node name specified"))
	}

	err := setupModel.DeleteNaplesNodes(strings.Split(nodeNames, ","))

	if err != nil {
		errorExit("Error delete naples node", err)
	}
}

func doNaplesRemoveAdd(percent int) error {

	naples, err := setupModel.Naples().SelectByPercentage(percent)

	if err != nil {
		return err
	}

	return setupModel.Action().RemoveAddNaples(naples)
}

func doNaplesMgmtLinkFlap(percent int) error {

	naples, err := setupModel.Naples().SelectByPercentage(percent)

	if err != nil {
		return err
	}

	return setupModel.Action().FlapMgmtLinkNaples(naples)
}
