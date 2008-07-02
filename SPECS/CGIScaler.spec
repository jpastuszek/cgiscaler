Name:           CGIScaler
Version:        2.0.1
Release:        1%{?dist}
Summary:        ImageMagick based CGI image thumbnailer

Group:          Applications/Internet
License:        GPLv2+
URL:            http://sourceforge.net/projects/cgiscaler
Source0:        http://downloads.sourceforge.net/sourceforge/cgiscaler/%{name}-%{version}.tar.bz2
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:  ImageMagick-devel >= 6.3.5.9-1, cmake, gcc
Requires:       ImageMagick >= 6.3.5.9

%description
CGI C program that will serve thumbnail images to given format.
Supports caching, transparency removal and all ImageMagick
supported formats.

%prep
%setup -q


%build
%configure
%cmake . -DCMAKE_BUILD_TYPE=Release
make


%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT


%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,root,-)
/usr/libexec/cgiscaler

%doc /usr/share/doc/%{name}-%{version}/PERFORMANCE.html
%doc /usr/share/doc/%{name}-%{version}/INSTALL.html
%doc /usr/share/doc/%{name}-%{version}/RELEASE_NOTES
%doc /usr/share/doc/%{name}-%{version}/COPYING
%doc /usr/share/doc/%{name}-%{version}/README
%doc /usr/share/doc/%{name}-%{version}/TODO



%changelog
* Wed Jul 2 2008 Jakub Pastuszek <jpastuszek at gmail.com> - 2.0.1-1
- Initial RPM release.

