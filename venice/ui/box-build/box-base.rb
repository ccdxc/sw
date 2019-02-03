from "node"

import "constants.rb"

run "npm install -g @angular/cli"

run "echo 'deb http://dl.google.com/linux/chrome/deb/ stable main' >> /etc/apt/sources.list"
run "wget https://dl-ssl.google.com/linux/linux_signing_key.pub"
run "apt-key add linux_signing_key.pub"
run "apt-get update"
run "apt-get install -y google-chrome-stable"



run "ln /usr/bin/google-chrome-stable /usr/bin/chrome"
run "chmod 755 /usr/bin/chrome"

workdir DIR

entrypoint nil

