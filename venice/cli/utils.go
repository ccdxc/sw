package vcli

import (
	goContext "context"
	"encoding/json"
	"fmt"
	"io"
	"os"
	"regexp"
	"sort"
	"strings"
	"text/tabwriter"

	"github.com/ghodss/yaml"

	"github.com/pensando/sw/api"
	tqclient "github.com/pensando/sw/api/generated/telemetry_query"
	loginctx "github.com/pensando/sw/api/login/context"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/telemetryclient"
)

func dumpStructStdout(dumpYml bool, obj interface{}) {
	os.Stdout.Write(dumpStruct(dumpYml, obj))
	os.Stdout.WriteString("\n")
}

// dumpStruct prints the structure in yaml or json format
func dumpStruct(dumpYml bool, obj interface{}) []byte {
	j, err := json.MarshalIndent(obj, "", "  ")
	if err != nil {
		fmt.Printf("Unable to decode following output to json:\n%v\n", obj)
		return []byte{}
	}
	if !dumpYml {
		return j
	}

	y, err := yaml.JSONToYAML(j)
	if err != nil {
		fmt.Printf("Unable to convert following json to yaml:\n%s\n", j)
		return []byte{}
	}
	return y
}

// sliceToMap converts a kvSplitter separted command line option strings to a go map
func sliceToMap(mapStrs []string) (map[string]string, error) {
	labels := make(map[string]string)
	for _, label := range mapStrs {
		kv := []string{}
		if !strings.Contains(label, kvSplitter) {
			return labels, fmt.Errorf("No separator found in label '%s', e.g. `--label dept%sfinance`", label, kvSplitter)
		}
		kv = strings.Split(label, kvSplitter)
		if len(kv) != 2 {
			return labels, fmt.Errorf("More separators found in label '%s', e.g. `--label dept%sfinance`", label, kvSplitter)
		}
		labels[kv[0]] = kv[1]
	}
	return labels, nil
}

// sortKeys sorts the keys of a map, used for predictable output
func sortKeys(m map[string]bool) []string {
	mk := make([]string, len(m))
	i := 0
	for k := range m {
		mk[i] = k
		i++
	}
	sort.Strings(mk)
	return mk
}

// matchString return true if the name matches any from the specified list
func matchString(names []string, name string) bool {
	return matchStringInternal(names, name, false)
}

// case insensitive match of a sub string with in a list
func matchSubString(names []string, name string) bool {
	return matchStringInternal(names, name, true)
}

// matchStringInternal provides multiple variations of matching a substring in the specified list
func matchStringInternal(names []string, name string, wildCard bool) bool {
	// nil (unspecified) list of names matches everything
	if len(names) == 0 {
		return true
	}

	for _, n := range names {
		if wildCard {
			if strings.Contains(strings.ToLower(name), strings.ToLower(n)) {
				return true
			}
		} else if n == name {
			return true
		}
	}
	return false
}

// matchRe returns true if the object name matches a regular expression
// used to filter the output of certain specific fields
func matchRe(re *regexp.Regexp, objName string) bool {
	// nil (unspecified) regex matches everything
	if re == nil || re.MatchString(objName) {
		return true
	}
	return false
}

// findRe returns true if the object matches any of the specified match labels
func matchLabel(matchLabels map[string]string, objLabels map[string]string) bool {
	// nil (unspecfied) context label matches everything
	if matchLabels == nil || len(matchLabels) == 0 {
		return true
	}

	// nil object labels doesn't match anything
	if objLabels == nil {
		return false
	}

	for key, label := range matchLabels {
		if objLabel, ok := objLabels[key]; ok && objLabel == label {
			return true
		}
	}

	return false
}

// lookForJSON looks for a json record in the given (random output) and decodes in the provide object
func lookForJSON(inp string, obj interface{}) error {
	jsonData, endIdx := findJSONRecord(inp, 0)
	if jsonData == "" || endIdx <= 0 {
		return fmt.Errorf("unable to find json struct --%s-- '%s' idx %d", inp, jsonData, endIdx)
	}

	return json.Unmarshal([]byte(jsonData), obj)
}

// isJSON returns true if the given string contains a json record
func isJSON(inp string) bool {
	jsonRec, endIdx := findJSONRecord(inp, 0)
	return jsonRec != "" && endIdx > 0
}

