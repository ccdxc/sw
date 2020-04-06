package main

import (
	"context"
	"encoding/json"
	"flag"
	"fmt"
	"io/ioutil"
	"net"
	"os"
	"time"

	"github.com/pensando/sw/venice/utils/log"

	flowutils "github.com/calmh/ipfix"

	"github.com/pensando/sw/venice/utils/ipfix"
	"github.com/pensando/sw/venice/utils/ipfix/server"
)

func main() {
	f := flag.String("f", "", "decode from file")
	t := flag.Bool("t", false, "scapy templates")
	e := flag.Bool("e", false, "enterprise elements")
	j := flag.Bool("json", false, "templates in json format")
	v := flag.Bool("v", false, "template details")
	s := flag.String("s", "", "send templates to ip:port")
	flag.Parse()

	file := *f
	genTemplate := *t
	jsonfmt := *j
	addr := *s
	ee := *e
	verbose := *v

	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	switch {
	case file != "":
		decodeFromFile(file)

	case genTemplate != false:
		scapyTemplates(jsonfmt)

	case verbose != false:
		showTemplates()

	case ee != false:
		enterpriseElements()

	case addr != "":
		sendTemplates(addr)

	default:
		startServer(ctx)
	}

}

func enterpriseElements() {
	ee := ipfix.GenerateEnterpriseElements()
	for _, e := range ee {
		fmt.Printf("name:\t\t%v \n", e.Name)
		fmt.Printf("type:\t\t%v\n", getFieldType(e.Type))
		fmt.Printf("field-id:\t%v\n", e.FieldID)
		fmt.Printf("enterprise-id:\t%v\n\n", e.EnterpriseID)
	}
}

func decodeFromFile(fname string) {
	fd, err := os.Open(fname)
	if err != nil {
		panic("failed to open file, error " + err.Error())
	}
	buff, err := ioutil.ReadAll(fd)
	if err != nil {
		panic("failed to read file, error " + err.Error())
	}

	m := decode(buff)
	for i := range m {
		fmt.Printf("got %+v \n", m[i])
	}
}

func sendTemplates(addr string) {
	conn, err := net.Dial("udp", addr)
	if err != nil {
		fmt.Printf("%v\n", err)
	}
	defer conn.Close()

	tmplt, err := ipfix.CreateTemplateMsg()
	if err != nil {
		fmt.Printf("%v\n", err)
	}
	for {
		if _, err := conn.Write(tmplt); err != nil {
			fmt.Printf("fail, %v", err)
			return
		}
		time.Sleep(time.Second)
	}
}

func startServer(ctx context.Context) {
	udpaddr, udpch, err := server.NewServer(ctx, ":0")
	if err != nil {
		panic("failed to start ipfix udp server " + err.Error())
	}
	fmt.Printf("starting udp server %v\n", udpaddr)
	for {
		m := <-udpch
		fmt.Printf("udp://%v: %+v \n"+
			"", udpaddr, m)
	}
}

// decode decodes ipfix message, only for tests
func decode(buff []byte) []flowutils.InterpretedField {
	session := flowutils.NewSession()
	session.LoadTemplateRecords(ipfix.GeneratePensandoTemplates())
	i := flowutils.NewInterpreter(session)
	entry := ipfix.GenerateEnterpriseElements()

	for d := range entry {
		i.AddDictionaryEntry(entry[d])
	}

	ds, err := session.ParseBuffer(buff[:])
	if err != nil {
		log.Errorf("ipfix parse error, %v", err)
		return nil
	}

	log.Infof("+++ got %v", ds)

	var fieldList []flowutils.InterpretedField
	for _, rec := range ds.DataRecords {
		log.Infof("++ %v \n", rec)
		fieldList = i.InterpretInto(rec, fieldList[:cap(fieldList)])
		// handle the field list
	}

	return fieldList
}

func showTemplates() {
	session := flowutils.NewSession()
	session.LoadTemplateRecords(ipfix.GeneratePensandoTemplates())
	i := flowutils.NewInterpreter(session)
	ee := ipfix.GenerateEnterpriseElements()
	for d := range ee {
		i.AddDictionaryEntry(ee[d])
	}

	templates := ipfix.GeneratePensandoTemplates()
	for _, t := range templates {
		fmt.Printf("template-id: %v \n", t.TemplateID)
		data := i.InterpretTemplate(t)
		for i, d := range data {
			fmt.Printf("%v \t name:%v, FieldID:%v, EnterpriseID:%v, length:%v\n",
				i+1, d.Name, d.FieldID, d.EnterpriseID, d.Length)
		}
		fmt.Println()
	}
}

// scapyTemplates to match with scapy, only for tests
func scapyTemplates(jsonfmt bool) {
	session := flowutils.NewSession()
	session.LoadTemplateRecords(ipfix.GeneratePensandoTemplates())
	i := flowutils.NewInterpreter(session)
	ee := ipfix.GenerateEnterpriseElements()
	for d := range ee {
		i.AddDictionaryEntry(ee[d])
	}

	if jsonfmt {
		templates := session.ExportTemplateRecords()
		jsonData, err := json.Marshal(&templates)
		if err != nil {
			log.Errorf("failed to parse %+v", templates)
			return
		}
		fmt.Printf("%v", string(jsonData))
		return
	}

	templates := ipfix.GeneratePensandoTemplates()
	for _, t := range templates {
		fmt.Printf("template-id: %v \n", t.TemplateID)
		data := i.InterpretTemplate(t)
		for i := range data {
			if data[i].EnterpriseID == 0 {
				fmt.Printf("BitField(%v, 0, %v) field-id:%v \n", data[i].Name, data[i].Length*8, data[i].FieldID)
			} else {
				fmt.Printf("BitField(%v, 0, %v) field-id:%v  enterprise-id:%v \n", data[i].Name, data[i].Length*8, data[i].FieldID, data[i].EnterpriseID)
			}
		}
		fmt.Println()

	}
}

func getFieldType(t flowutils.FieldType) string {
	for k, v := range flowutils.FieldTypes {
		if t == v {
			return k
		}
	}
	return ""
}
