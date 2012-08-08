# This Makefile is a tad convoluted. I've tried to make it so that the vapi
# can be built at the same time as the library in the hope that this will make
# it easier to keep it up-to-date.
#
# The vapi is generated in 2 steps:
#
#   vala-gen-introspect looks at headers and libs and generates a gidl for the
#   library. It takes a .files file containing paths to these. It also takes a
#   .namespace file containing the Vala namespace corresponding to this
#   library. Both these files are expected to be in the directory where you run
#   vala-gen-introspect.
#
#   vapigen takes the gidl, a .deps file containing the list of libs needed
#   to link this library, and a .metadata file containing annotations to the
#   gidl (e.g. foo() is the free function for object X).
#
# vala-gen-introspect runs with the -uninstalled .pc file. The .files is
# generated so that it has links to the headers and libraries from the source
# and build trees. The .namespace file is generated, though it doesn't need
# to be, so that it exists in the build tree where vala-gen-introspect is run
# (because this may not be the same as the source tree). Since we're working
# with the -uninstalled files, and vala-gen-introspect assumes filenames are
# named as <package>.<extension>, all the associated file names have an
# -uninstalled suffix.
#
# Running vapigen is simpler. It generates a the .vapi file using the gidl
# file from vala-gen-introspect. Since we want the final files to be in the
# form <libname>-<version>.*, we use the same convention for naming the files
# associated with this step (contrast with the -uninstalled suffix for the
# vala-gen-introspect step). The only file implicitly used by vapigen is the
# .deps file, so to make the build independent of a split source/build tree,
# I've put the deps on the vapigen command-line (it's redundant, but helps
# with distcheck).
#

targets = grilo-@GRL_MAJORMINOR@

if BUILD_GRILO_NET
targets += grilo-net-@GRL_MAJORMINOR@
endif

targets_u = $(foreach target,$(targets),$(subst @GRL_MAJORMINOR@,uninstalled,$(target)))

# "source" files for vala-gen-introspect
filesfiles_src = $(foreach target,$(targets_u),$(target).files.in)
filesfiles = $(foreach target,$(targets_u),$(target).files)
nsfiles_src = $(foreach target,$(targets_u),$(target).namespace.in)
nsfiles = $(foreach target,$(targets_u),$(target).namespace)

# "source" files for vapigen
depsfiles = $(foreach target,$(targets),$(target).deps)
gifiles = $(foreach target,$(targets),$(target).gi)
metadatafiles = $(foreach target,$(targets),$(target).metadata)

vapidir = $(datadir)/vala/vapi

if HAVE_VALA
vapifiles = $(foreach target,$(targets),$(target).vapi)
vapi_DATA = $(depsfiles) $(vapifiles)
endif

%-@GRL_MAJORMINOR@.gi: %-uninstalled.files %-uninstalled.namespace
	PKG_CONFIG_PATH=$(top_builddir)/:${PKG_CONFIG_PATH} \
		$(VALA_GEN_INTROSPECT) $*-uninstalled .
	@mv $*-uninstalled.gi $*-@GRL_MAJORMINOR@.gi

%.vapi: %.gi %.metadata %-custom.vala
	$(VAPIGEN) --library $* \
		$(top_srcdir)/bindings/vala/$*.metadata $*.gi \
		$*-custom.vala

all: $(vapifiles)

EXTRA_DIST = $(depsfiles) $(filesfiles_src) $(nsfiles_src) $(metadatafiles) $(vapifiles)
CLEANFILES = $(gifiles)
DISTCLEANFILES = $(vapifiles) $(filesfiles) $(nsfiles)
