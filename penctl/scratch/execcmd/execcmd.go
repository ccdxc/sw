package main

import (
	"fmt"
	"io/ioutil"
	"log"
	"os/exec"
)

func main() {
	cmd := exec.Command("/bin/ls", "-al", "/sw/nic/")
	stdout, err := cmd.StdoutPipe()
	if err != nil {
		log.Fatal(err)
	}
	if err := cmd.Start(); err != nil {
		log.Fatal(err)
	}
	slurp, _ := ioutil.ReadAll(stdout)
	fmt.Printf("%s\n", slurp)
	if err := cmd.Wait(); err != nil {
		log.Fatal(err)
	}
}
