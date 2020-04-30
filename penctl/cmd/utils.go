//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"fmt"
	"strings"
)

func naplesExecCmdNoPrint(v interface{}) error {
	_, err := restGetWithBody(v, "cmd/v1/naples/")
	if err != nil {
		if verbose {
			fmt.Println(err)
		}
		return err
	}
	return nil
}

func naplesExecCmd(v interface{}) error {
	resp, err := restGetWithBody(v, "cmd/v1/naples/")
	if err != nil {
		if verbose {
			fmt.Println(err)
		}
		return err
	}
	if len(resp) > 3 {
		fmt.Println(strings.Replace(string(resp), `\n`, "\n", -1))
	}
	return nil
}

func stripURLScheme(url string) string {
	if strings.HasPrefix(url, "https://") {
		return strings.TrimPrefix(url, "https://")
	} else if strings.HasPrefix(url, "http://") {
		return strings.TrimPrefix(url, "http://")
	}
	return url
}
