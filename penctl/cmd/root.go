//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"crypto/tls"
	"fmt"
	"io/ioutil"
	"net/http"
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
	v := strings.Split(PenCtlVer, "-")
	if len(v) > 1 {
		PenCtlVer = strings.Join(v[:len(v)-1], "-")
	}
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
	SilenceUsage:      true,
	Args:              cobra.NoArgs,
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
		os.Exit(1)
	}
}

var verbose bool
var version bool
var yamlFormat bool
var jsonFormat bool
var mockMode bool
var authTokenFile string

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

	rootCmd.PersistentFlags().BoolVarP(&verbose, "verbose", "", false, "display penctl debug log")
	rootCmd.PersistentFlags().BoolVarP(&version, "version", "v", false, "display version of penctl")
	rootCmd.PersistentFlags().BoolVarP(&yamlFormat, "yaml", "y", false, "display in yaml format")
	rootCmd.PersistentFlags().BoolVarP(&jsonFormat, "json", "j", true, "display in json format")
	rootCmd.PersistentFlags().BoolVarP(&mockMode, "localhost", "l", false, "run penctl in mock managedBy to localhost")
	rootCmd.PersistentFlags().StringVarP(&authTokenFile, "authtoken", "a", "", "path to file containing authorization token")

	rootCmd.PersistentFlags().MarkHidden("localhost")

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
	if cmd.Flags().Changed("version") {
		return nil
	}
	err := pickNetwork(cmd, args)
	if err != nil {
		return err
	}

	processFlags(cmd, args)

	//err = setSystemTimeCmdHandler(cmd, args)
	//if err != nil {
	//	return err
	//}

	if authTokenFile != "" {
		pemBlocks, err := ioutil.ReadFile(authTokenFile)
		if err != nil {
			return fmt.Errorf("Error opening authorization token file %s: %v", authTokenFile, err)
		}
		cert, err := tls.X509KeyPair(pemBlocks, pemBlocks)
		if err != nil {
			return fmt.Errorf("Error parsing X509 key pair: %v", err)
		}
		penHTTPClient.Transport = &http.Transport{
			TLSClientConfig: &tls.Config{
				InsecureSkipVerify: true, // do not check agent's certificate
				Certificates:       []tls.Certificate{cert},
			},
			MaxIdleConnsPerHost: 5,
		}
		naplesURL = "https://" + strings.TrimPrefix(naplesURL, "http://")
	}

	return nil
}

func processFlags(cmd *cobra.Command, args []string) {
	if verbose {
		fmt.Printf("jsonFormat: %t, yamlFormat: %t\n", jsonFormat, yamlFormat)
	}
}
