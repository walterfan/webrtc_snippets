SHELL := /bin/bash
.DEFAULT_GOAL := help

MAKEFILE_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
REPO_ROOT := $(abspath $(MAKEFILE_DIR)/../..)
BUILD_DIR ?= $(REPO_ROOT)/build/basic
WEB_BUILD_DIR ?= $(REPO_ROOT)/build/basic-web
WEB_CONAN_DIR ?= $(REPO_ROOT)/build/conan
WEB_PORT ?= 8080
WEB_PID_FILE ?= $(REPO_ROOT)/build/practice-web.pid
WEB_LOG_FILE ?= $(REPO_ROOT)/build/practice-web.log
CMAKE ?= cmake
CTEST ?= ctest
DAY ?=

GENERATOR_FLAG := $(shell command -v ninja >/dev/null 2>&1 && echo -G Ninja)

.PHONY: help configure build build-all test list info run check \
	web-build build-web run-web start-web stop-web restart-web

help:
	@printf "Terminal exercises (incremental; only stale objects are compiled):\n"
	@printf "  make build          configure if needed, then build cpp_rtc_practice\n"
	@printf "  make build-all      also build practice_01 .. practice_30\n"
	@printf "  make test           incremental build of tests, then ctest\n"
	@printf "  make list           list exercises\n"
	@printf "  make info DAY=02    show one exercise\n"
	@printf "  make run DAY=02     build incrementally and run one exercise\n"
	@printf "  make check DAY=02   build incrementally and check one exercise\n"
	@printf "  make run-02         same as make run DAY=02\n\n"
	@printf "Website:\n"
	@printf "  make web-build      install Crow with Conan, then build the web app\n"
	@printf "  make build-web      alias for web-build\n"
	@printf "  make run-web        run in the foreground; press Ctrl-C to stop\n"
	@printf "  make start-web      build if needed and start on 127.0.0.1:$(WEB_PORT)\n"
	@printf "  make stop-web       stop the process recorded by the Makefile\n"
	@printf "  make restart-web    stop then start\n"

configure:
	@if [ ! -f "$(BUILD_DIR)/CMakeCache.txt" ]; then \
		$(CMAKE) -S "$(MAKEFILE_DIR)" -B "$(BUILD_DIR)" -DCMAKE_BUILD_TYPE=Debug $(GENERATOR_FLAG); \
	fi

build: configure
	$(CMAKE) --build "$(BUILD_DIR)" --target cpp_rtc_practice --parallel

build-all: configure
	$(CMAKE) --build "$(BUILD_DIR)" --target cpp_rtc_practice practice_standalone --parallel

test: configure
	$(CMAKE) --build "$(BUILD_DIR)" --target practice_framework_test practice_acceptance_test cpp_rtc_practice --parallel
	$(CTEST) --test-dir "$(BUILD_DIR)" --output-on-failure

list: build
	"$(BUILD_DIR)/cpp_rtc_practice" --list

info: build
	$(call require_day,info)
	"$(BUILD_DIR)/cpp_rtc_practice" --info "$(PADDED_DAY)"

run: build
	$(call require_day,run)
	"$(BUILD_DIR)/cpp_rtc_practice" --run "$(PADDED_DAY)"

check: build
	$(call require_day,check)
	"$(BUILD_DIR)/cpp_rtc_practice" --check "$(PADDED_DAY)"

run-%:
	$(MAKE) run DAY=$*

check-%:
	$(MAKE) check DAY=$*

info-%:
	$(MAKE) info DAY=$*

web-build:
	@if [ ! -f "$(WEB_BUILD_DIR)/CMakeCache.txt" ]; then \
		conan install "$(MAKEFILE_DIR)" --output-folder "$(WEB_CONAN_DIR)" --build=missing -s build_type=Debug && \
		$(CMAKE) -S "$(MAKEFILE_DIR)" -B "$(WEB_BUILD_DIR)" \
			-DCMAKE_TOOLCHAIN_FILE="$(WEB_CONAN_DIR)/build/Debug/generators/conan_toolchain.cmake" \
			-DCMAKE_BUILD_TYPE=Debug \
			-DPRACTICE_BUILD_WEB_SERVER=ON $(GENERATOR_FLAG); \
	fi
	$(CMAKE) --build "$(WEB_BUILD_DIR)" --target practice_web_server --parallel

build-web: web-build

run-web: web-build
	"$(WEB_BUILD_DIR)/practice_web_server" --build-dir "$(WEB_BUILD_DIR)" --port "$(WEB_PORT)"

start-web: web-build
	@if [ -f "$(WEB_PID_FILE)" ] && kill -0 "$$(cat "$(WEB_PID_FILE)")" 2>/dev/null; then \
		printf "Already running (pid %s) on 127.0.0.1:%s\n" "$$(cat "$(WEB_PID_FILE)")" "$(WEB_PORT)"; \
		exit 0; \
	fi
	@mkdir -p "$(dir $(WEB_PID_FILE))" "$(dir $(WEB_LOG_FILE))"
	@nohup "$(WEB_BUILD_DIR)/practice_web_server" --build-dir "$(WEB_BUILD_DIR)" --port "$(WEB_PORT)" \
		> "$(WEB_LOG_FILE)" 2>&1 & echo $$! > "$(WEB_PID_FILE)"
	@printf "Started 127.0.0.1:%s (pid %s)\n" "$(WEB_PORT)" "$$(cat "$(WEB_PID_FILE)")"

stop-web:
	@if [ ! -f "$(WEB_PID_FILE)" ]; then \
		printf "No PID file at %s\n" "$(WEB_PID_FILE)"; \
		exit 0; \
	fi
	@pid="$$(cat "$(WEB_PID_FILE)")"; \
	if kill -0 "$$pid" 2>/dev/null; then \
		kill "$$pid"; \
		printf "Stopped pid %s\n" "$$pid"; \
	else \
		printf "Process %s is not running\n" "$$pid"; \
	fi
	@rm -f "$(WEB_PID_FILE)"

restart-web: stop-web start-web

PADDED_DAY = $(shell printf '%02d' '$(DAY)' 2>/dev/null)

define require_day
	@if [ -z "$(DAY)" ]; then \
		printf "Usage: make %s DAY=01\n" "$(1)"; \
		exit 1; \
	fi
	@if ! printf '%s' "$(DAY)" | grep -Eq '^[0-9]{1,2}$$'; then \
		printf "DAY must be a number from 01 to 30\n"; \
		exit 1; \
	fi
	@if [ "$(PADDED_DAY)" -lt 1 ] || [ "$(PADDED_DAY)" -gt 30 ]; then \
		printf "DAY must be a number from 01 to 30\n"; \
		exit 1; \
	fi
endef
