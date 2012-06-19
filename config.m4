dnl $Id$
dnl config.m4 for extension timecop

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(timecop, for timecop support,
dnl Make sure that the comment is aligned:
dnl [  --with-timecop             Include timecop support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(timecop, whether to enable timecop support,
 [  --enable-timecop        Enable timecop support])

if test "$PHP_TIMECOP" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-timecop -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/timecop.h"  # you most likely want to change this
  dnl if test -r $PHP_TIMECOP/$SEARCH_FOR; then # path given as parameter
  dnl   TIMECOP_DIR=$PHP_TIMECOP
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for timecop files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       TIMECOP_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$TIMECOP_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the timecop distribution])
  dnl fi

  dnl # --with-timecop -> add include path
  dnl PHP_ADD_INCLUDE($TIMECOP_DIR/include)

  dnl # --with-timecop -> check for lib and symbol presence
  dnl LIBNAME=timecop # you may want to change this
  dnl LIBSYMBOL=timecop # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $TIMECOP_DIR/lib, TIMECOP_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_TIMECOPLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong timecop lib version or lib not found])
  dnl ],[
  dnl   -L$TIMECOP_DIR/lib -lm
  dnl ])
  dnl
  dnl PHP_SUBST(TIMECOP_SHARED_LIBADD)

  AC_DEFINE(HAVE_TIMECOPLIB,1,[Whether timecop is present])
  PHP_NEW_EXTENSION(timecop, timecop.c, $ext_shared)
fi
