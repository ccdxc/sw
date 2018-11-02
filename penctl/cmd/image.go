//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"fmt"
	"io"
	//	"net/url"
	"os"
	"strings"

	"github.com/spf13/cobra"
)

var uploadImageCmd = &cobra.Command{
	Use:   "firmware",
	Short: "Copy firmware image to Naples",
	Long:  "\n-------------------------------\n Copy Firmware Image To Naples \n-------------------------------\n",
	Run:   uploadImageCmdHandler,
}

var uploadFile string

func init() {
	putCmd.AddCommand(uploadImageCmd)
	uploadImageCmd.Flags().StringVarP(&uploadFile, "file", "f", "", "Firmware file location/name")
	uploadImageCmd.MarkFlagRequired("file")
}

func uploadImageCmdHandler(cmd *cobra.Command, args []string) {
	//prepare the reader instances to encode
	values := map[string]io.Reader{
		"uploadFile": mustOpen(uploadFile),
		"uploadPath": strings.NewReader("/upload/"),
	}
	resp, err := restPostForm(revProxyPort, "upload/", values)
	if verbose {
		fmt.Println(err.Error())
	}
	fmt.Println(string(resp))
}

func mustOpen(f string) *os.File {
	r, err := os.Open(f)
	if err != nil {
		panic(err)
	}
	return r
}
