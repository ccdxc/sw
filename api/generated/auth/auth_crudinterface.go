// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

package auth

import (
	"context"

	api "github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/kvstore"
)

// Dummy vars to suppress unused imports message
var _ context.Context
var _ api.ObjectMeta
var _ kvstore.Interface

const KindAuthenticationPolicy ObjKind = "AuthenticationPolicy"
const KindRole ObjKind = "Role"
const KindRoleBinding ObjKind = "RoleBinding"
const KindUser ObjKind = "User"
const KindUserPreference ObjKind = "UserPreference"
