Name:           openrepos-books
Summary:        E-book reader
Version:        1.1.5
Release:        1
License:        BSD
Vendor:         slava
Group:          Applications/File
URL:            http://github.com/monich/harbour-books
Source0:        %{name}-%{version}.tar.gz

Requires:       sailfishsilica-qt5
Requires:       qt5-qtsvg-plugin-imageformat-svg
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(sailfishapp)
BuildRequires:  pkgconfig(Qt5Quick)
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Svg)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(mlite5)
BuildRequires:  pkgconfig(expat)
BuildRequires:  pkgconfig(libudev)
BuildRequires:  file-devel
BuildRequires:  bzip2-devel
BuildRequires:  desktop-file-utils
BuildRequires:  qt5-qttools-linguist

%{!?qtc_qmake5:%define qtc_qmake5 %qmake5}
%{!?qtc_make:%define qtc_make make}
%{?qtc_builddir:%define _builddir %qtc_builddir}

%description
FBReader-based e-book reader.

%if "%{?vendor}" == "chum"
Categories:
 - Office
Icon: https://raw.githubusercontent.com/monich/harbour-books/master/app/icons/harbour-books.svg
Screenshots:
- https://home.monich.net/chum/harbour-books/screenshots/screenshot-001.png
- https://home.monich.net/chum/harbour-books/screenshots/screenshot-002.png
- https://home.monich.net/chum/harbour-books/screenshots/screenshot-003.png
Url:
  Homepage: https://openrepos.net/content/slava/books
%endif

%prep
%setup -q -n %{name}-%{version}

%build
%qtc_qmake5 CONFIG+=openrepos CONFIG+=app_settings harbour-books.pro
%qtc_make %{?_smp_mflags}

%install
rm -rf %{buildroot}
cd app
%qmake5_install
cd settings
%qmake5_install

desktop-file-install --delete-original \
  --dir %{buildroot}%{_datadir}/applications \
   %{buildroot}%{_datadir}/applications/*.desktop

%check
make -C test test

%preun
if [ "$1" == 0 ] ; then \
  getent passwd | grep -v '/nologin$' | \
  while IFS=: read -r name pw uid gid comment home shell; \
  do rm -fr "$home/.local/share/openrepos-books"; done; fi || :

%files
%defattr(-,root,root,-)
%{_bindir}/%{name}
%{_datadir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/*/apps/%{name}.png
%{_datadir}/translations/%{name}*.qm
%{_datadir}/dbus-1/services/%{name}.service
%{_datadir}/jolla-settings/entries/%{name}.json
%{_libdir}/qt5/qml/openrepos/books/settings
