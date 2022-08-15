Name:           gnome-voice
Version:        0.0.9
Release:        1%{?dist}
Summary:        GNOME Voice for GNOME 43
License:        GPLv3+
URL:            https://wiki.gnome.org/Apps/Voice
Source:         %{url}/src/%{name}-%{version}.tar.xz

BuildRequires:  gcc
BuildRequires:  gtk3-devel
BuildRequires:  pango
BuildRequires:  libchamplain-devel
BuildRequires:  libxml2-devel
BuildRequires:  intltool
BuildRequires:  itstool
BuildRequires:  libappstream-glib
BuildRequires:  clutter-gtk
BuildRequires:  desktop-file-utils
BuildRequires:  geoclue2-devel
BuildRequires:  geocode-glib-devel
BuildRequires:  gstreamer1-devel
BuildRequires:  gstreamer1-plugins-bad-free-devel
BuildRequires:  gstreamer1-plugins-base-devel
Requires:       gstreamer1 >= 1.8.3
Requires:       gstreamer1-plugins-ugly-free >= 1.8.3
Requires:       geocode-glib >= 3.20.1
Requires:       gtk3 >= 3.24.28
Requires:       geoclue2 >= 2.5.7

%description
GNOME Voice is a Free Software program that allows you easily
participate on Free Public Voice broadcasts under GNOME 43.

GNOME Voice is developed on the GNOME desktop platform and
it requires GTK+ 3.0, Clutter and GStreamer 1.0 for playback.

Enjoy Free Public Voice broadcasts under GNOME 43.

%prep
%setup -q

%build
%configure --disable-silent-rules --disable-schemas
%make_build
%install
%make_install
%find_lang %{name}
%check
appstream-util validate-relax --nonet %{buildroot}%{_metainfodir}/%{name}.appdata.xml
desktop-file-validate %{buildroot}/%{_datadir}/applications/%{name}.desktop
%files -f %{name}.lang
%doc AUTHORS NEWS README ChangeLog
%license COPYING
%{_bindir}/%{name}
%{_metainfodir}/%{name}.appdata.xml
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/scalable/apps/gnome-voice.svg

%changelog
* Tue Aug 16 2022 Ole Aamot <ole@gnome.org> - 0.0.9-1
- gnome-voice 0.0.9 build on Fedora Linux 36

* Mon Aug 15 2022 Ole Aamot <ole@gnome.org> - 0.0.8-1
- gnome-voice 0.0.8 build on Fedora Linux 36

* Sat Jul 09 2022 Ole Aamot <ole@gnome.org> - 0.0.6-1
- gnome-voice 0.0.6 build on Fedora Linux 36

* Sat Jul 09 2022 Ole Aamot <ole@gnome.org> - 0.0.5-1
- gnome-voice 0.0.5 build on Fedora Linux 36

* Sat May 28 2022 Ole Aamot <ole@gnome.org> - 0.0.4-1
- gnome-voice 0.0.4 build on Fedora Linux 36

* Sat May 28 2022 Ole Aamot <ole@gnome.org> - 0.0.3-1
- gnome-voice 0.0.3 build on Fedora Linux 36

* Sun May 15 2022 Ole Aamot <ole@gnome.org> - 0.0.2-1
- gnome-voice 0.0.2 build on Fedora Linux 36

* Tue Feb 15 2022 Ole Aamot <ole@gnome.org> - 0.0.1-1
- gnome-voice 0.0.1 build on Fedora Linux 35
