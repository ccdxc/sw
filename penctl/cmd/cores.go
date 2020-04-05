//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"encoding/json"
	"errors"
	"fmt"
	"net/http"
	"strings"

	"github.com/spf13/cobra"
	"golang.org/x/net/html"
)

var coreShowCmd = &cobra.Command{
	Use:   "core-dumps",
	Short: "Show core dumps from Distributed Service Card",
	Long:  "\n-----------------------------\n Show Core Dumps From Distributed Service Card \n-----------------------------\n",
	Args:  cobra.NoArgs,
	RunE:  coreShowCmdHandler,
}

var coreDeleteCmd = &cobra.Command{
	Use:   "core-dump",
	Short: "Delete a core dump from Distributed Service Card",
	Long:  "\n--------------------------------\n Delete a Core Dump From Distributed Service Card \n--------------------------------\n",
	Args:  cobra.NoArgs,
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
	if resp == nil || resp.Body == nil {
		return nil, fmt.Errorf("Invalid HTTP response")
	}
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
	resp, err := restGetResp("cores/v1/naples/")
	if err != nil && !strings.Contains(err.Error(), "not found") {
		return err
	}

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
	resp, err := restDelete("cores/v1/naples/" + corefile)
	if err != nil {
		fmt.Println(err)
		return err
	}
	naplesCfg := NaplesConfigResp{}
	json.Unmarshal(resp, &naplesCfg)
	if naplesCfg.ErrorMsg != "" {
		fmt.Println("Error: ", naplesCfg.ErrorMsg)
		return errors.New("Error: " + naplesCfg.ErrorMsg)
	}
	return nil
}
