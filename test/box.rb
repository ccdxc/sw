import "box-base.rb" # references repo root (sw/) box-base.rb

# docker in docker
copy "tools/test-build/dind", "/dind"
run "chmod +x /dind"

copy "test/entrypoint.sh", "/entrypoint.sh"
run "chmod +x /entrypoint.sh"

entrypoint "/dind"

cmd %w[/bin/bash]
