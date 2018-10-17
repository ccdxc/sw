package main

import (
	"bytes"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"net/http"
	"strings"

	"github.com/pensando/sw/nic/agent/nmd/protos"
)

func main() {
	v := &nmd.NaplesCmdExecute{
		Executable: "/bin/ls",
		Opts:       "-al /sw/nic /sw/nic/upgrade_manager",
	}
	payloadBytes, err := json.Marshal(v)
	if err != nil {
		panic(err)
	}
	body := bytes.NewReader(payloadBytes)
	req, err := http.NewRequest("GET", "http://localhost:8888/cmd/v1/naples/", body)
	if err != nil {
		// handle err
	}
	resp, err := http.DefaultClient.Do(req)
	if err != nil {
		// handle err
		panic(err)
	}
	defer resp.Body.Close()
	bodyBytes, _ := ioutil.ReadAll(resp.Body)

	s := strings.Replace(string(bodyBytes[1:len(bodyBytes)-2]), `\n`, "\n", -1)
	fmt.Printf("%s", s)
}
