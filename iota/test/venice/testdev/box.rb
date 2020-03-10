from "registry.test.pensando.io:5000/pensando/nic:1.41"

# docker in docker
copy "tools/test-build/dind", "/dind"
run "chmod +x /dind"

copy "venice/entrypoint.sh", "/entrypoint.sh"
run "chmod +x /entrypoint.sh"

entrypoint "/dind"

cmd %w[make]
