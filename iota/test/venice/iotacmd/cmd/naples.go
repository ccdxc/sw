package cmd

import (
	"errors"

	"github.com/spf13/cobra"
)

var (
	nodeName string
)

func init() {
	rootCmd.AddCommand(naplesCmd)
	naplesCmd.AddCommand(naplesAddCmd)
	naplesCmd.AddCommand(naplesDelCmd)
	naplesAddCmd.Flags().StringVarP(&nodeName, "name", "", "", "Node name")
	naplesDelCmd.Flags().StringVarP(&nodeName, "name", "", "", "Node name")
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

	if nodeName == "" {
		errorExit("No node name specified", errors.New("No node name specified"))
	}

	err := setupModel.AddNaplesNode(nodeName)

	if err != nil {
		errorExit("Error adding  naples node", err)
	}
}

func naplesDeleteAction(cmd *cobra.Command, args []string) {

	if nodeName == "" {
		errorExit("No node name specified", errors.New("No node name specified"))
	}

	err := setupModel.DeleteNaplesNode(nodeName)

	if err != nil {
		errorExit("Error delete naples node", err)
	}
}
