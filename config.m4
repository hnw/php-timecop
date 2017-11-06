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

  dnl Check PHP version:
  AC_MSG_CHECKING(PHP version)
  if test ! -z "$phpincludedir"; then
    PHP_VERSION=`grep 'PHP_VERSION ' $phpincludedir/main/php_version.h | sed -e 's/.*"\([[0-9]]*\.[[0-9]]*\.[[0-9]]*.*\)".*/\1/g' 2>/dev/null`
  elif test ! -z "$PHP_CONFIG"; then
    PHP_VERSION=`$PHP_CONFIG --version 2>/dev/null`
  fi

  if test x"$PHP_VERSION" = "x"; then
    AC_MSG_WARN([not found])
  else
    PHP_MAJOR_VERSION=`echo $PHP_VERSION | sed -e 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\).*/\1/g' 2>/dev/null`
    PHP_MINOR_VERSION=`echo $PHP_VERSION | sed -e 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\).*/\2/g' 2>/dev/null`
    PHP_RELEASE_VERSION=`echo $PHP_VERSION | sed -e 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\).*/\3/g' 2>/dev/null`
    AC_MSG_RESULT([$PHP_VERSION])
  fi

  PHP_NEW_EXTENSION(timecop, timecop.c tc_timeval.c, $ext_shared)
fi
