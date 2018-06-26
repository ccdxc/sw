package authn

import (
	"context"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/utils/log"
)

// UpdateExternalUser updates or creates (if user doesn't exist) external user in Venice after successful external auth (RADIUS or LDAP)
func UpdateExternalUser(apicl apiclient.Services, username, tenant, fullname, email string, groups []string) (*auth.User, bool, error) {

	// Create external user
	objMeta := &api.ObjectMeta{
		Name:   username,
		Tenant: tenant,
	}
	user, err := apicl.AuthV1().User().Get(context.Background(), objMeta)

	// user object
	user = &auth.User{
		TypeMeta:   api.TypeMeta{Kind: "User"},
		ObjectMeta: *objMeta,
		Spec: auth.UserSpec{
			Fullname: fullname,
			Email:    email,
			Type:     auth.UserSpec_EXTERNAL.String(),
		},
		Status: auth.UserStatus{
			UserGroups: groups,
		},
	}

	if err != nil {
		user, err = apicl.AuthV1().User().Create(context.Background(), user)
		if err != nil {
			log.Errorf("Error creating external user [%s], Err: %v", user.Name, err)
			return nil, false, err
		}
		log.Debugf("External user [%s] created.", username)
	} else {
		user, err = apicl.AuthV1().User().Update(context.Background(), user)
		if err != nil {
			log.Errorf("Error updating external user [%s], Err: %v", user.Name, err)
			return nil, false, err
		}
		log.Debugf("External user [%s] updated.", username)
	}
	return user, true, nil
}
