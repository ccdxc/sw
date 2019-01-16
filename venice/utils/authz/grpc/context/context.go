package context

import (
	"context"
	"errors"

	"google.golang.org/grpc/metadata"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/auth"
)

const (
	userTenantKey = "pensando-venice-user-tenant-key"
	usernameKey   = "pensando-venice-user-key"
	// permsKey with -bin suffix tells grpc that value is binary. grpc auto base64 encodes and decodes it
	permsKey = "pensando-venice-perms-key-bin"
)

// NewOutgoingContextWithUserPerms creates a new context with user and permissions metadata to send to grpc backend.
func NewOutgoingContextWithUserPerms(ctx context.Context, user *auth.User, perms []auth.Permission) (context.Context, error) {
	md, ok := metadata.FromOutgoingContext(ctx)
	if !ok {
		md = metadata.MD{}
	} else {
		md = md.Copy()
	}
	// set user info in md
	if err := populateMetadataWithUserPerms(md, user, perms); err != nil {
		return ctx, err
	}
	return metadata.NewOutgoingContext(ctx, md), nil
}

// NewIncomingContextWithUserPerms creates a new context with user and permissions metadata to send to grpc backend. This is for testing only.
func NewIncomingContextWithUserPerms(ctx context.Context, user *auth.User, perms []auth.Permission) (context.Context, error) {
	md, ok := metadata.FromIncomingContext(ctx)
	if !ok {
		md = metadata.MD{}
	} else {
		md = md.Copy()
	}
	// set user info in md
	if err := populateMetadataWithUserPerms(md, user, perms); err != nil {
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
	vals, ok := md[permsKey]
	if !ok {
		return nil, ok, nil
	}
	var perms []auth.Permission
	for _, val := range vals {
		data := []byte(val)
		perm := &auth.Permission{}
		if err := perm.Unmarshal(data); err != nil {
			return nil, false, err
		}
		perms = append(perms, *perm)
	}
	return perms, true, nil
}

// UserMetaFromIncomingContext return user meta info from grpc metadata in incoming context
func UserMetaFromIncomingContext(ctx context.Context) (*api.ObjectMeta, bool) {
	md, ok := metadata.FromIncomingContext(ctx)
	if !ok || md == nil {
		return nil, false
	}
	names := md[usernameKey]
	tenants := md[userTenantKey]
	if len(names) == 0 || len(tenants) == 0 {
		return nil, false
	}
	return &api.ObjectMeta{Name: names[0], Tenant: tenants[0]}, true
}

// UserMetaFromOutgoingContext return user meta info from grpc metadata in outgoing context
func UserMetaFromOutgoingContext(ctx context.Context) (*api.ObjectMeta, bool) {
	md, ok := metadata.FromOutgoingContext(ctx)
	if !ok || md == nil {
		return nil, false
	}
	names := md[usernameKey]
	tenants := md[userTenantKey]
	if len(names) == 0 || len(tenants) == 0 {
		return nil, false
	}
	return &api.ObjectMeta{Name: names[0], Tenant: tenants[0]}, true
}

func populateMetadataWithUserPerms(md metadata.MD, user *auth.User, perms []auth.Permission) error {
	// validate user obj
	if user == nil {
		return errors.New("no user specified")
	}
	if user.Tenant == "" {
		return errors.New("tenant not populated in user")
	}
	if user.Name == "" {
		return errors.New("username not populated in user object")
	}
	// set user info in md
	md[userTenantKey] = []string{user.Tenant}
	md[usernameKey] = []string{user.Name}
	for _, perm := range perms {
		data, err := perm.Marshal()
		if err != nil {
			return err
		}
		md[permsKey] = append(md[permsKey], string(data))
	}
	return nil
}
