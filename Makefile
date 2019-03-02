DESTDIR=/usr/local
BINDIR=$(DESTDIR)/bin
DOCDIR=$(DESTDIR)/share/doc
MANDIR=$(DESTDIR)/share/man

VERSION=$(shell cat VERSION)
RELEASE=$(shell sed -n 's/^Release: *\([0-9]*\)/\1/p' mkhexgrid.spec)
DISTDIR=mkhexgrid-$(VERSION)
RPMDIR=/home/uckelman/rpmbuild

CC=g++
CPPFLAGS=-c -g -O2 -W -Wall -DVERSION='"$(VERSION)"'
LDFLAGS=-lm -lstdc++ -lgd

FILES=grid.h \
      grid.cpp \
      mkhexgrid.cpp \
      png.cpp \
      ps.cpp \
      svg.cpp \
      Makefile \
      Makefile.win32 \
      doc/mkhexgrid.1 \
      $(DOCS) \
      VERSION

DOCS=AUTHORS \
     COPYING \
     HISTORY \
     README \
     TODO \
     doc/mkhexgrid.html

.PHONY: all dist dist-rpm dist-windows dist-source install clean

all: mkhexgrid

mkhexgrid: mkhexgrid.o grid.o png.o ps.o svg.o

mkhexgrid-web: mkhexgrid-web.o grid.o png.o ps.o svg.o split.o urldecode.o

dist: dist-windows dist-source dist-rpm

dist-windows: clean
	$(MAKE) -f Makefile.win32 dist

dist-rpm: dist-source
	cp mkhexgrid-$(VERSION).src.tar.gz $(RPMDIR)/SOURCES
	rpmbuild -ba mkhexgrid.spec
	rpmbuild --clean --rmsource mkhexgrid.spec
	mv $(RPMDIR)/RPMS/i386/mkhexgrid-$(VERSION)-$(RELEASE).* .
	mv $(RPMDIR)/SRPMS/mkhexgrid-$(VERSION)-$(RELEASE).* .

dist-source:
	mkdir -p $(DISTDIR)
	cp --parents $(FILES) $(DISTDIR)
	tar czvf mkhexgrid-$(VERSION).src.tar.gz $(DISTDIR)
	tar cjvf mkhexgrid-$(VERSION).src.tar.bz2 $(DISTDIR)
	zip -9r mkhexgrid-$(VERSION).src.zip $(DISTDIR)
	rm -rf $(DISTDIR)

install:
	install -m 755 -o 0 -g 0 -d $(BINDIR)
	install -m 755 -o 0 -g 0 -s mkhexgrid $(BINDIR)
	install -m 755 -o 0 -g 0 -d $(MANDIR)/man1
	install -m 644 -o 0 -g 0 doc/mkhexgrid.1 $(MANDIR)/man1
	install -m 755 -o 0 -g 0 -d $(DOCDIR)/mkhexgrid-$(VERSION) 
	install -m 644 -o 0 -g 0 $(DOCS) $(DOCDIR)/mkhexgrid-$(VERSION)

clean:
	rm -rf mkhexgrid mkhexgrid-web mkhexgrid.o grid.o png.o ps.o svg.o \
          mkhexgrid-$(VERSION).src.* mkhexgrid-$(VERSION).windows.zip \
          mkhexgrid-$(VERSION)-$(RELEASE).*.rpm $(DISTDIR)
