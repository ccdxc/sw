package context

import (
	"context"
	"errors"
	"fmt"
	"reflect"
	"testing"

	"google.golang.org/grpc/metadata"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/api/login"
	"github.com/pensando/sw/venice/utils/authz"
	"github.com/pensando/sw/venice/utils/authz/rbac"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestUserMetaFromIncomingContext(t *testing.T) {
	tests := []struct {
		name     string
		md       metadata.MD
		usermeta *api.ObjectMeta
		ok       bool
	}{
		{
			name:     "no username",
			md:       metadata.MD{userTenantKey: []string{"default"}},
			usermeta: nil,
			ok:       false,
		},
		{
			name:     "no tenant",
			md:       metadata.MD{usernameKey: []string{"testuser"}},
			usermeta: nil,
			ok:       false,
		},
		{
			name:     "correct user info",
			md:       metadata.MD{usernameKey: []string{"testuser"}, userTenantKey: []string{"default"}},
			usermeta: &api.ObjectMeta{Name: "testuser", Tenant: "default"},
			ok:       true,
		},
		{
			name:     "nil metadata",
			md:       nil,
			usermeta: nil,
			ok:       false,
		},
	}
	for _, test := range tests {
		ctx := metadata.NewIncomingContext(context.TODO(), test.md)
		meta, ok := UserMetaFromIncomingContext(ctx)
		Assert(t, test.ok == ok, fmt.Sprintf("[%s] test failed", test.name))
		Assert(t, reflect.DeepEqual(test.usermeta, meta), fmt.Sprintf("[%s] test failed, expected user meta [%#v], got [%#v]", test.name, test.usermeta, meta))
	}
}

func TestNewOutgoingContextWithUserPerms(t *testing.T) {
	tests := []struct {
		name string
		ctx  context.Context
		user *auth.User
		err  error
	}{
		{
			name: "existing metadata",
			ctx:  metadata.NewOutgoingContext(context.TODO(), metadata.Pairs("testkey", "testval")),
			user: &auth.User{
				TypeMeta: api.TypeMeta{Kind: "User"},
				ObjectMeta: api.ObjectMeta{
					Tenant: "testTenant",
					Name:   "testUser",
				},
				Spec: auth.UserSpec{
					Fullname: "Test User",
					Password: "password",
					Email:    "testuser@pensandio.io",
					Type:     auth.UserSpec_Local.String(),
				},
			},
			err: nil,
		},
		{
			name: "nil user",
			ctx:  metadata.NewOutgoingContext(context.TODO(), metadata.Pairs("testkey", "testval")),
			user: nil,
			err:  errors.New("no user specified"),
		},
	}
	perms := []auth.Permission{
		login.NewPermission(
			"testTenant",
			"",
			auth.Permission_Search.String(),
			"",
			"",
			auth.Permission_Read.String()),
		login.NewPermission(
			"testTenant",
			string(apiclient.GroupSecurity),
			string(security.KindSGPolicy),
			authz.ResourceNamespaceAll,
			"",
			auth.Permission_AllActions.String()),
	}
	for _, test := range tests {
		ctx, err := NewOutgoingContextWithUserPerms(test.ctx, test.user, perms)
		Assert(t, reflect.DeepEqual(err, test.err), fmt.Sprintf("[%s] test failed, expected err [%v], got [%v]", test.name, test.err, err))
		if err == nil {
			md, ok := metadata.FromOutgoingContext(ctx)
			Assert(t, ok, fmt.Sprintf("[%s] test failed, no metadata in outgoing context", test.name))
			usernames, ok := md[usernameKey]
			Assert(t, ok == (test.user != nil && test.user.Name != ""), fmt.Sprintf("[%s] test failed, unexpected usernames [%v] in md", test.name, usernames))
			Assert(t, !ok || (len(usernames) == 1 && test.user.Name == usernames[0]), fmt.Sprintf("[%s] test failed, expected username [%s], got [%v] in md", test.name, test.user.Name, usernames))
			tenants, ok := md[userTenantKey]
			Assert(t, ok == (test.user != nil && test.user.Tenant != ""), fmt.Sprintf("[%s] test failed, unexpected user tenant [%v] in md", test.name, tenants))
			Assert(t, !ok || (len(tenants) == 1 && test.user.Tenant == tenants[0]), fmt.Sprintf("[%s] test failed, expected user tenant [%s], got [%v] in md", test.name, test.user.Tenant, tenants))
			perms, ok := md[permsKey]
			Assert(t, ok == (perms != nil), fmt.Sprintf("[%s] test failed, unexpected perms", test.name))
			Assert(t, len(perms) == len(perms), fmt.Sprintf("[%s] test failed, unexpected perms: %v", test.name, perms))
			vals, ok := md["testkey"]
			Assert(t, ok, fmt.Sprintf("[%s] test failed, missing test values in md: %#v", test.name, md))
			Assert(t, !ok || (len(vals) == 1 && vals[0] == "testval"), fmt.Sprintf("[%s] test failed, expected test value [%s], got [%v] in md", test.name, "testval", vals[0]))
		}
	}
}

func TestNewIncomingContextWithUserPerms(t *testing.T) {
	tests := []struct {
		name string
		ctx  context.Context
		user *auth.User
		err  error
	}{
		{
			name: "existing metadata",
			ctx:  metadata.NewIncomingContext(context.TODO(), metadata.Pairs("testkey", "testval")),
			user: &auth.User{
				TypeMeta: api.TypeMeta{Kind: "User"},
				ObjectMeta: api.ObjectMeta{
					Tenant: "testTenant",
					Name:   "testUser",
				},
				Spec: auth.UserSpec{
					Fullname: "Test User",
					Password: "password",
					Email:    "testuser@pensandio.io",
					Type:     auth.UserSpec_Local.String(),
				},
			},
			err: nil,
		},
		{
			name: "nil user",
			ctx:  metadata.NewIncomingContext(context.TODO(), metadata.Pairs("testkey", "testval")),
			user: nil,
			err:  errors.New("no user specified"),
		},
	}
	perms := []auth.Permission{
		login.NewPermission(
			"testTenant",
			"",
			auth.Permission_Search.String(),
			"",
			"",
			auth.Permission_Read.String()),
		login.NewPermission(
			"testTenant",
			string(apiclient.GroupSecurity),
			string(security.KindSGPolicy),
			authz.ResourceNamespaceAll,
			"",
			auth.Permission_AllActions.String()),
	}
	for _, test := range tests {
		ctx, err := NewIncomingContextWithUserPerms(test.ctx, test.user, perms)
		Assert(t, reflect.DeepEqual(err, test.err), fmt.Sprintf("[%s] test failed, expected err [%v], got [%v]", test.name, test.err, err))
		if err == nil {
			md, ok := metadata.FromIncomingContext(ctx)
			Assert(t, ok, fmt.Sprintf("[%s] test failed, no metadata in incoming context", test.name))
			usernames, ok := md[usernameKey]
			Assert(t, ok == (test.user != nil && test.user.Name != ""), fmt.Sprintf("[%s] test failed, unexpected usernames [%v] in md", test.name, usernames))
			Assert(t, !ok || (len(usernames) == 1 && test.user.Name == usernames[0]), fmt.Sprintf("[%s] test failed, expected username [%s], got [%v] in md", test.name, test.user.Name, usernames))
			tenants, ok := md[userTenantKey]
			Assert(t, ok == (test.user != nil && test.user.Tenant != ""), fmt.Sprintf("[%s] test failed, unexpected user tenant [%v] in md", test.name, tenants))
			Assert(t, !ok || (len(tenants) == 1 && test.user.Tenant == tenants[0]), fmt.Sprintf("[%s] test failed, expected user tenant [%s], got [%v] in md", test.name, test.user.Tenant, tenants))
			perms, ok := md[permsKey]
			Assert(t, ok == (perms != nil), fmt.Sprintf("[%s] test failed, unexpected perms", test.name))
			Assert(t, len(perms) == len(perms), fmt.Sprintf("[%s] test failed, unexpected perms: %v", test.name, perms))
			vals, ok := md["testkey"]
			Assert(t, ok, fmt.Sprintf("[%s] test failed, missing test values in md: %#v", test.name, md))
			Assert(t, !ok || (len(vals) == 1 && vals[0] == "testval"), fmt.Sprintf("[%s] test failed, expected test value [%s], got [%v] in md", test.name, "testval", vals[0]))
		}
	}
}

func TestPopulateMetadataWithUserPerms(t *testing.T) {
	tests := []struct {
		name  string
		user  *auth.User
		perms []auth.Permission
		err   error
	}{
		{
			name: "no metadata",
			user: &auth.User{
				TypeMeta: api.TypeMeta{Kind: "User"},
				ObjectMeta: api.ObjectMeta{
					Tenant: "testTenant",
					Name:   "testUser",
				},
				Spec: auth.UserSpec{
					Fullname: "Test User",
					Password: "password",
					Email:    "testuser@pensandio.io",
					Type:     auth.UserSpec_Local.String(),
				},
			},
			perms: []auth.Permission{
				login.NewPermission(
					"testTenant",
					"",
					auth.Permission_Search.String(),
					"",
					"",
					auth.Permission_Read.String()),
				login.NewPermission(
					"testTenant",
					string(apiclient.GroupSecurity),
					string(security.KindSGPolicy),
					authz.ResourceNamespaceAll,
					"",
					auth.Permission_AllActions.String()),
			},
			err: nil,
		},
		{
			name: "nil user",
			user: nil,
			perms: []auth.Permission{
				login.NewPermission(
					"testTenant",
					"",
					auth.Permission_Search.String(),
					"",
					"",
					auth.Permission_Read.String()),
				login.NewPermission(
					"testTenant",
					string(apiclient.GroupSecurity),
					string(security.KindSGPolicy),
					authz.ResourceNamespaceAll,
					"",
					auth.Permission_AllActions.String()),
			},
			err: errors.New("no user specified"),
		},
		{
			name: "empty tenant",
			user: &auth.User{
				TypeMeta: api.TypeMeta{Kind: "User"},
				ObjectMeta: api.ObjectMeta{
					Tenant: "",
					Name:   "testUser",
				},
				Spec: auth.UserSpec{
					Fullname: "Test User",
					Password: "password",
					Email:    "testuser@pensandio.io",
					Type:     auth.UserSpec_Local.String(),
				},
			},
			perms: []auth.Permission{
				login.NewPermission(
					"testTenant",
					"",
					auth.Permission_Search.String(),
					"",
					"",
					auth.Permission_Read.String()),
				login.NewPermission(
					"testTenant",
					string(apiclient.GroupSecurity),
					string(security.KindSGPolicy),
					authz.ResourceNamespaceAll,
					"",
					auth.Permission_AllActions.String()),
			},
			err: errors.New("tenant not populated in user"),
		},
		{
			name: "empty username",
			user: &auth.User{
				TypeMeta: api.TypeMeta{Kind: "User"},
				ObjectMeta: api.ObjectMeta{
					Tenant: "testTenant",
					Name:   "",
				},
				Spec: auth.UserSpec{
					Fullname: "Test User",
					Password: "password",
					Email:    "testuser@pensandio.io",
					Type:     auth.UserSpec_Local.String(),
				},
			},
			perms: []auth.Permission{
				login.NewPermission(
					"testTenant",
					"",
					auth.Permission_Search.String(),
					"",
					"",
					auth.Permission_Read.String()),
				login.NewPermission(
					"testTenant",
					string(apiclient.GroupSecurity),
					string(security.KindSGPolicy),
					authz.ResourceNamespaceAll,
					"",
					auth.Permission_AllActions.String()),
			},
			err: errors.New("username not populated in user object"),
		},
	}
	for _, test := range tests {
		md := metadata.MD{}
		err := populateMetadataWithUserPerms(md, test.user, test.perms)
		Assert(t, reflect.DeepEqual(err, test.err), fmt.Sprintf("[%s] test failed, expected err [%v], got [%v]", test.name, test.err, err))
		if err == nil {
			usernames, ok := md[usernameKey]
			Assert(t, ok == (test.user != nil && test.user.Name != ""), fmt.Sprintf("[%s] test failed, unexpected usernames [%v] in md", test.name, usernames))
			Assert(t, !ok || (len(usernames) == 1 && test.user.Name == usernames[0]), fmt.Sprintf("[%s] test failed, expected username [%s], got [%v] in md", test.name, test.user.Name, usernames))
			tenants, ok := md[userTenantKey]
			Assert(t, ok == (test.user != nil && test.user.Tenant != ""), fmt.Sprintf("[%s] test failed, unexpected user tenant [%v] in md", test.name, tenants))
			Assert(t, !ok || (len(tenants) == 1 && test.user.Tenant == tenants[0]), fmt.Sprintf("[%s] test failed, expected user tenant [%s], got [%v] in md", test.name, test.user.Tenant, tenants))
			perms, ok := md[permsKey]
			Assert(t, ok == (test.perms != nil), fmt.Sprintf("[%s] test failed, unexpected perms", test.name))
			Assert(t, len(perms) == len(test.perms), fmt.Sprintf("[%s] test failed, unexpected perms: %v", test.name, perms))
		}
	}
}

func TestPermsFromContext(t *testing.T) {
	tests := []struct {
		name  string
		md    metadata.MD
		perms []auth.Permission
		ok    bool
		err   error
	}{
		{
			name:  "no metadata",
			md:    nil,
			perms: nil,
			ok:    false,
			err:   nil,
		},
		{
			name:  "no perms",
			md:    metadata.MD{},
			perms: nil,
			ok:    false,
			err:   nil,
		},
		{
			name: "successful perm retrieval",
			md:   metadata.MD{},
			perms: []auth.Permission{
				login.NewPermission(
					"testTenant",
					"",
					auth.Permission_Search.String(),
					"",
					"",
					auth.Permission_Read.String()),
				login.NewPermission(
					"testTenant",
					string(apiclient.GroupSecurity),
					string(security.KindSGPolicy),
					authz.ResourceNamespaceAll,
					"",
					auth.Permission_AllActions.String()),
			},
			ok:  true,
			err: nil,
		},
	}
	for _, test := range tests {
		ctx := context.TODO()
		outctx := context.TODO()
		if test.md != nil {
			for _, perm := range test.perms {
				data, err := perm.Marshal()
				AssertOk(t, err, fmt.Sprintf("[%s] test failed, error marshalling perm", test.name))
				test.md[permsKey] = append(test.md[permsKey], string(data))
			}
			ctx = metadata.NewIncomingContext(ctx, test.md)
			outctx = metadata.NewOutgoingContext(ctx, test.md)
		}
		perms, ok, err := PermsFromIncomingContext(ctx)
		Assert(t, reflect.DeepEqual(err, test.err), fmt.Sprintf("[%s] test failed, expected err [%v], got [%v]", test.name, test.err, err))
		Assert(t, test.ok == ok, fmt.Sprintf("[%s] test failed", test.name))
		Assert(t, !ok || err != nil || rbac.ArePermsEqual(test.perms, perms),
			fmt.Sprintf("[%s] test failed, expected perms [%s], got [%s]", test.name, rbac.PrintPerms(test.name, test.perms), rbac.PrintPerms(test.name, perms)))
		perms, ok, err = PermsFromOutgoingContext(outctx)
		Assert(t, reflect.DeepEqual(err, test.err), fmt.Sprintf("[%s] test failed, expected err [%v], got [%v]", test.name, test.err, err))
		Assert(t, test.ok == ok, fmt.Sprintf("[%s] test failed", test.name))
		Assert(t, !ok || err != nil || rbac.ArePermsEqual(test.perms, perms),
			fmt.Sprintf("[%s] test failed, expected perms [%s], got [%s]", test.name, rbac.PrintPerms(test.name, test.perms), rbac.PrintPerms(test.name, perms)))
	}
}

// BenchmarkNewOutgoingContextWithUserPerms 100 perms   	   30000	     54533 ns/op
func BenchmarkNewOutgoingContextWithUserPerms(b *testing.B) {
	user := &auth.User{
		TypeMeta: api.TypeMeta{Kind: "User"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "testTenant",
			Name:   "testUser",
		},
		Spec: auth.UserSpec{
			Fullname: "Test User",
			Password: "password",
			Email:    "testuser@pensandio.io",
			Type:     auth.UserSpec_Local.String(),
		},
	}
	var perms []auth.Permission
	for i := 0; i < 50; i++ {
		perms = append(perms, login.NewPermission(
			"testTenant",
			"",
			auth.Permission_Search.String(),
			"",
			"",
			auth.Permission_Read.String()),
			login.NewPermission(
				"testTenant",
				string(apiclient.GroupSecurity),
				string(security.KindSGPolicy),
				authz.ResourceNamespaceAll,
				"",
				auth.Permission_AllActions.String()),
		)
	}
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		nctx, _ := NewOutgoingContextWithUserPerms(context.TODO(), user, perms)
		PermsFromOutgoingContext(nctx)
	}
}
