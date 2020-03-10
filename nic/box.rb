from "registry.test.pensando.io:5000/pensando/nic:1.41"

user = getenv("USER")
group = getenv("GROUP_NAME")
uid = getenv("USER_UID")
gid = getenv("USER_GID")

if user != ""
  # remove the games group as it conflicts with staff group for mac users
  run "groupdel games"

  # add user
  run "groupadd -g #{gid} #{group}"
  run "useradd -l -u #{uid} -g #{gid} #{user} -G docker"

  # go installs in /usr, make it world writeable
  run "chmod 777 /usr/bin"

  # update user .bash_profile
  run "echo 'export GOPATH=/usr' >> /home/#{user}/.bash_profile"
  run "echo 'export PATH=/usr/local/go/bin:$PATH' >> /home/#{user}/.bash_profile"
  run "echo 'sudo chown -R #{user} /sw/' >> /home/#{user}/.bash_profile"
  run "echo 'sudo chgrp -R #{user} /sw/' >> /home/#{user}/.bash_profile"
  run "echo 'cd /usr/src/github.com/pensando/sw/nic' >> /home/#{user}/.bash_profile"
  run "echo 'Defaults secure_path = /usr/local/go/bin:$PATH:/bin:/usr/sbin/' >> /etc/sudoers"

  run "echo '#{user} ALL=(root) NOPASSWD:ALL' > /etc/sudoers.d/#{user} && chmod 0440 /etc/sudoers.d/#{user}"

  run "localedef -i en_US -f UTF-8 en_US.UTF-8"
end

env GOPATH: "/usr"

inside "/etc" do
  run "rm localtime"
  run "ln -s /usr/share/zoneinfo/US/Pacific localtime"
end

workdir "/sw"

copy "nic/entrypoint.sh", "/entrypoint.sh"
run "chmod +x /entrypoint.sh"

entrypoint "/entrypoint.sh"
