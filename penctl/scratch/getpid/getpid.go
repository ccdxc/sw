package main

import (
	"fmt"
	"io/ioutil"
	"log"
	"os/exec"
	"strings"
)

func main() {
	cmd := exec.Command("ps", "-ef")
	stdout, err := cmd.StdoutPipe()
	if err != nil {
		log.Fatal(err)
	}
	if err := cmd.Start(); err != nil {
		log.Fatal(err)
	}
	slurp, _ := ioutil.ReadAll(stdout)
	if err := cmd.Wait(); err != nil {
		log.Fatal(err)
	}
	slurpSlice := strings.Split(strings.Replace(string(slurp[1:len(slurp)-2]), `\n`, "\n", -1), "\n")
	for _, line := range slurpSlice {
		items := strings.Split(line, " ")
		//		fmt.Printf("%v", items)
		//		for idx, item := range items {
		//			fmt.Printf("%d %s\n", idx, item)
		//		}
		if len(items) < 17 {
			continue
		}
		if items[16] == "nmd" {
			//if items[len(items)-1] == "nmd" {
			fmt.Printf("%s\n", items[17])
		}
	}
}
