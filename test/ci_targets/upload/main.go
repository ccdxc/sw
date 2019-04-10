package main

import (
	"log"
	"os"

	"github.com/pensando/sw/test/utils/infra"
)

func main() {
	// pass the make target, as in "make e2e"
	err := infra.UploadFile(os.Args[1], os.Args[2])
	if err != nil {
		log.Fatal(err)
	}
}
