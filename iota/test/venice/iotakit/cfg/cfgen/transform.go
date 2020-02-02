package cfgen

import (
	"fmt"
	"math"
	"math/rand"
	"regexp"
	"strconv"
	"strings"
)

// context for generating ip address
type ipMacCtx struct {
	counter uint64
}

// routines to change the structure
type iterContext struct {
	iters map[string]uint64
	vars  map[string]string
	ips   map[string]ipMacCtx
	macs  map[string]ipMacCtx
}

// newIterContext returns a new iterator
func newIterContext() *iterContext {
	return &iterContext{
		iters: make(map[string]uint64),
		vars:  make(map[string]string),
		ips:   make(map[string]ipMacCtx),
		macs:  make(map[string]ipMacCtx)}
}

// transform Int performs transforming integer value based on following conventions
// 99, 9999, 999999 - integers are converted to a random 2,4, 6 digit integer values
// 98, 9998, 999998 - integers are converted to an iterator that is 2, 4, 6 digit integer values
func (ctx *iterContext) transformUint(src uint64) uint64 {
	strInt := fmt.Sprintf("%d", src)
	tds := len(strInt) - 1
	for ii := 0; ii <= tds; ii++ {
		if strInt[ii] != '9' {
			// last letter could be 8, other all muts be 9
			if ii != tds || strInt[ii] != '8' {
				return src
			}
		}
	}
	tInt := (uint64)(12345)
	genRand := strInt[tds] == '9'
	maxValue := (int)(math.Pow10(tds+1)) - 1
	if genRand {
		minValue := maxValue / 10
		for {
			tInt = (uint64)(rand.Intn(maxValue - minValue))
			if tInt != 0 {
				break
			}
		}
		tInt = tInt + (uint64)(minValue)
	} else {
		if _, ok := ctx.iters[strInt]; !ok {
			ctx.iters[strInt] = 0
		}
		ctx.iters[strInt]++
		tInt = ctx.iters[strInt]
		if ctx.iters[strInt] == (uint64)(maxValue) {
			ctx.iters[strInt] = 0
		}
	}

	return tInt
}

// transformString performs a template based substitution as per following definitions
// {{var=varName}} - is replaced with empty string, but following string is save in the specified variable for substitution later
// {{$varName}} - is replaced with a string with a previously saved variable name
// {{iter}} - is replaced with a string with an iterator
// {{iter-iterName}} - is replaced with a string with an iterator; iterName is like iterator variable name; multiple iterators can be used
// {{iter:low-high}} - is replaced with a string with an iterator, possibly within a range
// {{iter:val1|val2|val3}} - is replaced with a string with iterating values of val1, val2, val3, etc.
// {{rand}} - is replaced with a string with a random integer value
// {{rand:low-high}} - replaces the
// {{rand:val1|val2|val3}} - is replace with a string that randomly selectes the value from val1, val2, val3, etc.
func (ctx *iterContext) transformString(src string) string {
	pat2 := regexp.MustCompile(`{{.*?}}`)
	varName := ""
	changeString := pat2.ReplaceAllStringFunc(src,
		func(inp string) string {
			inp = strings.TrimPrefix(inp, "{{")
			inp = strings.TrimSuffix(inp, "}}")
			replaced := "**INVALID**"
			if strings.Contains(inp, "var=") {
				varName = ctx.setVar(inp)
				// variable set replaces with empty string
				replaced = ""
			} else if strings.HasPrefix(inp, "$") {
				replaced = ctx.varSub(inp)
			} else if strings.HasPrefix(inp, "rand") {
				replaced = randSub(inp)
			} else if strings.HasPrefix(inp, "iter") {
				replaced = ctx.iterSub(inp)
			} else if strings.HasPrefix(inp, "ip") {
				replaced = ctx.ipSub(inp)
			} else if strings.HasPrefix(inp, "mac") {
				replaced = ctx.macSub(inp)
			}
			return replaced
		})
	if varName != "" {
		ctx.vars[varName] = changeString
		fmt.Printf("setting var %v, vars %v", varName, ctx.vars)
	}

	return changeString
}

// set variable if user indicated it be saved
// {{var=varName}}
func (ctx *iterContext) setVar(inp string) string {
	varName := strings.TrimPrefix(inp, "var=")
	if _, ok := ctx.vars[varName]; ok {
		panic(fmt.Sprintf("var %s already in use, vars %+v", varName, ctx.vars))
	}
	return varName
}

// permitted substituations
// {{$varName}}
func (ctx *iterContext) varSub(inp string) string {
	varName := strings.TrimPrefix(inp, "$")
	if _, ok := ctx.vars[varName]; !ok {
		panic(fmt.Sprintf("var %s not set, vars %+v", varName, ctx.vars))
	}
	return ctx.vars[varName]
}

