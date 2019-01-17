package libs

func GenPairs(content []string, count int) [][]string {
	var pairs [][]string
	for i := 0; i < len(content); i++ {
		for j := i + 1; j < len(content)-1; j++ {
			pairs = append(pairs, []string{content[i], content[j]})
			if len(pairs) == count {
				return pairs
			}
		}
	}
	return pairs
}
