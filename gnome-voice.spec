Name:           gnome-voice
Version:        48.0
Release:        1%{?dist}
Summary:        Voice for GNOME 48
License:        GPLv3+
URL:            https://www.gnomevoice.org
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
Voice is a Free Software program that allows you easily
participate on Free Public Voice broadcasts under GNOME 45.

Voice is developed on the GNOME desktop platform and
it requires GTK+ 3.0, Clutter and GStreamer 1.0 for playback.

Enjoy Free Public Voice broadcasts from
$HOME/Desktop/{$USERNAME}.voice and recordings to
$HOME/Desktop/{$USERNAME}.ogg under GNOME 45.

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
%{_datadir}/%{name}/gnome-voice.xml
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/scalable/apps/gnome-voice.svg

%changelog
* Thu Jan 23 2025 Ole Aamot <ole@aamot.org> - 48.0-1
- gnome-voice 48 build on Fedora Rawhide (Voice 1.0)
