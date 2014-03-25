changelog:
	@if test -d "$(srcdir)/.git"; \
        then \
                echo Creating ChangeLog && \
                ( cd "$(top_srcdir)" && \
                  $(top_srcdir)/missing --run git log --stat ) > ChangeLog.tmp \
                && mv -f ChangeLog.tmp $(top_distdir)/ChangeLog \
                || ( rm -f ChangeLog.tmp ; \
                     echo Failed to generate ChangeLog >&2 ); \
        else \
                echo A git clone is required to generate a ChangeLog >&2; \
        fi

authors:
	@if test -d "$(srcdir)/.git"; \
        then \
                echo Creating AUTHORS && \
                ( cd "$(top_srcdir)" && \
                  $(top_srcdir)/missing --run git ls-tree HEAD -r | awk '{ print $$4 }'  ) > _files.tmp && \
                ( cd "$(top_srcdir)" && \
                  $(top_srcdir)/missing --run git log --pretty="%an <%ae>" ) > _all.tmp && \
                ( cd "$(top_srcdir)" && \
                  cat _all.tmp | awk -F"<" '{ print $$2 }' | sort | uniq -c | sort -rn ) > _by_commits.tmp && \
                ( cd "$(top_srcdir)" && \
                  while read line ; do  $(top_srcdir)/missing --run git blame -c -e "$$line" | \
                        awk '{ print $$2 }' | cut -c2- ; done  < _files.tmp | sort -u ) > _current.tmp && \
                  touch _AUTHORS.current.tmp _AUTHORS.past.tmp && \
                  cat _by_commits.tmp | awk '{ print $$2 }' | while read line ; do \
                        if `grep -q "$$line" _current.tmp` ; then \
                           grep -m 1 "$$line" _all.tmp >> _AUTHORS.current.tmp ; \
                        else \
                           grep -m 1 "$$line" _all.tmp >> _AUTHORS.past.tmp ; \
                        fi \
                  done && \
                  echo Contributors: >> AUTHORS.tmp && \
                  echo ============= >> AUTHORS.tmp && \
                  cat _AUTHORS.current.tmp >> AUTHORS.tmp && \
                  echo >> AUTHORS.tmp && \
                  echo Past contributors: >> AUTHORS.tmp && \
                  echo ================== >> AUTHORS.tmp && \
                  cat _AUTHORS.past.tmp >> AUTHORS.tmp && \
                  rm -fr _files.tmp _all.tmp _by_commits.tmp _current.tmp _AUTHORS.current.tmp _AUTHORS.past.tmp && \
                  mv -f AUTHORS.tmp $(top_distdir)/AUTHORS; \
        else \
                echo A git clone is required to generate an AUTHORS >&2; \
        fi

dist-hook: changelog authors
