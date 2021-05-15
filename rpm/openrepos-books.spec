Name:           openrepos-books
Summary:        E-book reader
Version:        1.0.43
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

%prep
%setup -q -n %{name}-%{version}

%build
%qtc_qmake5 CONFIG+=openrepos CONFIG+=app_settings harbour-books.pro
%qtc_make %{?_smp_mflags}

%install
rm -rf %{buildroot}
cd app
%qmake5_install

desktop-file-install --delete-original \
  --dir %{buildroot}%{_datadir}/applications \
   %{buildroot}%{_datadir}/applications/*.desktop

%files
%defattr(-,root,root,-)
%{_bindir}/%{name}
%{_datadir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/*/apps/%{name}.png
%{_datadir}/translations/%{name}*.qm
%{_datadir}/jolla-settings/entries/%{name}.json

%check
make -C test test

%changelog
* Sat May 15 2021 Slava Monich <slava.monich@jolla.com> 1.0.43
- Integration with My Backup
- Updated Hungarian translation
- Updated Chinese translation
- Tweaked settings layout

* Tue Nov 3 2020 Slava Monich <slava.monich@jolla.com> 1.0.42
- Fixed detection of removable media on new fresh installs of Sailfish OS 3.4.0
- Implemented a fancy way of closing the book by swiping it up
- Optimized settings page for landscape orientation
- Made night mode brightness configurable
- Resolved a few issues with saving/restoring last page
- Eliminated unpleasant flicking when pages are being dragged

* Wed Feb 5 2020 Slava Monich <slava.monich@jolla.com> 1.0.41
- Fixed a problem with books opening at unexpected page
- Handle data: scheme for xhtml
- Various UI tweaks

* Wed Dec 4 2019 Slava Monich <slava.monich@jolla.com> 1.0.40
- Chinese translation

* Wed Dec 4 2019 Slava Monich <slava.monich@jolla.com> 1.0.39
- Fixed a permacrash

* Sun Mar 31 2019 Slava Monich <slava.monich@jolla.com> 1.0.38
- Fixed a problem with encoding in some format/language combinations
- Create sample book on the first time run
- Minor UI tweaks

* Fri Jul 27 2018 Slava Monich <slava.monich@jolla.com> 1.0.37
- Fixed a few CSS issues affecting layout

* Tue Jul 24 2018 Slava Monich <slava.monich@jolla.com> 1.0.36
- Updated Polish translations
- Updated Swedish translations

* Tue Jul 24 2018 Slava Monich <slava.monich@jolla.com> 1.0.35
- Added page layout option
- Added "turn page by tap" option

* Fri Jun 8 2018 Slava Monich <slava.monich@jolla.com> 1.0.34
- Support for SD-card labels containing spaces

* Fri Jun 8 2018 Slava Monich <slava.monich@jolla.com> 1.0.33
- Fixed SD-card support on Sailfish OS 2.2.0

* Mon May 21 2018 Slava Monich <slava.monich@jolla.com> 1.0.32
- Added Polish translations

* Mon May 21 2018 Slava Monich <slava.monich@jolla.com> 1.0.31
- Fixed "Keep display on while reading" on older systems

* Sat May 19 2018 Slava Monich <slava.monich@jolla.com> 1.0.30
- Added Brazilian Portuguese translations

* Sat May 19 2018 Slava Monich <slava.monich@jolla.com> 1.0.29
- Updated Swedish and Hungarian translations

* Thu May 17 2018 Slava Monich <slava.monich@jolla.com> 1.0.28
- Added "Keep display on while reading" option
- Fixed a few memory leaks

* Fri Apr 27 2018 Slava Monich <slava.monich@jolla.com> 1.0.27
- Added openrepos variant
