package password

import (
	"context"
	"fmt"
	"strings"
	"time"

	passwdgen "github.com/sethvargo/go-password/password"

	"github.com/pensando/sw/venice/utils"
)

const (
	// Length is minimum length of password
	Length = 9
	// Digits is number of digits required in password
	Digits = 1
	// Symbols is number of special characters required in password from "~!@#$%^&*()_+`-={}|[]\\:\"<>?,./"
	Symbols = 1
	// Uppercase is number of upper case letters required in password
	Uppercase = 1
)

var (
	// ErrInsufficientLength is returned when password is of insufficient length
	ErrInsufficientLength = fmt.Errorf("password should be at least %d characters", Length)
	// ErrInsufficientDigits is returned when password contains insufficient digits
	ErrInsufficientDigits = fmt.Errorf("password should contain atleast %d digits", Digits)
	// ErrInsufficientSymbols is returned when password contains insufficient characters
	ErrInsufficientSymbols = fmt.Errorf("password should contain at least %d special characters in %s", Symbols, passwdgen.Symbols)
	// ErrInsufficientUppercase is returned when password contains insufficient uppercase letters
	ErrInsufficientUppercase = fmt.Errorf("password should contain at least %d uppercase letters", Uppercase)
)

type validator func(password string) []error

type policyChecker struct {
	validators []validator
}

func defaultValidator() validator {
	return func(password string) []error {
		var errors []error
		if len(password) < 9 {
			errors = append(errors, ErrInsufficientLength)
		}
		if !strings.ContainsAny(password, passwdgen.Symbols) {
			errors = append(errors, ErrInsufficientSymbols)
		}
		if !strings.ContainsAny(password, passwdgen.Digits) {
			errors = append(errors, ErrInsufficientDigits)
		}
		if !strings.ContainsAny(password, passwdgen.UpperLetters) {
			errors = append(errors, ErrInsufficientUppercase)
		}
		return errors
	}
}

func (p *policyChecker) Validate(password string) []error {
	var errors []error
	for _, validator := range p.validators {
		errs := validator(password)
		errors = append(errors, errs...)
	}
	return errors
}

// NewPolicyChecker creates PolicyChecker to validate password based on password policy
func NewPolicyChecker() PolicyChecker {
	pc := &policyChecker{}
	pc.validators = append(pc.validators, defaultValidator())
	return pc
}

// CreatePassword generates password of length 12 with 1 digit and 1 special char
func CreatePassword() (string, error) {
	result, err := utils.ExecuteWithRetry(func(ctx context.Context) (interface{}, error) {
		passwd, err := passwdgen.Generate(12, Digits, Symbols, false, false)
		if err != nil {
			return "", err
		}
		if !strings.ContainsAny(passwd, passwdgen.UpperLetters) {
			return "", ErrInsufficientUppercase
		}
		return passwd, nil
	}, 10*time.Millisecond, 20)
	if err != nil {
		return "", err
	}
	password := result.(string)
	return password, nil
}
