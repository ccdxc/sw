//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"encoding/json"
	"errors"
	"fmt"
	"net/http"

	"golang.org/x/net/html"

	"github.com/spf13/cobra"

	"github.com/pensando/sw/nic/agent/nmd/state"
)

var coreShowCmd = &cobra.Command{
	Use:   "cores",
	Short: "Show cores from Naples",
	Long:  "\n------------------------\n Show Cores From Naples \n------------------------\n",
	RunE:  coreShowCmdHandler,
}

var coreDeleteCmd = &cobra.Command{
	Use:   "core",
	Short: "Delete a core from Naples",
	Long:  "\n---------------------------\n Delete a Core From Naples \n---------------------------\n",
	RunE:  coreDeleteCmdHandler,
}

var path string
var file string

func init() {
	listCmd.AddCommand(coreShowCmd)
	deleteCmd.AddCommand(coreDeleteCmd)

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
	if len(retS) == 0 {
		fmt.Println("No core files found")
	}
	for _, ret := range retS {
		fmt.Println(ret)
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
