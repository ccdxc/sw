#!/bin/sh

# this script is run by customer after extracting the image.
# this is supposed to install all the venice components in the system
for i in tars/pen* ; do docker load -i  $i; done
docker run --rm --name pen-install -v /var/log/pensando:/var/log/pensando -v /usr/pensando/bin:/host/usr/pensando/bin -v /usr/lib/systemd/system:/host/usr/lib/systemd/system -v /etc/pensando:/host/etc/pensando pen-install -c /initscript

if [ "$(id -u)" != "0" ]
then
    SUDO="sudo"
fi

${SUDO} systemctl daemon-reload
${SUDO} systemctl enable pensando.target
${SUDO} systemctl start pensando.target
${SUDO} systemctl enable pen-cmd
${SUDO} systemctl start pen-cmd
