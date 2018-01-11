import "box-base.rb"

# docker in docker
copy "tools/test-build/dind", "/dind"
run "chmod +x /dind"

copy "api/entrypoint.sh", "/entrypoint.sh"
run "chmod +x /entrypoint.sh"

entrypoint "/dind"

cmd %w[make build unit-test-verbose]
