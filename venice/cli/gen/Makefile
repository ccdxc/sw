default: ../genobjs.go ../testserver/tserver/genobjs.go ../gencmds.go ../testserver/tserver/tserver.go ./pregen/genobjs.go

# we use this % in one patten to match .go
# This rule implies that all these targets depend on all these dependencies and any changes of dependencies or any missing
# target needs one single command to run
../genobjs% ../testserver/tserver/genobjs% ../gencmds% ../testserver/tserver/tserver% ./pregen/genobjs%: \
	./templates/localObj.tmpl ./templates/cliObj.tmpl ./templates/cliObjOrder.tmpl ./templates/testServerObj.tmpl \
	./templates/cmds.tmpl ./templates/cmdFlags.tmpl ./templates/cmdBashCompleter.tmpl ./templates/testServer.tmpl
	../scripts/gen.sh
