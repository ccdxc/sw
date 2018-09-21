//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"github.com/spf13/cobra"
)

// clearCmd represents the show command
var clearCmd = &cobra.Command{
	Use:   "clear",
	Short: "clear commands",
	Long:  `clear commands`,
	// Run: func(cmd *cobra.Command, args []string) {
	// 	fmt.Println("show called")
	// 	cmd.Usage()
	// },
}

func init() {
	rootCmd.AddCommand(clearCmd)

	// Here you will define your flags and configuration settings.

	// Cobra supports Persistent Flags which will work for this command
	// and all subcommands, e.g.:
	// showCmd.PersistentFlags().String("foo", "", "A help for foo")

	// Cobra supports local flags which will only run when this command
	// is called directly, e.g.:
	// showCmd.Flags().BoolP("toggle", "t", false, "Help message for toggle")
}
