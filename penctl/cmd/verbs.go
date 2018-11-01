//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"github.com/spf13/cobra"
)

// setCmd represents the base sub-command verb
var setCmd = &cobra.Command{
	Use:   "set",
	Short: "Config CLIs",
	Long:  "\n--------------------\n Configuration CLIs \n--------------------\n",
}

// putCmd represents the base sub-command verb
var putCmd = &cobra.Command{
	Use:   "put",
	Short: "Put CLIs",
	Long:  "\n----------\n Put CLIs \n----------\n",
}

// deleteCmd represents the base sub-command verb
var deleteCmd = &cobra.Command{
	Use:   "delete",
	Short: "Delete CLIs",
	Long:  "\n-------------\n Delete CLIs \n-------------\n",
}

// getCmd represents the base sub-command verb
var getCmd = &cobra.Command{
	Use:   "get",
	Short: "Get CLIs",
	Long:  "\n----------\n Get CLIs \n----------\n",
}

func init() {
	rootCmd.AddCommand(setCmd)
	rootCmd.AddCommand(putCmd)
	rootCmd.AddCommand(deleteCmd)
	rootCmd.AddCommand(getCmd)
}
