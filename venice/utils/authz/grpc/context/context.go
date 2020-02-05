package context

import (
	"context"

	"google.golang.org/grpc/metadata"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/utils/authz"
)

// NewOutgoingContextWithUserPerms creates a new context with user and permissions metadata to send to grpc backend.
func NewOutgoingContextWithUserPerms(ctx context.Context, user *auth.User, isAdmin bool, perms []auth.Permission) (context.Context, error) {
	md, ok := metadata.FromOutgoingContext(ctx)
	if !ok {
		md = metadata.MD{}
	} else {
		md = md.Copy()
	}
	// set user info in md
	if err := populateMetadataWithUserPerms(md, user, isAdmin, perms); err != nil {
		return ctx, err
	}
	nctx := metadata.NewOutgoingContext(ctx, md)
	return nctx, nil
}

// NewIncomingContextWithUserPerms creates a new context with user and permissions metadata to send to grpc backend. This is for testing only.
func NewIncomingContextWithUserPerms(ctx context.Context, user *auth.User, isAdmin bool, perms []auth.Permission) (context.Context, error) {
	md, ok := metadata.FromIncomingContext(ctx)
	if !ok {
		md = metadata.MD{}
	} else {
		md = md.Copy()
	}
	// set user info in md
	if err := populateMetadataWithUserPerms(md, user, isAdmin, perms); err != nil {
		return ctx, err
	}
	return metadata.NewIncomingContext(ctx, md), nil
}

// PermsFromOutgoingContext returns user permissions from grpc metadata in outgoing context
func PermsFromOutgoingContext(ctx context.Context) ([]auth.Permission, bool, error) {
	md, ok := metadata.FromOutgoingContext(ctx)
	if !ok {
		return nil, ok, nil
	}
	return permsFromMD(md)
}

// PermsFromIncomingContext returns user permissions from grpc metadata in incoming context
func PermsFromIncomingContext(ctx context.Context) ([]auth.Permission, bool, error) {
	md, ok := metadata.FromIncomingContext(ctx)
	if !ok {
		return nil, ok, nil
	}
	return permsFromMD(md)
}

func permsFromMD(md metadata.MD) ([]auth.Permission, bool, error) {
	return authz.PermsFromMap(md, false)
}

// UserIsAdminFromOutgoingContext returns user roles from grpc metadata in outgoing context
func UserIsAdminFromOutgoingContext(ctx context.Context) (bool, bool) {
	md, ok := metadata.FromOutgoingContext(ctx)
	if !ok {
		return false, ok
	}
	return userIsAdminFromMD(md)
}

// UserIsAdminFromIncomingContext returns user roles from grpc metadata in incoming context
func UserIsAdminFromIncomingContext(ctx context.Context) (bool, bool) {
	md, ok := metadata.FromIncomingContext(ctx)
	if !ok {
		return false, ok
	}
	return userIsAdminFromMD(md)
}

func userIsAdminFromMD(md metadata.MD) (bool, bool) {
	return authz.UserIsAdminFromMap(md)
}

// UserMetaFromIncomingContext return user meta info from grpc metadata in incoming context
func UserMetaFromIncomingContext(ctx context.Context) (*api.ObjectMeta, bool) {
	md, ok := metadata.FromIncomingContext(ctx)
	if !ok || md == nil {
		return nil, false
	}
	return authz.UserMetaFromMap(md)
}

// UserMetaFromOutgoingContext return user meta info from grpc metadata in outgoing context
func UserMetaFromOutgoingContext(ctx context.Context) (*api.ObjectMeta, bool) {
	md, ok := metadata.FromOutgoingContext(ctx)
	if !ok || md == nil {
		return nil, false
	}
	return authz.UserMetaFromMap(md)
}

func populateMetadataWithUserPerms(md metadata.MD, user *auth.User, isAdmin bool, perms []auth.Permission) error {
	return authz.PopulateMapWithUserPerms(md, user, isAdmin, perms, false)
}