// permitted substituitions
// {{ipv4<name>:a.x.x.x}}
// {{ipv4<name>:a.b.x.x}}
// {{ipv4<name>:a.b.c.x}}
func (ctx *iterContext) ipSub(inp string) string {
	if !strings.HasPrefix(inp, "ipv4") {
		panic(fmt.Sprintf("only support ipv4 substituionts"))
	}
	inpStrs := strings.Split(inp, ":")
	if len(inpStrs) < 2 {
		panic(fmt.Sprintf("invalid ip format: %s", inp))
	}

	ipName := strings.TrimPrefix(inpStrs[0], "ipv4")
	if ipName == "" {
		ipName = "default"
	}
	if _, ok := ctx.ips[ipName]; !ok {
		ctx.ips[ipName] = ipMacCtx{}
	}
	ipCtx := ctx.ips[ipName]
	ipAddrBytes := strings.Split(inpStrs[1], ".")
	if len(ipAddrBytes) != 4 {
		panic(fmt.Sprintf("invalid ip syntax (must be a.b.c.d) - %s", inpStrs[1]))
	}
	ipCtx.counter++
	if ipCtx.counter%255 == 0 {
		ipCtx.counter++
	}
	ipval := ipCtx.counter
	for ii := 3; ii > 0; ii-- {
		if ipAddrBytes[ii] == "x" {
			ipAddrBytes[ii] = fmt.Sprintf("%d", ipval%255)
			ipval = ipval / 255
		}
	}
	ctx.ips[ipName] = ipCtx
	ipAddr := fmt.Sprintf("%s.%s.%s.%s", ipAddrBytes[0], ipAddrBytes[1], ipAddrBytes[2], ipAddrBytes[3])
	return ipAddr
}

// permitted substituitions
// {{mac}}
// {{mac<name>}}
// {{mac<name>:aa.bb.cc}}
func (ctx *iterContext) macSub(inp string) string {
	if !strings.HasPrefix(inp, "mac") {
		panic(fmt.Sprintf("only support mac substituionts"))
	}
	inpStrs := strings.Split(inp, ":")

	oui := "00.ae.dd"
	if len(inpStrs) > 1 {
		oui = inpStrs[1]
	}

	ouiBytes := strings.Split(oui, ".")
	if len(ouiBytes) != 3 {
		panic(fmt.Sprintf("invalid oui: %s", oui))
	}

	macName := strings.TrimPrefix(inpStrs[0], "mac")
	if macName == "" {
		macName = "default"
	}
	if _, ok := ctx.macs[macName]; !ok {
		ctx.macs[macName] = ipMacCtx{}
	}
	macCtx := ctx.macs[macName]
	macCtx.counter++
	macval := macCtx.counter

	bytes := make([]byte, 3)
	for ii := 2; ii >= 0; ii-- {
		bytes[ii] = (byte)(macval % 255)
		macval = macval / 255
	}

	ctx.macs[macName] = macCtx
	macAddr := fmt.Sprintf("%s%s.%s%02x.%02x%02x", ouiBytes[0], ouiBytes[1], ouiBytes[2], bytes[0], bytes[1], bytes[2])
	return macAddr
}

// permitted substitutions
//   {{iter}}
//   {{iter<name>:5-500}}
//   {{iter<name>:3|4|5}}
//   {{iter<name>:foo|bar}}
func (ctx *iterContext) iterSub(inp string) string {
	tString := "INVALID_SUBSTITUTION"
	subValues := strings.Split(inp, ":")
	iterName := strings.TrimPrefix(subValues[0], "iter")
	if iterName == "" {
		iterName = "global"
	}
	if _, ok := ctx.iters[iterName]; !ok {
		ctx.iters[iterName] = 0
	}
	if len(subValues) <= 1 {
		tString = fmt.Sprintf("%d", ctx.iters[iterName])
	} else {

		qualifier := subValues[1]
		if strings.Contains(qualifier, "-") {
			low, high := parseRange(qualifier)
			iterValue := ctx.iters[iterName] % (high - low + 1)
			tString = fmt.Sprintf("%d", low+iterValue)
		}
		if strings.Contains(qualifier, "|") {
			set := strings.Split(qualifier, "|")
			idx := ctx.iters[iterName] % (uint64)(len(set))
			tString = fmt.Sprintf("%s", set[idx])
		}
	}
	ctx.iters[iterName]++
	return tString
}

// permitted substitutions
//   {{var=var1}}{{rand}}
//   {{rand:5-500}}
//   {{rand:3|4|5}}
//   {{rand:foo|bar}}
func randSub(inp string) string {
	subValues := strings.Split(inp, ":")
	if len(subValues) <= 1 {
		return fmt.Sprintf("%d", rand.Intn(math.MaxInt32))
	}

	qualifier := subValues[1]
	if strings.Contains(qualifier, "-") {
		low, high := parseRange(qualifier)
		return fmt.Sprintf("%d", low+(uint64)(rand.Intn((int)(high-low))))
	}
	if strings.Contains(qualifier, "|") {
		set := strings.Split(qualifier, "|")
		idx := rand.Intn(len(set))
		return fmt.Sprintf("%s", set[idx])
	}
	return "INVALID_SUBSTITUTION"
}

// parses '-' separated range
func parseRange(inp string) (uint64, uint64) {
	high := (uint64)(math.MaxUint32)
	rangeValues := strings.Split(inp, "-")
	i, err := strconv.Atoi(rangeValues[0])
	if err != nil {
		panic(fmt.Errorf("rangeValues = %+v", rangeValues))
	}
	low := (uint64)(i)
	if len(rangeValues) > 1 {
		i, err := strconv.Atoi(rangeValues[1])
		if err != nil {
			panic(fmt.Errorf("rangeValues = %+v", rangeValues))
		}
		high = (uint64)(i)
	}
	return low, high
}
