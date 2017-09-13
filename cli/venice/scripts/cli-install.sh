#!/bin/sh

cd $GOPATH/src/github.com/pensando/sw/

if [ -d /etc/bash_completion.d ]
then 
	sudo cp ${PWD}/cli/venice/bash_autocomplete /etc/bash_completion.d/venice
else
	sudo cp ${PWD}/cli/venice/bash_autocomplete /usr/local/etc/bash_completion.d/
fi

echo "On linux system: source /etc/bash_completion.d/venice"
