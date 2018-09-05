//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"bytes"
	"encoding/json"
	"fmt"
	"net/http"
)

func restPost(v interface{}, url string) error {
	payloadBytes, err := json.Marshal(v)
	if err != nil {
		panic(err)
	}
	if verbose {
		var prettyJSON bytes.Buffer
		error := json.Indent(&prettyJSON, payloadBytes, "", "\t")
		if error != nil {
			return error
		}

		fmt.Println("Json output:", string(prettyJSON.Bytes()))
	}
	body := bytes.NewReader(payloadBytes)

	postReq, err := http.NewRequest("POST", url, body)
	if err != nil {
		return err
	}
	postReq.Header.Set("Content-Type", "application/json")
	postResp, err := http.DefaultClient.Do(postReq)
	if err != nil {
		return err
	}
	defer postResp.Body.Close()
	if verbose {
		fmt.Println("Successfully posted the request")
		fmt.Println("Status:", postResp.Status)
		fmt.Println("StatusCode: ", postResp.StatusCode)
	}
	return nil
}
