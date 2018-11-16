//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"github.com/spf13/cobra"
)

// updateCmd represents the base sub-command verb
var updateCmd = &cobra.Command{
	Use:   "update",
	Short: "Update Object",
	Long:  "\n---------------\n Update Object \n---------------\n",
}

// deleteCmd represents the base sub-command verb
var deleteCmd = &cobra.Command{
	Use:   "delete",
	Short: "Delete Object",
	Long:  "\n---------------\n Delete Object \n---------------\n",
}

// showCmd represents the base sub-command verb
var showCmd = &cobra.Command{
	Use:   "show",
	Short: "Show Object and Information",
	Long:  "\n-----------------------------\n Show Object and Information \n-----------------------------\n",
}

// sysCmd represents the base sub-command verb
var sysCmd = &cobra.Command{
	Use:   "system",
	Short: "System Operations",
	Long:  "\n-------------------\n System Operations \n-------------------\n",
}

// listCmd represents the base sub-command verb
var listCmd = &cobra.Command{
	Use:   "list",
	Short: "List Objects",
	Long:  "\n-------------\n List Objects \n-------------\n",
}

func init() {
	rootCmd.AddCommand(updateCmd)
	rootCmd.AddCommand(deleteCmd)
	rootCmd.AddCommand(showCmd)
	rootCmd.AddCommand(sysCmd)
	rootCmd.AddCommand(listCmd)
}
