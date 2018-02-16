import "venice/ui/box-build/constants.rb"

from IMAGE

run "echo 'deb http://dl.google.com/linux/chrome/deb/ stable main' >> /etc/apt/sources.list"
run "wget https://dl-ssl.google.com/linux/linux_signing_key.pub"
run "apt-key add linux_signing_key.pub"
run "apt-get update"
run "apt-get install -y google-chrome-stable"