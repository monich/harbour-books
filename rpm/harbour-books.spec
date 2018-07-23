Name:           harbour-books
Summary:        E-book reader
Version:        1.0.35
Release:        1
License:        BSD
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
%{_datadir}/icons/hicolor/*/apps/%{name}.png

%check
make -C test test

%changelog
* Tue Jun 24 2018 Slava Monich <slava.monich@jolla.com> 1.0.35
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

* Sun Feb 26 2018 Slava Monich <slava.monich@jolla.com> 1.0.26
- Fixed compatibility with Sailfish OS < 2.1.0
- Added Hungarian translations

* Sun Feb 4 2018 Slava Monich <slava.monich@jolla.com> 1.0.25
- Speed up book loading by caching page marks
- Added Dutch and Spanish translations

* Sat Jan 20 2018 Slava Monich <slava.monich@jolla.com> 1.0.24
- Haptic feedback when entering/leaving selection mode
- Added "Copied to clipboard" pop-up notification
- Corrected page stack behaviour
- Fixed FB2 footnotes

* Tue Sep 07 2017 Slava Monich <slava.monich@jolla.com> 1.0.23
- Copy text to clipboard on long press
- Added "Select all" function to the import view
- Fixed the behavior of the position stack
- Improved Swedish translations

* Tue Aug 08 2017 Slava Monich <slava.monich@jolla.com> 1.0.22
- Turn pages with volume keys

* Thu Aug 03 2017 Slava Monich <slava.monich@jolla.com> 1.0.21
- Implemented position stack (history)

* Sat Jan 28 2017 Slava Monich <slava.monich@jolla.com> 1.0.20
- Ignore images with display:none

* Fri Jan 20 2017 Slava Monich <slava.monich@jolla.com> 1.0.19
- Fixed a crash in CSS parser

* Wed Nov 02 2016 Slava Monich <slava.monich@jolla.com> 1.0.18
- Added support for footnotes in epub books

* Mon Oct 24 2016 Slava Monich <slava.monich@jolla.com> 1.0.17
- Updated Swedish translations

* Sun Oct 23 2016 Slava Monich <slava.monich@jolla.com> 1.0.16
- Made books folder on the memory card configurable
- Added support for internal hyperlinks
- Improved link detection (on long press)

* Sun Oct 09 2016 Slava Monich <slava.monich@jolla.com> 1.0.15
- Fixed the problem with wrong images popping up in zoom view

* Wed Oct 08 2016 Slava Monich <slava.monich@jolla.com> 1.0.14
- Improved image zoom transitions

* Wed Oct 08 2016 Slava Monich <slava.monich@jolla.com> 1.0.13
- Implemented long press actions (open links in brower, zoom images)
- Added settings page

* Wed Jul 28 2016 Slava Monich <slava.monich@jolla.com> 1.0.12
- Minor UI tweaks (wrapping of "No books" label)
- Fixed license in rpm spec

* Wed Jul 27 2016 Slava Monich <slava.monich@jolla.com> 1.0.11
- German translations
- Current folder is restored at startup
- Better support for devices with different screen DPI

* Sun Feb 21 2016 Slava Monich <slava.monich@jolla.com> 1.0.10
- Per-book text size
- Support for XML entities
- Fixed the problem with styleless XHTML
- Miscellaneous tweaks and UI improvements

* Mon Nov 2 2015 Slava Monich <slava.monich@jolla.com> 1.0.9
- Workaround for Qt crash

* Wed Oct 21 2015 Slava Monich <slava.monich@jolla.com> 1.0.8
- Fixed the cancel button problem

* Thu Oct 15 2015 Slava Monich <slava.monich@jolla.com> 1.0.7
- First official build for Jolla tablet
- Moved font controls from pulley menu to the toolbar
- Fixed a few EPUB rendering and decoding issues
- Made it easier to cancel book loading
- Plugged a few memory leaks

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
