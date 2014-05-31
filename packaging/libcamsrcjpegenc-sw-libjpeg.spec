Name:       libcamsrcjpegenc-sw-libjpeg
Summary:    Multimedia Framework Camera Src Jpeg Encoder Library (libjpeg) (unstripped)
Version:    0.1.14
Release:    0
Group:      libdevel
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(camsrcjpegenc)
BuildRequires:  pkgconfig(mmutil-imgp)
BuildRequires:  libjpeg-turbo-devel

%description
Multimedia Framework Camera Src Jpeg Encoder Library (libjpeg)

%prep
%setup -q -n %{name}-%{version}

%build
%if 0%{?sec_build_binary_debug_enable}
export CFLAGS+=" -DTIZEN_DEBUG_ENABLE"
%endif
./autogen.sh
%configure --disable-static --enable-dlog
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/usr/share/license
cp LICENSE %{buildroot}/usr/share/license/%{name}
%make_install

%files
%manifest libcamsrcjpegenc-sw-libjpeg.manifest
%defattr(-,root,root,-)
%{_libdir}/libcamsrcjpegenc-sw.so*
%{_datadir}/license/%{name}

