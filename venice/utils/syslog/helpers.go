package syslog

import "fmt"

// converts structured data to a string
func (sd StrData) stringify() string {
	var str string
	if sd != nil {
		for sdKey, sdParam := range sd {
			str += fmt.Sprintf("[%s", sdKey)
			for k, v := range sdParam {
				str += fmt.Sprintf(" %s=\"%s\"", k, v)
			}
			str += fmt.Sprintf("]")
		}
	} else {
		str += "-"
	}

	return str
}
