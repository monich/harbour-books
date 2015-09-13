Name:           harbour-books
Summary:        E-book reader
Version:        1.0.6
Release:        1
License:        GPL
Group:          Applications/File
URL:            http://github.com/monich/harbour-books
Source0:        %{name}-%{version}.tar.gz

Requires:       sailfishsilica-qt5
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(sailfishapp)
BuildRequires:  pkgconfig(Qt5Quick)
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Svg)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(mlite5)
BuildRequires:  pkgconfig(expat)
BuildRequires:  pkgconfig(systemd)
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
%qtc_qmake5 harbour-books.pro
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
%{_datadir}/icons/hicolor/86x86/apps/%{name}.png

%changelog
* Sun Aug 9 2015 Slava Monich <slava.monich@jolla.com> 1.0.6
- Improved rendering of EPUB books
- Fixed saving the shelf position for zipped books

* Sat Jul 18 2015 Slava Monich <slava.monich@jolla.com> 1.0.5
- Added support for zipped books
- Improved performance of scanning for new books
- Fixed a problem with some EPUBs being ignored

* Sun Jul 12 2015 Slava Monich <slava.monich@jolla.com> 1.0.4
- Implemented "night mode", switched by double-click
- Added left swipe hint
- Finnish and Swedish translations
- Fixed a bug with current page getting reset after rotation

* Tue Jul 7 2015 Slava Monich <slava.monich@jolla.com> 1.0.3
- Improved CSS support and XHTML formatting

* Mon Jul 6 2015 Slava Monich <slava.monich@jolla.com> 1.0.2
- Replaced Books with Documents/Books for internal storage

* Fri Jul 3 2015 Slava Monich <slava.monich@jolla.com> 1.0.1
- Improved recovery from infinite formatting loops
- Fixed one infinite formatting loop

* Sun Jun 28 2015 Slava Monich <slava.monich@jolla.com> 1.0.0
- Initial version
