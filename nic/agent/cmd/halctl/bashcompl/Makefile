default: halctl-bash-completion

halctl-bash-completion:
	go install
	bashcompl
	cp out.sh /etc/bash_completion.d/halctl_bash_complete.sh
	source /etc/bash_completion.d/halctl_bash_complete.sh
