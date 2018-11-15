//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"encoding/json"
	"errors"
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
	RunE:  coreShowCmdHandler,
}

var coreFetchCmd = &cobra.Command{
	Use:   "get",
	Short: "Get a core from Naples",
	Long:  "\n------------------------\n Get a Core From Naples \n------------------------\n",
	RunE:  coreFetchCmdHandler,
}

var coreDeleteCmd = &cobra.Command{
	Use:   "delete",
	Short: "Delete a core from Naples",
	Long:  "\n---------------------------\n Delete a Core From Naples \n---------------------------\n",
	RunE:  coreDeleteCmdHandler,
}

var path string
var file string

func init() {
	rootCmd.AddCommand(coreRootCmd)
	coreRootCmd.AddCommand(coreShowCmd)
	coreRootCmd.AddCommand(coreFetchCmd)
	coreRootCmd.AddCommand(coreDeleteCmd)
	coreFetchCmd.Flags().StringVarP(&path, "path", "p", "", "Path to copy files to")
	coreFetchCmd.Flags().StringVarP(&file, "file", "f", "", "Core file to copy")
	coreFetchCmd.MarkFlagRequired("file")
	coreDeleteCmd.Flags().StringVarP(&file, "file", "f", "", "Core file to delete")
	coreDeleteCmd.MarkFlagRequired("file")
}

var retSlice []string

func parseFiles(resp *http.Response) ([]string, error) {
	defer resp.Body.Close()
	doc, err := html.Parse(resp.Body)
	if err != nil {
		fmt.Println(err)
		return nil, err
	}
	// Recursively visit nodes in the parse tree
	var f func(*html.Node) ([]string, error)
	f = func(n *html.Node) ([]string, error) {
		if n.Type == html.ElementNode && n.Data == "a" {
			for _, a := range n.Attr {
				if a.Key == "href" {
					retSlice = append(retSlice, a.Val)
					break
				}
			}
		}
		for c := n.FirstChild; c != nil; c = c.NextSibling {
			f(c)
		}
		return retSlice, nil
	}
	return f(doc)
}

func coreShowCmdHandler(cmd *cobra.Command, args []string) error {
	resp, _ := restGetResp(revProxyPort, "cores/v1/naples/")
	retS, err := parseFiles(resp)
	if err != nil {
		return err
	}
	for _, ret := range retS {
		fmt.Println(ret)
	}
	return nil
}

func coreFetchCmdHandler(cmd *cobra.Command, args []string) error {
	if !cmd.Flags().Changed("path") {
		path = "./"
	}
	if _, err := os.Stat(path); os.IsNotExist(err) {
		// path/to/whatever does not exist
		os.MkdirAll(path, os.ModePerm)
	}

	corefile := file
	resp, err := restGetResp(revProxyPort, "cores/v1/naples/"+corefile)
	if err != nil {
		fmt.Println(err)
		return err
	}
	defer resp.Body.Close()
	corefile = path + "/" + corefile
	out, err := os.Create(corefile)
	if err != nil {
		fmt.Println(err)
		return err
	}
	defer out.Close()
	// Write the body to file
	_, err = io.Copy(out, resp.Body)
	if err != nil {
		fmt.Println(err)
		return err
	}
	return nil
}

func coreDeleteCmdHandler(cmd *cobra.Command, args []string) error {
	corefile := file
	resp, err := restDelete(revProxyPort, "cores/v1/naples/"+corefile)
	if err != nil {
		fmt.Println(err)
		return err
	}
	naplesCfg := state.NaplesConfigResp{}
	json.Unmarshal(resp, &naplesCfg)
	if tabularFormat && naplesCfg.ErrorMsg != "" {
		fmt.Println("Error: ", naplesCfg.ErrorMsg)
		return errors.New("Error: " + naplesCfg.ErrorMsg)
	}
	return nil
}
