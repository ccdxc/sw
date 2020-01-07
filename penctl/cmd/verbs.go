//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"github.com/spf13/cobra"
)

// createCmd represents the base sub-command verb
var createCmd = &cobra.Command{
	Use:   "create",
	Short: "Create Object",
	Long:  "\n---------------\n Create Object \n---------------\n",
	Args:  cobra.NoArgs,
}

// updateCmd represents the base sub-command verb
var updateCmd = &cobra.Command{
	Use:   "update",
	Short: "Update Object",
	Long:  "\n---------------\n Update Object \n---------------\n",
	Args:  cobra.NoArgs,
}

// deleteCmd represents the base sub-command verb
var deleteCmd = &cobra.Command{
	Use:   "delete",
	Short: "Delete Object",
	Long:  "\n---------------\n Delete Object \n---------------\n",
	Args:  cobra.NoArgs,
}

// showCmd represents the base sub-command verb
var showCmd = &cobra.Command{
	Use:   "show",
	Short: "Show Object and Information",
	Long:  "\n-----------------------------\n Show Object and Information \n-----------------------------\n",
	Args:  cobra.NoArgs,
}

// sysCmd represents the base sub-command verb
var sysCmd = &cobra.Command{
	Use:   "system",
	Short: "System Operations",
	Long:  "\n-------------------\n System Operations \n-------------------\n",
	Args:  cobra.NoArgs,
}

// listCmd represents the base sub-command verb
var listCmd = &cobra.Command{
	Use:   "list",
	Short: "List Objects",
	Long:  "\n-------------\n List Objects \n-------------\n",
	Args:  cobra.NoArgs,
}

func init() {
	rootCmd.AddCommand(createCmd)
	rootCmd.AddCommand(updateCmd)
	rootCmd.AddCommand(deleteCmd)
	rootCmd.AddCommand(showCmd)
	rootCmd.AddCommand(sysCmd)
	rootCmd.AddCommand(listCmd)
}
