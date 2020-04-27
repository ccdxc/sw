set -x

os_version=`awk -F= '$1=="VERSION_ID" { print $2 ;}' /etc/os-release | sed -e 's/\([678]\)\../\1/'`


if [ "$1" = "register" ]; then
	# register and add repos
	/usr/sbin/subscription-manager register --username rhel@pensando.io --password N0isystem$ --auto-attach
	
	# for RHEL 7	
        if [[ $os_version == *"7"* ]]; then
		subscription-manager repos --enable rhel-7-server-optional-rpms 
		subscription-manager repos --enable rhel-server-rhscl-7-rpms 
		subscription-manager repos --enable rhel-7-server-extras-rpms
		yum-config-manager --add-repo https://download.docker.com/linux/centos/docker-ce.repo

		yum install -y bind-utils net-tools wireshark

		yum install -y iperf3 vim sshpass sysfsutils net-tools libnl3-devel valgrind-devel hping3 lshw python-setuptools
		yum -y install elfutils-libelf-devel nmap
		yum -y install docker-ce container-selinux

		easy_install pip
		pip install pyyaml

		yum install -y rh-python36
		yum install -y rh-python36-setuptools
		ln -s /opt/rh/rh-python36/root/usr/bin/python3.6 /usr/bin/python3
		ln -s /opt/rh/rh-python36/root/usr/bin/pip3.6 /usr/bin/pip3
		#easy_install-3.6 pip
		pip3 install --upgrade pip
		pip3 install pyyaml

		# iperf
		rpm -ivh https://dl.fedoraproject.org/pub/epel/epel-release-latest-7.noarch.rpm
		yum -y install iperf
	fi
elif [ "$1" = "unregister" ]; then
	sudo subscription-manager remove --all
	sudo subscription-manager unregister
	sudo subscription-manager clean
fi
