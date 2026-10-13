# ==============================================================================
# Epistola Pauli, Petri et Joannis ad Sanctos: Pure Build and Tooling Pipeline
# St. Paul the Apostle, Tentmaker & Build Engineer
# St. Peter the Apostle, Master Builder & Core Developer
# St. John the Baptist, Patron of Releases & Packaging Engineer
# "Make straight the path of the Lord; seal and deploy the righteous work."
# ==============================================================================

SHELL := /bin/bash

# Ensure local user bin is included in PATH for packaging tools (e.g., dpkg-deb)
export PATH := $(HOME)/.local/bin:$(PATH)

# Compiler and Standard Diagnostic Flags
CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -Werror
INCLUDES ?= -Iengine

# Production compilation flags (optimized and release-ready)
PROD_CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -Werror -O3 -DPRODUCTION

# Local user library path detection for containerized/user environments
LOCAL_LIB64 := $(wildcard $(HOME)/.local/lib64)
ifneq ($(LOCAL_LIB64),)
    LOCAL_SANITIZE_LDFLAGS := -L$(HOME)/.local/lib64 -Wl,-rpath,$(HOME)/.local/lib64
endif

SANITIZE_FLAGS ?= -fsanitize=address,undefined -g $(LOCAL_SANITIZE_LDFLAGS)

# Directories and Binary Artifacts
BIN_DIR := bin
TEST_PARSER_SRC := tests/test_parser.cpp
TEST_PARSER_BIN := $(BIN_DIR)/test_parser
SANITIZE_PARSER_BIN := $(BIN_DIR)/test_parser_sanitize

TEST_IO_SRC := tests/test_io.cpp
TEST_IO_BIN := $(BIN_DIR)/test_io
SANITIZE_IO_BIN := $(BIN_DIR)/test_io_sanitize
TEST_IO_PROD_BIN := $(BIN_DIR)/test_io_prod

CLI_SRC := src/cli/main.cpp
CLI_BIN := $(BIN_DIR)/fictive-ai

# Release Packaging Configuration
VERSION ?= 1.0.0
RELEASE_DIR := release
DEB_PACKAGE := $(RELEASE_DIR)/fictive-ai_$(VERSION)_amd64.deb
RPM_PACKAGE := $(RELEASE_DIR)/fictive-ai-$(VERSION)-1.x86_64.rpm
DEB_STAGE_DIR := $(RELEASE_DIR)/debbuild
RPM_TOP_DIR := $(RELEASE_DIR)/rpmbuild

# Prefix and installation paths
PREFIX ?= $(HOME)/.local
INSTALL_BIN := $(PREFIX)/bin
INSTALL_INC := $(PREFIX)/include/fictive-ai
INSTALL_SHARE := $(PREFIX)/share/fictive-ai/templates

# Phony Targets
.PHONY: all test sanitize clean clean-release check-deps test-parser test-io cli cli-prod install package package-deb package-rpm package-verify

# Target 'all': builds tests and CLI compiler
all: test cli

# Directory creation
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(RELEASE_DIR):
	mkdir -p $(RELEASE_DIR)

# Compilation rule for standalone CLI compiler binary (development)
$(CLI_BIN): $(CLI_SRC) src/cli/templates.hpp engine/parser.hpp engine/io.hpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(CLI_SRC) -o $(CLI_BIN)

# Target 'cli': compiles the standalone CLI binary
cli: $(CLI_BIN)

# Target 'cli-prod': compiles and strips the standalone CLI binary with production optimizations (-O3 -DPRODUCTION)
cli-prod: $(CLI_SRC) src/cli/templates.hpp engine/parser.hpp engine/io.hpp | $(BIN_DIR)
	$(CXX) $(PROD_CXXFLAGS) $(INCLUDES) $(CLI_SRC) -o $(CLI_BIN)
	strip $(CLI_BIN)

