%define kernel %(uname -r)
%define debug_package %{nil}

Name:		ionic
Version:	0.0.0
Release:	1%{?dist}
Summary:	Pensando IONIC Driver

#Group:		
License:	GPLv2
#URL:		
Source:		ionic-0.0.0.tar.gz

BuildRequires:	kernel-devel
#Requires:	

%description
Pensando IONIC Driver

%prep

%setup -q

%build

%install
mkdir -p %{buildroot}/etc/modules-load.d
mkdir -p %{buildroot}/lib/modules/%{kernel}/kernel/drivers/net
install -m 644 ionic.conf %{buildroot}/etc/modules-load.d
install -m 644 ionic.ko %{buildroot}/lib/modules/%{kernel}/kernel/drivers/net

%files
%defattr(0644, root, -)
/etc/modules-load.d/ionic.conf
/lib/modules/%{kernel}/kernel/drivers/net/ionic.ko

%post
depmod -a
modprobe ionic

%postun
rmmod ionic

%changelog
