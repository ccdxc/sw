//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"encoding/json"
	"fmt"
	"io"
	"net/http"
	"os"

	"golang.org/x/net/html"

	"github.com/spf13/cobra"

	"github.com/pensando/sw/nic/agent/nmd/state"
)

var coreRootCmd = &cobra.Command{
	Use:   "core",
	Short: "Cores on Naples",
	Long:  "\n-----------------\n Cores on Naples \n-----------------\n",
}

var coreShowCmd = &cobra.Command{
	Use:   "list",
	Short: "Show cores from Naples",
	Long:  "\n------------------------\n Show Cores From Naples \n------------------------\n",
	Run:   coreShowCmdHandler,
}

var coreFetchCmd = &cobra.Command{
	Use:   "get",
	Short: "Get a core from Naples",
	Long:  "\n------------------------\n Get a Core From Naples \n------------------------\n",
	Run:   coreFetchCmdHandler,
}

var coreDeleteCmd = &cobra.Command{
	Use:   "delete",
	Short: "Delete a core from Naples",
	Long:  "\n---------------------------\n Delete a Core From Naples \n---------------------------\n",
	Run:   coreDeleteCmdHandler,
}

var path string

func init() {
	rootCmd.AddCommand(coreRootCmd)
	coreRootCmd.AddCommand(coreShowCmd)
	coreRootCmd.AddCommand(coreFetchCmd)
	coreRootCmd.AddCommand(coreDeleteCmd)
	coreFetchCmd.Flags().StringVarP(&path, "path", "p", "", "Path to copy files to")
}

func parseCoreFiles(resp *http.Response) {
	defer resp.Body.Close()
	doc, err := html.Parse(resp.Body)
	if err != nil {
		fmt.Println(err)
		return
	}
	// Recursively visit nodes in the parse tree
	var f func(*html.Node)
	f = func(n *html.Node) {
		if n.Type == html.ElementNode && n.Data == "a" {
			for _, a := range n.Attr {
				if a.Key == "href" {
					fmt.Println(a.Val)
					break
				}
			}
		}
		for c := n.FirstChild; c != nil; c = c.NextSibling {
			f(c)
		}
	}
	f(doc)
}

func coreShowCmdHandler(cmd *cobra.Command, args []string) {
	resp, _ := restGetResp(revProxyPort, "cores/v1/naples/")
	parseCoreFiles(resp)
}

func coreFetchCmdHandler(cmd *cobra.Command, args []string) {
	if !cmd.Flags().Changed("path") {
		path = "./"
	}
	for _, corefile := range args {
		resp, err := restGetResp(revProxyPort, "cores/v1/naples/"+corefile)
		if err != nil {
			fmt.Println(err)
			continue
		}
		defer resp.Body.Close()
		corefile = path + "/" + corefile
		out, err := os.Create(corefile)
		if err != nil {
			fmt.Println(err)
			return
		}
		defer out.Close()
		// Write the body to file
		_, err = io.Copy(out, resp.Body)
		if err != nil {
			fmt.Println(err)
			return
		}
	}
}

func coreDeleteCmdHandler(cmd *cobra.Command, args []string) {
	for _, corefile := range args {
		resp, err := restDelete(revProxyPort, "cores/v1/naples/"+corefile)
		if err != nil {
			fmt.Println(err)
			return
		}
		naplesCfg := state.NaplesConfigResp{}
		json.Unmarshal(resp, &naplesCfg)
		if tabularFormat && naplesCfg.ErrorMsg != "" {
			fmt.Println("Error: ", naplesCfg.ErrorMsg)
		}
	}
}
