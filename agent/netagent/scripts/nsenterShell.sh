sudo nsenter -t `sudo docker inspect --format "{{.State.Pid}}" $1` -n -F -- sh
