# This Makefile is meant to be used only in development environments.

.DEFAULT_GOAL := _bruh

TARGET = ftp-server

_bruh:
	@echo "You didn't read the README, did you?"

c:
	@cmake -E remove_directory build
	@cmake -E remove_directory .cache

bud:
	@cmake -B build -S . --preset unix-debug
	@cmake --build build

bur:
	@cmake -B build -S . --preset unix-release
	@cmake --build build

r:
	@./build/${TARGET}