// findJSONRecord looks for matching braces for a json string
// the purpose of this function is not to validate the json syntax,
// rather use it to find the boundaries to be given to json decoder
func findJSONRecord(inp string, beginIdx int) (string, int) {
	depth := 0
	recBegin := -1
	for ii := beginIdx; ii < len(inp); ii++ {
		if inp[ii] == '{' {
			if depth == 0 {
				recBegin = ii
			}
			depth++
		} else if recBegin >= 0 && inp[ii] == '}' {
			depth--
			if depth == 0 {
				return inp[recBegin : ii+1], ii + 1
			}
		}
	}

	return "", -1
}

// findYmlRecord looks for a yml separator for various records
// and returns first record starting the the specified line (beginning)
func findYmlRecord(inp string, recBegin int) (string, int) {
	lines := strings.Split(inp, "\n")
	for idx, line := range lines[recBegin:] {
		if line == "---" || idx == len(lines)-1 {
			yamlRec := strings.Join(lines[recBegin:idx], "\n")
			if jsonRec, err := yaml.YAMLToJSON([]byte(yamlRec)); err == nil {
				return string(jsonRec), idx
			}
		}
	}
	return "", -1
}

// returns an empty map (i.e. all fields set to false)
func getEmptyMap(fields []string) map[string]bool {
	fmap := make(map[string]bool)
	for _, field := range fields {
		fmap[field] = false
	}

	return fmap
}

// returns true if a map is all true
func isMapFull(fmap map[string]bool) bool {
	for _, v := range fmap {
		if v == false {
			return false
		}
	}
	return true
}

// matches all specified set of words in any line of the output
func matchLineFields(out string, fields []string) bool {
	lines := strings.Split(out, "\n")
	for _, line := range lines {
		words := strings.Split(line, " ")
		fmap := getEmptyMap(fields)
		for _, word := range words {
			word = strings.Trim(word, "\t ")
			if _, ok := fmap[word]; ok {
				fmap[word] = true
			}
		}
		if isMapFull(fmap) {
			return true
		}
	}

	return false
}

// matches specified lines (space trimmed) among any lines of the output
func matchLines(out string, fields []string) bool {
	fmap := getEmptyMap(fields)
	lines := strings.Split(out, "\n")
	for _, line := range lines {
		line = strings.Trim(line, "\n \t")
		if _, ok := fmap[line]; ok {
			fmap[line] = true
		}
	}
	return isMapFull(fmap)
}

// prints specified rows and columns in a metrics series
func printSeries(ioWriter io.Writer, rows int, cols []string, series *telemetryclient.MetricsResultSeries) {
	tw := tabwriter.NewWriter(ioWriter, 0, 2, 2, ' ', 0)
	defer tw.Flush()

	skip := make(map[int]bool)

	// print the header from the column names
	for colIdx, colName := range series.Columns {
		skip[colIdx] = false
		if colName != "time" && len(cols) > 0 && !matchSubString(cols, colName) {
			skip[colIdx] = true
			continue
		}
		fmt.Fprintf(tw, "%s", colName)

		if colIdx < len(series.Columns)-1 {
			fmt.Fprintf(tw, "\t")
		}
	}
	fmt.Fprintf(tw, "\n")

	for colIdx, colName := range series.Columns {
		if skip[colIdx] {
			continue
		}
		fmt.Fprintf(tw, "%s\t", getHdrUL(len(colName)))
	}
	fmt.Fprintf(tw, "\n")

	// values themselves are a set of rows each indicating valu corresponding to the columns
	for rowIdx, colValue := range series.Values {
		if rows > 0 && rowIdx > rows {
			return
		}
		for colIdx, colField := range colValue {
			if skip[colIdx] {
				continue
			}
			fmt.Fprintf(tw, "%v", colField)

			if colIdx < len(colValue)-1 {
				fmt.Fprintf(tw, "\t")
			}
		}
		fmt.Fprintf(tw, "\n")
	}
	fmt.Fprintf(tw, "\n")
}

// metrisQuery polls backend for a given table name, using provided token
func metricsQuery(server, tableName, token string) (*telemetryclient.MetricsQueryResponse, error) {
	tc, err := telemetryclient.NewTelemetryClient(server)
	if err != nil {
		return &telemetryclient.MetricsQueryResponse{}, err
	}

	metricQuery := &tqclient.MetricsQueryList{
		Tenant:    globals.DefaultTenant,
		Namespace: globals.DefaultNamespace,
		Queries: []*tqclient.MetricsQuerySpec{
			&tqclient.MetricsQuerySpec{
				TypeMeta: api.TypeMeta{
					Kind: tableName,
				},
			},
		},
	}
	loginCtx := loginctx.NewContextWithAuthzHeader(goContext.Background(), "Bearer "+token)
	resp, err := tc.Metrics(loginCtx, metricQuery)

	return resp, err
}
