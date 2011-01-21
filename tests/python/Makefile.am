# things to test

test: testrunner.py
if HAVE_INTROSPECTION
	LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:$(top_builddir)/src/.libs/					\
	GI_TYPELIB_PATH=${GI_TYPELIB_PATH}:$(top_builddir)/src/						\
	PYTHONPATH=$(top_builddir):$(top_builddir)/src/tests/python:$${PYTHONPATH:+$$PYTHONPATH:}	\
	$(PYTHON) $(srcdir)/testrunner.py
endif

if HAVE_INTROSPECTION
EXTRA_DIST = \
	testrunner.py \
	test_registry.py \
	test_plugin.py \
	test_metadata_source.py
endif

.PHONY: test test-report perf-report full-report
# run make test as part of make check
check-local: test

MAINTAINERCLEANFILES = \
	*.pyc \
	Makefile.in
