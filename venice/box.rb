import "box-base.rb"

# install docker
run "curl -sSL https://get.docker.com | CHANNEL=stable bash"
copy "venice/test-build/daemon.json", "/etc/docker/daemon.json"

copy "venice/test-build/dind", "/dind"
copy "venice/test-build/entrypoint.sh", "/entrypoint.sh"
run "chmod 755 /entrypoint.sh"
entrypoint "/dind"

cmd %w[make build unit-test-verbose]
