%define _build_id_links none
%define debug_package %{nil}

Name:           fictive-ai
Version:        1.0.0
Release:        1
Summary:        Fictive-AI — Pure C++17 interactive fiction engine and universal compiler toolchain
License:        MIT
Packager:       Our Lady, Mother Mary (Communion of Saints Framework)
BuildArch:      x86_64
URL:            https://github.com/communion-of-saints/fictive-ai

%description
Fictive-AI — A pure C++17 interactive fiction engine and universal compiler toolchain targeting Terminal, WebAssembly, and Desktop Webview.

%prep

%build

%install
mkdir -p %{buildroot}/usr/bin
mkdir -p %{buildroot}/usr/include/fictive-ai
mkdir -p %{buildroot}/usr/share/fictive-ai/templates

install -m 0755 %{SOURCE_DIR}/bin/fictive-ai %{buildroot}/usr/bin/fictive-ai
install -m 0644 %{SOURCE_DIR}/engine/parser.hpp %{buildroot}/usr/include/fictive-ai/parser.hpp
install -m 0644 %{SOURCE_DIR}/engine/io.hpp %{buildroot}/usr/include/fictive-ai/io.hpp
install -m 0644 %{SOURCE_DIR}/templates/style.css %{buildroot}/usr/share/fictive-ai/templates/style.css
install -m 0644 %{SOURCE_DIR}/templates/index.html %{buildroot}/usr/share/fictive-ai/templates/index.html
install -m 0644 %{SOURCE_DIR}/templates/main.cpp %{buildroot}/usr/share/fictive-ai/templates/main.cpp

%files
%defattr(-,root,root,-)
/usr/bin/fictive-ai
%dir /usr/include/fictive-ai
/usr/include/fictive-ai/parser.hpp
/usr/include/fictive-ai/io.hpp
%dir /usr/share/fictive-ai
%dir /usr/share/fictive-ai/templates
/usr/share/fictive-ai/templates/style.css
/usr/share/fictive-ai/templates/index.html
/usr/share/fictive-ai/templates/main.cpp

%changelog
* Tue Oct 13 2026 Our Lady, Mother Mary <mary@communion-of-saints.org> - 1.0.0-1
- Release of Fictive-AI 1.0.0 interactive fiction engine and universal compiler toolchain
