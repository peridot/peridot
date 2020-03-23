# Release makefile, this top level file is to be used by users to build the language
# It calls in the Makefile inside src/ with some syntax sugar.
# additionally it moves the binary from src/ to the root on release builds
#
# TODO: Is there a better way to do this?

release:
	cd src/ && make MODE=release && mv src/peridot .

debug:
	cd src/ && make MODE=debug

test:
	@echo Tests are currently unavailable.
	@echo Proceed as normal for now.
