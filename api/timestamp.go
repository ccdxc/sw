package api

import (
	"encoding/json"
	"fmt"
	"strings"
	"time"

	"github.com/gogo/protobuf/types"
	"github.com/pkg/errors"
)

const (
	// INVALID means uninitialized op
	INVALID = iota
	// NOP means this is a unary
	NOP
	// ADD means addition!
	ADD
	// SUB means subtraction!
	SUB
)

// UnmarshalJSON implements the json.Unmarshaller interface.
func (t *Timestamp) UnmarshalJSON(b []byte) error {
	var str string
	json.Unmarshal(b, &str)
	if str == "" {
		t.SetTime(time.Unix(0, 0))
		return nil
	}

	return t.Parse(str)
}

// MarshalJSON implements the json.Marshaler interface.
func (t Timestamp) MarshalJSON() ([]byte, error) {
	stdTime, err := types.TimestampFromProto(&t.Timestamp)
	if err != nil {
		return []byte("null"), err
	}
	if stdTime.Equal(time.Unix(0, 0)) {
		return json.Marshal("")
	}
	return json.Marshal(stdTime.Format(time.RFC3339Nano))
}

// Time returns go Time from api.Timestamp. This enables us to change the implementation later
func (t Timestamp) Time() (time.Time, error) {
	return types.TimestampFromProto(&t.Timestamp)
}

// SetTime sets Time . This enables us to change the implementation later
func (t *Timestamp) SetTime(pt time.Time) error {
	ts, err := types.TimestampProto(pt)
	if err != nil {
		return err
	}
	t.Timestamp = *ts
	return nil
}

// Parse parses a timestamp expression. The following expressions are
// supported:
// time.RFC3339Nano +/- time.Duration
// now() +/- time.Duration
// note: 1) white space is required between tokens.
//       2) +/- time.Duration is optional
func (t *Timestamp) Parse(e string) error {
	var expr binaryExpr
	err := expr.parse(e)
	if err != nil {
		return err
	}

	pt, err := expr.GetTime()
	if err != nil {
		return err
	}
	return t.SetTime(pt)

}

// Expr defines a time expression to be parsed
type Expr interface {
	parse(string) error
	GetTime() (time.Time, error)
	GetDuration() (time.Duration, error)
}

// unaryExpr implements Expr
// it is either a duration or a timestamp
type unaryExpr struct {
	d time.Duration
	t time.Time
}

func (u *unaryExpr) parse(s string) error {
	var dd time.Duration
	// trim white space
	ss := strings.TrimSpace(s)

	// look for now()
	if strings.ToLower(ss) == "now()" {
		u.t = time.Now()
		u.d = dd
		return nil
	}

	// look for a timestamp
	tt, err := time.Parse(time.RFC3339Nano, ss)
	if err == nil {
		u.t = tt
		u.d = dd
		return nil
	}

	tt, err = time.Parse(time.RFC3339, ss)
	if err == nil {
		u.t = tt
		u.d = dd
		return nil
	}

	// look for a duration
	dur, err := time.ParseDuration(ss)
	if err != nil {
		return errors.Wrapf(err, "Parsing %s as duration", s)
	}

	u.t = time.Time{}
	u.d = dur
	return nil
}

// GetTime gets the timestamp -- not supported for unaryExpr
func (u *unaryExpr) GetTime() (time.Time, error) {

	if u.t.IsZero() {
		err := fmt.Errorf("Timestamp is not available")
		return u.t, err
	}

	return u.t, nil
}

// GetDuration gets the duration from the unaryExpr
func (u *unaryExpr) GetDuration() (time.Duration, error) {
	if u.d.Nanoseconds() == 0 {
		err := fmt.Errorf("Duration is not available")
		return u.d, err
	}

	return u.d, nil
}

// binaryExpr implements Expr, with a left hand side and right hand side
type binaryExpr struct {
	lhs Expr
	rhs Expr
	op  int
}

func (b *binaryExpr) parse(s string) error {
	// only a single add or subtract is allowed now.
	// this could change in the future

	addCount := strings.Count(s, " + ")
	subCount := strings.Count(s, " - ")
	if (addCount + subCount) > 1 {
		return fmt.Errorf("Only a single add or subtract is allowed")
	}

	if (addCount + subCount) == 0 {
		// unaryExpr expression
		var u unaryExpr
		err := u.parse(s)
		if err != nil {
			return errors.Wrap(err, "Parsing unary")
		}
		b.op = NOP
		b.lhs = &u
		return nil
	}

	var terms []string

	if addCount == 1 {
		b.op = ADD
		terms = strings.Split(s, " + ")
	} else {
		b.op = SUB
		terms = strings.Split(s, " - ")
	}

	var lhs binaryExpr
	var rhs binaryExpr
	err := lhs.parse(terms[0])
	if err != nil {
		return errors.Wrapf(err, "Parsing LHS of %s", s)
	}

	err = rhs.parse(terms[1])
	if err != nil {
		return errors.Wrapf(err, "Parsing RHS of %s", s)
	}

	b.rhs = &rhs
	b.lhs = &lhs

	return nil
}

func (b *binaryExpr) GetTime() (time.Time, error) {

	var t time.Time

	switch b.op {
	case NOP:
		return b.lhs.GetTime()

	case ADD:
		ref, offset, err := b.getTerms()
		if err != nil {
			return t, err
		}
		return ref.Add(offset), nil

	case SUB:
		ref, offset, err := b.getTerms()
		if err != nil {
			return t, err
		}
		return ref.Add(-offset), nil

	default:
		return t, fmt.Errorf("Invalid expression")
	}
}

func (b *binaryExpr) GetDuration() (time.Duration, error) {
	var t time.Duration

	switch b.op {
	case NOP:
		return b.lhs.GetDuration()
	default:
		return t, fmt.Errorf("Invalid expression")
	}
}

func (b *binaryExpr) getTerms() (ref time.Time, offset time.Duration, err error) {
	ref, err = b.lhs.GetTime()
	if err != nil {
		err = errors.Wrap(err, "from lhs")
		return
	}

	offset, err = b.rhs.GetDuration()
	if err != nil {
		err = errors.Wrap(err, "from rhs")
		return
	}

	return
}
