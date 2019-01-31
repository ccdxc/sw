//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"fmt"
	"os"
	"strings"

	"github.com/spf13/cobra"
	"github.com/spf13/cobra/doc"

	"github.com/pensando/sw/venice/utils/log"
)

// GitCommit is the variable to hold the sha
var GitCommit string

// PenCtlVer is the variable to hold penctl version
var PenCtlVer string

// BuiltTime is the built time of penctl
var BuiltTime string

func getPenctlVer() string {
	return fmt.Sprintf("Sha:       %s\nVersion:   %s\nBuiltTime: %s\n", GitCommit, PenCtlVer, BuiltTime)
}

func printPenctlVer() {
	fmt.Println(getPenctlVer())
}

// rootCmd represents the base command when called without any subcommands
var rootCmd = &cobra.Command{
	Use:               "penctl",
	Short:             "Pensando CLIs",
	Long:              "\n--------------------------\n Pensando Management CLIs \n--------------------------\n",
	PersistentPreRunE: cliPreRunInit,
	DisableAutoGenTag: true,
	Run: func(cmd *cobra.Command, args []string) {
		if cmd.Flags().Changed("version") {
			printPenctlVer()
		} else {
			fmt.Println(cmd.Help())
		}
	},
}

// Execute adds all child commands to the root command and sets flags appropriately.
// This is called by main.main(). It only needs to happen once to the rootCmd.
func Execute() {
	genDocs()
	if err := rootCmd.Execute(); err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
}

var verbose bool
var version bool
var yamlFormat bool
var jsonFormat bool
var tabularFormat bool
var mockMode bool

func init() {
	// Fill logger config params
	logConfig := &log.Config{
		Module:      "Pensando Control CLI",
		Format:      log.JSONFmt,
		Filter:      log.AllowErrorFilter,
		CtxSelector: log.ContextAll,
		LogToStdout: true,
		LogToFile:   false,
	}
	log.SetConfig(logConfig)

	rootCmd.PersistentFlags().BoolVarP(&verbose, "log", "z", false, "debug output")
	rootCmd.PersistentFlags().BoolVarP(&version, "version", "v", false, "display version of penctl")
	rootCmd.PersistentFlags().BoolVarP(&yamlFormat, "yaml", "y", false, "display in yaml format")
	rootCmd.PersistentFlags().BoolVarP(&jsonFormat, "json", "j", false, "display in json format")
	rootCmd.PersistentFlags().BoolVarP(&tabularFormat, "tabular", "t", true, "display in tabular format")
	rootCmd.PersistentFlags().BoolVarP(&mockMode, "localhost", "l", false, "run penctl in mock mode to localhost")

	rootCmd.PersistentFlags().MarkHidden("localhost")
	rootCmd.PersistentFlags().MarkHidden("log")

	// TODO generate this file only for linux/etc
	// rootCmd.GenBashCompletionFile("penctl.sh")
}

// NewPenctlCommand exports the rootCmd for bash-completion
func NewPenctlCommand() *cobra.Command {
	return rootCmd
}

func genMarkdownDocs() {
	err := doc.GenMarkdownTree(rootCmd, "./docs/markdown/")
	if err != nil && verbose {
		fmt.Println(err)
	}
}

func genRestTreeDocs() {
	err := doc.GenReSTTree(rootCmd, "./docs/rest/")
	if err != nil && verbose {
		fmt.Println(err)
	}
}

func genManTreeDocs() {
	header := &doc.GenManHeader{
		Title:   "MINE",
		Section: "3",
	}
	err := doc.GenManTree(rootCmd, header, "./docs/man/")

	if err != nil && verbose {
		fmt.Println(err)
	}
}

var list []string

func listAppend(cmd *cobra.Command, use string) {
	if cmd == rootCmd {
		if strings.Contains(use, "help") {
			return
		}
		list = append(list, cmd.Use+" "+use)
		return
	}
	listAppend(cmd.Parent(), cmd.Use+" "+use)
}

func genCmdList(cmd *cobra.Command) {
	if list == nil {
		list = make([]string, 0)
	}
	listAppend(cmd, "")
	cmds := cmd.Commands()
	for _, x := range cmds {
		genCmdList(x)
	}
}

func genDocs() {
	genManTreeDocs()
	genMarkdownDocs()
	genRestTreeDocs()
	genCmdList(rootCmd)
	//for _, x := range list {
	//	fmt.Println(x)
	//}
}

func cliPreRunInit(cmd *cobra.Command, args []string) error {
	err := pickNetwork(cmd, args)
	if err != nil {
		return err
	}
	processFlags(cmd, args)
	return nil
}

func processFlags(cmd *cobra.Command, args []string) {
	if cmd.Flags().Changed("yaml") || cmd.Flags().Changed("json") {
		tabularFormat = false
	}
	if verbose {
		fmt.Printf("jsonFormat: %t, yamlFormat: %t, tabularFormat: %t\n", jsonFormat, yamlFormat, tabularFormat)
	}
}
