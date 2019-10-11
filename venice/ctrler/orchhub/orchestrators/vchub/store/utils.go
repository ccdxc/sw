package store

import (
	"fmt"
)

func createGlobalKey(vcID, objName string) string {
	// TODO: Add in datacenter name
	return fmt.Sprintf("%s-%s", vcID, objName)
}
