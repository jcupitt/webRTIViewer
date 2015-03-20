dnl process with autoconf to make configure

AC_INIT([webGLRTIMaker], [1.0], [gianpaolo.palma@isti.cnr.it])
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_HEADERS(config.h)

AC_PROG_CC
AC_PROG_CXX

AC_CHECK_HEADERS([stdio.h math.h])

PKG_CHECK_MODULES(QT, QtGui QtXml)
AC_SUBST(QT_INCLUDES)
AC_SUBST(QT_LIBS)

AC_OUTPUT(Makefile)

