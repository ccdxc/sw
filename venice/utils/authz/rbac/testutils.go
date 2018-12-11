package rbac

import (
	"fmt"
	"reflect"
	"sort"

	"github.com/pensando/sw/api/generated/auth"
)

// PrintPerms returns formatted string for permission slice for logging purposes
func PrintPerms(name string, perms []auth.Permission) (message string) {
	message = fmt.Sprintln("-----------" + name + "------------")
	for _, perm := range perms {
		message = message + fmt.Sprintln(perm)
	}
	return
}

func sortPerms(perms []auth.Permission) {
	sort.Slice(perms, func(i, j int) bool {
		return perms[i].String() < perms[j].String()
	})
}

// ArePermsEqual checks equality of permissions using reflect.DeepEqual. It is used for testing.
func ArePermsEqual(expected []auth.Permission, returned []auth.Permission) bool {
	sortPerms(expected)
	sortPerms(returned)
	return reflect.DeepEqual(expected, returned)
}
