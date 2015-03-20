#!/bin/sh

echo cleaning area of all configure files ...
rm -f aclocal.m4
rm -rf autom4te.cache
rm -f configure depcomp install-sh missing
rm -f INSTALL Makefile Makefile.in
rm -f config.log config.status config.h config.h.in

echo rebuilding configure system ...
aclocal
autoconf
autoheader
automake --add-missing --copy
