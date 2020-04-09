//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"errors"
	"fmt"
	"net"
	"time"
)

var revProxyPort string
var naplesURL string
var naplesIP string

func getNaplesURL() (string, error) {
	if verbose {
		fmt.Printf("naplesURL: %s\n", naplesURL)
	}
	if naplesURL != "" {
		return naplesURL, nil
	}
	return "", errors.New("Could not figure out dsc URL")
}

func isNaplesReachable() error {
	seconds := 5
	timeOut := time.Duration(seconds) * time.Second
	_, err := net.DialTimeout("tcp", naplesIP+":"+revProxyPort, timeOut)

	if err != nil {
		fmt.Printf("Could not reach Distributed Service Card on %s\n", naplesIP+":"+revProxyPort)
		return err
	}
	return nil
}
