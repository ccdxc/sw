package main

import (
	"fmt"
	"log"
	"os/exec"
)

func main() {
	out, err := exec.Command("/bin/ls", "-al").Output()
	if err != nil {
		log.Fatal(err)
	}
	fmt.Printf("%s\n", out)
}
