# -*- Mode: makefile-gmake -*-

.PHONY: all compile test test-css

BUILD_DIR = build
PATH_BACK = ..

all: build test

test: test-css

test-css: compile
	@echo "====== $@ ======="
	$(BUILD_DIR)/test-css/test-css -a -d test-css/data

compile: $(BUILD_DIR)
	(cd $(BUILD_DIR); qmake CONFIG+=debug $(PATH_BACK)/tests.pro)
	@make -C $(BUILD_DIR)

$(BUILD_DIR):
	@mkdir -p $@
