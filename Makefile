# -*- Makefile -*-
ifeq ($(DESTDIR),)
 DESTDIR := 
 PREFIX := /usr/local
else
 ifeq ($(PREFIX),)
  PREFIX := /usr
 endif
endif

CC = cc
CFLAGS = -fsanitize=address -fsanitize=leak -Wall -Wextra
DEBUG_CFLAGS = -g
LD_FLAGS = -lm
SOURCES = src/main.c src/util.c src/tab_complete.c src/fuzzy_finder.c src/readline.c
TEST_SOURCES = tests/unit_tests/test_main.c tests/unit_tests/test_util.c tests/unit_tests/test_fuzzy_finder.c tests/unit_tests/test_tab_complete.c tests/unit_tests/test_readline.c
test_target = $(basename $(notdir $(TEST_SOURCES)))
CURRENT_DIR = $(shell pwd)
INSTALLDIR ?= /usr/local/bin
# IN_SHELL_LIST = $(shell cat /etc/shells | grep psh)

psh: ${SOURCES} ## Compile the production shell
	${CC} ${CFLAGS} ${SOURCES} -o psh ${LD_FLAGS}

.PHONY: debug
debug: ${SOURCES} ## Compile the debug shell for development
	${CC} ${CFLAGS} ${DEBUG_CFLAGS} ${SOURCES} -o psh ${LD_FLAGS}

.PHONY: install uninstall
install:
	install -D -m 755 psh ${DESTDIR}${PREFIX}/bin/psh
uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/psh


.SILENT:
compile_and_run_debug:
	make debug
	./src/bin/psh

help:  ## Display this help
	@awk 'BEGIN {FS = ":.*##"; printf "\nUsage:\n  make \033[36m<target>\033[0m\n"} /^[a-zA-Z_-]+:.*?##/ { printf "  \033[36m%-15s\033[0m %s\n", $$1, $$2 } /^##@/ { printf "\n\033[1m%s\033[0m\n", substr($$0, 5) } ' $(MAKEFILE_LIST)

docker_shell: ${SOURCES}
	if [ ! -d "./src/bin" ]; then mkdir ./src/bin ; fi
	${CC} ${SOURCES} -o src/bin/psh -ldl -lm

.PHONY: run_tests
run_tests: $(TEST_SOURCES) $(SOURCES) ## Run all tests (needs criterion)
	if [ ! -d "./tests/unit_tests/bin" ]; then mkdir ./tests/unit_tests/bin ; fi
	${CC} ${CFLAGS} -o ./tests/unit_tests/bin/compiled_tests -D TEST ${TEST_SOURCES} ${SOURCES} -lcriterion ${LD_FLAGS}
	./tests/unit_tests/bin/compiled_tests -l
	./tests/unit_tests/bin/compiled_tests

#$(test_target): $(TEST_SOURCES) $(SOURCES) ## Run individual tests (needs criterion)
#	if [ ! -d "./tests/unit_tests/bin" ]; then mkdir ./tests/unit_tests/bin ; fi
#	if [ $@ = "test_util" -o $@ = "test_main" -o $@ = "test_readline" ]; then\
#		${CC} ${CFLAGS} -o ./tests/unit_tests/bin/compiled_$(subst test_,'',$@)_tests -D TEST tests/unit_tests/$@.c $(SOURCES) -L/usr/local/Cellar/criterion/2.3.3/lib/ -I/usr/local/Cellar/criterion/2.3.3/include/ -lcriterion.3.1.0;\
#	else\
#		${CC} ${CFLAGS} -o ./tests/unit_tests/bin/compiled_$(subst test_,'',$@)_tests -D TEST tests/unit_tests/$@.c src/util.c src/$(subst test_,'',$@).c -L/usr/local/Cellar/criterion/2.3.3/lib/ -I/usr/local/Cellar/criterion/2.3.3/include/ -lcriterion.3.1.0;\
#  fi
#	./tests/unit_tests/bin/compiled_$(subst test_,'',$@)_tests -l
#	./tests/unit_tests/bin/compiled_$(subst test_,'',$@)_tests

integration_tests: ## have to run 'docker -t testing_container .'
	make clean
	docker run -ti --rm -v $(CURRENT_DIR):/pshell testing_container \
		"make docker_shell && cat ./tests/integration_tests/autocomplete_tests.txt >> /root/.psh_history && ruby ./tests/integration_tests/test_pshell.rb"
	make clean

clean: ## Cleans up all binary and object files
	rm -f -R psh *.o ./tests/unit_tests/bin/* ./src/bin/*
