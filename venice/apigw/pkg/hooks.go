package apigwpkg

import (
	"github.com/pensando/sw/venice/apigw"
)

type svcProfile struct {
	defProf  apigw.ServiceProfile
	preauthn []apigw.PreAuthNHook
	preauthz []apigw.PreAuthZHook
	precall  []apigw.PreCallHook
	postcall []apigw.PostCallHook
}

// preauthNHooks returns all registered pre authn hooks
func (s *svcProfile) PreAuthNHooks() []apigw.PreAuthNHook {
	if len(s.preauthn) == 0 && s.defProf != nil {
		return s.defProf.PreAuthNHooks()
	}
	return s.preauthn
}

// preauthzHooks returns all registered pre authn hooks
func (s *svcProfile) PreAuthZHooks() []apigw.PreAuthZHook {
	if len(s.preauthz) == 0 && s.defProf != nil {
		return s.defProf.PreAuthZHooks()
	}
	return s.preauthz
}

// PreCallHooks returns all registered pre call hooks
func (s *svcProfile) PreCallHooks() []apigw.PreCallHook {
	if len(s.precall) == 0 && s.defProf != nil {
		return s.defProf.PreCallHooks()
	}
	return s.precall
}

// PostCallHooks returns all registered post call hooks
func (s *svcProfile) PostCallHooks() []apigw.PostCallHook {
	if len(s.postcall) == 0 && s.defProf != nil {
		return s.defProf.PostCallHooks()
	}
	return s.postcall
}

// AddPreAuthNHook registers a pre authn hook
func (s *svcProfile) AddPreAuthNHook(hook apigw.PreAuthNHook) error {
	s.preauthn = append(s.preauthn, hook)
	return nil
}

// AddPreAuthZHook registers a pre authn hook
func (s *svcProfile) AddPreAuthZHook(hook apigw.PreAuthZHook) error {
	s.preauthz = append(s.preauthz, hook)
	return nil
}

// AddPreCallHook registers a pre call hook
func (s *svcProfile) AddPreCallHook(hook apigw.PreCallHook) error {
	s.precall = append(s.precall, hook)
	return nil
}

// AddPostCallHook registers a post call hook
func (s *svcProfile) AddPostCallHook(hook apigw.PostCallHook) error {
	s.postcall = append(s.postcall, hook)
	return nil
}

// NewServiceProfile creates a new service profile object
func NewServiceProfile(fallback apigw.ServiceProfile) apigw.ServiceProfile {
	return &svcProfile{
		defProf: fallback,
	}
}
