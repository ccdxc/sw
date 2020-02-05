package http

import (
	"net/http"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/utils/authz"
)

// PermsFromRequest gets permissions from http request
func PermsFromRequest(request *http.Request) ([]auth.Permission, bool, error) {
	return authz.PermsFromMap(request.Header, true)
}

// UserMetaFromRequest gets user meta from http request
func UserMetaFromRequest(request *http.Request) (*api.ObjectMeta, bool) {
	return authz.UserMetaFromMap(request.Header)
}

// AddUserToRequest adds username and user tenant to http request
func AddUserToRequest(request *http.Request, user *auth.User) error {
	return authz.PopulateMapWithUserPerms(request.Header, user, false, nil, true)
}

// AddUserPermsToRequest adds username, user tenant and user permissions to http request. GoLang http server limits headers size to 1 MB by default.
// This can be overridden by setting Server.MaxHeaderBytes.
func AddUserPermsToRequest(request *http.Request, user *auth.User, isAdmin bool, perms []auth.Permission) error {
	return authz.PopulateMapWithUserPerms(request.Header, user, isAdmin, perms, true)
}

// RemoveUserPermsFromRequest removes username, user tenant and user permissions from http request
func RemoveUserPermsFromRequest(request *http.Request) {
	authz.RemoveUserPerms(request.Header)
	return
}
