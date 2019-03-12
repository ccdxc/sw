//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"fmt"
	"strings"
)

func naplesExecCmd(v interface{}) error {
	resp, err := restGetWithBody(v, "cmd/v1/naples/")
	if err != nil {
		fmt.Println(err)
		return err
	}
	if len(resp) > 3 {
		fmt.Println(strings.Replace(string(resp[0:len(resp)-2]), `\n`, "\n", -1))
	}
	return nil
}
