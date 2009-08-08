include Config.mk

################ Source files ##########################################

SRCS	= $(wildcard *.cpp)
INCS	= $(wildcard *.h)
OBJS	= $(SRCS:.cpp=.o)

################ Library link names ####################################

TOCLEAN	+= ${LIBA}
ALLINST	= install-incs
ALLLIBS	+= ${LIBA}
ALLINST	+= install-static

################ Compilation ###########################################

.PHONY:	all install uninstall install-incs uninstall-incs
.PHONY: install-static install-shared uninstall-static uninstall-shared
.PHONY:	clean depend dox html check dist distclean maintainer-clean

all:	${ALLLIBS}

%.o:	%.cc
	@echo "    Compiling $< ..."
	@${CXX} ${CXXFLAGS} -o $@ -c $<

%.s:	%.cc
	@echo "    Compiling $< to assembly ..."
	@${CXX} ${CXXFLAGS} -S -o $@ -c $<

${LIBA}:	${OBJS}
	@echo "Linking $@ ..."
	@${AR} r $@ $?
	@${RANLIB} $@

${LIBSOBLD}:	${OBJS}
	@echo "Linking $@ ..."
	@${LD} ${LDFLAGS} ${SHBLDFL} -o $@ $^ ${LIBS}

html:
dox:
	@${DOXYGEN} ${DOCT}

################ Installation ##########################################

install:	${ALLINST}
uninstall:	$(subst install,uninstall,${ALLINST})

install-shared: ${LIBSOBLD}
	@echo "Installing ${LIBSOBLD} to ${LIBDIR} ..."
	@${INSTALLDIR} ${LIBDIR}
	@${INSTALLLIB} ${LIBSOBLD} ${LIBDIR}
	@(cd ${LIBDIR}; \
	    rm -f ${LIBSO} ${LIBSOLNK}; \
	    ln -sf ${LIBSOBLD} ${LIBSO}; \
	    ln -sf ${LIBSOBLD} ${LIBSOLNK})

uninstall-shared:
	@echo "Removing ${LIBSOBLD} from ${LIBDIR} ..."
	@(cd ${LIBDIR}; \
	    rm -f ${LIBSO} ${LIBSOLNK} ${LIBSOBLD})

install-static: ${LIBA}
	@echo "Installing ${LIBA} to ${LIBDIR} ..."
	@${INSTALLDIR} ${LIBDIR}
	@${INSTALLLIB} ${LIBA} ${LIBDIR}

uninstall-static:
	@echo "Removing ${LIBA} from ${LIBDIR} ..."
	@rm -f ${LIBDIR}/${LIBA}

install-incs: ${INCS}
	@echo "Installing headers to ${INCDIR} ..."
	@${INSTALLDIR} ${INCDIR}/${LIBNAME}
	@for i in $(filter-out ${LIBNAME}.h,${INCS}); do	\
	    ${INSTALLDATA} $$i ${INCDIR}/${LIBNAME}/$$i;	\
	done;
	@${INSTALLDATA} ${LIBNAME}.h ${INCDIR}

uninstall-incs:
	@echo "Removing headers from ${INCDIR} ..."
	@rm -rf ${INCDIR}/${LIBNAME} ${INCDIR}/${LIBNAME}.h

################ Maintenance ###########################################

clean:
	@echo "Removing generated files ..."
	@rm -f ${OBJS} ${TOCLEAN} *.rpo

depend: ${SRCS}
	@${CXX} ${CXXFLAGS} -M ${SRCS} > .depend;

check:	install
	@echo "Compiling and running build verification tests ..."

TMPDIR	= /tmp
DISTDIR	= ${HOME}/stored
ifeq (${BUILD},0)
DISTVER	= ${MAJOR}.${MINOR}
else
DISTVER	= ${MAJOR}.${MINOR}.${BUILD}
endif
DISTNAM	= ${LIBNAME}-${DISTVER}
DISTLSM	= ${DISTNAM}.lsm
DISTTAR	= ${DISTNAM}.tar.bz2
DDOCTAR	= ${DISTNAM}-docs.tar.bz2

dist:
	mkdir ${TMPDIR}/${DISTNAM}
	cp -r . ${TMPDIR}/${DISTNAM}
	+${MAKE} -C ${TMPDIR}/${DISTNAM} dox distclean
	(cd ${TMPDIR};							\
	    tar jcf ${DISTDIR}/${DDOCTAR} ${DISTNAM}/docs/html;		\
	    rm -f ${DISTNAM}/.git; rm -rf ${DISTNAM}/docs/html;		\
	    cp ${DISTNAM}/docs/${LIBNAME}.lsm ${DISTDIR}/${DISTLSM};	\
	    tar --numeric-owner --same-owner -jcf ${DISTDIR}/${DISTTAR} ${DISTNAM}; rm -rf ${DISTNAM})

distclean:	clean
	@rm -f Config.mk config.h ${LIBNAME}.spec bsconf.o bsconf .depend bvt/.depend

maintainer-clean: distclean
	@rm -rf docs/html

-include .depend
 
