%define __ext_name timecop
Name:		php-%{__ext_name}
Version:	1.0.4
Release:	1%{?dist}
Summary:	php-timecop module

Group:		Development/Languages
License:	MIT
URL:		https://github.com/hnw/php-timecop
# git archive --format=tar --prefix=php-timecop-1.0.4/ v1.0.4 | gzip > php-timecop-1.0.4.tar.gz
Source0:	php-timecop-%{version}.tar.gz
BuildRoot:	%(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)

BuildRequires:	php, php-devel
Requires:	php
Requires:	php(api) = %{php_core_api}

%description
A PHP extension providing "time travel" and "time freezing" capabilities, 
inspired by ruby timecop gem(https://github.com/travisjeffery/timecop).

%prep
%setup -q
cat > %{__ext_name}.ini << 'EOF'
; Enable %{__ext_name} extension module
extension = %{__ext_name}.so

EOF

%build
phpize
%configure
make %{?_smp_mflags}


%install
rm -rf %{buildroot}
make install INSTALL_ROOT=%{buildroot}
install -D -m 644 %{__ext_name}.ini %{buildroot}%{_sysconfdir}/php.d/%{__ext_name}.ini


%clean
rm -rf %{buildroot}


%files
%defattr(-,root,root,-)
%config(noreplace) %{_sysconfdir}/php.d/%{__ext_name}.ini
%{php_extdir}/%{__ext_name}.so




%changelog
* Wed Jun 11 2014 withgod <noname@withgod.jp>
- created