# Target 'install': installs binary and engine headers to user local environment
install: $(CLI_BIN)
	mkdir -p $(INSTALL_BIN)
	cp $(CLI_BIN) $(INSTALL_BIN)/fictive-ai
	mkdir -p $(INSTALL_INC)
	cp engine/parser.hpp engine/io.hpp $(INSTALL_INC)/
	mkdir -p $(INSTALL_SHARE)
	cp templates/* $(INSTALL_SHARE)/
	@echo "Fictive-AI CLI and engine headers installed successfully to $(INSTALL_BIN)/fictive-ai"


# Compilation rules for standard test binaries
$(TEST_PARSER_BIN): $(TEST_PARSER_SRC) engine/parser.hpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(TEST_PARSER_SRC) -o $(TEST_PARSER_BIN)

$(TEST_IO_BIN): $(TEST_IO_SRC) engine/io.hpp engine/parser.hpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(TEST_IO_SRC) -o $(TEST_IO_BIN)

$(TEST_IO_PROD_BIN): $(TEST_IO_SRC) engine/io.hpp engine/parser.hpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -DPRODUCTION $(INCLUDES) $(TEST_IO_SRC) -o $(TEST_IO_PROD_BIN)

# Target 'test-parser': compiles and executes test_parser
test-parser: $(TEST_PARSER_BIN)
	$(TEST_PARSER_BIN)

# Target 'test-io': compiles and executes test_io (both dev and prod modes)
test-io: $(TEST_IO_BIN) $(TEST_IO_PROD_BIN)
	$(TEST_IO_BIN)
	$(TEST_IO_PROD_BIN)

# Target 'test': runs complete test suite
test: test-parser test-io

# Compilation rules for sanitized test binaries
$(SANITIZE_PARSER_BIN): $(TEST_PARSER_SRC) engine/parser.hpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(SANITIZE_FLAGS) $(INCLUDES) $(TEST_PARSER_SRC) -o $(SANITIZE_PARSER_BIN)

$(SANITIZE_IO_BIN): $(TEST_IO_SRC) engine/io.hpp engine/parser.hpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(SANITIZE_FLAGS) $(INCLUDES) $(TEST_IO_SRC) -o $(SANITIZE_IO_BIN)

# Target 'sanitize': compiles with address/undefined sanitizers and runs tests
sanitize: $(SANITIZE_PARSER_BIN) $(SANITIZE_IO_BIN)
	$(SANITIZE_PARSER_BIN)
	$(SANITIZE_IO_BIN)

# Target 'check-deps': zero-dependency header isolation verification
check-deps: | $(BIN_DIR)
	@echo "Verifying zero-dependency header isolation for engine/parser.hpp..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -x c++ -c /dev/null -include engine/parser.hpp -o $(BIN_DIR)/check_deps_parser.o
	@rm -f $(BIN_DIR)/check_deps_parser.o
	@echo "Zero-dependency verification succeeded: parser.hpp is pure and self-contained."
	@echo "Verifying zero-dependency header isolation for engine/io.hpp..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -x c++ -c /dev/null -include engine/io.hpp -o $(BIN_DIR)/check_deps_io.o
	@rm -f $(BIN_DIR)/check_deps_io.o
	@echo "Zero-dependency verification succeeded: io.hpp is pure and self-contained."
	@echo "Verifying include compatibility for fictive-ai paths..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -x c++ -c /dev/null -include fictive-ai/parser.hpp -include fictive-ai/io.hpp -o $(BIN_DIR)/check_deps_paths.o
	@rm -f $(BIN_DIR)/check_deps_paths.o
	@echo "Include path verification succeeded: fictive-ai/ include aliases are rock-solid."

# Release packaging rules
$(DEB_PACKAGE): cli-prod packaging/debian/control engine/parser.hpp engine/io.hpp templates/style.css templates/index.html templates/main.cpp | $(RELEASE_DIR)
	@echo "Building Debian package $(DEB_PACKAGE)..."
	rm -rf $(DEB_STAGE_DIR)
	mkdir -p $(DEB_STAGE_DIR)/DEBIAN
	mkdir -p $(DEB_STAGE_DIR)/usr/bin
	mkdir -p $(DEB_STAGE_DIR)/usr/include/fictive-ai
	mkdir -p $(DEB_STAGE_DIR)/usr/share/fictive-ai/templates
	cp packaging/debian/control $(DEB_STAGE_DIR)/DEBIAN/control
	chmod 0755 $(DEB_STAGE_DIR)/DEBIAN
	chmod 0644 $(DEB_STAGE_DIR)/DEBIAN/control
	install -m 0755 $(CLI_BIN) $(DEB_STAGE_DIR)/usr/bin/fictive-ai
	install -m 0644 engine/parser.hpp $(DEB_STAGE_DIR)/usr/include/fictive-ai/parser.hpp
	install -m 0644 engine/io.hpp $(DEB_STAGE_DIR)/usr/include/fictive-ai/io.hpp
	install -m 0644 templates/style.css $(DEB_STAGE_DIR)/usr/share/fictive-ai/templates/style.css
	install -m 0644 templates/index.html $(DEB_STAGE_DIR)/usr/share/fictive-ai/templates/index.html
	install -m 0644 templates/main.cpp $(DEB_STAGE_DIR)/usr/share/fictive-ai/templates/main.cpp
	dpkg-deb --root-owner-group --build $(DEB_STAGE_DIR) $(DEB_PACKAGE)
	rm -rf $(DEB_STAGE_DIR)
	@echo "Debian package created: $(DEB_PACKAGE)"

$(RPM_PACKAGE): cli-prod packaging/rpm/fictive-ai.spec engine/parser.hpp engine/io.hpp templates/style.css templates/index.html templates/main.cpp | $(RELEASE_DIR)
	@echo "Building RPM package $(RPM_PACKAGE)..."
	rm -rf $(RPM_TOP_DIR)
	mkdir -p $(RPM_TOP_DIR)/{BUILD,RPMS,SOURCES,SPECS,SRPMS,BUILDROOT}
	rpmbuild -bb \
		--define "_topdir $(CURDIR)/$(RPM_TOP_DIR)" \
		--define "_rpmdir $(CURDIR)/$(RELEASE_DIR)" \
		--define "_rpmfilename %%{NAME}-%%{VERSION}-%%{RELEASE}.%%{ARCH}.rpm" \
		--define "SOURCE_DIR $(CURDIR)" \
		packaging/rpm/fictive-ai.spec
	rm -rf $(RPM_TOP_DIR)
	@echo "RPM package created: $(RPM_PACKAGE)"

package-deb: $(DEB_PACKAGE)

package-rpm: $(RPM_PACKAGE)

# Verification rule for inspect-checking both packages
package-verify: $(DEB_PACKAGE) $(RPM_PACKAGE)
	@echo "=============================================================================="
	@echo "Inspecting Debian Package: $(DEB_PACKAGE)"
	@echo "=============================================================================="
	dpkg-deb -c $(DEB_PACKAGE)
	@echo ""
	dpkg-deb -I $(DEB_PACKAGE)
	@echo ""
	@echo "=============================================================================="
	@echo "Inspecting RPM Package: $(RPM_PACKAGE)"
	@echo "=============================================================================="
	rpm -qpl $(RPM_PACKAGE)
	@echo ""
	rpm -qpi $(RPM_PACKAGE)

# Target 'package': builds production binary, builds .deb & .rpm, and verifies both
package: $(DEB_PACKAGE) $(RPM_PACKAGE) package-verify
	@echo ""
	@echo "=============================================================================="
	@echo "Rejoice! Both Fictive-AI packages are sealed, verified, and ready for release:"
	@echo "  Debian: $(DEB_PACKAGE)"
	@echo "  RPM:    $(RPM_PACKAGE)"
	@echo "=============================================================================="

# Target 'clean': removes bin/ directory and build staging artifacts
clean:
	rm -rf $(BIN_DIR) *.o $(DEB_STAGE_DIR) $(RPM_TOP_DIR)

# Target 'clean-release': removes entire release directory
clean-release: clean
	rm -rf $(RELEASE_DIR)
