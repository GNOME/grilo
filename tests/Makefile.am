SUBDIRS = .

if HAVE_INTROSPECTION
SUBDIRS += python
endif

# things to test

noinst_PROGRAMS = $(TEST_PROGS)

AM_CFLAGS = -I$(top_srcdir)/src -I$(top_srcdir)/src/data $(DEPS_CFLAGS)
progs_ldadd = $(top_builddir)/src/lib@GRL_NAME@.la $(DEPS_LIBS)

TEST_PROGS       = registry
registry_SOURCES = registry.c
registry_LDADD = $(progs_ldadd)

TEST_PROGS += metadata_source
metadata_source_SOURCES = metadata_source.c
metadata_source_LDADD = $(progs_ldadd)

### testing rules (from glib)

GTESTER = gtester

# test: run all tests in cwd and subdirs
test:	${TEST_PROGS}
	@test -z "${TEST_PROGS}" || ${GTESTER} --verbose ${TEST_PROGS}

# test-report: run tests in subdirs and generate report
# perf-report: run tests in subdirs with -m perf and generate report
# full-report: like test-report: with -m perf and -m slow
test-report perf-report full-report:	${TEST_PROGS}
	@test -z "${TEST_PROGS}" || { \
	  case $@ in \
	  test-report) test_options="-k";; \
	  perf-report) test_options="-k -m=perf";; \
	  full-report) test_options="-k -m=perf -m=slow";; \
	  esac ; \
	  if test -z "$$GTESTER_LOGDIR" ; then	\
	    ${GTESTER} --verbose $$test_options -o test-report.xml ${TEST_PROGS} ; \
	  elif test -n "${TEST_PROGS}" ; then \
	    ${GTESTER} --verbose $$test_options -o `mktemp "$$GTESTER_LOGDIR/log-XXXXXX"` ${TEST_PROGS} ; \
	  fi ; \
	}
	@ ignore_logdir=true ; \
	  if test -z "$$GTESTER_LOGDIR" ; then \
	    GTESTER_LOGDIR=`mktemp -d "\`pwd\`/.testlogs-XXXXXX"`; export GTESTER_LOGDIR ; \
	    ignore_logdir=false ; \
	  fi ; \
	  for subdir in $(SUBDIRS) . ; do \
	    test "$$subdir" = "." -o "$$subdir" = "po" || \
	    ( cd $$subdir && $(MAKE) $(AM_MAKEFLAGS) $@ ) || exit $? ; \
	  done ; \
	  $$ignore_logdir || { \
	    echo '<?xml version="1.0"?>' > $@.xml ; \
	    echo '<report-collection>'  >> $@.xml ; \
	    for lf in `ls -L "$$GTESTER_LOGDIR"/.` ; do \
	      sed '1,1s/^<?xml\b[^>?]*?>//' <"$$GTESTER_LOGDIR"/"$$lf" >> $@.xml ; \
	    done ; \
	    echo >> $@.xml ; \
	    echo '</report-collection>' >> $@.xml ; \
	    rm -rf "$$GTESTER_LOGDIR"/ ; \
	    ${GTESTER_REPORT} --version 2>/dev/null 1>&2 ; test "$$?" != 0 || ${GTESTER_REPORT} $@.xml >$@.html ; \
	  }
.PHONY: test test-report perf-report full-report
# run make test as part of make check
check-local: test

MAINTAINERCLEANFILES = \
	Makefile.in
