package password

import (
	"fmt"
	"reflect"
	"testing"

	passwdgen "github.com/sethvargo/go-password/password"

	"strings"

	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestValidate(t *testing.T) {
	tests := []struct {
		name     string
		password string
		errors   []error
	}{
		{
			"empty password",
			"",
			[]error{ErrInsufficientLength, ErrInsufficientSymbols, ErrInsufficientDigits, ErrInsufficientUppercase},
		},
		{
			"insufficient symbols",
			"Abcd135789",
			[]error{ErrInsufficientSymbols},
		},
		{"insufficient digits",
			"Abcdefghij$",
			[]error{ErrInsufficientDigits},
		},
		{
			"valid password",
			"Abcdefg$123",
			nil,
		},
	}
	pc := NewPolicyChecker().(*policyChecker)
	for _, test := range tests {
		errs := pc.Validate(test.password)
		SortErrors(errs)
		SortErrors(test.errors)
		Assert(t, reflect.DeepEqual(errs, test.errors), fmt.Sprintf("%s test failed, expected errors: %v, got: %v", test.name, test.errors, errs))
	}
}

func TestCreatePassword(t *testing.T) {
	password, err := CreatePassword()
	AssertOk(t, err, "unable to generated password")
	Assert(t, len(password) == 12, fmt.Sprintf("password (%s) should contain 12 chars", password))
	Assert(t, strings.ContainsAny(password, passwdgen.Digits), fmt.Sprintf("password (%s) should contain 1 digit", password))
	Assert(t, strings.ContainsAny(password, passwdgen.Symbols), fmt.Sprintf("password (%s) should contain 1 special char", password))
	Assert(t, strings.ContainsAny(password, passwdgen.UpperLetters), fmt.Sprintf("password (%s) should contain 1 uppercase letter", password))
}
